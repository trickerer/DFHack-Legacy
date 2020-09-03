// deque20 utility header
#pragma once
#ifndef _XUTILITY_DEQUE20_
#define _XUTILITY_DEQUE20_
#ifndef RC_INVOKED
#include <climits>
#include <utility>
#include <crtdbg.h>

#ifdef _MSC_VER
 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)
#endif  /* _MSC_VER */

_STD_BEGIN

//#if _HAS_ITERATOR_DEBUGGING == 0
//
//__declspec(noreturn) __declspec(deprecated) _CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL _Debug_message(const char *, const char *);
//#if !defined(_CLI_MEMORY_)
//_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL _Debug_message(const wchar_t *, const wchar_t *, unsigned int line);
//#else
//void _Debug_message(const wchar_t *, const wchar_t *, unsigned int line);
//#endif //_CLI_MEMORY_
//#endif //_HAS_ITERATOR_DEBUGGING

#define _DEBUG_DEQ20_ERROR(mesg)    _DEBUG_DEQ20_ERROR2(mesg, __FILEW__, __LINE__)
#define _DEBUG_DEQ20_ERROR2(mesg, filew, linew) ((void)0)// _Debug_message(L ## mesg, filew, linew)

		// Used to initialize _Myfirstiter and _Mynextiter when _HAS_ITERATOR_DEBUGGING is off
#define _IGNORE_MYITERLIST_DEQUE20 ((_Iterator_deque20_base *)-3)

// #if _HAS_ITERATOR_DEBUGGING

		// CLASS _Container_base
class _Iterator_deque20_base;

class _Container_base_deque20_secure
	{	// store head of iterator chain
public:
	friend class _Iterator_deque20_base;

	__CLR_OR_THIS_CALL _Container_base_deque20_secure()
		: _Myfirstiter(0)
		{	// construct childless container
		}

	__CLR_OR_THIS_CALL _Container_base_deque20_secure(const _Container_base_deque20_secure&)
		: _Myfirstiter(0)
		{	// copy a container
		}

	_Container_base_deque20_secure& __CLR_OR_THIS_CALL operator=(const _Container_base_deque20_secure&)
		{	// assign a container
		return (*this);
		}

	__CLR_OR_THIS_CALL ~_Container_base_deque20_secure()
		{	// destroy the container
		_Orphan_all();
		}

	void __CLR_OR_THIS_CALL _Orphan_all() const;	// orphan all iterators
	void __CLR_OR_THIS_CALL _Swap_all(_Container_base_deque20_secure&) const;	// swaps all iterators

	void __CLR_OR_THIS_CALL _Swap_aux(_Container_base_deque20_secure&)
		{
		// Do nothing: we don't have an aux object.
		}

	_Iterator_deque20_base *_Myfirstiter;
	};

typedef _Container_base_deque20_secure _Container_deque20_base;

		// CLASS _Iterator_deque20_base
class _Iterator_deque20_base : public _Iterator_base_universal
	{	// store links to container, next iterator
public:
	friend class _Container_base_deque20_secure;

	__CLR_OR_THIS_CALL _Iterator_deque20_base()
		: _Mycont(0), _Mynextiter(0)
		{	// construct orphaned iterator
		}

	__CLR_OR_THIS_CALL _Iterator_deque20_base(const _Iterator_deque20_base& _Right)
		: _Mycont(0), _Mynextiter(0)
		{	// copy an iterator
		*this = _Right;
		}

	_Iterator_deque20_base& __CLR_OR_THIS_CALL operator=(const _Iterator_deque20_base& _Right)
		{	// assign an iterator
		if (_Mycont != _Right._Mycont)
			{	// change parentage
			_Lockit _Lock(_LOCK_DEBUG);
			_Orphan_me();
			_Adopt(_Right._Mycont);
			}
		return (*this);
		}

	__CLR_OR_THIS_CALL ~_Iterator_deque20_base()
		{	// destroy the iterator
		_Lockit _Lock(_LOCK_DEBUG);
		_Orphan_me();
		}

	void __CLR_OR_THIS_CALL _Adopt(const _Container_base_deque20_secure *_Parent)
		{	// adopt this iterator by parent
		if (_Mycont != _Parent)
			{	// change parentage
			_Lockit _Lock(_LOCK_DEBUG);
			_Orphan_me();
			if (_Parent != 0 && _Parent->_Myfirstiter != _IGNORE_MYITERLIST_DEQUE20)
				{	// switch to new parent
				_Mynextiter = _Parent->_Myfirstiter;
				((_Container_base_deque20_secure *)_Parent)->_Myfirstiter = this;
				}
			_Mycont = _Parent;
			}
		}

	void __CLR_OR_THIS_CALL _Orphan_me()
		{	// cut ties with parent
		if (_Mycont != 0 && _Mycont->_Myfirstiter != _IGNORE_MYITERLIST_DEQUE20)
			{	// adopted, remove self from list
			_Iterator_deque20_base **_Pnext =
				(_Iterator_deque20_base **)&_Mycont->_Myfirstiter;
			while (*_Pnext != 0 && *_Pnext != this)
				_Pnext = &(*_Pnext)->_Mynextiter;

			if (*_Pnext == 0)
				_DEBUG_DEQ20_ERROR("ITERATOR LIST CORRUPTED!");
			*_Pnext = _Mynextiter;
			_Mycont = 0;
			}
		}

	const _Container_base_deque20_secure * __CLR_OR_THIS_CALL _Getmycont() const
		{	// This member function always exists when we can get a container pointer
		return _Mycont;
		}

	bool __CLR_OR_THIS_CALL _Same_container(const _Iterator_deque20_base& _Other) const
		{	// This member function always exists when we can get a container pointer
		return _Mycont == _Other._Mycont;
		}

	bool __CLR_OR_THIS_CALL _Has_container() const
		{	// This member function always exists when we can get a container pointer
		return _Mycont != 0;
		}

	const _Container_base_deque20_secure *_Mycont;
	_Iterator_deque20_base *_Mynextiter;
	};

//typedef _Iterator_deque20_base _Iterator_base_secure;
//typedef _Iterator_deque20_base _Iterator_deque20_base_secure;

template<class _Category,
	class _Ty,
	class _Diff = ptrdiff_t,
	class _Pointer = _Ty *,
	class _Reference = _Ty&,
	class _Base_class = _Iterator_deque20_base>
		struct _Iterator_deque20_with_base
			: public _Base_class

	{	// base type for all iterator classes
	typedef _Category iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

template<class _Ty,
	class _Diff,
	class _Pointer,
	class _Reference>
	struct _Ranit_deque20
		: public _Iterator_deque20_with_base<random_access_iterator_tag,
			_Ty, _Diff, _Pointer, _Reference>
	{	// base type for container random-access iterator classes
	};

inline void __CLR_OR_THIS_CALL _Container_base_deque20_secure::_Orphan_all() const
	{	// orphan all iterators
	_Lockit _Lock(_LOCK_DEBUG);
	if (_Myfirstiter != _IGNORE_MYITERLIST_DEQUE20)
		{
		for (_Iterator_deque20_base **_Pnext = (_Iterator_deque20_base **)&_Myfirstiter;
			*_Pnext != 0; *_Pnext = (*_Pnext)->_Mynextiter)
			(*_Pnext)->_Mycont = 0;
		*(_Iterator_deque20_base **)&_Myfirstiter = 0;
		}
	}

inline void __CLR_OR_THIS_CALL _Container_base_deque20_secure::_Swap_all(_Container_base_deque20_secure& _Right) const
	{	// swap all iterators
	_Lockit _Lock(_LOCK_DEBUG);
	_Iterator_deque20_base *_Pnext;
	_Iterator_deque20_base *_Temp = (_Iterator_deque20_base *)_Myfirstiter;
	*(_Iterator_deque20_base **)&_Myfirstiter = (_Iterator_deque20_base *)_Right._Myfirstiter;
	*(_Iterator_deque20_base **)&_Right._Myfirstiter = _Temp;

	if (_Myfirstiter != _IGNORE_MYITERLIST_DEQUE20)
		{
		for (_Pnext = (_Iterator_deque20_base *)_Myfirstiter;
			_Pnext != 0; _Pnext = _Pnext->_Mynextiter)
			_Pnext->_Mycont = this;
		}
	if (_Right._Myfirstiter != _IGNORE_MYITERLIST_DEQUE20)
		{
		for (_Pnext = (_Iterator_deque20_base *)_Right._Myfirstiter;
			_Pnext != 0; _Pnext = _Pnext->_Mynextiter)
			_Pnext->_Mycont = &_Right;
		}
	}

// #endif /* _HAS_ITERATOR_DEBUGGING */

template<class _Alloc>
	class _Container_deque20_base_aux_alloc_empty
		: public _Container_deque20_base
	{ // base class for containers to avoid holding allocator _Alaux
	protected:
	explicit _Container_deque20_base_aux_alloc_empty(_Alloc) { }

	_Container_deque20_base_aux_alloc_empty(const _Container_deque20_base_aux_alloc_empty&) { }

	_Container_deque20_base_aux_alloc_empty& operator=(const _Container_deque20_base_aux_alloc_empty&)
		{
		return *this;
		}

	~_Container_deque20_base_aux_alloc_empty() { }
	};

_STD_END


#ifdef _MSC_VER
 #pragma warning(pop)
 #pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _XUTILITY_DEQUE20_ */
