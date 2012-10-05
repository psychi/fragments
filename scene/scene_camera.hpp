#ifndef PSYQ_SCENE_CAMERA_HPP_
#define PSYQ_SCENE_CAMERA_HPP_

namespace psyq
{
	template< typename, typename, typename, typename > class scene_camera;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief scene-stageに配置するcamera。
    @tparam t_hash      event-packageで使われているhash関数。
    @tparam t_real      event-packageで使われている実数の型。
    @tparam t_name      scene-nodeの名前の型。
    @tparam t_allocator 使用するmemory割当子の型。
 */
template<
	typename t_hash,
	typename t_real,
	typename t_name,
	typename t_allocator >
class psyq::scene_camera:
	private boost::noncopyable
{
	public: typedef psyq::scene_camera< t_hash, t_real, t_name, t_allocator >
		this_type;

	//-------------------------------------------------------------------------
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef t_real real;
	public: typedef t_name name;
	public: typedef t_allocator allocator;
	public: typedef psyq::scene_token< t_hash, t_real > token;

	//-------------------------------------------------------------------------
	private: typedef std::vector<
		typename this_type::token::shared_ptr,
		typename t_allocator::template rebind<
			typename this_type::token::shared_ptr >::other >
				token_container;

	//-------------------------------------------------------------------------
	public: explicit scene_camera(t_allocator const& i_allocator):
	tokens_(i_allocator),
	camera_node_(NULL),
	focus_node_(NULL)
	{
		// pass
	}

	public: explicit scene_camera(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_name)
	{
		this->set_node(i_token, i_name);
	}

	/** @param[in] i_camera_token cameraとして使うscene-token。
	    @param[in] i_camera_name  cameraとして使うnodeのID文字列。
		@param[in] i_focus_token  焦点として使うscene-token。
	    @param[in] i_focus_name   焦点として使うnodeのID文字列。
	 */
	public: scene_camera(
		typename this_type::token::shared_ptr const& i_camera_token,
		t_name const                                 i_camera_name,
		typename this_type::token::shared_ptr const& i_focus_token,
		t_name const                                 i_focus_name)
	{
		this->set_node(
			i_camera_token, i_camera_name, i_focus_token, i_focus_name);
	}

	//-------------------------------------------------------------------------
	/** @brief cameraを設定。
	    @param[in] i_token camera-nodeを持つscene-token。
	    @param[in] i_name  cameraとして使うnodeのID名。
	 */
	public: psyq_extern::scene_node const* set_node(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_name)
	{
		typename this_type::token* const a_token(i_token.get());
		if (NULL != a_token)
		{
			psyq_extern::scene_node* const a_node(
				psyq_extern::find_camera_node(a_token->scene_, i_name));
			if (NULL != a_node)
			{
				this->camera_token_ = i_token;
				this->camera_node_ = a_node;
				return a_node;
			}
		}
		return NULL;
	}

	/** @brief cameraと焦点を設定。
	    @param[in] i_token  camera-nodeを持つscene-token。
	    @param[in] i_camera cameraとして使うnodeのID名。
	    @param[in] i_focus  焦点として使うnodeのID名。
	 */
	public: psyq_extern::scene_node const* set_node(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_camera,
		t_name const                                 i_focus)
	{
		return this->set_node(i_token, i_camera, i_token, i_focus);
	}

	/** @brief cameraと焦点を設定。
	    @param[in] i_camera_token camera-nodeを持つscene-token。
	    @param[in] i_camera_name  cameraとして使うnodeのID名。
	    @param[in] i_focus_token  焦点nodeを持つscene-token。
	    @param[in] i_focus_name   焦点として使うnodeのID名。
	 */
	public: psyq_extern::scene_node const* set_node(
		typename this_type::token::shared_ptr const& i_camera_token,
		t_name const                                 i_camera_name,
		typename this_type::token::shared_ptr const& i_focus_token,
		t_name const                                 i_focus_name)
	{
		psyq_extern::scene_node* const a_focus_node(
			this_type::find_focus_node(i_focus_token, i_focus_name));
		if (NULL != a_focus_node)
		{
			psyq_extern::scene_node* const a_camera_node(
				this->set_node(i_camera_token, i_camera_name));
			if (NULL != a_camera_node)
			{
				this->focus_token_ = i_focus_token;
				this->focus_node_ = a_focus_node;
				return a_camera_node;
			}
		}
		return NULL;
	}

	/** @brief camera-nodeを取得。
	 */
	public: psyq_extern::scene_node const* get_node() const
	{
		return this->camera_node_;
	}

	/** @brief camera-nodeを持つscene-tokenを取得。
	 */
	public: typename this_type::token::shared_ptr const& get_token() const
	{
		return this->camera_token_;
	}

	//-------------------------------------------------------------------------
	/** @brief 焦点を設定。
	    @param[in] i_token 焦点nodeを持つscene-token。
	    @param[in] i_name  焦点として使うnodeのID文字列。
	 */
	public: psyq_extern::scene_node const* set_focus_node(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_name)
	{
		psyq_extern::scene_node* const a_node(
			this_type::find_focus_node(i_token, i_name));
		if (NULL != a_node)
		{
			this->focus_token_ = i_token;
			this->focus_node_ = a_node;
			return a_node;
		}
		return NULL;
	}

	/** @brief 焦点nodeを取得。
	 */
	public: psyq_extern::scene_node const* get_focus_node()
	{
		return this->focus_node_;
	}

	/** @brief 焦点nodeを持つscene-tokenを取得。
	 */
	public: typename this_type::token::shared_ptr const& get_focus_token() const
	{
		return this->focus_token_;
	}

	/** @brief 焦点距離を取得。
	 */
	public: float get_focus_distance() const
	{
		return NULL != this->camera_node_ && NULL != this->focus_node_?
			psyq_extern::distance(*this->camera_node_, *this->focus_node_): 0;
	}

	private: static psyq_extern::scene_node const* find_focus_node(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_name)
	{
		typename this_type::token* const a_token(i_token.get());
		return NULL != i_name && NULL != a_token?
			psyq_extern::find_node(a_token->scene_, i_name): NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 描画scene-tokenを追加。
	    @param[in] i_token 追加するscene-token。
	 */
	public: bool insert_token(
		typename this_type::token::shared_ptr const& i_token)
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

	/** @brief 描画scene-tokensを検索。
	    @param[in] i_token 検索するscene-token。
	    @return trueなら見つかった。falseなら見つからなかった。
	 */
	public: bool find_token(
		typename this_type::token::shared_ptr const& i_token)
	const
	{
		return this->find_token_index(i_token.get()) < this->tokens_.size();
	}

	/** @brief 描画scene-tokenを削除。
	    @param[in] i_token 削除するscene-token。
	 */
	public: bool erase_token(
		typename this_type::token::shared_ptr const& i_token)
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
	private: std::size_t find_token_index(
		typename this_type::token const* const i_token)
	const
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
	public:  typename this_type::token::shared_ptr light_;
	private: typename this_type::token_container   tokens_;
	private: typename this_type::token::shared_ptr camera_token_;
	private: psyq_extern::scene_node const*        camera_node_;
	private: typename this_type::token::shared_ptr focus_token_;
	private: psyq_extern::scene_node const*        focus_node_;
};

#endif // PSYQ_SCENE_CAMERA_HPP_
