#include "check-structures-sanity.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
//#define _WIN32_WINNT 0x0501
//#define WINVER 0x0501
#include <windows.h>
#endif

bool Checker::is_in_global(const QueueItem & item)
{
    struct_field_info const* fields = df::global::_identity.getFields();
    for (struct_field_info const* field = fields; field->mode != struct_field_info::END; field++)
    {
        size_t size = CheckedStructure(field).full_size();
        const void * const start = *reinterpret_cast<const void * const*>(field->offset);
        uintptr_t offset = uintptr_t(item.ptr) - uintptr_t(start);
        if (offset < size)
        {
            return true;
        }
    }

    return false;
}
bool Checker::is_valid_dereference(const QueueItem & item, const CheckedStructure & cs, size_t size, bool quiet)
{
    void* base = const_cast<void *>(item.ptr);
    if (!base)
    {
        // cannot dereference null pointer, but not an error
        return false;
    }

    // assumes MALLOC_PERTURB_=45
#ifdef DFHACK64
#define UNINIT_PTR 0xd2d2d2d2d2d2d2d2
#define FAIL_PTR(message) FAIL((stl_sprintf("0x%016zx: ", reinterpret_cast<uintptr_t>(base)) + message).c_str())
#else
#define UNINIT_PTR 0xd2d2d2d2
#define FAIL_PTR(message) FAIL((stl_sprintf("0x%08zx: ", reinterpret_cast<uintptr_t>(base)) + message).c_str())
#endif
    if (uintptr_t(base) == UNINIT_PTR)
    {
        if (!quiet)
        {
            FAIL_PTR("uninitialized pointer");
        }
        return false;
    }

    bool found = true;
    void* expected_start = base;
    size_t remaining_size = size;
    while (found)
    {
        found = false;

        //for (auto & range : mapped)
        for (std::vector12<t_memrange>::iterator it = mapped.begin(); it != mapped.end(); ++it)
        {
            t_memrange range = *it;
            if (!range.isInRange(expected_start))
            {
                continue;
            }

            found = true;

            if (!range.valid || !range.read)
            {
                if (!quiet)
                {
                    FAIL_PTR("pointer to invalid memory range");
                }
                return false;
            }

            void* expected_end = const_cast<void *>(PTR_ADD(expected_start, remaining_size - 1));
            if (size && !range.isInRange(expected_end))
            {
                const void* next_start = PTR_ADD(range.end, 1);
                remaining_size -= ptrdiff_t(next_start) - ptrdiff_t(expected_start);
                expected_start = const_cast<void *>(next_start);
                break;
            }

            return true;
        }
    }

    if (quiet)
    {
        return false;
    }

    if (expected_start == base)
    {
        FAIL_PTR("pointer not in any mapped range");
    }
    else
    {
        std::ostringstream failmsg;
        failmsg << "pointer exceeds mapped memory bounds (size " << size << ")";
        FAIL_PTR(failmsg.str().c_str());
    }

    return false;
#undef FAIL_PTR
}

int64_t Checker::get_int_value(const QueueItem & item, type_identity *type, bool quiet)
{
    if (type == df::identity_traits<int32_t>::get())
    {
        return validate_and_dereference<int32_t>(item, quiet);
    }
    else if (type == df::identity_traits<uint32_t>::get())
    {
        return validate_and_dereference<uint32_t>(item, quiet);
    }
    else if (type == df::identity_traits<int16_t>::get())
    {
        return validate_and_dereference<int16_t>(item, quiet);
    }
    else if (type == df::identity_traits<uint16_t>::get())
    {
        return validate_and_dereference<uint16_t>(item, quiet);
    }
    else if (type == df::identity_traits<int64_t>::get())
    {
        return validate_and_dereference<int64_t>(item, quiet);
    }
    else if (type == df::identity_traits<uint64_t>::get())
    {
        return int64_t(validate_and_dereference<uint64_t>(item, quiet));
    }
    else if (type == df::identity_traits<int8_t>::get())
    {
        return validate_and_dereference<int8_t>(item, quiet);
    }
    else if (type == df::identity_traits<uint8_t>::get())
    {
        return validate_and_dereference<uint8_t>(item, quiet);
    }
    else
    {
        UNEXPECTED;
        return 0;
    }
}

const char *Checker::get_vtable_name(const QueueItem & item, const CheckedStructure & cs, bool quiet)
{
    const void *const* vtable = validate_and_dereference<const void *const*>(QueueItem(item, "?vtable?", item.ptr), quiet);
    if (!vtable)
        return NULL;

    const char *const* info = validate_and_dereference<const char *const*>(QueueItem(item, "?vtable?.info", vtable - 1), quiet);
    if (!info)
        return NULL;

#ifdef WIN32
#ifdef DFHACK64
    void *base;
    if (!RtlPcToFileHeader(const_cast<void *>(reinterpret_cast<const void *>(info)), &base))
        return NULL;

    const char *typeinfo = reinterpret_cast<const char *>(base) + reinterpret_cast<const int32_t *>(info)[3];
    const char *name = typeinfo + 16;
#else
    const char *name = reinterpret_cast<const char *>(info) + 8;
#endif
#else
    auto name = validate_and_dereference<const char *>(QueueItem(item, "?vtable?.info.name", info + 1), quiet);
#endif

    //for (auto & range : mapped)
    for (std::vector12<t_memrange>::iterator it = mapped.begin(); it != mapped.end(); ++it)
    {
        t_memrange range = *it;
        if (!range.isInRange(const_cast<char *>(name)))
        {
            continue;
        }

        if (!range.valid || !range.read)
        {
            if (!quiet)
            {
                FAIL("pointer to invalid memory range");
            }
            return NULL;
        }

        const char *first_letter = NULL;
        bool letter = false;
        for (const char *p = name; ; p++)
        {
            if (!range.isInRange(const_cast<char *>(p)))
            {
                return NULL;
            }

            if ((*p >= 'a' && *p <= 'z') || *p == '_')
            {
                if (!letter)
                {
                    first_letter = p;
                }
                letter = true;
            }
            else if (!*p)
            {
                return first_letter;
            }
        }
    }

    return NULL;
}

std::pair<const void *, CheckedStructure> Checker::validate_vector_size(const QueueItem & item, const CheckedStructure & cs, bool quiet)
{
    typedef std::pair<const void*,CheckedStructure> ret_type;
    struct vector_data
    {
        uintptr_t start;
        uintptr_t finish;
        uintptr_t end_of_storage;
    };

    vector_data vector = *reinterpret_cast<const vector_data *>(item.ptr);

    ptrdiff_t length = vector.finish - vector.start;
    ptrdiff_t capacity = vector.end_of_storage - vector.start;

    bool local_ok = true;
    size_t item_size = cs.identity ? cs.identity->byte_size() : 0;
    if (!item_size)
    {
        item_size = 1;
        local_ok = false;
    }

    if (vector.start > vector.finish)
    {
        local_ok = false;
        if (!quiet)
        {
            FAIL("vector length is negative (" << (length / ptrdiff_t(item_size)) << ")");
        }
    }
    if (vector.start > vector.end_of_storage)
    {
        local_ok = false;
        if (!quiet)
        {
            FAIL("vector capacity is negative (" << (capacity / ptrdiff_t(item_size)) << ")");
        }
    }
    else if (vector.finish > vector.end_of_storage)
    {
        local_ok = false;
        if (!quiet)
        {
            FAIL("vector capacity (" << (capacity / ptrdiff_t(item_size)) << ") is less than its length (" << (length / ptrdiff_t(item_size)) << ")");
        }
    }

    size_t ulength = size_t(length);
    size_t ucapacity = size_t(capacity);
    if (ulength % item_size != 0)
    {
        local_ok = false;
        if (!quiet)
        {
            FAIL("vector length is non-integer (" << (ulength / item_size) << " items plus " << (ulength % item_size) << " bytes)");
        }
    }
    if (ucapacity % item_size != 0)
    {
        local_ok = false;
        if (!quiet)
        {
            FAIL("vector capacity is non-integer (" << (ucapacity / item_size) << " items plus " << (ucapacity % item_size) << " bytes)");
        }
    }

    if (local_ok && capacity && !vector.start)
    {
        if (!quiet)
        {
            FAIL("vector has null pointer but capacity " << (capacity / item_size));
        }
        return ret_type();
    }

    const void* start_ptr = reinterpret_cast<const void *>(vector.start);

    if (capacity && !is_valid_dereference(QueueItem(item, "?items?", start_ptr), CheckedStructure(cs.identity, capacity / item_size), quiet))
    {
        local_ok = false;
    }

    if (!local_ok)
    {
        return ret_type();
    }

    CheckedStructure ret_cs(cs.identity, ulength / item_size);
    ret_cs.allocated_count = ucapacity / item_size;
    return std::make_pair(start_ptr, ret_cs);
}

size_t Checker::get_allocated_size(const QueueItem & item)
{
    if (!sizes)
    {
        return 0;
    }

    if (uintptr_t(item.ptr) % 32 != 16)
    {
        return 0;
    }

    uint32_t tag = *reinterpret_cast<const uint32_t *>(PTR_ADD(item.ptr, -8));
    if (tag == 0xdfdf4ac8)
    {
        return *reinterpret_cast<const size_t *>(PTR_ADD(item.ptr, -16));
    }

    return 0;
}

#ifndef WIN32
const std::string24 *Checker::validate_stl_string_pointer(const void *const* base)
{
    std::string24 empty_string;
    if (*base == *reinterpret_cast<void **>(&empty_string))
    {
        return reinterpret_cast<const std::string24 *>(base);
    }

    const struct string_data_inner
    {
        size_t length;
        size_t capacity;
        int32_t refcount;
    } *str_data = static_cast<const string_data_inner *>(*base) - 1;

    if (!is_valid_dereference(QueueItem("str", PTR_ADD(str_data, -16)), 16, true))
    {
        return NULL;
    }

    uint32_t tag = *reinterpret_cast<const uint32_t *>(PTR_ADD(str_data, -8));
    if (tag == 0xdfdf4ac8)
    {
        size_t allocated_size = *reinterpret_cast<const size_t *>(PTR_ADD(str_data, -16));
        size_t expected_size = sizeof(*str_data) + str_data->capacity + 1;

        if (allocated_size != expected_size)
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }

    if (str_data->capacity < str_data->length)
    {
        return NULL;
    }

    const char *ptr = reinterpret_cast<const char *>(*base);
    for (size_t i = 0; i < str_data->length; i++)
    {
        if (!*ptr++)
        {
            return NULL;
        }
    }

    if (*ptr++)
    {
        return NULL;
    }

    return reinterpret_cast<const std::string24 *>(base);
}
#endif

const char *const *Checker::get_enum_item_key(enum_identity *identity, int64_t value)
{
    return get_enum_item_attr_or_key(identity, value, NULL);
}

const char *const *Checker::get_enum_item_attr_or_key(enum_identity *identity, int64_t value, const char *attr_name)
{
    size_t index;
    if (enum_identity::ComplexData* cplx = const_cast<enum_identity::ComplexData*>(identity->getComplex()))
    {
        std::map<int64_t, size_t>::iterator it = cplx->value_index_map.find(value);
        if (it == cplx->value_index_map.end())
        {
            return NULL;
        }
        index = it->second;
    }
    else
    {
        if (value < identity->getFirstItem() || value > identity->getLastItem())
        {
            return NULL;
        }
        index = value - identity->getFirstItem();
    }

    if (attr_name)
    {
        const void* attrs = identity->getAttrs();
        struct_identity* attr_type = identity->getAttrType();
        if (!attrs || !attr_type)
        {
            return NULL;
        }

        attrs = PTR_ADD(attrs, attr_type->byte_size() * index);
        for (const struct_field_info* field = attr_type->getFields(); field->mode != struct_field_info::END; field++)
        {
            if (!strcmp(field->name, attr_name))
            {
                return reinterpret_cast<const char *const *>(PTR_ADD(attrs, field->offset));
            }
        }

        return NULL;
    }

    return &identity->getKeys()[index];
}
