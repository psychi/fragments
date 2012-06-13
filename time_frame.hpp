#ifndef PSYQ_TIME_FRAME_HPP_
#define PSYQ_TIME_FRAME_HPP_

//#include <psyq/lerp.hpp>

namespace psyq
{
	template< typename, typename, typename > class time_frame;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_scale, typename t_count, typename t_tag = void >
class psyq::time_frame
{
	typedef psyq::time_frame< t_scale, t_count, t_tag > this_type;

//.............................................................................
public:
	typedef t_scale scale;
	typedef t_count count;
	typedef t_tag tag;
	typedef psyq::lerp< t_scale, t_count > lerp;
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	explicit time_frame(
		typename this_type::lerp const&       i_scale = this_type::lerp(1),
		typename this_type::shared_ptr const& i_super =
			this_type::shared_ptr()):
	super_(i_super),
	scale_(i_scale),
	current_(i_scale.current()),
	count_(this_type::counter())
	{
		this_type* const a_super(i_super.get());
		if (NULL != a_super)
		{
			this->current_ *= a_super->get_scale();
		}
	}

	//-------------------------------------------------------------------------
	void reset(typename this_type::lerp const& i_scale)
	{
		this->scale_ = i_scale;
		this->count_ = this_type::counter();
		this->current_ = i_scale.current();
		this_type* const a_super(this->super_.get());
		if (NULL != a_super)
		{
			this->current_ *= a_super->get_scale();
		}
	}

	bool reset(typename this_type::shared_ptr const& i_super)
	{
		this_type const* const a_super(i_super.get());
		if (NULL != a_super && a_super->find_super(*this))
		{
			return false;
		}
		this->super_ = i_super;
		return true;
	}

	bool reset(
		typename this_type::lerp const&       i_scale,
		typename this_type::shared_ptr const& i_super)
	{
		this_type* const a_super(i_super.get());
		if (NULL == a_super)
		{
			this->current_ = i_scale.current();
		}
		else if (a_super->find_super(*this))
		{
			return false;
		}
		else
		{
			this->current_ = i_scale.current() * a_super->get_scale();
		}
		this->super_ = i_super;
		this->scale_ = i_scale;
		this->count_ = this_type::counter();
		return true;
	}

	//-------------------------------------------------------------------------
	t_scale get_scale()
	{
		t_count const a_count(this_type::counter());
		if (a_count != this->count_)
		{
			// count値が異なっていたら更新する。
			this->scale_.update(a_count - this->count_);
			this_type* const a_super(this->super_.get());
			this->current_ = this->scale_.current() * (
				NULL != a_super? a_super->get_scale(): 1);
			this->count_ = a_count;
		}
		return this->current_;
	}

	//-------------------------------------------------------------------------
	static t_count get_count()
	{
		return this_type::counter();
	}

	static t_count update_count()
	{
		return ++this_type::counter();
	}

//.............................................................................
private:
	bool find_super(this_type const& i_timer) const
	{
		if (&i_timer != this)
		{
			this_type const* const a_super(this->super_.get());
			return NULL != a_super? a_super->find_super(i_timer): false;
		}
		return true;
	}

	static t_count& counter()
	{
		static t_count s_count(0);
		return s_count;
	}

//.............................................................................
private:
	typename this_type::shared_ptr super_;
	typename this_type::lerp       scale_;
	t_scale                        current_;
	t_count                        count_;
};

#endif // PSYQ_TIME_FRAME_HPP_
