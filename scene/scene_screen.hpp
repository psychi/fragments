#ifndef PSYQ_SCENE_SCREEN_HPP_
#define PSYQ_SCENE_SCREEN_HPP_

//#include <psyq/scene/scene_token.hpp>

/// @cond
namespace psyq
{
	template< typename, typename, typename, typename > class scene_screen;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief sceneを描画するscreen。
    @tparam t_hash      @copydoc scene_screen::hash
    @tparam t_real      @copydoc scene_screen::real
    @tparam t_name      @copydoc scene_screen::name
    @tparam t_allocator @copydoc scene_screen::allocator
 */
template<
	typename t_hash,
	typename t_real,
	typename t_name,
	typename t_allocator >
class psyq::scene_screen:
	private boost::noncopyable
{
	/// このobjectの型。
	public: typedef psyq::scene_screen< t_hash, t_real, t_name, t_allocator >
		this_type;

	//-------------------------------------------------------------------------
	/// このinstanceの保持子。
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;

	/// このinstanceの監視子。
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	/// event-packageで使われているhash関数の型。
	public: typedef t_hash hash;

	/// event-packageで使われている実数の型。
	public: typedef t_real real;

	/// scene-nodeの識別名の型。
	public: typedef t_name name;

	/// 使用するmemory割当子の型。
	public: typedef t_allocator allocator;

	/// このinstanceで使用する scene_token の型。
	public: typedef psyq::scene_token< t_hash, t_real > token;

	//-------------------------------------------------------------------------
	/// this_type::token の配列型。
	private: typedef std::vector<
		typename this_type::token::shared_ptr,
		typename t_allocator::template rebind<
			typename this_type::token::shared_ptr >::other >
				token_container;

	//-------------------------------------------------------------------------
	/** @param[in] i_allocator 初期化に使うmemory割当子。
	 */
	public: explicit scene_screen(
		t_allocator const& i_allocator = t_allocator()):
	tokens_(i_allocator),
	camera_node_(NULL),
	focus_node_(NULL)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 値を交換。
	    @param[in,out] io_target 値を交換するinstance。
	 */
	public: void swap(this_type& io_target)
	{
		this->light_.swap(io_target.light_);
		this->tokens_.swap(io_target.tokens_);
		this->camera_token_.swap(io_target.camera_token_);
		std::swap(this->camera_node_, io_target.camera_node_);
		this->focus_token_.swap(io_target.focus_token_);
		std::swap(this->focus_node_, io_target.focus_node_);
	}

	//-------------------------------------------------------------------------
	/** @brief render-targetに描画する。
	    @param[in] i_target 描画先render-target。
	    @param[in] i_camera
	        描画に使うcamera。NULLの場合は設定されているcameraを使う。
	    @param[in] i_light
	        描画に使うlight。NULLの場合は設定されているlightを使う。
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
	/** @brief 描画するscene-tokenを追加。
	    @param[in] i_token 追加するscene-token。
		@retval !=false 成功。
	    @retval ==false 失敗。追加するtokenが空だった。
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

	/** @brief 描画するscene-tokensを検索。
	    @param[in] i_token 検索するscene-token。
	    @retval !=false tokenを見つけた。
	    @retval ==false tokenが見つからなかった。
	 */
	public: bool find_token(
		typename this_type::token::shared_ptr const& i_token)
	const
	{
		return this->find_token_index(i_token.get()) < this->tokens_.size();
	}

	/** @brief 描画するscene-tokenを取り除く。
	    @param[in] i_token 取り除くscene-token。
	    @retval !=false 成功。
	    @retval ==false 失敗。取り除くtokenが見つからなかった。
	 */
	public: bool remove_token(
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

	/** @brief 描画するscene-tokenをすべて取り除く。
	 */
	public: void remove_tokens()
	{
		typename this_type::token_container().swap(this->tokens_);
	}

	//-------------------------------------------------------------------------
	/** @brief camera-nodeを取得。
	    @retval !=NULL camera-nodeへのpointer。
	    @retval ==NULL camera-nodeが設定されていない。
	 */
	public: psyq_extern::scene_node const* get_camera_node() const
	{
		return this->camera_node_;
	}

	/** @brief camera-nodeを持つscene-tokenを取得。
	    @return
	        camera-nodeを持つscene-tokenを指すsmart-pointer。
	        ただし、焦点nodeが設定されてない場合は空。
	 */
	public: typename this_type::token::shared_ptr const& get_camera_token()
	const
	{
		return this->camera_token_;
	}

	/** @brief cameraを設定。
	    @param[in] i_token camera-nodeを持つscene-token。
	    @param[in] i_name  camera-nodeの識別名。
	    @retval !=NULL 設定したcamera-nodeへのpointer。
	    @retval ==NULL 設定に失敗。
	 */
	public: psyq_extern::scene_node const* set_camera(
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
	    @param[in] i_token  camera-nodeと焦点nodeを持つscene-token。
	    @param[in] i_camera camera-nodeの識別名。
	    @param[in] i_focus  焦点nodeの識別名。
	    @retval !=NULL 設定したcamera-nodeへのpointer。
	    @retval ==NULL 設定に失敗。
	 */
	public: psyq_extern::scene_node const* set_camera(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_camera,
		t_name const                                 i_focus)
	{
		return this->set_node(i_token, i_camera, i_token, i_focus);
	}

	/** @brief cameraを取り除く。
	 */
	public: void remove_camera()
	{
		this->camera_token_.reset();
		this->camera_node_ = NULL;
	}

	/** @brief cameraと焦点を設定。
	    @param[in] i_camera_token camera-nodeを持つscene-token。
	    @param[in] i_camera_name  camera-nodeの識別名。
	    @param[in] i_focus_token  焦点nodeを持つscene-token。
	    @param[in] i_focus_name   焦点nodeの識別名。
	    @retval !=NULL 設定したcamera-nodeへのpointer。
	    @retval ==NULL 設定に失敗。
	 */
	public: psyq_extern::scene_node const* set_camera(
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

	//-------------------------------------------------------------------------
	/** @brief 焦点nodeを取得。
	    @retval !=NULL 焦点nodeへのpointer。
	    @retval ==NULL 焦点nodeが設定されていない。
	 */
	public: psyq_extern::scene_node const* get_focus_node()
	{
		return this->focus_node_;
	}

	/** @brief 焦点nodeを持つscene-tokenを取得。
	    @return
	        焦点nodeを持つscene-tokenを指すsmart-pointer。
	        ただし、焦点nodeが設定されてない場合は空。
	 */
	public: typename this_type::token::shared_ptr const& get_focus_token()
	const
	{
		return this->focus_token_;
	}

	/** @brief 焦点距離を取得。
	    @retval camera-nodeから焦点nodeまでの距離。
	 */
	public: float get_focus_distance() const
	{
		return NULL != this->camera_node_ && NULL != this->focus_node_?
			psyq_extern::distance(*this->camera_node_, *this->focus_node_): 0;
	}

	/** @brief 焦点を取り除く。
	 */
	public: void remove_focus()
	{
		this->focus_token_.reset();
		this->focus_node_ = NULL;
	}

	/** @brief 焦点を設定。
	    @param[in] i_token 焦点nodeを持つscene-token。
	    @param[in] i_name  焦点nodeの識別名。
	    @retval !=NULL 設定した焦点nodeへのpointer。
	    @retval ==NULL 設定に失敗。
	 */
	public: psyq_extern::scene_node const* set_focus(
		typename this_type::token::shared_ptr const& i_token,
		t_name const                                 i_name)
	{
		psyq_extern::scene_node const* const a_node(
			this_type::find_focus_node(i_token, i_name));
		if (NULL != a_node)
		{
			this->focus_token_ = i_token;
			this->focus_node_ = a_node;
			return a_node;
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	private: std::size_t find_token_index(
		typename this_type::token const* const i_token)
	const
	{
		if (NULL != i_token)
		{
			std::size_t a_rest(this->tokens_.size());
			while (0 < a_rest)
			{
				--a_rest;
				if (this->tokens_.at(a_rest).get() == i_token)
				{
					return a_rest;
				}
			}
		}
		return (std::numeric_limits< std::size_t >::max)();
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
	public:  typename this_type::token::shared_ptr light_;
	private: typename this_type::token_container   tokens_;
	private: typename this_type::token::shared_ptr camera_token_;
	private: psyq_extern::scene_node const*        camera_node_;
	private: typename this_type::token::shared_ptr focus_token_;
	private: psyq_extern::scene_node const*        focus_node_;
};

//-----------------------------------------------------------------------------
namespace std
{
	template<
		typename t_hash,
		typename t_real,
		typename t_name,
		typename t_allocator >
	void swap(
		psyq::scene_screen< t_hash, t_real, t_name, t_allocator >& io_left,
		psyq::scene_screen< t_hash, t_real, t_name, t_allocator >& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // PSYQ_SCENE_SCREEN_HPP_
