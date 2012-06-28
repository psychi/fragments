#ifndef PSYQ_SCENE_CAMERA_HPP_
#define PSYQ_SCENE_CAMERA_HPP_

namespace psyq
{
	class scene_camera;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_camera:
	private boost::noncopyable
{
	typedef psyq::scene_camera this_type;

//.............................................................................
public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	scene_camera():
	camera_node_(NULL),
	focus_node_(NULL)
	{
		// pass
	}

	explicit scene_camera(
		psyq::scene_token::shared_ptr const& i_token,
		char const* const                    i_name = NULL)
	{
		this->set_node(i_token, i_name);
	}

	scene_camera(
		psyq::scene_token::shared_ptr const& i_camera_token,
		char const* const                    i_camera_name,
		psyq::scene_token::shared_ptr const& i_focus_token,
		char const* const                    i_focus_name)
	{
		this->set_node(i_camera_token, i_camera_name);
		this->set_focus_node(i_focus_token, i_focus_name);
	}

	//-------------------------------------------------------------------------
	/** @brief cameraを設定。
	    @param[in] i_token cameraとして使うscene-token。
	    @param[in] i_name  cameraとして使うnodeのID文字列。
	 */
	psyq_extern::scene_node const* set_node(
		psyq::scene_token::shared_ptr const& i_token,
		char const* const                    i_name = NULL,
		char const* const                    i_focus = NULL)
	{
		psyq::scene_token* const a_token(i_token.get());
		if (NULL != a_token)
		{
			psyq_extern::scene_node* const a_node(
				psyq_extern::find_camera_node(*a_token, i_name));
			if (NULL != a_node)
			{
				this->camera_token_ = i_token;
				this->camera_node_ = a_node;
				this->set_focus_node(i_token, i_focus);
				return a_node;
			}
		}
		return NULL;
	}

	psyq_extern::scene_node const* get_node()
	{
		return this->camera_node_;
	}

	psyq::scene_token::shared_ptr const& get_token() const
	{
		return this->camera_token_;
	}

	//-------------------------------------------------------------------------
	psyq_extern::scene_node const* set_focus_node(
		psyq::scene_token::shared_ptr const& i_token,
		char const* const                    i_name)
	{
		psyq::scene_token* const a_token(i_token.get());
		if (NULL != i_name && NULL != a_token)
		{
			psyq_extern::scene_node* const a_node(
				psyq_extern::find_node(*a_token, i_name));
			if (NULL != a_node)
			{
				this->focus_token_ = i_token;
				this->focus_node_ = a_node;
				return a_node;
			}
		}
		return NULL;
	}

	psyq_extern::scene_node const* get_focus_node()
	{
		return this->focus_node_;
	}

	psyq::scene_token::shared_ptr const& get_focus_token() const
	{
		return this->focus_token_;
	}

	float get_focus_distance() const
	{
		if (NULL == this->camera_node_ || NULL == this->focus_node_)
		{
			return 0;
		}
		return psyq_extern::distance(*this->camera_node_, *this->focus_node_);
	}

//.............................................................................
private:
	psyq::scene_token::shared_ptr  camera_token_;
	psyq_extern::scene_node const* camera_node_;
	psyq::scene_token::shared_ptr  focus_token_;
	psyq_extern::scene_node const* focus_node_;
};

#endif // DSSG_SCENE_CAMERA_HPP_
