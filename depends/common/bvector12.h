////////////////////////////
#pragma once
#ifndef _BVECTOR12_
#define _BVECTOR12_
#ifndef RC_INVOKED
#include "vector12.h"
#include <memory>
#include <stdexcept>

#ifdef _MSC_VER
 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)
 #pragma warning(disable: 4244)
#endif  /* _MSC_VER */

_STD_BEGIN

typedef unsigned _V12base;	// word type for vector12<bool> representation
const int _V12BITS = 8 * sizeof (_V12base);	// at least CHAR_BITS bits per word

		// CLASS _Vb12_iter_base
template<class _Sizet,
	class _Difft,
	class _MycontTy>
	class _Vb12_iter_base
		: public _Ranit<_Bool, _Difft, bool *, bool>
	{	// store information common to reference and iterators
public:
//#if _SECURE_SCL
//	typedef _Range_checked_iterator_tag _Checked_iterator_category;
//#endif

	_Vb12_iter_base()
		: _Myptr(0), _Myoff(0)
		{	// construct with null pointer
		}

 #if _HAS_ITERATOR_DEBUGGING
	_Vb12_iter_base(_V12base *_Ptr, _Sizet _Off,
		const _Container_base *_Mypvbool)
		: _Myptr(_Ptr), _Myoff(_Off)
		{	// construct with offset and pointer
		//_SCL_SECURE_VALIDATE(_Mypvbool != NULL);
		this->_Adopt(_Mypvbool);
		}

 //#elif _SECURE_SCL
	//_Vb12_iter_base(_V12base *_Ptr, _Sizet _Off,
	//	const _Container_base *_Mypvbool)
	//	: _Myptr(_Ptr), _Myoff(_Off)
	//	{	// construct with offset and pointer
	//	_SCL_SECURE_VALIDATE(_Mypvbool != NULL);
	//	this->_Set_container(_Mypvbool);
	//	}
 #else
	_Vb12_iter_base(_V12base *_Ptr, _Sizet _Off)
		: _Myptr(_Ptr), _Myoff(_Off)
		{	// construct with offset and pointer
		}
 #endif

	_V12base *_Myptr;
	_Sizet _Myoff;

	static void _Xlen()
		{	// report a length_error
		_THROW(length_error, "vector12<bool> too long");
			}

	static void _Xran()
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid vector12<bool> subscript");
		}

	static void _Xinvarg()
		{	// report an invalid_argument error
		_THROW(invalid_argument, "invalid vector12<bool> argument");
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//_V12base * _My_cont_begin() const
	//	{
	//		return _VEC_ITER_BASE(((_MycontTy *)this->_Getmycont())->_Myvec.begin());
	//	}

	//_Sizet _My_actual_offset() const
	//	{
	//	_Sizet _Off = this->_Myoff;
	//	_Off += _V12BITS * (this->_Myptr - _My_cont_begin());
	//	return _Off;
	//	}
 //#endif
	};

		// CLASS _Vb12_reference
template<class _Sizet,
	class _Difft,
	class _MycontTy>
	class _Vb12_reference
		: public _Vb12_iter_base<_Sizet, _Difft, _MycontTy>
	{	// reference to a bit within a base word
public:
	typedef _Vb12_iter_base<_Sizet, _Difft, _MycontTy> _Mybase;
	typedef _Vb12_reference<_Sizet, _Difft, _MycontTy> _Mytype;

	_Vb12_reference()
		{	// construct with null pointer
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//_Vb12_reference(const _Mybase& _Right)
	//	: _Mybase(_Right._Myptr, _Right._Myoff, _Right._Getmycont())
	//	{	// construct with base
	//	}

 //#else /* _HAS_ITERATOR_DEBUGGING */
	_Vb12_reference(const _Mybase& _Right)
		: _Mybase(_Right._Myptr, _Right._Myoff)
		{	// construct with base
		}
 //#endif /* _HAS_ITERATOR_DEBUGGING */

	_Mytype& operator=(const _Mytype& _Right)
		{	// assign _Vb12_reference _Right to bit
		return (*this = bool(_Right));
		}

	_Mytype& operator=(bool _Val)
		{	// assign _Val to bit
		if (_Val)
			*_Getptr() |= _Mask();
		else
			*_Getptr() &= ~_Mask();
		return (*this);
		}

	void flip()
		{	// toggle the bit
		*_Getptr() ^= _Mask();
		}

	bool operator~() const
		{	// test if bit is reset
		return (!bool(*this));
		}

	operator bool() const
		{	// test if bit is set
		return ((*_Getptr() & _Mask()) != 0);
		}

	_V12base *_Getptr() const
		{	// get pointer to base word

 #if _HAS_ITERATOR_DEBUGGING
		if (this->_Mycont == 0 || this->_Myptr == 0)
			{
			_DEBUG_ERROR("vector12<bool> iterator not dereferencable");
			_SCL_SECURE_OUT_OF_RANGE;
			}
 //#else /* _HAS_ITERATOR_DEBUGGING */
 //		_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Myptr != NULL);
	//	_SCL_SECURE_VALIDATE_RANGE(this->_My_actual_offset() < ((_MycontTy *)this->_Getmycont())->_Mysize);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (this->_Myptr);
		}

protected:
	_V12base _Mask() const
		{	// convert offset to mask
		return ((_V12base)(1 << this->_Myoff));
		}
	};

template<class _Sizet,
	class _Difft,
	class _MycontTy>
	void swap(_Vb12_reference<_Sizet, _Difft, _MycontTy> _Left,
		_Vb12_reference<_Sizet, _Difft, _MycontTy> _Right)
	{	// swap _Left and _Right vector12<bool> elements
	bool _Val = _Left;
	_Left = _Right;
	_Right = _Val;
	}

		// CLASS _Vb12_const_iterator
template<class _Sizet,
	class _Difft,
	class _MycontTy>
	class _Vb12_const_iterator
		: public _Vb12_iter_base<_Sizet, _Difft, _MycontTy>
	{	// iterator for nonmutable vector12<bool>
public:
	typedef _Vb12_iter_base<_Sizet, _Difft, _MycontTy> _Mybase;
	typedef _Vb12_const_iterator<_Sizet, _Difft, _MycontTy> _Mytype;

	typedef _Vb12_reference<_Sizet, _Difft, _MycontTy> _Reft;
	typedef bool const_reference;

	typedef random_access_iterator_tag iterator_category;
	typedef _Bool value_type;
	typedef _Sizet size_type;
	typedef _Difft difference_type;
	typedef const_reference *pointer;
	typedef const_reference reference;

	_Vb12_const_iterator()
		{	// construct with null reference
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//_Vb12_const_iterator(const _V12base *_Ptr, const _Container_base *_Mypvbool)
	//	: _Mybase((_V12base *)_Ptr, 0, (_Container_base *)_Mypvbool)

 //#else
	_Vb12_const_iterator(const _V12base *_Ptr)
		: _Mybase((_V12base *)_Ptr, 0)

 //#endif
		{	// construct with offset and pointer
		}

	const_reference operator*() const
		{	// return (reference to) designated object
		return (_Reft(*this));
		}

	_Mytype& operator++()
		{	// preincrement
		_Inc();
		return (*this);
		}

	_Mytype operator++(int)
		{	// postincrement
		_Mytype _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_Mytype& operator--()
		{	// predecrement
		_Dec();
		return (*this);
		}

	_Mytype operator--(int)
		{	// postdecrement
		_Mytype _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_Mytype& operator+=(difference_type _Off)
		{	// increment by integer
		if (_Off == 0)
			return (*this); // early out
		//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Myptr != NULL);
		if (_Off < 0)
			{
			//_SCL_SECURE_VALIDATE_RANGE(this->_My_actual_offset() >= ((size_type)-_Off));
			}
		else
			{
			//_SCL_SECURE_VALIDATE_RANGE((this->_My_actual_offset() + _Off) <= ((_MycontTy *)this->_Getmycont())->_Mysize);
			}
		if (_Off < 0 && this->_Myoff < 0 - (size_type)_Off)
			{	/* add negative increment */
			this->_Myoff += _Off;
			this->_Myptr -= 1 + ((size_type)(-1) - this->_Myoff) / _V12BITS;
			this->_Myoff %= _V12BITS;
			}
		else
			{	/* add non-negative increment */
			this->_Myoff += _Off;
			this->_Myptr += this->_Myoff / _V12BITS;
			this->_Myoff %= _V12BITS;
			}
		return (*this);
		}

	_Mytype operator+(difference_type _Off) const
		{	// return this + integer
		_Mytype _Tmp = *this;
		return (_Tmp += _Off);
		}

	_Mytype& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_Mytype operator-(difference_type _Off) const
		{	// return this - integer
		_Mytype _Tmp = *this;
		return (_Tmp -= _Off);
		}

	difference_type operator-(
		const _Mytype& _Right) const
		{	// return difference of iterators

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (_V12BITS * (this->_Myptr - _Right._Myptr)
			+ (difference_type)this->_Myoff
			- (difference_type)_Right._Myoff);
		}

	const_reference operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}

	bool operator==(const _Mytype& _Right) const
		{	// test for iterator equality

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (this->_Myptr == _Right._Myptr
			&& this->_Myoff == _Right._Myoff);
		}

	bool operator!=(const _Mytype& _Right) const
		{	// test for iterator inequality
		return (!(*this == _Right));
		}

	bool operator<(const _Mytype& _Right) const
		{	// test if this < _Right

 #if _HAS_ITERATOR_DEBUGGING
		_Compat(_Right);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (this->_Myptr < _Right._Myptr
			|| this->_Myptr == _Right._Myptr
				&& this->_Myoff < _Right._Myoff);
		}

	bool operator>(const _Mytype& _Right) const
		{	// test if this > _Right
		return (_Right < *this);
		}

	bool operator<=(const _Mytype& _Right) const
		{	// test if this <= _Right
		return (!(_Right < *this));
		}

	bool operator>=(const _Mytype& _Right) const
		{	// test if this >= _Right
		return (!(*this < _Right));
		}

protected:

 #if _HAS_ITERATOR_DEBUGGING
	void _Compat(const _Mytype& _Right) const
		{	// test for compatible iterator pair
		if (this->_Mycont == 0 || this->_Mycont != _Right._Mycont)
			_DEBUG_ERROR("vector12<bool> iterators incompatible");
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	void _Dec()
		{	// decrement bit position
		if (this->_Myoff != 0)
			{
			--this->_Myoff;
			}
		else
			{
			//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Myptr != NULL);
			//_SCL_SECURE_VALIDATE_RANGE(this->_Myptr > this->_My_cont_begin());
			--this->_Myptr;
			this->_Myoff = _V12BITS - 1;
			}
		}

	void _Inc()
		{	// increment bit position
		//_SCL_SECURE_VALIDATE(this->_Has_container() && this->_Myptr != NULL);
		//_SCL_SECURE_VALIDATE_RANGE((this->_My_actual_offset() + 1) <= ((_MycontTy *)this->_Getmycont())->_Mysize);
		if (this->_Myoff < _V12BITS - 1)
			++this->_Myoff;
		else
			this->_Myoff = 0, ++this->_Myptr;
		}
	};

template<class _Sizet,
	class _Difft,
	class _MycontTy>
	_Vb12_const_iterator<_Sizet, _Difft, _MycontTy> operator+(_Difft _Off,
		_Vb12_const_iterator<_Sizet, _Difft, _MycontTy> _Right)
		{	// return _Right + integer
		return (_Right += _Off);
		}

	// CLASS _Vb12_iterator
template<class _Sizet,
	class _Difft,
	class _MycontTy>
	class _Vb12_iterator
		: public _Vb12_const_iterator<_Sizet, _Difft, _MycontTy>
	{	// iterator for mutable vector12<bool>
public:
	typedef _Vb12_const_iterator<_Sizet, _Difft, _MycontTy> _Mybase;
	typedef _Vb12_iterator<_Sizet, _Difft, _MycontTy> _Mytype;

	typedef _Vb12_reference<_Sizet, _Difft, _MycontTy> _Reft;
	typedef bool const_reference;

	typedef random_access_iterator_tag iterator_category;
	typedef _Bool value_type;
	typedef _Sizet size_type;
	typedef _Difft difference_type;
	typedef _Reft *pointer;
	typedef _Reft reference;

	_Vb12_iterator()
		{	// construct with null reference
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//_Vb12_iterator(_V12base *_Ptr, _Container_base *_Mypvbool)
	//	: _Mybase(_Ptr, _Mypvbool)

 //#else
	_Vb12_iterator( _V12base *_Ptr)
		: _Mybase(_Ptr)
 //#endif /* _HAS_ITERATOR_DEBUGGING */

		{	// construct with offset and pointer
		}

	reference operator*() const
		{	// return (reference to) designated object
		return (_Reft(*this));
		}

	_Mytype& operator++()
		{	// preincrement
		++*(_Mybase *)this;
		return (*this);
		}

	_Mytype operator++(int)
		{	// postincrement
		_Mytype _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	_Mytype& operator--()
		{	// predecrement
		--*(_Mybase *)this;
		return (*this);
		}

	_Mytype operator--(int)
		{	// postdecrement
		_Mytype _Tmp = *this;
		--*this;
		return (_Tmp);
		}

	_Mytype& operator+=(difference_type _Off)
		{	// increment by integer
		*(_Mybase *)this += _Off;
		return (*this);
		}

	_Mytype operator+(difference_type _Off) const
		{	// return this + integer
		_Mytype _Tmp = *this;
		return (_Tmp += _Off);
		}

	_Mytype& operator-=(difference_type _Off)
		{	// decrement by integer
		return (*this += -_Off);
		}

	_Mytype operator-(difference_type _Off) const
		{	// return this - integer
		_Mytype _Tmp = *this;
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

template<class _Sizet,
	class _Difft,
	class _MycontTy>
	_Vb12_iterator<_Sizet, _Difft, _MycontTy> operator+(_Difft _Off,
		_Vb12_iterator<_Sizet, _Difft, _MycontTy> _Right)
		{	// return _Right + integer
		return (_Right += _Off);
		}

		// CLASS vector_bool
template<class _Alloc>
	class vector12<_Bool, _Alloc>
		: public _Container_base_aux_alloc_empty_no_alloc<_Alloc>
	{	// varying size array of bits
public:
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type _Dift;
	typedef std::vector12<_V12base,
		typename _Alloc::template rebind<_V12base>::other>
			_Vbtype;
	typedef std::vector12<_Bool, _Alloc> _MyType;


	typedef _Dift difference_type;
	typedef _Bool _Type;
	typedef _Alloc allocator_type;

	typedef _Vb12_reference<size_type, _Dift, _MyType> reference;
	typedef bool const_reference;
	typedef bool value_type;

	typedef reference _Reft;
	typedef _Vb12_const_iterator<size_type, difference_type, _MyType> const_iterator;
	typedef _Vb12_iterator<size_type, difference_type, _MyType> iterator;

	friend class _Vb12_iter_base<size_type, difference_type, _MyType>;
	friend class _Vb12_reference<size_type, difference_type, _MyType>;
	friend class _Vb12_const_iterator<size_type, difference_type, _MyType>;
	friend class _Vb12_iterator<size_type, difference_type, _MyType>;

	typedef iterator pointer;
	typedef const_iterator const_pointer;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	static const int _V12BITS = std::_V12BITS;

	vector12()
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Alloc()), _Mysize(0), _Myvec()
		{	// construct empty vector12
		}

	vector12(const _MyType& _Right)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Right.get_allocator()), _Mysize(_Right._Mysize), _Myvec(_Right._Myvec)
		{	// copy construct vector12; an implicitly defined copy constructor would not create an aux object.
		}

	explicit vector12(const _Alloc& _Al)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Al), _Mysize(0), _Myvec(_Al)
		{	// construct empty vector12, with allocator
		}

	explicit vector12(size_type _Count, bool _Val = false)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Alloc()), _Mysize(0), _Myvec(_Nw(_Count), (_V12base)(_Val ? -1 : 0))
		{	// construct from _Count * _Val
		_Trim(_Count);
		}

	vector12(size_type _Count, bool _Val, const _Alloc& _Al)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Al), _Mysize(0), _Myvec(_Nw(_Count), (_V12base)(_Val ? -1 : 0), _Al)
		{	// construct from _Count * _Val, with allocator
		_Trim(_Count);
		}

	template<class _Iter>
		vector12(_Iter _First, _Iter _Last)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Alloc()), _Mysize(0), _Myvec()
		{	// construct from [_First, _Last)
		_BConstruct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		vector12(_Iter _First, _Iter _Last, const _Alloc& _Al)
		: _Container_base_aux_alloc_empty_no_alloc<_Alloc>(_Al), _Mysize(0), _Myvec(_Al)
		{	// construct from [_First, _Last), with allocator
		_BConstruct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _BConstruct(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// initialize from _Count * _Val
		size_type _Num = (size_type)_Count;
		_Myvec.assign(_Num, (_Type)_Val ? -1 : 0);
		_Trim(_Num);
		}

	template<class _Iter>
		void _BConstruct(_Iter _First, _Iter _Last, input_iterator_tag)
		{	// initialize from [_First, _Last), input iterators
		insert(begin(), _First, _Last);
		}

	~vector12()
		{	// destroy the object
		_Mysize = 0;
		}

	void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
		_Myvec.reserve(_Nw(_Count));
		}

	size_type capacity() const
		{	// return current length of allocated storage
		return (_Myvec.capacity() * _V12BITS);
		}

 //#if _HAS_ITERATOR_DEBUGGING || _SECURE_SCL
	//iterator begin()
	//	{	// return iterator for beginning of mutable sequence
	//	return (iterator(_VEC_ITER_BASE(_Myvec.begin()), this));
	//	}

	//const_iterator begin() const
	//	{	// return iterator for beginning of nonmutable sequence
	//	return (const_iterator(_VEC_ITER_BASE(_Myvec.begin()), this));
	//	}

 //#else
	iterator begin()
		{	// return iterator for beginning of mutable sequence
		return (iterator(_VEC_ITER_BASE(_Myvec.begin())));
		}

	const_iterator begin() const
		{	// return iterator for beginning of nonmutable sequence
		return (const_iterator(_VEC_ITER_BASE(_Myvec.begin())));
		}
 //#endif

	iterator end()
		{	// return iterator for end of mutable sequence
		iterator _Tmp = begin();
		if (0 < _Mysize)
			_Tmp += _Mysize;
		return (_Tmp);
		}

	const_iterator end() const
		{	// return iterator for end of nonmutable sequence
		const_iterator _Tmp = begin();
		if (0 < _Mysize)
			_Tmp += _Mysize;
		return (_Tmp);
		}

	iterator _Make_iter(const_iterator _Where)
		{	// make iterator from const_iterator
		iterator _Tmp = begin();
		if (0 < _Mysize)
			_Tmp += _Where - begin();
		return (_Tmp);
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

	void resize(size_type _Newsize, bool _Val = false)
		{	// determine new length, padding with _Val elements as needed
		if (size() < _Newsize)
			_Insert_n(end(), _Newsize - size(), _Val);
		else if (_Newsize < size())
			erase(begin() + _Newsize, end());
		}

	size_type size() const
		{	// return length of sequence
		return (_Mysize);
		}

	size_type max_size() const
		{	// return maximum possible length of sequence
		const size_type _Maxsize = _Myvec.max_size();
		return (_Maxsize < (size_type)(-1) / _V12BITS
			? _Maxsize * _V12BITS : (size_type)(-1));
		}

	bool empty() const
		{	// test if sequence is empty
		return (size() == 0);
		}

	_Alloc get_allocator() const
		{	// return allocator object for values
		// Work around a BE problem.
		_Alloc _Alret = _Myvec.get_allocator();
		return _Alret;
		}

	const_reference at(size_type _Off) const
		{	// subscript nonmutable sequence with checking
		if (size() <= _Off)
			_Xran();
		return (*(begin() + _Off));
		}

	reference at(size_type _Off)
		{	// subscript mutable sequence with checking
		if (size() <= _Off)
			_Xran();
		return (*(begin() + _Off));
		}

	const_reference operator[](size_type _Off) const
		{	// subscript nonmutable sequence
		return (*(begin() + _Off));
		}

	reference operator[](size_type _Off)
		{	// subscript mutable sequence
		return (*(begin() + _Off));
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

	void push_back(bool _Val)
		{	// insert element at end
		insert(end(), _Val);
		}

	void pop_back()
		{	// erase element at end
		if (!empty())
			erase(end() - 1);
		}

	template<class _Iter>
		void assign(_Iter _First, _Iter _Last)
		{	// assign [_First, _Last)
		_Assign(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Assign(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// assign _Count * _Val
		_Assign_n((size_type)_Count, (bool)_Val);
		}

	template<class _Iter>
		void _Assign(_Iter _First, _Iter _Last, input_iterator_tag)
		{	// assign [_First, _Last), input iterators
		erase(begin(), end());
		insert(begin(), _First, _Last);
		}

	void assign(size_type _Count, bool _Val)
		{	// assign _Count * _Val
		_Assign_n(_Count, _Val);
		}

	iterator insert(const_iterator _Where, bool _Val)
		{	// insert _Val at _Where
		size_type _Off = _Where - begin();
		_Insert_n(_Where, (size_type)1, _Val);
		return (begin() + _Off);
		}

	void insert(const_iterator _Where, size_type _Count, bool _Val)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, _Count, _Val);
		}

	template<class _Iter>
		void insert(const_iterator _Where, _Iter _First, _Iter _Last)
		{	// insert [_First, _Last) at _Where
		_Insert(_Where, _First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Insert(const_iterator _Where, _Iter _Count, _Iter _Val,
			_Int_iterator_tag)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, (size_type)_Count, (bool)_Val);
		}

	template<class _Iter>
		void _Insert(const_iterator _Where, _Iter _First, _Iter _Last,
			input_iterator_tag)
		{	// insert [_First, _Last) at _Where, input iterators
		size_type _Off = _Where - begin();

		for (; _First != _Last; ++_First, ++_Off)
			insert(begin() + _Off, *_First);
		}

	template<class _Iter>
		void _Insert(const_iterator _Where,
			_Iter _First, _Iter _Last,
			forward_iterator_tag)
		{	// insert [_First, _Last) at _Where, forward iterators

 #if _HAS_ITERATOR_DEBUGGING
		_DEBUG_RANGE(_First, _Last);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		size_type _Count = 0;
		_Distance(_First, _Last, _Count);

		size_type _Off = _Insert_x(_Where, _Count);
		std::copy(_First, _Last, begin() + _Off);
		}

	iterator erase(const_iterator _Where_arg)
		{	// erase element at _Where
		iterator _Where = _Make_iter(_Where_arg);
		size_type _Off = _Where - begin();

 #if _HAS_ITERATOR_DEBUGGING
		if (end() <= _Where)
			_DEBUG_ERROR("vector12<bool> erase iterator outside range");
		std::copy(_Where + 1, end(), _Where);
		_Orphan_range(_Off, _Mysize);

 #else /* _HAS_ITERATOR_DEBUGGING */
		std::copy(_Where + 1, end(), _Where);
 #endif /* _HAS_ITERATOR_DEBUGGING */

		_Trim(_Mysize - 1);
		return (begin() + _Off);
		}

	iterator erase(const_iterator _First_arg, const_iterator _Last_arg)
		{	// erase [_First, _Last)
		iterator _First = _Make_iter(_First_arg);
		iterator _Last = _Make_iter(_Last_arg);
		size_type _Off = _First - begin();

 #if _HAS_ITERATOR_DEBUGGING
		if (_Last < _First || end() < _Last)
			_DEBUG_ERROR("vector12<bool> erase iterator outside range");
		iterator _Next = std::copy(_Last, end(), _First);
		size_type _Newsize = _Next - begin();
		_Orphan_range(_Newsize, _Mysize);
		_Trim(_Newsize);

 #else /* _HAS_ITERATOR_DEBUGGING */
		iterator _Next = std::copy(_Last, end(), _First);
		_Trim(_Next - begin());
 #endif /* _HAS_ITERATOR_DEBUGGING */

		return (begin() + _Off);
		}

	void clear()
		{	// erase all elements
		erase(begin(), end());
		}

	void flip()
		{	// toggle all elements
		for (_Vbtype::iterator _Next = _Myvec.begin();
			_Next != _Myvec.end(); ++_Next)
			*_Next = (_V12base)~*_Next;
		_Trim(_Mysize);
		}

	void swap(_MyType& _Right)
		{	// exchange contents with right
		if (this != &_Right)
			{	// different, worth swapping

 #if _HAS_ITERATOR_DEBUGGING
			this->_Swap_all(_Right);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			this->_Swap_aux(_Right);

			std::swap(_Mysize, _Right._Mysize);
			_Myvec.swap(_Right._Myvec);
			}
		}



	static void swap(reference _Left, reference _Right)
		{	// swap _Left and _Right vector12<bool> elements
		bool _Val = _Left;

		_Left = _Right;
		_Right = _Val;
		}


protected:
	void _Assign_n(size_type _Count, bool _Val)
		{	// assign _Count * _Val
		erase(begin(), end());
		_Insert_n(begin(), _Count, _Val);
		}

	void _Insert_n(const_iterator _Where,
		size_type _Count, bool _Val)
		{	// insert _Count * _Val at _Where
		size_type _Off = _Insert_x(_Where, _Count);
		std::fill(begin() + _Off, begin() + (_Off + _Count), _Val);
		}

	size_type _Insert_x(const_iterator _Where, size_type _Count)
		{	// make room to insert _Count elements at _Where
		size_type _Off = _Where - begin();

 #if _HAS_ITERATOR_DEBUGGING
		if (end() < _Where)
			_DEBUG_ERROR("vector12<bool> insert iterator outside range");
		bool _Realloc = capacity() - size() < _Count;
 #endif /* _HAS_ITERATOR_DEBUGGING */

		if (_Count == 0)
			;
		else if (max_size() - size() < _Count)
			_Xlen();	// result too long
		else
			{	// worth doing
			_Myvec.resize(_Nw(size() + _Count), 0);
			if (size() == 0)
				_Mysize += _Count;
			else
				{	// make room and copy down suffix
				iterator _Oldend = end();
				_Mysize += _Count;
				std::copy_backward(begin() + _Off, _Oldend, end());
				}

 #if _HAS_ITERATOR_DEBUGGING
			_Orphan_range(_Realloc ? 0 : _Off, _Mysize);
 #endif /* _HAS_ITERATOR_DEBUGGING */

			}
		return (_Off);
		}

	static size_type _Nw(size_type _Count)
		{	// return number of base words from number of bits
		return ((_Count + _V12BITS - 1) / _V12BITS);
		}

 #if _HAS_ITERATOR_DEBUGGING
	void _Orphan_range(size_type _Offlo, size_type _Offhi) const
		{	// orphan iterators within specified (inclusive) range
		typedef _Vb12_iter_base<size_type, difference_type, _MyType> _Myiterbase;

		_Lockit _Lock(_LOCK_DEBUG);
		_V12base *_Base = (_V12base *)_VEC_ITER_BASE(_Myvec.begin());

		_Myiterbase **_Pnext =
			(_Myiterbase **)&this->_Myfirstiter;
		while (*_Pnext != 0)
			{	// test offset from beginning of vector12
			size_type _Off = _V12BITS * ((*_Pnext)->_Myptr - _Base)
				+ (*_Pnext)->_Myoff;
			if (_Off < _Offlo || _Offhi < _Off)
				_Pnext = (_Myiterbase **)&(*_Pnext)->_Mynextiter;
			else
				{	// orphan the iterator
				(*_Pnext)->_Mycont = 0;
				*_Pnext = (_Myiterbase *)(*_Pnext)->_Mynextiter;
				}
			}
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	void _Trim(size_type _Size)
		{	// trim base vector12 to exact length in bits
		if (max_size() < _Size)
			_Xlen();	// result too long
		size_type _Words = _Nw(_Size);

		if (_Words < _Myvec.size())
			_Myvec.erase(_Myvec.begin() + _Words, _Myvec.end());
		_Mysize = _Size;
		_Size %= _V12BITS;
		if (0 < _Size)
			_Myvec[_Words - 1] &= (_V12base)((1 << _Size) - 1);
		}

	void _Xlen() const
		{	// report a length_error
		_THROW(length_error, "vector12<bool> too long");
		}

	void _Xran() const
		{	// throw an out_of_range error
		_THROW(out_of_range, "invalid vector12<bool> subscript");
		}

    //vector container and size are swapped
	_Vbtype _Myvec;	// base vector12 of words
	size_type _Mysize;	// current length of sequence
	};

_STD_END

#ifdef _MSC_VER
 #pragma warning(default: 4244)
 #pragma warning(pop)
 #pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _BVECTOR12_ */
