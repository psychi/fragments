#ifndef PSYQ_SCENE_RENDER_TARGET_HPP_
#define PSYQ_SCENE_RENDER_TARGET_HPP_

namespace psyq
{
	class render_target;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::render_target:
	private boost::noncopyable
{
	typedef psyq::render_target this_type;

//.............................................................................
public:
	enum type
	{
		type_NONE,
		type_FRAME_BUFFER,
		type_TEXTURE,
		type_CUBE_TEXTURE,
		type_POST_EFFECT,
	};

	//-------------------------------------------------------------------------
	~render_target()
	{
		this->end_draw();
		this->reset_post_effect();
		this->release_post_effect();
	}

	render_target():
	//glare_(NULL),
	//tonemap_(NULL),
	//dof_(NULL),
	clear_color_(0, 0, 0, 1),
	clear_depth_(1),
	clear_stencil_(0),
	clear_buffer_(psyq_extern::surface_COLOR | psyq_extern::surface_DEPTH),
	flags_(this_type::type_FRAME_BUFFER)
	{
		this->texture_ = NULL;
	}

	//-------------------------------------------------------------------------
	bool begin_draw()
	{
		switch (this->get_draw_type())
		{
		case this_type::type_FRAME_BUFFER:
			this->clear_buffer();
			break;

		case this_type::type_TEXTURE:
			if (NULL == this->texture_)
			{
				PSYQ_ASSERT(false);
				return false;
			}
			psyq_extern::bind_texture(*this->texture_);
			this->clear_buffer();
			break;

		case this_type::type_CUBE_TEXTURE:
			if (NULL == this->texture_)
			{
				PSYQ_ASSERT(false);
				return false;
			}
			break;

		case this_type::type_POST_EFFECT:
			if (NULL != this->post_effect_)
			{
				//psyq_extern::apply_post_effect(*this->post_effect_, i_camera, i_focus);

				psyq_extern::begin_post_effect(
					*this->post_effect_,
					this->clear_buffer_,
					this->clear_color_,
					this->clear_depth_,
					this->clear_stencil_);
			}
			break;

		default:
			return false;
		}

		this->flags_ |= this_type::flag_DRAWING;
		return true;
	}

	//-------------------------------------------------------------------------
	bool end_draw()
	{
		switch (this->get_draw_type())
		{
		case this_type::flag_DRAWING | this_type::type_FRAME_BUFFER:
			break;

		case this_type::flag_DRAWING | this_type::type_TEXTURE:
		case this_type::flag_DRAWING | this_type::type_CUBE_TEXTURE:
			psyq_extern::bind_frame_buffer();
			break;

		case this_type::flag_DRAWING | this_type::type_POST_EFFECT:
			if (NULL != this->post_effect_)
			{
				psyq_extern::end_post_effect(*this->post_effect_);
			}
			break;

		default:
			return false;
		}

		this->flags_ &= ~this_type::flag_DRAWING;
		return true;
	}

	//-------------------------------------------------------------------------
	this_type::type get_type() const
	{
		return static_cast< this_type::type >(
			this_type::flag_TYPE & this->flags_);
	}

	bool is_drawing() const
	{
		return 0 != (this->flags_ & this_type::flag_DRAWING);
	}

	//-------------------------------------------------------------------------
	/** @brief render-targetにdefaultのframe-bufferが設定されているか判定。
	 */
	bool is_frame_buffer() const
	{
		return this_type::type_FRAME_BUFFER == this->get_type();
	}

	/** @brief defaultのframe-bufferをrender-targetに設定。
	 */
	bool set_frame_buffer()
	{
		if (this->is_drawing())
		{
			return false;
		}

		this->release_post_effect();
		this->post_effect_ = NULL;
		this->set_draw_type(this_type::type_FRAME_BUFFER);
		return true;
	}

	//-------------------------------------------------------------------------
	/** @brief render-targetに設定されているtextureを取得。
	 */
	psyq_extern::render_texture* get_texture()
	{
		return this_type::type_TEXTURE == this->get_type()?
			this->texture_: NULL;
	}

	/** @brief textureをrender-targetに設定。
		@param[in] i_texture 設定するtexture。
	 */
	bool set_texture(psyq_extern::render_texture* const i_texture)
	{
		if (this->is_drawing() || NULL == i_texture)
		{
			return false;
		}

		this->release_post_effect();
		this->texture_ = i_texture;
		this->set_draw_type(this_type::type_TEXTURE);
		return true;
	}

	//-------------------------------------------------------------------------
	psyq_extern::render_texture* get_cube_texture() const
	{
		return this_type::type_CUBE_TEXTURE == this->get_type()?
			this->texture_: NULL;
	}

	/** @brief cube-textureをrender-targetに設定。
		@param[in] i_texture 設定するcube-texture。
	 */
	bool set_cube_texture(psyq_extern::render_texture* const i_texture)
	{
		if (this->is_drawing() || NULL == i_texture)
		{
			return false;
		}

		this->release_post_effect();
		this->texture_ = i_texture;
		this->set_draw_type(this_type::type_CUBE_TEXTURE);
		return true;
	}

	void bind_cube_texture(
		psyq_extern::math_matrix4 const& i_view,
		unsigned const            i_face)
	const
	{
		psyq_extern::render_texture* const a_cube_texture(this->get_cube_texture());
		if (NULL != a_cube_texture)
		{
			psyq_extern::bind_cube_texture(*a_cube_texture, i_view, i_face);
			this->clear_buffer();
		}
	}

	//-------------------------------------------------------------------------
	psyq_extern::post_effect* get_post_effect() const
	{
		return this_type::type_POST_EFFECT == this->get_type()?
			this->post_effect_: NULL;
	}

	/** @brief post-effectをrender-targetに設定。
		@param[in,out] i_post_effect 設定するpost-effect。
	 */
	bool set_post_effect(psyq_extern::post_effect* const io_post_effect)
	{
		if (this->is_drawing())
		{
			return false;
		}

		this->release_post_effect();
		this->post_effect_ = io_post_effect;
		this->set_draw_type(this_type::type_POST_EFFECT);
		if (NULL != io_post_effect)
		{
			psyq_extern::hold_post_effect(*io_post_effect);
		}
		return true;
	}

	//-------------------------------------------------------------------------
	/** @brief color-bufferのclear値を設定。
	 */
	void set_clear_color(psyq_extern::math_vector4 const& i_color)
	{
		this->clear_buffer_ |= psyq_extern::surface_COLOR;
		this->clear_color_ = i_color;
	}

	/** @brief color-bufferのclearを解除。
	 */
	void reset_clear_color()
	{
		this->clear_buffer_ &= ~psyq_extern::surface_COLOR;
	}

	//-------------------------------------------------------------------------
	/** @brief depth-bufferのclear値を設定。
	 */
	void set_clear_depth(float const i_depth)
	{
		this->clear_buffer_ |= psyq_extern::surface_DEPTH;
		this->clear_depth_ = i_depth;
	}

	/** @brief depth-bufferのclearを解除。
	 */
	void reset_clear_depth()
	{
		this->clear_buffer_ &= ~psyq_extern::surface_DEPTH;
	}

	//-------------------------------------------------------------------------
	/** @brief stencil-bufferのclear値を設定。
	 */
	void set_clear_stencil(boost::uint32_t i_stencil)
	{
		this->clear_buffer_ |= psyq_extern::surface_STENCIL;
		this->clear_stencil_ = i_stencil;
	}

	/** @brief stencil-bufferのclearを解除。
	 */
	void reset_clear_stencil()
	{
		this->clear_buffer_ &= ~psyq_extern::surface_STENCIL;
	}

//.............................................................................
private:
	enum flag
	{
		flag_TYPE    = 0x07,
		flag_DRAWING = 0x80,
	};

	//-------------------------------------------------------------------------
	unsigned get_draw_type() const
	{
		return (this_type::flag_DRAWING | this_type::flag_TYPE) & this->flags_;
	}

	void set_draw_type(this_type::type const i_type)
	{
		this->flags_ = (~this_type::flag_TYPE & this->flags_) | i_type;
	}

	void clear_buffer() const
	{
		psyq_extern::clear_render_target(
			this->clear_buffer_,
			this->clear_color_,
			this->clear_depth_,
			this->clear_stencil_);
	}

	void release_post_effect()
	{
		if (NULL!= this->post_effect_ &&
			this_type::type_POST_EFFECT == this->get_type())
		{
			psyq_extern::release_post_effect(*this->post_effect_);
		}
	}

	void reset_post_effect()
	{
		/*
		this->reset_glare();
		this->reset_tonemap();
		this->reset_dof();
		this->reset_color_matrix();
		 */
	}

//.............................................................................
private:
	union
	{
		psyq_extern::render_texture* texture_;
		psyq_extern::post_effect*    post_effect_;
	};
	//pfx_glare*                glare_;
	//pfx_tonemap*              tonemap_;
	//pfx_dof*                  dof_;
	//this_type::color_matrices color_matrices_;
	psyq_extern::math_vector4  clear_color_;
	float                      clear_depth_;
	boost::uint32_t            clear_stencil_;
	boost::uint32_t            clear_buffer_;
	boost::uint32_t            flags_;
};

#endif // !PSYQ_SCENE_RENDER_TARGET_HPP_
