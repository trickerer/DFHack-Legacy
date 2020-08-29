/*
 * This is the implementation of VS2008 vector class which is
 * compatible with VS2015 vector structure, achieved by removing
 * '_Alty _Alval;' allocator member
 */

#pragma once
#ifndef _VECTOR12_
#define _VECTOR12_
#ifndef RC_INVOKED
#include <memory>
#include <stdexcept>

#ifdef _MSC_VER
 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)
 #pragma warning(disable: 4244)
#endif  /* _MSC_VER */

_STD_BEGIN
template<class _Type,
	class _Ax = allocator<_Type> >
	class vector12;

		// TEMPLATE CLASS _Vector12_const_iterator
template<class _Type,
	class _Alloc>
	class _Vector12_const_iterator
		: public _Ranit<_Type, typename _Alloc::difference_type,
			typename _Alloc::const_pointer, typename _Alloc::const_reference>
	{	// iterator for nonmutable vector12
public:
	typedef _Vector12_const_iterator<_Type, _Alloc> _MyType;
	typedef vector12<_Type, _Alloc> _Myvec;
	typedef typename _Alloc::pointer _Tptr;

	typedef random_access_iterator_tag iterator_category;
	typedef _Type value_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::const_pointer pointer;
	typedef typename _Alloc::const_reference reference;

//#if _SECURE_SCL
//	typedef _Range_checked_iterator_tag _Checked_iterator_category;
//#endif

//#if _SECURE_SCL && !_HAS_ITERATOR_DEBUGGING
//	typedef pointer _Checked_iterator_base_type;
//
//	_Checked_iterator_base_type _Checked_iterator_base() const
//	{
//		return _Myptr;
//	}
//
//	void _Checked_iterator_assign_from_base(_Checked_iterator_base_type _Base)
//	{
//		this->_Myptr = const_cast<_Tptr>(_Base);
//	}
//#endif
//
	typedef _Tptr _Inner_type;

	_Vector12_const_iterator()
		{	// construct with null pointer
		_Myptr = 0;
		}

 #if _HAS_ITERATOR_DEBUGGING
	_Vector12_const_iterator(_Tptr _Ptr, const _Container_base *_Pvector)
		{	// construct with pointer _Ptr
		//_SCL_SECURE_VALIDATE(_Pvector == NULL || (((_Myvec *)_Pvector)->_Myfirst <= _Ptr && _Ptr <= ((_Myvec *)_Pvector)->_Mylast));
		this->_Adopt(_Pvector);
		_Myptr = _Ptr;
		}

 //#elif _SECURE_SCL
	//_Vector12_const_iterator(_Tptr _Ptr, const _Container_base *_Pvector)
	//	{	// construct with pointer _Ptr
	//	_SCL_SECURE_VALIDATE(_Pvector != NULL && ((_Myvec *)_Pvector)->_Myfirst <= _Ptr && _Ptr <= ((_Myvec *)_Pvector)->_Mylast);
	//	this->_Set_container(_Pvector);
	//	_Myptr = _Ptr;
	//	}

 #else
	explicit _Vector12_const_iterator(_Tptr _Ptr)
		{	// construct with pointer _Ptr
		_Myptr = _Ptr;
		}
#endif /* _HAS_ITERATOR_DEBUGGING */

	reference operator*() const
		{	// return designated object

 #if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0
			|| _Myptr < ((_Myvec *)this->_Mycont)->_Myfirst
			|| ((_Myvec *)this->_Mycont)->_Mylast <= _Myptr)
			{
			_DEBUG_ERROR("vector12 iterator not dereferencable");
			_SCL_SECURE_OUT_OF_RANGE;
			}
 #else
 		//_SCL_SECURE_VALIDATE(this->_Has_container());
		//_SCL_SECURE_VALIDATE_RANGE(_Myptr < ((_Myvec *)(this->_Getmycont()))->_Mylast);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (*_Myptr);
		}

	pointer operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_MyType& operator++()
		{	// preincrement
		//_SCL_SECURE_VALIDATE(this->_Has_container());
		//_SCL_SECURE_VALIDATE_RANGE(_Myptr < ((_Myvec *)(this->_Getmycont()))->_Mylast);

 #if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0
			|| ((_Myvec *)this->_Mycont)->_Mylast <= _Myptr)
			_DEBUG_ERROR("vector12 iterator not incrementable");
 #endif /* _HAS_ITERATOR_DEBUGGING */

		++_Myptr;
		return (*this);
		}

	_MyType operator++(int)
		{	// postincrement
		_MyType _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_MyType& operator--()
		{	// predecrement
		//_SCL_SECURE_VALIDATE(this->_Has_container());
		//_SCL_SECURE_VALIDATE_RANGE(_Myptr > ((_Myvec *)(this->_Getmycont()))->_Myfirst);

 #if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0
			|| _Myptr == ((_Myvec *)this->_Mycont)->_Myfirst)
			_DEBUG_ERROR("vector12 iterator not decrementable");
 #endif /* _HAS_ITERATOR_DEBUGGING */

		--_Myptr;
		return (*this);
		}

	_MyType operator--(int)
		{	// postdecrement
		_MyType _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_MyType& operator+=(difference_type _Off)
		{	// increment by integer
		//_SCL_SECURE_VALIDATE(this->_Has_container());
		//_SCL_SECURE_VALIDATE_RANGE(
			//_Myptr + _Off <= ((_Myvec *)(this->_Getmycont()))->_Mylast &&
			//_Myptr + _Off >= ((_Myvec *)(this->_Getmycont()))->_Myfirst);
		_Myptr += _Off;
		return (*this);
		}

	_MyType operator+(difference_type _Off) const
		{	// return this + integer
		_MyType _Tmp = *this;
		return (_Tmp += _Off);
		}

	_MyType& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_MyType operator-(difference_type _Off) const
		{	// return this - integer
		_MyType _Tmp = *this;
		return (_Tmp -= _Off);
		}

	difference_type operator-(const _MyType& _Right) const
		{	// return difference of iterators

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #else
		//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (_Myptr - _Right._Myptr);
		}

	reference operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}

	bool operator==(const _MyType& _Right) const
		{	// test for iterator equality

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #else
		//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (_Myptr == _Right._Myptr);
		}

	bool operator!=(const _MyType& _Right) const
		{	// test for iterator inequality
		return (!(*this == _Right));
		}

	bool operator<(const _MyType& _Right) const
		{	// test if this < _Right

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #else
		//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Same_container(_Right));
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (_Myptr < _Right._Myptr);
		}

	bool operator>(const _MyType& _Right) const
		{	// test if this > _Right
		return (_Right < *this);
		}

	bool operator<=(const _MyType& _Right) const
		{	// test if this <= _Right
		return (!(_Right < *this));
		}

	bool operator>=(const _MyType& _Right) const
		{	// test if this >= _Right
		return (!(*this < _Right));
		}

 #if _HAS_ITERATOR_DEBUGGING
	void _Compat(const _MyType& _Right) const
		{	// test for compatible iterator pair
		if (this->_Mycont == 0 || this->_Mycont != _Right._Mycont)
			{
			_DEBUG_ERROR("vector12 iterators incompatible");
			_SCL_SECURE_INVALID_ARGUMENT;
			}
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	static void _Xlen()
		{	// report a length_error
		_THROW(length_error, "vector12<T> too long");
		}

	static void _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid vector12<T> subscript");
		}

	static void _Xinvarg()
		{	// report an invalid_argument error
		_THROW(invalid_argument, "invalid vector12<T> argument");
		}

	_Tptr _Myptr;	// offset of element in vector12
	};

template<class _Type,
	class _Alloc> inline
	_Vector12_const_iterator<_Type, _Alloc> operator+(
		typename _Vector12_const_iterator<_Type, _Alloc>::difference_type _Off,
		_Vector12_const_iterator<_Type, _Alloc> _Next)
	{	// add offset to iterator
	return (_Next += _Off);
	}

		// TEMPLATE CLASS _Vector12_iterator
template<class _Type,
	class _Alloc>
	class _Vector12_iterator
		: public _Vector12_const_iterator<_Type, _Alloc>
	{	// iterator for mutable vector12
public:
	typedef _Vector12_iterator<_Type, _Alloc> _MyType;
	typedef _Vector12_const_iterator<_Type, _Alloc> _Mybase;

	typedef random_access_iterator_tag iterator_category;
	typedef _Type value_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::pointer pointer;
	typedef typename _Alloc::reference reference;

//#if _SECURE_SCL && !_HAS_ITERATOR_DEBUGGING
//	typedef pointer _Checked_iterator_base_type;
//
//	_Checked_iterator_base_type _Checked_iterator_base() const
//	{
//		return this->_Myptr;
//	}
//
//	void _Checked_iterator_assign_from_base(_Checked_iterator_base_type _Base)
//	{
//		this->_Myptr = _Base;
//	}
//#endif

	_Vector12_iterator()
		{	// construct with null vector12 pointer
		}

 #if _HAS_ITERATOR_DEBUGGING
	_Vector12_iterator(pointer _Ptr, const _Container_base *_Pvector)
		: _Mybase(_Ptr, _Pvector)
		{	// construct with pointer _Ptr
		}

 //#elif _SECURE_SCL
	//_Vector12_iterator(pointer _Ptr, const _Container_base *_Pvector)
	//	: _Mybase(_Ptr, _Pvector)
	//	{	// construct with pointer _Ptr
	//	}

 #else
	explicit _Vector12_iterator(pointer _Ptr)
		: _Mybase(_Ptr)
		{	// construct with pointer _Ptr
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	reference operator*() const
		{	// return designated object
		return ((reference)**(_Mybase *)this);
		}

	pointer operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_MyType& operator++()
		{	// preincrement
		++(*(_Mybase *)this);
		return (*this);
		}

	_MyType operator++(int)
		{	// postincrement
		_MyType _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_MyType& operator--()
		{	// predecrement
		--(*(_Mybase *)this);
		return (*this);
		}

	_MyType operator--(int)
		{	// postdecrement
		_MyType _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_MyType& operator+=(difference_type _Off)
		{	// increment by integer
		(*(_Mybase *)this) += _Off;
		return (*this);
		}

	_MyType operator+(difference_type _Off) const
		{	// return this + integer
		_MyType _Tmp = *this;
		return (_Tmp += _Off);
		}

	_MyType& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_MyType operator-(difference_type _Off) const
		{	// return this - integer
		_MyType _Tmp = *this;
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

template<class _Type,
	class _Alloc> inline
	_Vector12_iterator<_Type, _Alloc> operator+(
		typename _Vector12_iterator<_Type, _Alloc>::difference_type _Off,
		_Vector12_iterator<_Type, _Alloc> _Next)
	{	// add offset to iterator
	return (_Next += _Off);
	}

class _Container_base_no_alloc
	{	// base of all containers
	public:
	void _Swap_aux(_Container_base_no_alloc&)
		{
		// Do nothing: we don't have an aux object.
		}
	};

template<class _Alloc>
	class _Container_base_aux_alloc_empty_no_alloc
		: public _Container_base_no_alloc
	{ // base class for containers to avoid holding allocator _Alaux
	protected:
	explicit _Container_base_aux_alloc_empty_no_alloc(_Alloc) { }

	_Container_base_aux_alloc_empty_no_alloc(const _Container_base_aux_alloc_empty_no_alloc&) { }

	_Container_base_aux_alloc_empty_no_alloc& operator=(const _Container_base_aux_alloc_empty_no_alloc&)
		{
		return *this;
		}

	~_Container_base_aux_alloc_empty_no_alloc() { }
	};

		// TEMPLATE CLASS _Vector12_val
template<class _Type,
	class _Alloc>
	class _Vector12_val
		: public _Container_base_aux_alloc_empty_no_alloc<_Alloc>
	{	// base class for vector12 to hold allocator _Alval
protected:
	_Vector12_val(_Alloc _Al = _Alloc())
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Al)//, _Alval(_Al)
		{	// construct allocator from _Al
		}

	typedef typename _Alloc::template
		rebind<_Type>::other _Alty;

    _Alty _Alval() const
        {
            return _Alty();
        }

	//_Alty _Alval;	// allocator object for values
	};

		// TEMPLATE CLASS vector12
template<class _Type,
	class _Ax>
	class vector12
		: public _Vector12_val<_Type, _Ax>
	{	// varying size array of values
public:
	typedef vector12<_Type, _Ax> _MyType;
	typedef _Vector12_val<_Type, _Ax> _Mybase;
	typedef typename _Mybase::_Alty _Alloc;
	typedef _Alloc allocator_type;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type _Dift;
	typedef _Dift difference_type;
	typedef typename _Alloc::pointer _Tptr;
	typedef typename _Alloc::const_pointer _Ctptr;
	typedef _Tptr pointer;
	typedef _Ctptr const_pointer;
	typedef typename _Alloc::reference _Reft;
	typedef _Reft reference;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::value_type value_type;

  #define _VEC_ITER_BASE(it)	(it)._Myptr

	typedef _Vector12_iterator<_Type, _Alloc> iterator;
	typedef _Vector12_const_iterator<_Type, _Alloc> const_iterator;

//	friend class _Vector12_iterator<_Type, _Alloc>;
	friend class _Vector12_const_iterator<_Type, _Alloc>;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	vector12()
		: _Mybase()
		{	// construct empty vector12
		_Buy(0);
		}

	explicit vector12(const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct empty vector12 with allocator
		_Buy(0);
		}

	explicit vector12(size_type _Count)
		: _Mybase()
		{	// construct from _Count * _Type()
		_Construct_n(_Count, _Type());
		}

	vector12(size_type _Count, const _Type& _Val)
		: _Mybase()
		{	// construct from _Count * _Val
		_Construct_n(_Count, _Val);
		}

	vector12(size_type _Count, const _Type& _Val, const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct from _Count * _Val, with allocator
		_Construct_n(_Count, _Val);
		}

	vector12(const _MyType& _Right)
		: _Mybase(_Right._Alval())
		{	// construct by copying _Right
		if (_Buy(_Right.size()))
			_TRY_BEGIN
			_Mylast = _Ucopy(_Right.begin(), _Right.end(), _Myfirst);
			_CATCH_ALL
			_Tidy();
			_RERAISE;
			_CATCH_END
		}

	template<class _Iter>
		vector12(_Iter _First, _Iter _Last)
		: _Mybase()
		{	// construct from [_First, _Last)
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		vector12(_Iter _First, _Iter _Last, const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct from [_First, _Last), with allocator
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Construct(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// initialize with _Count * _Val
		size_type _Size = (size_type)_Count;
		_Construct_n(_Size, (_Type)_Val);
		}

	template<class _Iter>
		void _Construct(_Iter _First,
			_Iter _Last, input_iterator_tag)
		{	// initialize with [_First, _Last), input iterators
		_Buy(0);
		_TRY_BEGIN
		insert(begin(), _First, _Last);
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	void _Construct_n(size_type _Count, const _Type& _Val)
		{	// construct from _Count * _Val
		if (_Buy(_Count))
			{	// nonzero, fill it
			_TRY_BEGIN
			_Mylast = _Ufill(_Myfirst, _Count, _Val);
			_CATCH_ALL
			_Tidy();
			_RERAISE;
			_CATCH_END
			}
		}

	~vector12()
		{	// destroy the object
		_Tidy();
		}

	_MyType& operator=(const _MyType& _Right)
		{	// assign _Right
		if (this != &_Right)
			{	// worth doing

 #if _HAS_ITERATOR_DEBUGGING
			this->_Orphan_all();
 #endif /* _HAS_ITERATOR_DEBUGGING */

			if (_Right.size() == 0)
				clear();	// new sequence empty, erase existing sequence
			else if (_Right.size() <= size())
				{	// enough elements, copy new and destroy old
				pointer _Ptr = _STDEXT unchecked_copy(_Right._Myfirst, _Right._Mylast,
					_Myfirst);	// copy new
				_Destroy(_Ptr, _Mylast);	// destroy old
				_Mylast = _Myfirst + _Right.size();
				}
			else if (_Right.size() <= capacity())
				{	// enough room, copy and construct new
				pointer _Ptr = _Right._Myfirst + size();
				_STDEXT unchecked_copy(_Right._Myfirst, _Ptr, _Myfirst);
				_Mylast = _Ucopy(_Ptr, _Right._Mylast, _Mylast);
				}
			else
				{	// not enough room, allocate new array and construct new
				if (_Myfirst != 0)
					{	// discard old array
					_Destroy(_Myfirst, _Mylast);
					this->_Alval().deallocate(_Myfirst, _Myend - _Myfirst);
					}
				if (_Buy(_Right.size()))
					_Mylast = _Ucopy(_Right._Myfirst, _Right._Mylast,
						_Myfirst);
				}
			}
		return (*this);
		}

	void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
		if (max_size() < _Count)
			_Xlen();	// result too long
		else if (capacity() < _Count)
			{	// not enough room, reallocate
			pointer _Ptr = this->_Alval().allocate(_Count);

			_TRY_BEGIN
			_Umove(begin(), end(), _Ptr);
			_CATCH_ALL
			this->_Alval().deallocate(_Ptr, _Count);
			_RERAISE;
			_CATCH_END

			size_type _Size = size();
			if (_Myfirst != 0)
				{	// destroy and deallocate old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval().deallocate(_Myfirst, _Myend - _Myfirst);
				}

 #if _HAS_ITERATOR_DEBUGGING
			this->_Orphan_all();
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_Myend = _Ptr + _Count;
			_Mylast = _Ptr + _Size;
			_Myfirst = _Ptr;
			}
		}

	size_type capacity() const
		{	// return current length of allocated storage
		return (_Myfirst == 0 ? 0 : _Myend - _Myfirst);
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//iterator begin()
	//	{	// return iterator for beginning of mutable sequence
	//	return (iterator(_Myfirst, this));
	//	}

	//const_iterator begin() const
	//	{	// return iterator for beginning of nonmutable sequence
	//	return (const_iterator(_Myfirst, this));
	//	}

	//iterator end()
	//	{	// return iterator for end of mutable sequence
	//	return (iterator(_Mylast, this));
	//	}

	//const_iterator end() const
	//	{	// return iterator for end of nonmutable sequence
	//	return (const_iterator(_Mylast, this));
	//	}

	//iterator _Make_iter(const_iterator _Where) const
	//	{	// make iterator from const_iterator
	//	return (iterator(_Where._Myptr, this));
	//	}

 //#else /* _HAS_ITERATOR_DEBUGGING */
	iterator begin()
		{	// return iterator for beginning of mutable sequence
		return (iterator(_Myfirst));
		}

	const_iterator begin() const
		{	// return iterator for beginning of nonmutable sequence
		return (const_iterator(_Myfirst));
		}

	iterator end()
		{	// return iterator for end of mutable sequence
		return (iterator(_Mylast));
		}

	const_iterator end() const
		{	// return iterator for end of nonmutable sequence
		return (const_iterator(_Mylast));
		}

	iterator _Make_iter(const_iterator _Where) const
		{	// make iterator from const_iterator
		return (iterator(_Where._Myptr));
		}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

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
		if (size() < _Newsize)
			_Insert_n(end(), _Newsize - size(), _Val);
		else if (_Newsize < size())
			erase(begin() + _Newsize, end());
		}

	size_type size() const
		{	// return length of sequence
		return (_Mylast - _Myfirst);
		}

	size_type max_size() const
		{	// return maximum possible length of sequence
		return (this->_Alval().max_size());
		}

	bool empty() const
		{	// test if sequence is empty
		return (size() == 0);
		}

	_Alloc get_allocator() const
		{	// return allocator object for values
		return (this->_Alval());
		}

	const_reference at(size_type _Pos) const
		{	// subscript nonmutable sequence with checking
		if (size() <= _Pos)
			_Xran();
		return (*(begin() + _Pos));
		}

	reference at(size_type _Pos)
		{	// subscript mutable sequence with checking
		if (size() <= _Pos)
			_Xran();
		return (*(begin() + _Pos));
		}

	const_reference operator[](size_type _Pos) const
		{	// subscript nonmutable sequence

 #if _HAS_ITERATOR_DEBUGGING
		if (size() <= _Pos)
			{
			_DEBUG_ERROR("vector12 subscript out of range");
			_SCL_SECURE_OUT_OF_RANGE;
			}
 #endif /* _HAS_ITERATOR_DEBUGGING */
		//_SCL_SECURE_VALIDATE_RANGE(_Pos < size());

		return (*(_Myfirst + _Pos));
		}

	reference operator[](size_type _Pos)
		{	// subscript mutable sequence

 #if _HAS_ITERATOR_DEBUGGING
		if (size() <= _Pos)
			{
			_DEBUG_ERROR("vector12 subscript out of range");
			_SCL_SECURE_OUT_OF_RANGE;
			}
 #endif /* _HAS_ITERATOR_DEBUGGING */
		//_SCL_SECURE_VALIDATE_RANGE(_Pos < size());

		return (*(_Myfirst + _Pos));
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

	void push_back(const _Type& _Val)
		{	// insert element at end
		if (size() < capacity())

 #if _HAS_ITERATOR_DEBUGGING
			{ // room at end, construct it there
			_Orphan_range(_Mylast, _Mylast);
			_Mylast = _Ufill(_Mylast, 1, _Val);
			}

 #else /* _HAS_ITERATOR_DEBUGGING */
			_Mylast = _Ufill(_Mylast, 1, _Val);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		else
			insert(end(), _Val);
		}

 #if _HAS_ITERATOR_DEBUGGING
	void pop_back()
		{	// erase element at end
		if (empty())
			_DEBUG_ERROR("vector12 empty before pop");
		else
			{	// erase last element
			_Orphan_range(_Mylast - 1, _Mylast);
			_Destroy(_Mylast - 1, _Mylast);
			--_Mylast;
			}
		}

 #else /* _HAS_ITERATOR_DEBUGGING */
	void pop_back()
		{	// erase element at end
		if (!empty())
			{	// erase last element
			_Destroy(_Mylast - 1, _Mylast);
			--_Mylast;
			}
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	template<class _Iter>
		void assign(_Iter _First, _Iter _Last)
		{	// assign [_First, _Last)
		_Assign(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Assign(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// assign _Count * _Val
		_Assign_n((size_type)_Count, (_Type)_Val);
		}

	template<class _Iter>
		void _Assign(_Iter _First, _Iter _Last, input_iterator_tag)
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
		size_type _Off = size() == 0 ? 0 : _Where - begin();
		_Insert_n(_Where, (size_type)1, _Val);
		return (begin() + _Off);
		}

	void insert(const_iterator _Where, size_type _Count, const _Type& _Val)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, _Count, _Val);
		}

	template<class _Iter>
		void insert(const_iterator _Where, _Iter _First, _Iter _Last)
		{	// insert [_First, _Last) at _Where
		_Insert(_Where, _First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Insert(const_iterator _Where, _Iter _First, _Iter _Last,
			_Int_iterator_tag)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, (size_type)_First, (_Type)_Last);
		}

	template<class _Iter>
		void _Insert(const_iterator _Where, _Iter _First, _Iter _Last,
			input_iterator_tag)
		{	// insert [_First, _Last) at _Where, input iterators

 #if _HAS_ITERATOR_DEBUGGING
		if (_Where._Mycont != this
			|| _Where._Myptr < _Myfirst || _Mylast < _Where._Myptr)
			_DEBUG_ERROR("vector12 insert iterator outside range");
 #endif /* _HAS_ITERATOR_DEBUGGING */

		if (_First != _Last)
			{	// worth doing, gather at end and rotate into place
			size_type _Oldsize = size();
			size_type _Whereoff = _Where._Myptr - _Myfirst;

			for (; _First != _Last; ++_First)
				_Insert_n(end(), (size_type)1, (value_type)*_First);

			_Reverse(_Myfirst + _Whereoff, _Myfirst + _Oldsize);
			_Reverse(_Myfirst + _Oldsize, _Mylast);
			_Reverse(_Myfirst + _Whereoff, _Mylast);
			}
		}

	template<class _Iter>
		void _Insert(const_iterator _Where,
			_Iter _First, _Iter _Last, forward_iterator_tag)
		{	// insert [_First, _Last) at _Where, forward iterators

 #if _HAS_ITERATOR_DEBUGGING
		if (_Where._Mycont != this
			|| _Where._Myptr < _Myfirst || _Mylast < _Where._Myptr)
			_DEBUG_ERROR("vector12 insert iterator outside range");
		_DEBUG_RANGE(_First, _Last);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		size_type _Count = 0;
		_Distance(_First, _Last, _Count);
		size_type _Capacity = capacity();

		if (_Count == 0)
			;
		else if (max_size() - size() < _Count)
			_Xlen();	// result too long
		else if (_Capacity < size() + _Count)
			{	// not enough room, reallocate
			_Capacity = max_size() - _Capacity / 2 < _Capacity
				? 0 : _Capacity + _Capacity / 2;	// try to grow by 50%
			if (_Capacity < size() + _Count)
				_Capacity = size() + _Count;
			pointer _Newvec = this->_Alval().allocate(_Capacity);
			pointer _Ptr = _Newvec;

			_TRY_BEGIN
			_Ptr = _Umove(_Myfirst, _VEC_ITER_BASE(_Where),
				_Newvec);	// copy prefix
			_Ptr = _Ucopy(_First, _Last, _Ptr);	// add new stuff
			_Umove(_VEC_ITER_BASE(_Where), _Mylast, _Ptr);	// copy suffix
			_CATCH_ALL
			_Destroy(_Newvec, _Ptr);
			this->_Alval().deallocate(_Newvec, _Capacity);
			_RERAISE;
			_CATCH_END

			_Count += size();
			if (_Myfirst != 0)
				{	// destroy and deallocate old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval().deallocate(_Myfirst, _Myend - _Myfirst);
				}

 #if _HAS_ITERATOR_DEBUGGING
			this->_Orphan_all();
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_Myend = _Newvec + _Capacity;
			_Mylast = _Newvec + _Count;
			_Myfirst = _Newvec;
			}
		else
			{	// new stuff fits, append and rotate into place
			_Ucopy(_First, _Last, _Mylast);

			_Reverse(_Where._Myptr, _Mylast);
			_Reverse(_Mylast, _Mylast + _Count);
			_Reverse(_Where._Myptr, _Mylast + _Count);

			_Mylast += _Count;

 #if _HAS_ITERATOR_DEBUGGING
			_Orphan_range(_Where._Myptr, _Mylast);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			}
		}

 	void _Reverse(pointer _First, pointer _Last)
		{	// reverse a subrange
		for (; _First != _Last && _First != --_Last; ++_First)
			{	// swap distinct _First and _Last
			value_type _Temp = *_First;

			*_First = *_Last;
			*_Last = _Temp;
			}
		}

 #if _HAS_ITERATOR_DEBUGGING
	iterator erase(const_iterator _Where)
		{	// erase element at where
		if (_Where._Mycont != this
			|| _Where._Myptr < _Myfirst || _Mylast <= _Where._Myptr)
			_DEBUG_ERROR("vector12 erase iterator outside range");
		_STDEXT unchecked_copy(_Where._Myptr + 1, _Mylast, _Where._Myptr);
		_Destroy(_Mylast - 1, _Mylast);
		_Orphan_range(_Where._Myptr, _Mylast);
		--_Mylast;
		return (iterator(_Where._Myptr, this));
		}

 #else /* _HAS_ITERATOR_DEBUGGING */
	iterator erase(const_iterator _Where)
		{	// erase element at where
		_STDEXT unchecked_copy(_VEC_ITER_BASE(_Where) + 1, _Mylast,
			_VEC_ITER_BASE(_Where));
		_Destroy(_Mylast - 1, _Mylast);
		--_Mylast;
		return (_Make_iter(_Where));
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	iterator erase(const_iterator _First_arg,
		const_iterator _Last_arg)
		{	// erase [_First, _Last)
		iterator _First = _Make_iter(_First_arg);
		iterator _Last = _Make_iter(_Last_arg);

		if (_First != _Last)
			{	// worth doing, copy down over hole

 #if _HAS_ITERATOR_DEBUGGING
			if (_Last < _First || _First._Mycont != this
				|| _First._Myptr < _Myfirst || _Mylast < _Last._Myptr)
				_DEBUG_ERROR("vector12 erase iterator outside range");
			pointer _Ptr = _STDEXT unchecked_copy(_VEC_ITER_BASE(_Last), _Mylast,
				_VEC_ITER_BASE(_First));
			_Orphan_range(_First._Myptr, _Mylast);

 #else /* _HAS_ITERATOR_DEBUGGING */
			pointer _Ptr = _STDEXT unchecked_copy(_VEC_ITER_BASE(_Last), _Mylast,
				_VEC_ITER_BASE(_First));
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_Destroy(_Ptr, _Mylast);
			_Mylast = _Ptr;
			}
#if _HAS_ITERATOR_DEBUGGING
        return (iterator(_First._Myptr, this));
#else
		return (_First);
#endif
		}

	void clear()
		{	// erase all
		erase(begin(), end());
		}

	void swap(_MyType& _Right)
		{	// exchange contents with _Right
		if (this == &_Right)
			;	// same object, do nothing
		else if (this->_Alval() == _Right._Alval())
			{	// same allocator, swap control information

 #if _HAS_ITERATOR_DEBUGGING
			this->_Swap_all(_Right);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			this->_Swap_aux(_Right);

			std::swap(_Myfirst, _Right._Myfirst);
			std::swap(_Mylast, _Right._Mylast);
			std::swap(_Myend, _Right._Myend);
			}
		else
			{	// different allocator, do multiple assigns
			this->_Swap_aux(_Right);

			_MyType _Ts = *this;

			*this = _Right;
			_Right = _Ts;
			}
		}



protected:
	void _Assign_n(size_type _Count, const _Type& _Val)
		{	// assign _Count * _Val
		_Type _Tmp = _Val;	// in case _Val is in sequence
		erase(begin(), end());
		insert(begin(), _Count, _Tmp);
		}

	bool _Buy(size_type _Capacity)
		{	// allocate array with _Capacity elements
		_Myfirst = 0, _Mylast = 0, _Myend = 0;
		if (_Capacity == 0)
			return (false);
		else if (max_size() < _Capacity)
			_Xlen();	// result too long
		else
			{	// nonempty array, allocate storage
			_Myfirst = this->_Alval().allocate(_Capacity);
			_Mylast = _Myfirst;
			_Myend = _Myfirst + _Capacity;
			}
		return (true);
		}

	void _Destroy(pointer _First, pointer _Last)
		{	// destroy [_First, _Last) using allocator
		_Destroy_range(_First, _Last, this->_Alval());
		}

	void _Tidy()
		{	// free all storage
		if (_Myfirst != 0)
			{	// something to free, destroy and deallocate it

 #if _HAS_ITERATOR_DEBUGGING
			this->_Orphan_all();
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_Destroy(_Myfirst, _Mylast);
			this->_Alval().deallocate(_Myfirst, _Myend - _Myfirst);
			}
		_Myfirst = 0, _Mylast = 0, _Myend = 0;
		}

	template<class _Iter>
		pointer _Ucopy(_Iter _First, _Iter _Last, pointer _Ptr)
		{	// copy initializing [_First, _Last), using allocator
		return (_STDEXT unchecked_uninitialized_copy(_First, _Last,
			_Ptr, this->_Alval()));
		}

	template<class _Iter>
		pointer _Umove(_Iter _First, _Iter _Last, pointer _Ptr)
		{	// move initializing [_First, _Last), using allocator
		return (_STDEXT _Unchecked_uninitialized_move(_First, _Last,
			_Ptr, this->_Alval()));
		}

	void _Insert_n(const_iterator _Where,
		size_type _Count, const _Type& _Val)
		{	// insert _Count * _Val at _Where

 #if _HAS_ITERATOR_DEBUGGING
		if (_Where._Mycont != this
			|| _Where._Myptr < _Myfirst || _Mylast < _Where._Myptr)
			_DEBUG_ERROR("vector12 insert iterator outside range");
 #endif /* _HAS_ITERATOR_DEBUGGING */

		size_type _Capacity = capacity();

		if (_Count == 0)
			;
		else if (max_size() - size() < _Count)
			_Xlen();	// result too long
		else if (_Capacity < size() + _Count)
			{	// not enough room, reallocate
			_Capacity = max_size() - _Capacity / 2 < _Capacity
				? 0 : _Capacity + _Capacity / 2;	// try to grow by 50%
			if (_Capacity < size() + _Count)
				_Capacity = size() + _Count;
			pointer _Newvec = this->_Alval().allocate(_Capacity);
			pointer _Ptr = _Newvec;

			_TRY_BEGIN
			_Ptr = _Umove(_Myfirst, _VEC_ITER_BASE(_Where),
				_Newvec);	// copy prefix
			_Ptr = _Ufill(_Ptr, _Count, _Val);	// add new stuff
			_Umove(_VEC_ITER_BASE(_Where), _Mylast, _Ptr);	// copy suffix
			_CATCH_ALL
			_Destroy(_Newvec, _Ptr);
			this->_Alval().deallocate(_Newvec, _Capacity);
			_RERAISE;
			_CATCH_END

			_Count += size();
			if (_Myfirst != 0)
				{	// destroy and deallocate old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval().deallocate(_Myfirst, _Myend - _Myfirst);
				}

 #if _HAS_ITERATOR_DEBUGGING
			this->_Orphan_all();
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_Myend = _Newvec + _Capacity;
			_Mylast = _Newvec + _Count;
			_Myfirst = _Newvec;
			}
		else if ((size_type)(_Mylast - _VEC_ITER_BASE(_Where)) < _Count)
			{	// new stuff spills off end
			_Type _Tmp = _Val;	// in case _Val is in sequence

			_Umove(_VEC_ITER_BASE(_Where), _Mylast,
				_VEC_ITER_BASE(_Where) + _Count);	// copy suffix

			_TRY_BEGIN
			_Ufill(_Mylast, _Count - (_Mylast - _VEC_ITER_BASE(_Where)),
				_Tmp);	// insert new stuff off end
			_CATCH_ALL
			_Destroy(_VEC_ITER_BASE(_Where) + _Count, _Mylast + _Count);
			_RERAISE;
			_CATCH_END

			_Mylast += _Count;

 #if _HAS_ITERATOR_DEBUGGING
			_Orphan_range(_Where._Myptr, _Mylast);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			std::fill(_VEC_ITER_BASE(_Where), _Mylast - _Count,
				_Tmp);	// insert up to old end
			}
		else
			{	// new stuff can all be assigned
			_Type _Tmp = _Val;	// in case _Val is in sequence

			pointer _Oldend = _Mylast;
			_Mylast = _Umove(_Oldend - _Count, _Oldend,
				_Mylast);	// copy suffix

 #if _HAS_ITERATOR_DEBUGGING
			_Orphan_range(_Where._Myptr, _Mylast);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			_STDEXT _Unchecked_move_backward(_VEC_ITER_BASE(_Where), _Oldend - _Count,
				_Oldend);	// copy hole
			std::fill(_VEC_ITER_BASE(_Where), _VEC_ITER_BASE(_Where) + _Count,
				_Tmp);	// insert into hole
			}
		}

	pointer _Ufill(pointer _Ptr, size_type _Count, const _Type &_Val)
		{	// copy initializing _Count * _Val, using allocator
		_STDEXT unchecked_uninitialized_fill_n(_Ptr, _Count, _Val, this->_Alval());
		return (_Ptr + _Count);
		}

	static void _Xlen()
		{	// report a length_error
		_THROW(length_error, "vector12<T> too long");
		}

	static void _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid vector12<T> subscript");
		}

	static void _Xinvarg()
		{	// report an invalid_argument error
		_THROW(invalid_argument, "invalid vector12<T> argument");
		}

 #if _HAS_ITERATOR_DEBUGGING
	void _Orphan_range(pointer _First, pointer _Last) const
		{	// orphan iterators within specified (inclusive) range
		_Lockit _Lock(_LOCK_DEBUG);
		const_iterator **_Pnext = (const_iterator **)&this->_Myfirstiter;
		while (*_Pnext != 0)
			if ((*_Pnext)->_Myptr < _First || _Last < (*_Pnext)->_Myptr)
				_Pnext = (const_iterator **)&(*_Pnext)->_Mynextiter;
			else
				{	// orphan the iterator
				(*_Pnext)->_Mycont = 0;
				*_Pnext = (const_iterator *)(*_Pnext)->_Mynextiter;
				}
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	pointer _Myfirst;	// pointer to beginning of array
	pointer _Mylast;	// pointer to current end of sequence
	pointer _Myend;	// pointer to end of array
	};

	// vector12 implements a performant swap
template <class _Type, class _Ax>
	class _Move_operation_category<vector12<_Type, _Ax> >
	{
	public:
		typedef _Swap_move_tag _Move_cat;
	};

		// vector12 TEMPLATE FUNCTIONS
template<class _Type,
	class _Alloc> inline
	bool operator==(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test for vector12 equality
	return (_Left.size() == _Right.size()
		&& equal(_Left.begin(), _Left.end(), _Right.begin()));
	}

template<class _Type,
	class _Alloc> inline
	bool operator!=(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test for vector12 inequality
	return (!(_Left == _Right));
	}

template<class _Type,
	class _Alloc> inline
	bool operator<(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test if _Left < _Right for vectors
	return (lexicographical_compare(_Left.begin(), _Left.end(),
		_Right.begin(), _Right.end()));
	}

template<class _Type,
	class _Alloc> inline
	bool operator>(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test if _Left > _Right for vectors
	return (_Right < _Left);
	}

template<class _Type,
	class _Alloc> inline
	bool operator<=(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test if _Left <= _Right for vectors
	return (!(_Right < _Left));
	}

template<class _Type,
	class _Alloc> inline
	bool operator>=(const vector12<_Type, _Alloc>& _Left,
		const vector12<_Type, _Alloc>& _Right)
	{	// test if _Left >= _Right for vectors
	return (!(_Left < _Right));
	}

template<class _Type,
	class _Alloc> inline
	void swap(vector12<_Type, _Alloc>& _Left, vector12<_Type, _Alloc>& _Right)
	{	// swap _Left and _Right vectors
	_Left.swap(_Right);
	}

#if _HAS_TRADITIONAL_STL
 typedef _Bvector bit_vector;
 #define __vector__	vector12
#endif /* _HAS_TRADITIONAL_STL */

_STD_END

#ifdef _MSC_VER
 #pragma warning(default: 4244)
 #pragma warning(pop)
 #pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _VECTOR12_ */
