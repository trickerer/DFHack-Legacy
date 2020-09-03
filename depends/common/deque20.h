/*
 * This is the implementation of VS2008 deque20 class which is
 * compatible with VS2015 deque20 structure, achieved by
 * 1) removing '_Alty _Alval;' allocator member x3
 * 2) changing grow strategy from 50% to 100%
 */

#pragma once
#ifndef _DEQUE20_
#define _DEQUE20_
#ifndef RC_INVOKED
#include <memory>
#include <stdexcept>

#include "deque20_utility.h"

#ifdef _MSC_VER
 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)
#endif  /* _MSC_VER */

_STD_BEGIN
	// DEQUE PARAMETERS
#define _DEQUE20MAPSIZ	8	/* minimum map size, at least 1 */
#define _DEQUE20SIZ	(sizeof (_Type) <= 1 ? 16 \
	: sizeof (_Type) <= 2 ? 8 \
	: sizeof (_Type) <= 4 ? 4 \
	: sizeof (_Type) <= 8 ? 2 : 1)	/* elements per block (a power of 2) */

template<class _Type,
	class _Ax = allocator<_Type> >
	class deque20;

		// TEMPLATE CLASS _Deque20_const_iterator
template<class _Type, class _Alloc, bool _SECURE_VALIDATION>
	class _Deque20_const_iterator
 #if !defined(_DEBUG) && !_SECURE_SCL
		: public _Ranit_base<_Type, typename _Alloc::difference_type,
			typename _Alloc::const_pointer, typename _Alloc::const_reference, _Iterator_base_aux>
#else
		: public _Ranit_deque20<_Type, typename _Alloc::difference_type,
			typename _Alloc::const_pointer, typename _Alloc::const_reference>
#endif
	{	// iterator for nonmutable vector
public:
	// helper data used by the expression evaluator
	enum { _EEN_HID = _HAS_ITERATOR_DEBUGGING };

	typedef _Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION> _Myt;
	typedef deque20<_Type, _Alloc> _Mydeque;

#if !defined(_DEBUG) && !_SECURE_SCL
	typedef _Container_base_aux _Mydequebase;
#else
	typedef _Container_deque20_base _Mydequebase;
#endif

	typedef random_access_iterator_tag iterator_category;
	typedef _Type value_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::const_pointer pointer;
	typedef typename _Alloc::const_reference reference;

	typedef typename _Alloc::size_type size_type;

 #if _SECURE_SCL
	typedef typename _Secure_validation_helper<_SECURE_VALIDATION>::_Checked_iterator_category _Checked_iterator_category;
	typedef typename _If<_SECURE_VALIDATION,
		_Deque20_const_iterator<_Type, _Alloc, false>,
		_Unchanged_checked_iterator_base_type_tag>::_Result _Checked_iterator_base_type;

	friend _Deque20_const_iterator<_Type, _Alloc, false>;
	friend _Deque20_const_iterator<_Type, _Alloc, true>;

	_Deque20_const_iterator<_Type, _Alloc, false> _Checked_iterator_base() const
	{
		_Deque20_const_iterator<_Type, _Alloc, false> _Base(this->_Myoff, this->_Getmycont());
		return _Base;
	}

	void _Checked_iterator_assign_from_base(_Deque20_const_iterator<_Type, _Alloc, false> _Base)
	{
		_SCL_SECURE_VALIDATE(this->_Same_container(_Base));
		this->_Myoff = _Base._Myoff;
	}
 #endif

 //#if _HAS_ITERATOR_DEBUGGING
		_Deque20_const_iterator()
			{	// construct with null deque20 pointer
			_Myoff = 0;
			}

		_Deque20_const_iterator(const _Myt& _Right)
			: _Myoff(_Right._Myoff)
			{	// construct with copy of _Right
			this->_Adopt(_Right._Mycont);
			}

		_Deque20_const_iterator(size_type _Off, const _Mydequebase *_Pdeque)
			{	// construct with offset _Off in *_Pdeque
			_SCL_SECURE_TRAITS_VALIDATE(
				_Pdeque != NULL &&
				((_Mydeque *)_Pdeque)->_Myoff <= _Off && _Off <= (((_Mydeque *)_Pdeque)->_Myoff + ((_Mydeque *)_Pdeque)->_Mysize));

			this->_Adopt(_Pdeque);
			_Myoff = _Off;
			}

		reference operator*() const
			{	// return designated object
			size_type _Block = _Myoff / _DEQUE20SIZ;
			size_type _Off = _Myoff & (_DEQUE20SIZ - 1);	// assume power of 2
			if (this->_Mycont == 0
				|| _Myoff < ((_Mydeque *)this->_Mycont)->_Myoff
				|| ((_Mydeque *)this->_Mycont)->_Myoff
					+ ((_Mydeque *)this->_Mycont)->_Mysize <= _Myoff)
			{
				_DEBUG_DEQ20_ERROR("deque20 iterator not dereferencable");
				_SCL_SECURE_TRAITS_OUT_OF_RANGE;
			}
			if (((_Mydeque *)this->_Mycont)->_Mapsize <= _Block)
				_Block -= ((_Mydeque *)this->_Mycont)->_Mapsize;
			return ((((_Mydeque *)this->_Mycont)->_Map)[_Block][_Off]);
			}

 //#else /* _HAS_ITERATOR_DEBUGGING */
	//	_Deque20_const_iterator()
	//		{	// construct with null deque20 pointer
	//		_Myoff = 0;
	//		}

	//	_Deque20_const_iterator(size_type _Off, const _Mydequebase *_Pdeque)
	//		{	// construct with offset _Off in *_Pdeque
	//		_SCL_SECURE_TRAITS_VALIDATE(
	//			_Pdeque != NULL &&
	//			((_Mydeque *)_Pdeque)->_Myoff <= _Off && _Off <= (((_Mydeque *)_Pdeque)->_Myoff + ((_Mydeque *)_Pdeque)->_Mysize));
	//			
	//		this->_Set_container(_Pdeque);
	//		_Myoff = _Off;
	//		}

	//	reference operator*() const
	//		{	// return designated object
	//		size_type _Block = _Myoff / _DEQUE20SIZ;
	//		size_type _Off = _Myoff & (_DEQUE20SIZ - 1);	// assume power of 2
	//		_SCL_SECURE_VALIDATE(this->_Has_container());
	//		_SCL_SECURE_VALIDATE_RANGE(_Myoff < ((_Mydeque *)(this->_Getmycont()))->_Myoff + ((_Mydeque *)(this->_Getmycont()))->_Mysize);
	//		if (static_cast<const _Mydeque *>(this->_Getmycont())->_Mapsize <= _Block)
	//			_Block -= static_cast<const _Mydeque *>(this->_Getmycont())->_Mapsize;
	//		return ((static_cast<const _Mydeque *>(this->_Getmycont())->_Map)[_Block][_Off]);
	//		}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	pointer operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_Myt& operator++()
		{	// preincrement
		_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container());
		_SCL_SECURE_TRAITS_VALIDATE_RANGE(_Myoff < ((_Mydeque *)(this->_Getmycont()))->_Myoff + ((_Mydeque *)(this->_Getmycont()))->_Mysize);

 //#if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0
			|| ((_Mydeque *)this->_Mycont)->_Myoff
				+ ((_Mydeque *)this->_Mycont)->_Mysize == _Myoff)
			_DEBUG_DEQ20_ERROR("deque20 iterator not incrementable");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		++_Myoff;
		return (*this);
		}

	_Myt operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_Myt& operator--()
		{	// predecrement
		_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container());
		_SCL_SECURE_TRAITS_VALIDATE_RANGE(_Myoff > ((_Mydeque *)(this->_Getmycont()))->_Myoff);

 //#if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0
			|| _Myoff == ((_Mydeque *)this->_Mycont)->_Myoff)
			_DEBUG_DEQ20_ERROR("deque20 iterator not decrementable");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		--_Myoff;
		return (*this);
		}

	_Myt operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_Myt& operator+=(difference_type _Off)
		{	// increment by integer
		_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container());
		_SCL_SECURE_TRAITS_VALIDATE_RANGE(
			_Myoff + _Off <= ((_Mydeque *)(this->_Getmycont()))->_Myoff + ((_Mydeque *)(this->_Getmycont()))->_Mysize &&
			_Myoff + _Off >= ((_Mydeque *)(this->_Getmycont()))->_Myoff);
		_Myoff += _Off;
		return (*this);
		}

	_Myt operator+(difference_type _Off) const
		{	// return this + integer
		_Myt _Tmp = *this;
		return (_Tmp += _Off);
		}

	_Myt& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_Myt operator-(difference_type _Off) const
		{	// return this - integer
		_Myt _Tmp = *this;
		return (_Tmp -= _Off);
		}

	difference_type operator-(const _Myt& _Right) const
		{	// return difference of iterators

 //#if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 //#else
	//	_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		return (_Right._Myoff <= _Myoff ? _Myoff - _Right._Myoff
			: -(difference_type)(_Right._Myoff - _Myoff));
		}

	reference operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}

	bool operator==(const _Myt& _Right) const
		{	// test for iterator equality

 //#if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
		return (_Myoff == _Right._Myoff);
		}

 //#elif _SECURE_SCL
	//	_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
	//	return (_Myoff == _Right._Myoff);
	//	}
	//		
 //#else
	//	return (this->_Same_container(_Right) && _Myoff == _Right._Myoff);
	//	}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	bool operator!=(const _Myt& _Right) const
		{	// test for iterator inequality
		return (!(*this == _Right));
		}

	bool operator<(const _Myt& _Right) const
		{	// test if this < _Right

 //#if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
		return (_Myoff < _Right._Myoff);
		}

 //#elif _SECURE_SCL
	//	_SCL_SECURE_TRAITS_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
	//	return (_Myoff < _Right._Myoff);
	//	}
	//		
 //#else
	//	return (this->_Same_container(_Right) && _Myoff < _Right._Myoff);
	//	}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	bool operator>(const _Myt& _Right) const
		{	// test if this > _Right
		return (_Right < *this);
		}

	bool operator<=(const _Myt& _Right) const
		{	// test if this <= _Right
		return (!(_Right < *this));
		}

	bool operator>=(const _Myt& _Right) const
		{	// test if this >= _Right
		return (!(*this < _Right));
		}

	static void _Xlen()
		{	// report length error
		_THROW(length_error, "deque20<T> too long");
		}

	static void _Xinvarg()
		{	// report an invalid_argument error
		_THROW(invalid_argument, "invalid deque20 <T> argument");
		}

	static void _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid deque20 <T> subscript");
		}

 //#if _HAS_ITERATOR_DEBUGGING
	void _Compat(const _Myt& _Right) const
		{	// test for compatible iterator pair
		if (this->_Mycont == 0 || this->_Mycont != _Right._Mycont)
			{
			_DEBUG_DEQ20_ERROR("deque20 iterators incompatible");
			_SCL_SECURE_TRAITS_INVALID_ARGUMENT;
			}
		}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	size_type _Myoff;	// offset of element in deque20
	};

template<class _Type, class _Alloc, bool _SECURE_VALIDATION>
	inline
	_Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION> operator+(
		typename _Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION>::difference_type _Off,
		_Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION> _Next)
	{	// add offset to iterator
	return (_Next += _Off);
	}

		// TEMPLATE CLASS _Deque20_iterator
template<class _Type, class _Alloc, bool _SECURE_VALIDATION>
	class _Deque20_iterator
		: public _Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION>
		{	// iterator for mutable vector
public:
	typedef _Deque20_iterator<_Type, _Alloc, _SECURE_VALIDATION> _Myt;
	typedef _Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION> _Mybase;
	typedef deque20<_Type, _Alloc> _Mydeque;

	typedef random_access_iterator_tag iterator_category;
	typedef _Type value_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::pointer pointer;
	typedef typename _Alloc::reference reference;

	typedef typename _Alloc::size_type size_type;

 #if _SECURE_SCL
	typedef typename _If<_SECURE_VALIDATION,
		_Deque20_iterator<_Type, _Alloc, false>,
		_Unchanged_checked_iterator_base_type_tag>::_Result _Checked_iterator_base_type;

	friend _Deque20_iterator<_Type, _Alloc, false>;
	friend _Deque20_iterator<_Type, _Alloc, true>;

	_Deque20_iterator<_Type, _Alloc, false> _Checked_iterator_base() const
	{
		_Deque20_iterator<_Type, _Alloc, false> _Base(this->_Myoff, this->_Getmycont());
		return _Base;
	}

	void _Checked_iterator_assign_from_base(_Deque20_iterator<_Type, _Alloc, false> _Base)
	{
		_SCL_SECURE_VALIDATE(this->_Same_container(_Base));
		this->_Myoff = _Base._Myoff;
	}
 #endif

	_Deque20_iterator()
		{	// construct with null vector pointer
		}

	_Deque20_iterator(size_type _Off, const _Mybase::_Mydequebase *_Pdeque)
		: _Mybase(_Off, _Pdeque)
		{	// construct with offset _Off in *_Pdeque
		}

	reference operator*() const
		{	// return designated object
		return ((reference)**(_Mybase *)this);
		}

	pointer operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_Myt& operator++()
		{	// preincrement
		++*(_Mybase *)this;
		return (*this);
		}

	_Myt operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_Myt& operator--()
		{	// predecrement
		--*(_Mybase *)this;
		return (*this);
		}

	_Myt operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_Myt& operator+=(difference_type _Off)
		{	// increment by integer
		*(_Mybase *)this += _Off;
		return (*this);
		}

	_Myt operator+(difference_type _Off) const
		{	// return this + integer
		_Myt _Tmp = *this;
		return (_Tmp += _Off);
		}

	_Myt& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_Myt operator-(difference_type _Off) const
		{	// return this - integer
		_Myt _Tmp = *this;
		return (_Tmp -= _Off);
		}

	difference_type operator-(const _Mybase& _Right) const
		{	// return difference of iterators
		return (*(_Mybase *)this - _Right);
		}

	reference operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}
	};

template<class _Type, class _Alloc, bool _SECURE_VALIDATION>
	inline
	_Deque20_iterator<_Type, _Alloc, _SECURE_VALIDATION> operator+(
		typename _Deque20_iterator<_Type, _Alloc, _SECURE_VALIDATION>::difference_type _Off,
		_Deque20_iterator<_Type, _Alloc, _SECURE_VALIDATION> _Next)
	{	// add offset to iterator
	return (_Next += _Off);
	}

template<class _Alloc>
	class _Container_base_aux_alloc_real_no_alloc
		: public _Container_base_aux
	{ // base class for containers to hold allocator _Alaux
	protected:
	explicit _Container_base_aux_alloc_real_no_alloc(_Alloc _Al)
		//: _Alaux(_Al)
		{
		_Myownedaux = new (_Alaux().allocate(1)) _Aux_cont(this);
		}

	_Container_base_aux_alloc_real_no_alloc(const _Container_base_aux_alloc_real_no_alloc& _Right)
		//: _Alaux(_Right._Alaux)
		{
		_Myownedaux = new (_Alaux().allocate(1)) _Aux_cont(this);
		}

	_Container_base_aux_alloc_real_no_alloc& operator=(const _Container_base_aux_alloc_real_no_alloc&)
		{
		// Do nothing: keep our aux object.
		return *this;
		}

	~_Container_base_aux_alloc_real_no_alloc()
		{
		_Myownedaux->~_Aux_cont();

		_Alaux().deallocate(_Myownedaux, 1);
		}

	//typename _Alloc::template rebind<_Aux_cont>::other _Alaux; // allocator object for aux objects
#if !defined(_DEBUG) && !_SECURE_SCL
        typedef typename _Alloc::template rebind<_Aux_cont>::other _Aux_alloc;
    static _Aux_alloc _Alaux()
        {
            return _Aux_alloc();
        }
#endif
	};

#if !defined(_DEBUG) && !_SECURE_SCL
	#define _DEQUE20_BASE _Container_base_aux_alloc_real_no_alloc<_Alloc>
#else
	#define _DEQUE20_BASE _Container_deque20_base_aux_alloc_empty<_Alloc>
#endif

		// TEMPLATE CLASS _Deque20_map
template<class _Type,
	class _Alloc>
	class _Deque20_map
		: public _DEQUE20_BASE
	{	// ultimate base class for deque20 to hold allocator _Almap
protected:
	_Deque20_map(_Alloc _Al) : _DEQUE20_BASE(_Al)
        //, _Almap(_Al)
		{	// construct allocator from _Al
		}

	typedef typename _Alloc::template rebind<_Type>::other _Ty_alloc;
	typedef typename _Ty_alloc::pointer _Tptr;

	typedef typename _Alloc::template rebind<_Tptr>::other _Tptr_alloc;
	typedef typename _Tptr_alloc::pointer _Mptr;

	//_Tptr_alloc _Almap;	// allocator object for maps
    static _Tptr_alloc _Almap()
        {
        return _Tptr_alloc();
        }
	};

		// TEMPLATE CLASS _Deque20_val
template<class _Type,
	class _Alloc>
	class _Deque20_val
		: public _Deque20_map<_Type, _Alloc>
	{	// base class for deque20 to hold allocator _Alval
protected:
	_Deque20_val(_Alloc _Al = _Alloc()) : _Deque20_map<_Type, _Alloc>(_Al)
        //, _Alval(_Al)
		{	// construct allocator and base from _Al
		}

	typedef _Deque20_map<_Type, _Alloc> _Mybase;
	typedef typename _Alloc::template rebind<_Type>::other _Alty;

	//_Alty _Alval;	// allocator object for stored elements
    static _Alty _Alval()
        {
        return _Alty();
        }
	};

		// TEMPLATE CLASS deque20
template<class _Type,
	class _Ax>
	class deque20
		: public _Deque20_val<_Type, _Ax>
	{	// circular queue of pointers to blocks
public:

	// helper data used by the expression evaluator
	static const int _EEM_DS = _DEQUE20SIZ;
	enum { _EEN_DS = _DEQUE20SIZ };

	typedef deque20<_Type, _Ax> _Myt;
	typedef _Deque20_val<_Type, _Ax> _Mybase;
	typedef typename _Mybase::_Alty _Alloc;
	typedef _Alloc allocator_type;
	typedef typename _Mybase::_Tptr_alloc _Tptr_alloc;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type _Dift;
	typedef _Dift difference_type;
	typedef typename _Alloc::pointer _Tptr;
	typedef typename _Alloc::const_pointer _Ctptr;
	typedef _Tptr pointer;
	typedef _Ctptr const_pointer;
	typedef typename _Mybase::_Mptr _Mapptr;
	typedef typename _Alloc::reference _Reft;
	typedef _Reft reference;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::value_type value_type;

	typedef _Deque20_iterator<_Type, _Alloc, _SECURE_VALIDATION_DEFAULT> iterator;
	typedef _Deque20_const_iterator<_Type, _Alloc, _SECURE_VALIDATION_DEFAULT> const_iterator;

//	friend class _Deque20_iterator<_Type, _Alloc>;
	friend class _Deque20_const_iterator<_Type, _Alloc, false>;
#if _SECURE_SCL
	friend class _Deque20_const_iterator<_Type, _Alloc, true>;
#endif

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	deque20()
		: _Mybase(), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct empty deque20
		}

	explicit deque20(const _Alloc& _Al)
		: _Mybase(_Al), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct empty deque20 with allocator
		}

	explicit deque20(size_type _Count)
		: _Mybase(), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct from _Count * _Type()
		_Construct_n(_Count, _Type());
		}

	deque20(size_type _Count, const _Type& _Val)
		: _Mybase(), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct from _Count * _Val
		_Construct_n(_Count, _Val);
		}

	deque20(size_type _Count, const _Type& _Val, const _Alloc& _Al)
		: _Mybase(_Al), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct from _Count * _Val with allocator
		_Construct_n(_Count, _Val);
		}

	deque20(const _Myt& _Right)
		: _Mybase(_Right._Alval()), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct by copying _Right
		_TRY_BEGIN
		insert(begin(), _Right.begin(), _Right.end());
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	template<class _It>
		deque20(_It _First, _It _Last)
		: _Mybase(), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct from [_First, _Last)
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _It>
		deque20(_It _First, _It _Last, const _Alloc& _Al)
		: _Mybase(_Al), _Map(0),
			_Mapsize(0), _Myoff(0), _Mysize(0)
		{	// construct from [_First, _Last) with allocator
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _It>
		void _Construct(_It _Count, _It _Val, _Int_iterator_tag)
		{	// initialize from _Count * _Val
		_Construct_n((size_type)_Count, (_Type)_Val);
		}

	template<class _It>
		void _Construct(_It _First, _It _Last, input_iterator_tag)
		{	// initialize from [_First, _Last), input iterators
		_TRY_BEGIN
		insert(begin(), _First, _Last);
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	void _Construct_n(size_type _Count, const _Type& _Val)
		{	// construct from _Count * _Val
		_TRY_BEGIN
		_Insert_n(begin(), _Count, _Val);
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	~deque20()
		{	// destroy the deque20
		_Tidy();
		}

	_Myt& operator=(const _Myt& _Right)
		{	// assign _Right
		if (this == &_Right)
			;
		else if (_Right._Mysize == 0)
			clear();
		else if (_Right._Mysize <= _Mysize)
			{	// new sequence not longer, assign elements and erase unused
			iterator _Mid = std::copy(_Right.begin(), _Right.end(), begin());
			erase(_Mid, end());
			}
		else
			{	// new sequence longer, assign elements and append rest
			const_iterator _Mid = _Right.begin() + _Mysize;
			std::copy(_Right.begin(), _Mid, begin());
			insert(end(), _Mid, _Right.end());
			}
		return (*this);
		}

	iterator begin()
		{	// return iterator for beginning of mutable sequence
		return (iterator(_Myoff, this));
		}

	const_iterator begin() const
		{	// return iterator for beginning of nonmutable sequence
		return (const_iterator(_Myoff, this));
		}

	iterator end()
		{	// return iterator for end of mutable sequence
		return (iterator(_Myoff + _Mysize, this));
		}

	const_iterator end() const
		{	// return iterator for end of nonmutable sequence
		return (const_iterator(_Myoff + _Mysize, this));
		}

	iterator _Make_iter(const_iterator _Where) const
		{	// make iterator from const_iterator
		return (iterator(_Where._Myoff, this));
		}

	reverse_iterator rbegin()
		{	// return iterator for beginning of reversed mutable sequence
		return (reverse_iterator(end()));
		}

	const_reverse_iterator rbegin() const
		{	// return iterator for beginning of reversed nonmutable sequence
		return (const_reverse_iterator(end()));
		}

	reverse_iterator rend()
		{	// return iterator for end of reversed mutable sequence
		return (reverse_iterator(begin()));
		}

	const_reverse_iterator rend() const
		{	// return iterator for end of reversed nonmutable sequence
		return (const_reverse_iterator(begin()));
		}

	void resize(size_type _Newsize)
		{	// determine new length, padding with _Type() elements as needed
		resize(_Newsize, _Type());
		}

	void resize(size_type _Newsize, _Type _Val)
		{	// determine new length, padding with _Val elements as needed
		if (_Mysize < _Newsize)
			_Insert_n(end(), _Newsize - _Mysize, _Val);
		else if (_Newsize < _Mysize)
			erase(begin() + _Newsize, end());
		}

	size_type size() const
		{	// return length of sequence
		return (_Mysize);
		}

	size_type max_size() const
		{	// return maximum possible length of sequence
		return (this->_Alval().max_size());
		}

	bool empty() const
		{	// test if sequence is empty
		return (_Mysize == 0);
		}

	allocator_type get_allocator() const
		{	// return allocator object for values
		return (this->_Alval());
		}

	const_reference at(size_type _Pos) const
		{	// subscript nonmutable sequence with checking
		if (_Mysize <= _Pos)
			_Xran();
		return (*(begin() + _Pos));
		}

	reference at(size_type _Pos)
		{	// subscript mutable sequence with checking
		if (_Mysize <= _Pos)
			_Xran();
		return (*(begin() + _Pos));
		}

	const_reference operator[](size_type _Pos) const
		{	// subscript nonmutable sequence

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Mysize <= _Pos)
			_DEBUG_DEQ20_ERROR("deque20 subscript out of range");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		return (*(begin() + _Pos));
		}

	reference operator[](size_type _Pos)
		{	// subscript mutable sequence

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Mysize <= _Pos)
			_DEBUG_DEQ20_ERROR("deque20 subscript out of range");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		return (*(begin() + _Pos));
		}

	reference front()
		{	// return first element of mutable sequence
		return (*begin());
		}

	const_reference front() const
		{	// return first element of nonmutable sequence
		return (*begin());
		}

	reference back()
		{	// return last element of mutable sequence
		return (*(end() - 1));
		}

	const_reference back() const
		{	// return last element of nonmutable sequence
		return (*(end() - 1));
		}

	void push_front(const _Type& _Val)
		{	// insert element at beginning

 //#if _HAS_ITERATOR_DEBUGGING
		this->_Orphan_all();
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		if (_Myoff % _DEQUE20SIZ == 0
			&& _Mapsize <= (_Mysize + _DEQUE20SIZ) / _DEQUE20SIZ)
			_Growmap(1);
		size_type _Newoff = _Myoff != 0 ? _Myoff
			: _Mapsize * _DEQUE20SIZ;
		size_type _Block = --_Newoff / _DEQUE20SIZ;
		if (_Map[_Block] == 0)
			_Map[_Block] = this->_Alval().allocate(_DEQUE20SIZ);
		this->_Alval().construct(_Map[_Block] + _Newoff % _DEQUE20SIZ, _Val);
		_Myoff = _Newoff;
		++_Mysize;
		}

	void pop_front()
		{	// erase element at beginning

// #if _HAS_ITERATOR_DEBUGGING
		if (empty())
			_DEBUG_DEQ20_ERROR("deque20 empty before pop");
		else
			{	// something to erase, do it
			_Orphan_off(_Myoff);

 //#else /* _HAS_ITERATOR_DEBUGGING */
	//	if (!empty())
	//		{	// something to erase, do it
 //#endif /* _HAS_ITERATOR_DEBUGGING */

			size_type _Block = _Myoff / _DEQUE20SIZ;
			this->_Alval().destroy(_Map[_Block] + _Myoff % _DEQUE20SIZ);
			if (_Mapsize * _DEQUE20SIZ <= ++_Myoff)
				_Myoff = 0;
			if (--_Mysize == 0)
				_Myoff = 0;
			}
			}

	void push_back(const _Type& _Val)
		{	// insert element at end

 //#if _HAS_ITERATOR_DEBUGGING
		this->_Orphan_all();
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		if ((_Myoff + _Mysize) % _DEQUE20SIZ == 0
			&& _Mapsize <= (_Mysize + _DEQUE20SIZ) / _DEQUE20SIZ)
			_Growmap(1);
		size_type _Newoff = _Myoff + _Mysize;
		size_type _Block = _Newoff / _DEQUE20SIZ;
		if (_Mapsize <= _Block)
			_Block -= _Mapsize;
		if (_Map[_Block] == 0)
			_Map[_Block] = this->_Alval().allocate(_DEQUE20SIZ);
		this->_Alval().construct(_Map[_Block] + _Newoff % _DEQUE20SIZ, _Val);
		++_Mysize;
		}

	void pop_back()
		{	// erase element at end

 //#if _HAS_ITERATOR_DEBUGGING
		if (empty())
			_DEBUG_DEQ20_ERROR("deque20 empty before pop");
		else
			{	// something to erase, do it
			_Orphan_off(_Myoff + _Mysize - 1);

 //#else /* _HAS_ITERATOR_DEBUGGING */
	//	if (!empty())
	//		{	// something to erase, do it
 //#endif /* _HAS_ITERATOR_DEBUGGING */

			size_type _Newoff = _Mysize + _Myoff - 1;
			size_type _Block = _Newoff / _DEQUE20SIZ;
			if (_Mapsize <= _Block)
				_Block -= _Mapsize;
			this->_Alval().destroy(_Map[_Block] + _Newoff % _DEQUE20SIZ);
			if (--_Mysize == 0)
				_Myoff = 0;
			}
			}

	template<class _It>
		void assign(_It _First, _It _Last)
		{	// assign [_First, _Last)
		_Assign(_First, _Last, _Iter_cat(_First));
		}

	template<class _It>
		void _Assign(_It _Count, _It _Val, _Int_iterator_tag)
		{	// assign _Count * _Val
		_Assign_n((size_type)_Count, (_Type)_Val);
		}

	template<class _It>
		void _Assign(_It _First, _It _Last, input_iterator_tag)
		{	// assign [_First, _Last), input iterators
		erase(begin(), end());
		insert(begin(), _First, _Last);
		}

	void assign(size_type _Count, const _Type& _Val)
		{	// assign _Count * _Val
		_Assign_n(_Count, _Val);
		}

	iterator insert(const_iterator _Where, const _Type& _Val)
		{	// insert _Val at _Where
		if (_Where == begin())
			{	// insert at front
			push_front(_Val);
			return (begin());
			}
		else if (_Where == end())
			{	// insert at back
			push_back(_Val);
			return (end() - 1);
			}
		else
			{	// insert inside sequence
			iterator _Mid;
			size_type _Off = _Where - begin();
			_Type _Tmp = _Val;	// in case _Val is in sequence

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Mysize < _Off)
			_DEBUG_DEQ20_ERROR("deque20 insert iterator outside range");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

			if (_Off < _Mysize / 2)
				{	// closer to front, push to front then copy
				push_front(front());
				_Mid = begin() + _Off;
				std::copy(begin() + 2, _Mid + 1, begin() + 1);
				}
			else
				{	// closer to back, push to back then copy
				push_back(back());
				_Mid = begin() + _Off;
				std::copy_backward(_Mid, end() - 2, end() - 1);
				}

			*_Mid = _Tmp;	// store inserted value
			return (_Make_iter(_Mid));
			}
		}

	void insert(const_iterator _Where, size_type _Count, const _Type& _Val)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, _Count, _Val);
		}

	template<class _It>
		void insert(const_iterator _Where, _It _First, _It _Last)
		{	// insert [_First, _Last) at _Where
		_Insert(_Where, _First, _Last, _Iter_cat(_First));
		}

	template<class _It>
		void _Insert(const_iterator _Where, _It _Count, _It _Val,
			_Int_iterator_tag)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, (size_type)_Count, (_Type)_Val);
		}

	template<class _It>
		void _Insert(const_iterator _Where,
			_It _First, _It _Last, input_iterator_tag)
		{	// insert [_First, _Last) at _Where, input iterators
		size_type _Off = _Where - begin();

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Mysize < _Off)
			_DEBUG_DEQ20_ERROR("deque20 insert iterator outside range");
		_DEBUG_RANGE(_First, _Last);
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		size_type _Rem = _Mysize - _Off;
		size_type _Oldsize = _Mysize;

		if (_First == _Last)
			;
		else if (_Off < _Rem)
			{	// closer to front
			_TRY_BEGIN
			for (; _First != _Last; ++_First)
				push_front((value_type)*_First);	// prepend flipped

			_CATCH_ALL
			for (; _Oldsize < _Mysize; )
				pop_front();	// restore old size, at least
			_RERAISE;
			_CATCH_END

			size_type _Num = _Mysize - _Oldsize;

			if (0 < _Off)
				{	// insert not at beginning, flip new stuff into place
				_Reverse(_Num, _Num + _Off);
				_Reverse(0, _Num + _Off);
				}
			else
				_Reverse(0, _Num);	// flip new stuff in place
			}
		else
			{	// closer to back
			_TRY_BEGIN
			for (; _First != _Last; ++_First)
				push_back((value_type)*_First);	// append

			_CATCH_ALL
			for (; _Oldsize < _Mysize; )
				pop_back();	// restore old size, at least
			_RERAISE;
			_CATCH_END

			if (_Off < _Oldsize)
				{	// insert not at end, flip new stuff into place
				_Reverse(_Off, _Oldsize);
				_Reverse(_Oldsize, _Mysize);
				_Reverse(_Off, _Mysize);
				}
			}
		}

	void _Reverse(size_type _First, size_type _Last)
		{	// reverse a subrange
		for (; _First != _Last && _First != --_Last; ++_First)
			{	// swap distinct _First and _Last
			iterator _Start = begin();
			value_type _Temp = _Start[_First];

			_Start[_First] = _Start[_Last];
			_Start[_Last] = _Temp;
			}
		}

	iterator erase(const_iterator _Where)
		{	// erase element at _Where
		return (erase(_Where, _Where + 1));
		}

	iterator erase(const_iterator _First_arg,
		const_iterator _Last_arg)
		{	// erase [_First, _Last)
		iterator _First = _Make_iter(_First_arg);
		iterator _Last = _Make_iter(_Last_arg);

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Last < _First
			|| _First < begin() || end() < _Last)
			_DEBUG_DEQ20_ERROR("deque20 erase iterator outside range");
		_DEBUG_RANGE(_First, _Last);

		size_type _Off = _First - begin();
		size_type _Count = _Last - _First;
		bool _Moved = 0 < _Off && _Off + _Count < _Mysize;

 //#else /* _HAS_ITERATOR_DEBUGGING */
	//	size_type _Off = _First - begin();
	//	size_type _Count = _Last - _First;
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		if (_Off < (size_type)(end() - _Last))
			{	// closer to front
			std::copy_backward(begin(), _First, _Last);	// copy over hole
			for (; 0 < _Count; --_Count)
				pop_front();	// pop copied elements
			}
		else
			{	// closer to back
			std::copy(_Last, end(), _First);	// copy over hole
			for (; 0 < _Count; --_Count)
				pop_back();	// pop copied elements
			}

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Moved)
			this->_Orphan_all();
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		return (begin() + _Off);
		}

	void clear()
		{	// erase all
		_Tidy();
		}

	void swap(_Myt& _Right)
		{	// exchange contents with _Right
		if (this == &_Right)
			;	// same object, do nothing
		else if (this->_Alval() == _Right._Alval())
			{	// same allocator, swap control information

 //#if _HAS_ITERATOR_DEBUGGING
			this->_Swap_all(_Right);
 //#endif /* _HAS_ITERATOR_DEBUGGING */

			this->_Swap_aux(_Right);

			std::swap(_Map, _Right._Map);
			std::swap(_Mapsize, _Right._Mapsize);
			std::swap(_Myoff, _Right._Myoff);
			std::swap(_Mysize, _Right._Mysize);
			}
		else
			{	// different allocator, do multiple assigns
			this->_Swap_aux(_Right);

			_Myt _Ts = *this;

			*this = _Right;
			_Right = _Ts;
			}
		}



protected:
	void _Assign_n(size_type _Count, const _Type& _Val)
		{	// assign _Count * _Val
		_Type _Tmp = _Val;	// in case _Val is in sequence
		erase(begin(), end());
		_Insert_n(begin(), _Count, _Tmp);
		}

	void _Insert_n(const_iterator _Where,
		size_type _Count, const _Type& _Val)
		{	// insert _Count * _Val at _Where
		iterator _Mid;
		size_type _Num;
		size_type _Off = _Where - begin();
		size_type _Rem = _Mysize - _Off;
		size_type _Oldsize = _Mysize;

 //#if _HAS_ITERATOR_DEBUGGING
		if (_Mysize < _Off)
			_DEBUG_DEQ20_ERROR("deque20 insert iterator outside range");
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		if (_Off < _Rem)
			{	// closer to front
			_TRY_BEGIN
			if (_Off < _Count)
				{	// insert longer than prefix
				for (_Num = _Count - _Off; 0 < _Num; --_Num)
					push_front(_Val);	// push excess values
				for (_Num = _Off; 0 < _Num; --_Num)
					push_front(begin()[_Count - 1]);	// push prefix

				_Mid = begin() + _Count;
				std::fill(_Mid, _Mid + _Off,
					_Val);	// fill in rest of values
				}
			else
				{	// insert not longer than prefix
				for (_Num = _Count; 0 < _Num; --_Num)
					push_front(begin()[_Count - 1]);	// push part of prefix

				_Mid = begin() + _Count;
				_Type _Tmp = _Val;	// in case _Val is in sequence
				std::copy(_Mid + _Count, _Mid + _Off,
					_Mid);	// copy rest of prefix
				std::fill(begin() + _Off, _Mid + _Off,
					_Tmp);	// fill in values
				}
			_CATCH_ALL
			for (; _Oldsize < _Mysize; )
				pop_front();	// restore old size, at least
			_RERAISE;
			_CATCH_END
			}
		else
			{		// closer to back
			_TRY_BEGIN
			if (_Rem < _Count)
				{	// insert longer than suffix
				for (_Num = _Count - _Rem; 0 < _Num; --_Num)
					push_back(_Val);	// push excess values
				for (_Num = 0; _Num < _Rem; ++_Num)
					push_back(begin()[_Off + _Num]);	// push suffix

				_Mid = begin() + _Off;
				std::fill(_Mid, _Mid + _Rem,
					_Val);	// fill in rest of values
				}
			else
				{	// insert not longer than prefix
				for (_Num = 0; _Num < _Count; ++_Num)
					push_back(begin()[_Off + _Rem
						- _Count + _Num]);	// push part of prefix

				_Mid = begin() + _Off;
				_Type _Tmp = _Val;	// in case _Val is in sequence
				std::copy_backward(_Mid, _Mid + _Rem - _Count,
					_Mid + _Rem);	// copy rest of prefix
				std::fill(_Mid, _Mid + _Count,
					_Tmp);	// fill in values
				}
			_CATCH_ALL
			for (; _Oldsize < _Mysize; )
				pop_back();	// restore old size, at least
			_RERAISE;
			_CATCH_END
			}
		}

	static void _Xlen()
		{	// report length error
		_THROW(length_error, "deque20<T> too long");
		}

	static void _Xinvarg()
		{	// report an invalid_argument error
		_THROW(invalid_argument, "invalid deque20 <T> argument");
		}

	static void _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid deque20 <T> subscript");
		}

	void _Growmap(size_type _Count)
		{	// grow map by _Count pointers
		if (max_size() / _DEQUE20SIZ - _Mapsize < _Count)
			_Xlen();	// result too long

		size_type _Inc = _Mapsize;	// try to grow by 100%
		if (_Inc < _DEQUE20MAPSIZ)
			_Inc = _DEQUE20MAPSIZ;
		if (_Count < _Inc && _Mapsize <= max_size() / _DEQUE20SIZ - _Inc)
			_Count = _Inc;
		size_type _Myboff = _Myoff / _DEQUE20SIZ;
		_Mapptr _Newmap = this->_Almap().allocate(_Mapsize + _Count);
		_Mapptr _Myptr = _Newmap + _Myboff;

		_Myptr = _STDEXT unchecked_uninitialized_copy(_Map + _Myboff,
			_Map + _Mapsize, _Myptr, this->_Almap());	// copy initial to end
		if (_Myboff <= _Count)
			{	// increment greater than offset of initial block
			_Myptr = _STDEXT unchecked_uninitialized_copy(_Map,
				_Map + _Myboff, _Myptr, this->_Almap());	// copy rest of old
			_STDEXT unchecked_uninitialized_fill_n(_Myptr, _Count - _Myboff,
				(_Tptr)0, this->_Almap());	// clear suffix of new
			_STDEXT unchecked_uninitialized_fill_n(_Newmap, _Myboff,
				(_Tptr)0, this->_Almap());	// clear prefix of new
			}
		else
			{	// increment not greater than offset of initial block
			_STDEXT unchecked_uninitialized_copy(_Map,
				_Map + _Count, _Myptr, this->_Almap());	// copy more old
			_Myptr = _STDEXT unchecked_uninitialized_copy(_Map + _Count,
				_Map + _Myboff, _Newmap, this->_Almap());	// copy rest of old
			_STDEXT unchecked_uninitialized_fill_n(_Myptr, _Count,
				(_Tptr)0, this->_Almap());	// clear rest to initial block
			}

		_Destroy_range(_Map + _Myboff, _Map + _Mapsize, this->_Almap());
		if (_Map)
			this->_Almap().deallocate(_Map, _Mapsize);	// free storage for old

		_Map = _Newmap;	// point at new
		_Mapsize += _Count;
		}

	void _Tidy()
		{	// free all storage
		while (!empty())
			pop_back();
		for (size_type _Count = _Mapsize; 0 < _Count; )
			{	// free storage for a block and destroy pointer
			if (*(_Map + --_Count) != 0)
				this->_Alval().deallocate(*(_Map + _Count), _DEQUE20SIZ);
			this->_Almap().destroy(_Map + _Count);
			}

		if (_Map)
			this->_Almap().deallocate(_Map, _Mapsize);	// free storage for map
		_Mapsize = 0;
		_Map = 0;
		}

 //#if _HAS_ITERATOR_DEBUGGING
	void _Orphan_off(size_type _Offlo) const
		{	// orphan iterators with specified offset(s)
		if (_Mysize == 0)
			_DEBUG_DEQ20_ERROR("deque20 empty before pop");
		size_type _Offhigh = _Myoff + _Mysize <= _Offlo + 1
			? (size_type)(-1) : _Offlo;
		if (_Offlo == _Myoff)
			_Offlo = 0;

		_Lockit _Lock(_LOCK_DEBUG);
		const_iterator **_Pnext = (const_iterator **)&this->_Myfirstiter;
		while (*_Pnext != 0)
			if ((*_Pnext)->_Myoff < _Offlo || _Offhigh < (*_Pnext)->_Myoff)
				_Pnext = (const_iterator **)&(*_Pnext)->_Mynextiter;
			else
				{	// orphan the iterator
				(*_Pnext)->_Mycont = 0;
				*_Pnext = (const_iterator *)(*_Pnext)->_Mynextiter;
				}
		}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	_Mapptr _Map;	// pointer to array of pointers to blocks
	size_type _Mapsize;	// size of map array
	size_type _Myoff;	// offset of initial element
	size_type _Mysize;	// current length of sequence
	};

	// deque20 implements a performant swap
template <class _Type, class _Ax>
	class _Move_operation_category<deque20<_Type, _Ax> >
	{
	public:
		typedef _Swap_move_tag _Move_cat;
	};

	// deque20 TEMPLATE OPERATORS

template<class _Type,
	class _Alloc> inline
	void swap(deque20<_Type, _Alloc>& _Left, deque20<_Type, _Alloc>& _Right)
	{	// swap _Left and _Right deques
	_Left.swap(_Right);
	}

template<class _Type,
	class _Alloc> inline
	bool operator==(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test for deque20 equality
	return (_Left.size() == _Right.size()
		&& equal(_Left.begin(), _Left.end(), _Right.begin()));
	}

template<class _Type,
	class _Alloc> inline
	bool operator!=(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test for deque20 inequality
	return (!(_Left == _Right));
	}

template<class _Type,
	class _Alloc> inline
	bool operator<(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test if _Left < _Right for deques
	return (lexicographical_compare(_Left.begin(), _Left.end(),
		_Right.begin(), _Right.end()));
	}

template<class _Type,
	class _Alloc> inline
	bool operator<=(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test if _Left <= _Right for deques
	return (!(_Right < _Left));
	}

template<class _Type,
	class _Alloc> inline
	bool operator>(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test if _Left > _Right for deques
	return (_Right < _Left);
	}

template<class _Type,
	class _Alloc> inline
	bool operator>=(const deque20<_Type, _Alloc>& _Left,
		const deque20<_Type, _Alloc>& _Right)
	{	// test if _Left >= _Right for deques
	return (!(_Left < _Right));
	}
_STD_END

  #pragma warning(default:4284)

#ifdef _MSC_VER
 #pragma warning(pop)
 #pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _DEQUE20_ */
