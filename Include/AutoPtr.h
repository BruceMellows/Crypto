// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#define SPECIALIZED_TEMPLATE_CLASS template<>
#define TYPENAME typename

//////////////////////////////////////////////////////////////////////////
// generic resource handling
template <class Style>
struct ManagedResource
{
	typedef Style						resource_style_t;
	typedef TYPENAME resource_style_t::resource_t		resource_t;
	typedef TYPENAME resource_style_t::const_resource_t	const_resource_t;
	typedef TYPENAME resource_style_t::handle_t		handle_t;
	typedef TYPENAME resource_style_t::type_t		type_t;

protected:
	handle_t	h;

	ManagedResource() :
		h(resource_style_t::EmptyHandle())
	{
	}

	ManagedResource(resource_t r) :
		h(resource_style_t::AcquireResource(r))
	{
	}

	ManagedResource(const ManagedResource& that) :
		h(resource_style_t::EmptyHandle())
	{
		resource_style_t::AcquireHandle(h, that.h);
	}

	ManagedResource(ManagedResource&& that) :
		h(that.h)
	{
		this.h = resource_style_t::EmptyHandle();
	}

	~ManagedResource()
	{
		resource_style_t::ReleaseHandle(h);
	}

	ManagedResource&	operator = (const ManagedResource& that)
	{
		if (this != &that)
		{
			resource_style_t::AcquireHandle(h, that.h);
		}

		return *this;
	}

	void			operatorAssign(resource_t r)
	{
		if (r != GetResource())
		{
			resource_style_t::ReleaseHandle(h);
			h = resource_style_t::AcquireResource(r);
		}
	}

public:
	resource_t		GetResource()
	{
		return resource_style_t::ResourceFromHandle(h);
	}

	const_resource_t	GetResource() const
	{
		return resource_style_t::ConstResourceFromHandle(h);
	}

	const_resource_t	GetConstResource()
	{
		return resource_style_t::ConstResourceFromHandle(h);
	}

	const_resource_t	GetConstResource() const
	{
		return resource_style_t::ConstResourceFromHandle(h);
	}

	handle_t		Release()
	{
		handle_t rc = h;
		h = resource_style_t::EmptyHandle();
		return rc;
	}

	// what about operator bool - damn
	bool			operator! () const
	{
		return (0 == GetConstResource() ? TRUE : FALSE);
	}
};

//////////////////////////////////////////////////////////////////////////
// mixins

// construct/assign from const this&
template <class Base>
struct Copyable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

	Copyable(const Copyable& that) : Base(that)
	{
	}

	Copyable& operator = (const Copyable& that)
	{
		Base::operator=(that);
		return *this;
	}

protected:
	Copyable() {}
	Copyable(resource_t r) : Base(r) {}
};

// CANNOT construct/assign from const this&
template <class Base>
struct NonCopyable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

private:
	NonCopyable(const NonCopyable&);
	NonCopyable& operator = (const NonCopyable&);

protected:
	NonCopyable() {}
	NonCopyable(resource_t r) : Base(r) {}
};

// construct from this&&
template <class Base>
struct Movable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

	Movable(Movable&& that) : Base(that)
	{
	}

protected:
	Movable() {}
	Movable(resource_t r) : Base(r) {}
};

// CANNOT construct from this&&
template <class Base>
struct NonMovable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

private:
	NonMovable(NonMovable&&);

protected:
	NonMovable() {}
	NonMovable(resource_t r) : Base(r) {}
};

// cast to resource_t and const_resource_t
template <class Base>
struct Castable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

	operator	resource_t ()
	{
		return resource_style_t::ResourceFromHandle(h);
	}

	operator	const_resource_t () const
	{
		return resource_style_t::ConstResourceFromHandle(h);
	}

	resource_t
	operator+(int i)
	{
		return resource_style_t::ResourceFromHandle(h) + i;
	}

	const_resource_t
	operator+(int i) const
	{
		return resource_style_t::ConstResourceFromHandle(h) + i;
	}

	operator bool () const
	{
		return resource_style_t::ConstResourceFromHandle(h) != 0;
	}

	bool
	operator!() const
	{
		return resource_style_t::ConstResourceFromHandle(h) == 0;
	}

	bool
	operator==(const_resource_t ptr) const
	{
		return resource_style_t::ConstResourceFromHandle(h) == ptr;
	}

	bool
	operator!=(const_resource_t ptr) const
	{
		return resource_style_t::ConstResourceFromHandle(h) != ptr;
	}

	type_t &operator *()
	{
		return *resource_style_t::ResourceFromHandle(h);
	}

	type_t const &operator *() const
	{
		return *resource_style_t::ConstResourceFromHandle(h);
	}

private:
	// having these operators BREAK short circuit evaluation - not nice
	bool operator&& (bool b) const;
	bool operator|| (bool b) const;

protected:
	Castable() {}
	Castable(resource_t r) : Base(r) {}
};

// operator []
template <class Base>
struct Indexable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

	indexed_t	operator [] (unsigned n)
	{
		return resource_style_t::ResourceFromHandle(h)[n];
	}

protected:
	Indexable() {}
	Indexable(resource_t r) : Base(r) {}
};

// operator ->
template <class Base>
struct Arrowable : public Base
{
	typedef TYPENAME Base::resource_style_t		resource_style_t;
	typedef TYPENAME resource_style_t::indexed_t	indexed_t;
	typedef TYPENAME Base::resource_t		resource_t;
	typedef TYPENAME Base::const_resource_t		const_resource_t;
	typedef TYPENAME Base::type_t			type_t;

	resource_t		operator -> ()
	{
		return resource_style_t::ResourceFromHandle(h);
	}

	const_resource_t	operator -> () const
	{
		return resource_style_t::ConstResourceFromHandle(h);
	}

protected:
	Arrowable() {}
	Arrowable(resource_t r) : Base(r) {}
};

//////////////////////////////////////////////////////////////////////////
// general pointer typedefs and methods
template <class T>
struct PointerResource
{
	typedef T&		indexed_t;
	typedef T*		resource_t;
	typedef const T*	const_resource_t;
	typedef resource_t	handle_t;
	typedef T		type_t;

	static	handle_t		EmptyHandle()
	{
		return 0;
	}

	static	handle_t		AcquireResource(resource_t r)
	{
		return r;
	}

	static	void			AcquireHandle(handle_t& dst, const handle_t& src)
	{
		dst = EmptyHandle();
	}

	static	handle_t		AcquireHandle(handle_t& h)
	{
		return h;
	}

	static	resource_t		ResourceFromHandle(handle_t& h)
	{
		return h;
	}

	static	const_resource_t	ConstResourceFromHandle(const handle_t& h)
	{
		return h;
	}
};

// pointer to new'd memory
template <class T>
struct HeapPointer : public PointerResource<T>
{
	typedef HeapPointer<T>	resource_style_t;

	static	void	ReleaseHandle(PointerResource<T>::handle_t& h)
	{
		if (h)
			delete h;
		h = EmptyHandle();
	}
};

// pointer to new[]'d memory
template <class T>
struct ArrayPointer : public PointerResource<T>
{
	typedef ArrayPointer<T> resource_style_t;

	static	void	ReleaseHandle(PointerResource<T>::handle_t& h)
	{
		if (h)
			delete [] h;
		h = EmptyHandle();
	}
};

// pointer to stack memory (no destroy action)
template <class T>
struct StackPointer : public PointerResource<T>
{
	typedef StackPointer<T> resource_style_t;

	static	void	ReleaseHandle(PointerResource<T>::handle_t& h)
	{
		h = EmptyHandle();
	}
};

//////////////////////////////////////////////////////////////////////////
// generalised counted sharing of a managed resource
template <class Resource>
struct SharedResource
{
	friend struct ManagedResource<Resource>;

	typedef TYPENAME Resource::resource_t		resource_t;
	typedef TYPENAME Resource::const_resource_t	const_resource_t;
	typedef TYPENAME Resource::indexed_t		indexed_t;
	typedef SharedResource<Resource>		resource_style_t;
	typedef TYPENAME Resource::type_t		type_t;

private:
	struct Data
	{
		typedef TYPENAME Resource::handle_t		handle_t;

		handle_t	h;
		unsigned	n;

		Data(handle_t h_) :
			h(h_),
			n(1)
		{
		}
	};
public:
	typedef Data*					handle_t;

	static	handle_t		EmptyHandle()
	{
		return new Data(Resource::EmptyHandle());
	}

	static	handle_t		AcquireResource(resource_t r)
	{
		return new Data(Resource::AcquireResource(r));
	}

	static	void			AcquireHandle(handle_t& dst, const handle_t& src)
	{
		ReleaseHandle(dst);
		dst = src ;
		AcquireHandle(dst);
	}

	static	handle_t		AcquireHandle(handle_t& h)
	{
		++h->n;
		return h;
	}

	static	void			ReleaseHandle(handle_t& h)
	{
		if (h->n && (0 == --h->n))
			Resource::ReleaseHandle(h->h);

		if (0 == h->n)
		{
			delete h;

			// I dont think this null pointer can cause a GPF
			// just being safe - rather a GPF than abused memory
			h = 0;
		}
	}

	static	resource_t		ResourceFromHandle(handle_t& h)
	{
		return Resource::ResourceFromHandle(h->h);
	}

	static	const_resource_t	ConstResourceFromHandle(const handle_t& h)
	{
		return Resource::ConstResourceFromHandle(h->h);
	}
};

//////////////////////////////////////////////////////////////////////////
// typing savers

////////////////////////////////////////////////////////////////// AUTOPTR
#define AUTOPTR(THIS, BASE)                                              \
template <class T>                                                       \
struct THIS : public BASE                                                \
{                                                                        \
	THIS(T* p =0) : BASE(p)	 { }                                         \
	T*       GetPtr()        { return GetResource(); }                   \
	const T* GetPtr() const  { return GetConstResource(); }              \
	THIS&    operator=(T* p) { operatorAssign(p); return *this; }        \
}

AUTOPTR(HeapPtr, Movable< NonCopyable< Arrowable< Castable< ManagedResource< HeapPointer<T> > > > > > );
AUTOPTR(ArrayPtr, Movable< NonCopyable< Arrowable< Indexable< Castable< ManagedResource< ArrayPointer<T> > > > > > > );
AUTOPTR(SharedPtr, NonMovable< Copyable< Arrowable< Castable< ManagedResource< SharedResource< HeapPointer<T> > > > > > > );
AUTOPTR(SharedArrayPtr, NonMovable< Copyable< Arrowable< Indexable< Castable< ManagedResource< SharedResource< ArrayPointer<T> > > > > > > > );
AUTOPTR(StackPtr, NonMovable< NonCopyable< Arrowable< Indexable< Castable< ManagedResource< StackPointer<T> > > > > > > );
#define RefCountedHeapPtr SharedPtr
#define RefCountedArrayPtr SharedArrayPtr

//////////////////////////////////////////////////////////////////////////
template <class T>
T* release(T*& p)
{
	T*	rc = p;
	p = 0;
	return rc;
}
