// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////// AutoHandle
template <class T>
class AutoHandle
{
	typedef typename T::handle_type	handle_t;
	handle_t	handle;

public:
	AutoHandle()
		: handle(T::DefaultHandle())
	{
	}

	AutoHandle(handle_t handle_)
		: handle(handle_)
	{
	}

	AutoHandle(AutoHandle&& that)
		: handle(that.handle)
	{
		that.handle = T::DefaultHandle();
	}

	~AutoHandle()
	{
		destroyHandle();
	}

	operator handle_t () const
	{
		return handle;
	}

	handle_t Yoink()
	{
		auto result = this->handle;
		this->handle = T::DefaultHandle();
		return result;
	}

	bool operator ! () const
	{
		return handle == T::DefaultHandle();
	}

	AutoHandle& operator = (handle_t handle_)
	{
		if (handle != handle_)
		{
			destroyHandle();
			handle = handle_;
		}

		return *this;
	}

private:
	void destroyHandle()
	{
		if (handle != T::DefaultHandle())
		{
			T::DestroyHandle(handle);
		}
	}

private:
	AutoHandle(const AutoHandle&);
	AutoHandle& operator = (const AutoHandle&);
};
