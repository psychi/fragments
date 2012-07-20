#ifndef PSYQ_SCENE_SECTION_HPP_
#define PSYQ_SCENE_SECTION_HPP_

namespace psyq
{
	class scene_section;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_section:
	private boost::noncopyable
{
	typedef psyq::scene_section this_type;

	//-------------------------------------------------------------------------
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	private: typedef std::vector<
		psyq::scene_token::shared_ptr,
		psyq_extern::allocator::rebind< scene_token::shared_ptr >::other >
			token_container;

	//-------------------------------------------------------------------------
	public: template< typename t_allocator >
	explicit scene_section(t_allocator const& i_allocator):
	tokens_(i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief scene-tokenを追加。
	    @param[in] i_token 追加するscene-token。
	 */
	public: bool add_token(psyq::scene_token::shared_ptr const& i_token)
	{
		if (NULL == i_token.get())
		{
			return false;
		}
		if (!this->find_token(i_token))
		{
			this->tokens_.push_back(i_token);
		}
		return true;
	}

	/** @brief scene-tokensを検索。
	    @param[in] i_token 検索するscene-token。
	    @return trueなら見つかった。falseなら見つからなかった。
	 */
	public: bool find_token(psyq::scene_token::shared_ptr const& i_token) const
	{
		return this->find_token_index(i_token.get()) < this->tokens_.size();
	}

	/** @brief scene-tokenを削除。
	    @param[in] i_token 削除するscene-token。
	 */
	public: bool remove_token(psyq::scene_token::shared_ptr const& i_token)
	{
		std::size_t const a_index(this->find_token_index(i_token.get()));
		if (a_index < this->tokens_.size())
		{
			this->tokens_.at(a_index).swap(this->tokens_.back());
			this->tokens_.pop_back();
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------
	/** @brief render-targetに描画する。
	    @param[in] i_target 描画先render-target。
	    @param[in] i_camera
	        描画に使うcamera。NULLの場合はset_camera()で指定されたcameraを使う。
	    @param[in] i_light
	        描画に使うlight。NULLの場合はset_light()で指定されたlightを使う。
	 */
	public: void draw(
		psyq::render_target const&           i_target,
		psyq_extern::scene_node const* const i_camera = NULL,
		psyq_extern::scene_unit const* const i_light = NULL)
	{
		PSYQ_ASSERT(i_target.is_drawing());

		// cameraを設定。
		psyq_extern::scene_node const* const a_camera(
			psyq_extern::set_camera(i_camera, this->camera_.get()));

		// lightを設定。
		psyq_extern::set_light(i_light, this->light_.get());

		// 描画。
		if (NULL != a_camera)
		{
			psyq_extern::draw_tokens(this->tokens_, i_target);
		}
	}

	//-------------------------------------------------------------------------
	private: std::size_t find_token_index(scene_token const* const i_token) const
	{
		if (NULL != i_token)
		{
			for (std::size_t i = 0; i < this->tokens_.size(); ++i)
			{
				if (this->tokens_.at(i).get() == i_token)
				{
					return i;
				}
			}
		}
		return (std::numeric_limits< std::size_t >::max)();
	}

	//-------------------------------------------------------------------------
	public:  psyq::scene_camera::shared_ptr camera_;
	public:  psyq::scene_token::shared_ptr  light_;
	private: this_type::token_container     tokens_;
};

#endif // !PSYQ_SCENE_SECTION_HPP_
