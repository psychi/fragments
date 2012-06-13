#ifndef PSYQ_TIME_FRAME_HPP_
#define PSYQ_TIME_FRAME_HPP_

//#include <psyq/lerp.hpp>

namespace psyq
{
	template< typename, typename > class time_frame;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_scale, typename t_count >
class psyq::time_frame
{
	typedef psyq::time_frame< t_scale, t_count > this_type;

//.............................................................................
public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	typedef t_count count;
	typedef psyq::lerp< t_scale, t_count > scale;

	//-------------------------------------------------------------------------
	explicit time_frame(
		t_scale const                         i_scale = 1,
		typename this_type::shared_ptr const& i_super =
			this_type::shared_ptr()):
	super_(i_super),
	scale_(i_scale),
	current_(i_scale)
	{
		this->construct(i_super.get());
	}

	explicit time_frame(
		typename this_type::scale const&      i_scale,
		typename this_type::shared_ptr const& i_super =
			this_type::shared_ptr()):
	super_(i_super),
	scale_(i_scale),
	current_(i_scale.current())
	{
		this->construct(i_super.get());
	}

	//-------------------------------------------------------------------------
	void reset(typename this_type::scale const& i_scale)
	{
		this->scale_ = i_scale;
		this->current_ = this->scale_.current();
		this_type* const a_super(this->super_.get());
		if (NULL != a_super)
		{
			this->current_ *= a_super->get_scale();
		}
	}

	void reset(typename this_type::shared_ptr const& i_super)
	{
		this_type* const a_super(i_super.get());
		if (NULL != a_super)
		{
			this->current_ = this->get_scale() * a_super->get_scale();
			this->count_ = a_super->count_;
			this->super_ = i_super;
		}
		else
		{
			this->super_.reset();
		}
	}

	void reset(
		typename this_type::scale const&      i_scale,
		typename this_type::shared_ptr const& i_super)
	{
		this->~time_frame();
		new(this) this_type(i_scale, i_super);
	}

	//-------------------------------------------------------------------------
	typename this_type::scale::value_type get_scale()
	{
		this->update(0);
		return this->current_;
	}

	//-------------------------------------------------------------------------
	t_count get_count() const
	{
		return this->count_;
	}

	void update_count()
	{
		this->update(1);
	}

//.............................................................................
private:
	void construct(this_type* const i_super)
	{
		if (NULL == i_super)
		{
			this->count_ = 0;
		}
		else if (!i_super->find_super(*this))
		{
			this->current_ *= i_super->get_scale();
			this->count_ = i_super->get_count();
		}
		else
		{
			PSYQ_ASSERT(false);
			this->super_.reset();
			this->count_ = 0;
		}
	}

	void update(typename this_type::count const i_count)
	{
		this_type* const a_super(this->super_.get());
		float a_scale;
		this_type::count a_count;
		if (NULL != a_super)
		{
			a_scale = a_super->get_scale();
			a_count = a_super->get_count();
		}
		else
		{
			a_scale = 1;
			a_count = this->count_ + i_count;
		}

		if (a_count != this->count_)
		{
			this->scale_.update(a_count - this->count_);
			this->count_ = a_count;
			this->current_ = this->scale_.current() * a_scale;
		}
	}

	bool find_super(this_type const& i_timer) const
	{
		if (&i_timer == this)
		{
			return true;
		}
		if (NULL == this->super_.get())
		{
			return false;
		}
		return this->super_->find_super(i_timer);
	}

//.............................................................................
private:
	typename this_type::shared_ptr super_;
	typename this_type::scale      scale_;
	t_scale                        current_;
	t_count                        count_;
};

#endif // PSYQ_TIME_FRAME_HPP_
