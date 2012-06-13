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
	count_(this_type::frame_count())
	{
		this_type* const a_super(i_super.get());
		if (NULL != a_super)
		{
			if (a_super->find_super(*this))
			{
				PSYQ_ASSERT(false);
				this->super_.reset();
			}
			else
			{
				this->current_ *= a_super->get_scale();
			}
		}
	}

	//-------------------------------------------------------------------------
	void reset(typename this_type::lerp const& i_scale)
	{
		this->scale_ = i_scale;
		this->current_ = this->scale_.current();
		this_type* const a_super(this->super_.get());
		if (NULL != a_super)
		{
			this->current_ *= a_super->get_scale();
			this->count_ = a_super->count_;
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
		typename this_type::lerp const&       i_scale,
		typename this_type::shared_ptr const& i_super)
	{
		this->~time_frame();
		new(this) this_type(i_scale, i_super);
	}

	//-------------------------------------------------------------------------
	t_scale get_scale()
	{
		// 上位instanceのscaleとcountを取得。
		this_type* const a_super(this->super_.get());
		float a_scale;
		t_count a_count;
		if (NULL != a_super)
		{
			a_scale = a_super->get_scale();
			a_count = a_super->count_;
		}
		else
		{
			a_scale = 1;
			a_count = this_type::frame_count();
		}

		// count値が異なっていたら更新する。
		if (a_count != this->count_)
		{
			this->scale_.update(a_count - this->count_);
			this->count_ = a_count;
			this->current_ = this->scale_.current() * a_scale;
		}
		return this->current_;
	}

	//-------------------------------------------------------------------------
	static t_count get_count()
	{
		return this_type::frame_count();
	}

	static t_count update_count()
	{
		return ++this_type::frame_count();
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

	static t_count& frame_count()
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
