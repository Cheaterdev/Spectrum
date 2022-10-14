// Copyright (c) 2015 Jakob Riedle (DuffsDevice)
// 
// Version 1.0
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

export module shared_ptr;

export import stl.core;
export import stl.memory;

import serialization;
export namespace Core
{
//	template<class T>
//	using shared_ptr = std::shared_ptr<T>;
//	template<class T>
//	using weak_ptr = std::weak_ptr<T>;
//
//	template<class T>
//	using enable_shared_from_this = std::enable_shared_from_this<T>;
//
//
//	template<class T>
//	using static_pointer_cast = std::static_pointer_cast<T>;
//
//
//	template<class T>
//	using dynamic_pointer_cast = std::dynamic_pointer_cast<T>;

//
//namespace detail
//{
//	struct reference_count
//	{
//		typedef uint16_t	size_type;
//		typedef int16_t		difference_type;
//
//		difference_type	shared_count;
//		size_type		pointer_count;
//
//		reference_count() :
//			shared_count(1)
//			, pointer_count(1)
//		{}
//
//		void add_weak() {
//			pointer_count++;
//		}
//		void add_shared() {
//			pointer_count++;
//			shared_count += shared_count > 0 ? 1 : -1;
//		}
//		// Call only, when the pointer shall not be deleted by the shared_ptr
//		void release_shared(reference_count*& pointerToMe) {
//			shared_count--;
//			pointerToMe = nullptr;
//			if (!--pointer_count)
//				delete this;
//		}
//		template<typename T>
//		void release_shared(reference_count*& pointerToMe, T*& value)
//		{
//			pointerToMe = nullptr;
//			if (shared_count < 0)
//				shared_count++;
//			else if (!--shared_count)
//			{
//				// Indicate, the destructor was called
//				shared_count = -1;
//
//				// Delete data
//				delete value;
//
//				// Set Value to zero, in case the destructor stored
//				// the pointer inside an outside, that is persisting sharedPtr
//				value = nullptr;
//
//				// Inform all weak ptrs, this reference_count is
//				// not used by any sharedPtr's anymore
//				shared_count = 0;
//
//				if (!--pointer_count)
//					delete this;
//			}
//		}
//		void release_weak(reference_count*& pointerToMe) {
//			pointerToMe = nullptr;
//			if (!--pointer_count)
//				delete this;
//		}
//
//		// No Copying or moving!
//		reference_count(reference_count&&) = delete;
//		reference_count(const reference_count&) = delete;
//		reference_count& operator=(reference_count&&) = delete;
//		reference_count& operator=(const reference_count&) = delete;
//
//		bool operator==(const reference_count& r) const = default;
//		auto operator<=>(const  reference_count& r)  const = default;
//	};
//}
//
//template<class T> class shared_ptr;
//template<class T> class weak_ptr;
//template<class T> class enable_shared_from_this;
//
//template<typename T>
//class shared_ptr
//{
//	template<typename T1, typename T2>
//	friend shared_ptr<T1> static_pointer_cast(const shared_ptr<T2>&);
//
//	template<typename T1>
//	friend shared_ptr<T1> const_pointer_cast(const shared_ptr<const T1>&);
//
//	template<typename T1, typename T2>
//	friend shared_ptr<T1> dynamic_pointer_cast(const shared_ptr<T2>&);
//
//	template<typename>
//	friend class weak_ptr;
//
//	template<typename>
//	friend class shared_ptr;
//
//	template<typename>
//	friend class enable_shared_from_this;
//
//public:
//
//	typedef T element_type;
//
//	//! Default Contructor
//	shared_ptr() : rc(nullptr), ptr(nullptr)
//	{}
//
//	//! Ctor from nullptr
//	shared_ptr(std::nullptr_t) : rc(nullptr), ptr(nullptr)
//	{}
//
//	//! Ctor from Pointer
//	template<typename T1>
//	explicit shared_ptr(T1*&& ptr) : rc(nullptr) {
//		prepare_this(ptr, ptr);
//	}
//
//	//! Ctor from Weak
//	explicit shared_ptr(const weak_ptr<T>& weak) : rc(weak.rc), ptr(weak.ptr) {
//		if (rc)
//			rc->add_shared();
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	explicit shared_ptr(const weak_ptr<T1>& weak) : rc(weak.rc), ptr(weak.ptr) {
//		if (rc)
//			rc->add_shared();
//	}
//
//	//! Copy Ctor
//	shared_ptr(const shared_ptr& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_shared();
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	shared_ptr(const shared_ptr<T1>& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_shared();
//	}
//
//	//! Move Ctor
//	shared_ptr(shared_ptr&& other) : rc(other.rc), ptr(other.ptr) {
//		other.rc = nullptr;
//		other.ptr = nullptr;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	shared_ptr(shared_ptr<T1>&& other) : rc(other.rc), ptr(other.ptr) {
//		other.rc = nullptr;
//		other.ptr = nullptr;
//	}
//
//	//! Move Assign
//	shared_ptr& operator=(shared_ptr&& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		other.rc = nullptr;
//		other.ptr = nullptr;
//		return *this;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	shared_ptr& operator=(shared_ptr<T1>&& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		other.rc = nullptr;
//		other.ptr = nullptr;
//		return *this;
//	}
//
//	bool operator==(const shared_ptr& r) const = default;
//	auto operator<=>(const  shared_ptr& r)  const = default;
//
//	//! Copy Assign
//	shared_ptr& operator=(const shared_ptr& other) {
//		reset();
//		ptr = other.ptr;
//		rc = other.rc;
//		if (rc)
//			rc->add_shared();
//		return *this;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	shared_ptr& operator=(const shared_ptr<T1>& other) {
//		reset();
//		ptr = other.ptr;
//		rc = other.rc;
//		if (rc)
//			rc->add_shared();
//		return *this;
//	}
//
//	//! Pointer Assign
//	template<typename T1>
//	shared_ptr& operator=(T1*&& ptr) {
//		return *this = shared_ptr(std::move(ptr));
//	}
//
//	//! Nullpointer Assign
//	shared_ptr& operator=(std::nullptr_t) {
//		reset();
//		return *this;
//	}
//
//	//! Reset contained data
//	template<typename T1>
//	void reset(T1*&& ptr) {
//		reset();
//		*this = shared_ptr(std::move(ptr));
//	}
//	void reset(std::nullptr_t = nullptr) {
//		if (rc)
//			rc->release_shared(rc, ptr);
//	}
//
//	//! Get Contained Pointer
//	T* get() const { return ptr; }
//
//	//! Dereference Operators
//	T* operator->() const { return ptr; }
//	T& operator*() const { return *ptr; }
//
//	//! Get number of users on the ptr
//	int use_count() const {
//		return rc ? rc->shared_count : 0;
//	}
//
//	//! Sorting sharedPtr's depending on their internal reference_count Object
//	template<typename T1>
//	bool owner_before(const shared_ptr<T1>& other) const {
//		return other.rc < rc;
//	}
//	template<typename T1>
//	bool owner_before(const weak_ptr<T1>& other) const;
//
//	//! Tasting sharedPtr's on equality depending on their internal reference_count Object
//	template<typename T1>
//	bool owner_equal(const shared_ptr<T1>& other) const {
//		return other.rc == rc;
//	}
//	template<typename T1>
//	bool owner_equal(const weak_ptr<T1>& other) const;
//
//	//! Check if the contained pointer is not null
//	explicit operator bool() const { return ptr; }
//
//	//! Check if the contained pointer is only
//	//! being managed by this sharedPtr instance
//	bool unique() {
//		return rc && rc->shared_count == 1;
//	}
//
//	//! Swap Function
//	void swap(shared_ptr& other) {
//		std::swap(rc, other.rc);
//		std::swap(ptr, other.ptr);
//	}
//
//	//! Releases the contained object, if this sharedPtr is the only instance holding the data
//	T* release() {
//		if (!rc || rc->shared_count != 1)
//			return nullptr;
//		rc->release_shared(rc);
//		T* tmp = ptr;
//		ptr = nullptr;
//		return tmp;
//	}
//
//	//! Destructor
//	~shared_ptr() {
//		reset();
//	}
//
//private:
//
//	detail::reference_count* rc;
//	T* ptr;
//
//	// Ctor from Reference count object & pointer
//	shared_ptr(detail::reference_count* rc, T* ptr) :
//		rc(rc)
//		, ptr(ptr)
//	{
//		if (rc)
//			rc->add_shared();
//	}
//
//	template<typename T1, typename T2>
//	void prepare_this(T1* value, enable_shared_from_this<T2>* sft) {
//		rc = sft->get_ref_count();
//		ptr = static_cast<T*>(value);
//	}
//
//	template<typename T1>
//	void prepare_this(T1* value, ...) {
//		rc = new detail::reference_count();
//		ptr = value;
//	}
//
//	SERIALIZE()
//	{
//
//	}
//};
//
////! Pointer Casts
//template<typename T1, typename T2>
//shared_ptr<T1> static_pointer_cast(const shared_ptr<T2>& other) {
//	return shared_ptr<T1>(other.rc, static_cast<T1*>(other.ptr));
//}
//template<typename T1>
//shared_ptr<T1> const_pointer_cast(const shared_ptr<const T1>& other) {
//	return shared_ptr<T1>(other.rc, const_cast<T1*>(other.ptr));
//}
//template<typename T1, typename T2>
//shared_ptr<T1> dynamic_pointer_cast(const shared_ptr<T2>& other) {
//
//	if (auto p = dynamic_cast<typename std::shared_ptr<T1>::element_type*>(other.ptr)) {
//		return shared_ptr<T1>(other.rc, p);
//	}
//	else {
//		return std::shared_ptr<T1>{};
//	}
//}
//
////! Make Shared
//template<typename T1, typename... T2, typename = typename std::enable_if<!std::is_array<T1>::value>::type>
//shared_ptr<T1> make_shared(T2&&... args) {
//	return shared_ptr<T1>(new T1(std::forward<T2>(args)...));
//}
//
////! Pointer Comparisons with themselves, with raw pointers, and with nullptr_t's
//template<typename T, typename T2>
//inline bool operator==(const shared_ptr<T>& left, const shared_ptr<T2>& right) {
//	return left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator!=(const shared_ptr<T>& left, const shared_ptr<T2>& right) {
//	return !left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator==(const shared_ptr<T>& left, T2* right) {
//	return left.get() == right;
//}
//template<typename T, typename T2>
//inline bool operator==(T* left, const shared_ptr<T2>& right) {
//	return right.get() == left;
//}
//template<typename T, typename T2>
//inline bool operator!=(const shared_ptr<T>& left, T2* right) {
//	return left.get() != right;
//}
//template<typename T, typename T2>
//inline bool operator!=(T* left, const shared_ptr<T2>& right) {
//	return right.get() != left;
//}
//template<typename T>
//inline bool operator==(const shared_ptr<T>& left, std::nullptr_t) {
//	return left.get() == nullptr;
//}
//template<typename T, typename T2>
//inline bool operator==(std::nullptr_t, const shared_ptr<T2>& right) {
//	return right.get() == nullptr;
//}
//template<typename T>
//inline bool operator!=(const shared_ptr<T>& left, std::nullptr_t) {
//	return left.get() != nullptr;
//}
//template<typename T>
//inline bool operator!=(std::nullptr_t, const shared_ptr<T>& right) {
//	return right.get() != nullptr;
//}
//
////! Swap two sharedPtrs
//template<typename T>
//inline void swap(shared_ptr<T>& left, shared_ptr<T>& right) {
//	left.swap(right);
//}
//
//
////! The weakPtr class
//template<typename T>
//class weak_ptr
//{
//	template<typename>
//	friend class weak_ptr;
//
//private:
//
//	detail::reference_count* rc;
//	T* ptr;
//
//public:
//
//	//! Default Ctor
//	weak_ptr(std::nullptr_t = nullptr) :
//		rc(nullptr)
//		, ptr(nullptr)
//	{}
//
//	//! Ctor from shared_ptr
//	weak_ptr(const shared_ptr<T>& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_weak();
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr(const shared_ptr<T1>& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_weak();
//	}
//
//	//! Copy Ctor
//	weak_ptr(const weak_ptr& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_weak();
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr(const weak_ptr<T1>& other) : rc(other.rc), ptr(other.ptr) {
//		if (rc)
//			rc->add_weak();
//	}
//
//	//! Move Ctor
//	weak_ptr(weak_ptr&& other) : rc(other.rc), ptr(other.ptr) {
//		other.rc = nullptr;
//		other.ptr = nullptr;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr(weak_ptr<T1>&& other) : rc(other.rc), ptr(other.ptr) {
//		other.rc = nullptr;
//		other.ptr = nullptr;
//	}
//
//	//! Copy Assign
//	weak_ptr& operator=(const weak_ptr& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		if (rc)
//			rc->add_weak();
//		return *this;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr& operator=(const weak_ptr<T1>& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		if (rc)
//			rc->add_weak();
//		return *this;
//	}
//
//	//! Move Assign
//	weak_ptr& operator=(weak_ptr&& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		other.rc = nullptr;
//		other.ptr = nullptr;
//		return *this;
//	}
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr& operator=(weak_ptr<T1>&& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		other.rc = nullptr;
//		other.ptr = nullptr;
//		return *this;
//	}
//
//	//! Assign from shared_ptr
//	template<typename T1, typename = typename std::enable_if<std::is_convertible<T1*, T*>::value>::type>
//	weak_ptr& operator=(const shared_ptr<T1>& other) {
//		reset();
//		rc = other.rc;
//		ptr = other.ptr;
//		if (rc)
//			rc->add_weak();
//		return *this;
//	}
//
//	//! Lock the weakPtr to try to get a valid pointer from it
//	shared_ptr<T> lock() const {
//		if (rc && rc->shared_count)
//			return shared_ptr<T>(rc, ptr);
//		return shared_ptr<T>();
//	}
//
//	//! Reset contained data
//	void reset(std::nullptr_t = nullptr) {
//		if (rc)
//			rc->release_weak(rc);
//	}
//
//	//! Swap Function
//	void swap(weak_ptr& other) {
//		std::swap(rc, other.rc);
//		std::swap(ptr, other.ptr);
//	}
//
//	//! Get number users on the ptr
//	int use_count() const {
//		return rc ? rc->shared_count : 0;
//	}
//
//	//! Check if the weakPtr has expired, that the old object has been destroyed
//	bool expired() const { return !rc || !rc->shared_count; }
//
//	//! Sorting weakPtr's depending on their internal reference_count Object
//	template<typename T1>
//	bool owner_before(const weak_ptr<T1>& other) const {
//		return other.rc < rc;
//	}
//	template<typename T1>
//	bool owner_before(const shared_ptr<T1>& other) const {
//		return other.rc < rc;
//	}
//
//	//! Testing weakPtr's on equality depending on their internal reference_count Object
//	template<typename T1>
//	bool owner_equal(const weak_ptr<T1>& other) const {
//		return other.rc == rc;
//	}
//	template<typename T1>
//	bool owner_equal(const shared_ptr<T1>& other) const {
//		return other.rc == rc;
//	}
//
//	//! Check if the contained pointer is not null
//	explicit operator bool() const { return !expired() && ptr; }
//
//	//! Dtor
//	~weak_ptr() {
//		reset();
//	}
//};
//
////! Pointer Comparisons with themselves, with raw pointers and sharedPtrs, and with nullptr_t's
//template<typename T, typename T2>
//inline bool operator==(const weak_ptr<T>& left, const weak_ptr<T2>& right) {
//	return left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator!=(const weak_ptr<T>& left, const weak_ptr<T2>& right) {
//	return !left.owner_equal(right);
//}
//template<typename T>
//inline bool operator==(const weak_ptr<T>& left, std::nullptr_t) {
//	return left.expired();
//}
//template<typename T>
//inline bool operator==(std::nullptr_t, const weak_ptr<T>& right) {
//	return right.expired();
//}
//template<typename T>
//inline bool operator!=(const weak_ptr<T>& left, std::nullptr_t) {
//	return !left.expired();
//}
//// weak <-> shared
//template<typename T, typename T2>
//inline bool operator!=(const shared_ptr<T>& left, const weak_ptr<T2>& right) {
//	return !left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator==(const weak_ptr<T>& left, const shared_ptr<T2>& right) {
//	return left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator==(const shared_ptr<T>& left, const weak_ptr<T2>& right) {
//	return !left.owner_equal(right);
//}
//template<typename T, typename T2>
//inline bool operator!=(const weak_ptr<T>& left, const shared_ptr<T2>& right) {
//	return left.owner_equal(right);
//}
//
////! Swap two sharedPtrs
//template<typename T>
//inline void swap(weak_ptr<T>& left, weak_ptr<T>& right) {
//	left.swap(right);
//}
//
//
//
//template<typename T>
//class enable_shared_from_this
//{
//	template<typename T1>
//	friend class shared_ptr;
//
//private:
//
//	bool								owned;
//	mutable detail::reference_count* rc;
//
//	void init_shared_this_once() const {
//		if (!rc)
//			rc = new detail::reference_count();
//	}
//
//	// Moves the contained sharedPtr out of the enableSharedFromThis class
//	detail::reference_count* get_ref_count() {
//		init_shared_this_once();
//		if (owned)
//			rc->add_shared(); // Indicate, we're not moving but creating a new shared_ptr
//		else
//			owned = true;
//		return rc;
//	}
//
//public:
//
//	//! Default Ctor
//	enable_shared_from_this() :
//		owned(false)
//		, rc(nullptr)
//	{}
//
//	//! Returns a shared_ptr to the owning the current object
//	shared_ptr<const T> shared() const {
//		init_shared_this_once();
//		return shared_ptr<const T>(rc, static_cast<const T*>(this));
//	}
//	shared_ptr<T> shared() {
//		init_shared_this_once();
//		return shared_ptr<T>(rc, static_cast<T*>(this));
//	}
//
//	//! Dtor
//	~enable_shared_from_this() {
//		if (!owned)
//			delete rc;
//	}
//};
//
//
////! Pointer Comparison
//template<typename T, typename U>
//struct generic_owner_less //: public std::binary_function<T, T, bool>
//{
//	bool operator()(const T& lhs, const T& rhs) const {
//		return lhs.owner_before(rhs);
//	}
//	bool operator()(const T& lhs, const U& rhs) const {
//		return lhs.owner_before(rhs);
//	}
//	bool operator()(const U& lhs, const T& rhs) const {
//		return lhs.owner_before(rhs);
//	}
//};
//
//// Extern function declarations that were impossible
//// inside the class, because weak_ptr was not declared yet
//template<typename T> template<typename T1>
//bool shared_ptr<T>::owner_before(const weak_ptr<T1>& other) const {
//	return other.rc < rc;
//}
//template<typename T> template<typename T1>
//bool shared_ptr<T>::owner_equal(const weak_ptr<T1>& other) const {
//	return other.rc == rc;
//}
//
//// std::owner_less overload
//template<typename T> struct owner_less;
//
//template<typename T>
//struct owner_less<shared_ptr<T>> : public generic_owner_less<shared_ptr<T>, weak_ptr<T>>
//{};
//
//template<typename T>
//struct owner_less<weak_ptr<T>> : public generic_owner_less<weak_ptr<T>, shared_ptr<T>>
//{};


}