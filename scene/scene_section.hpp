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

//.............................................................................
public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	template< typename t_allocator >
	explicit scene_section(t_allocator const& i_allocator):
	tokens_(i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief scene-token��ǉ��B
	    @param[in] i_token �ǉ�����scene-token�B
	 */
	bool add_token(psyq::scene_token::shared_ptr const& i_token)
	{
		if (NULL == i_token.get() || this->find_token(i_token))
		{
			return false;
		}
		this->tokens_.push_back(i_token);
		return true;
	}

	/** @brief scene-tokens�������B
	    @param[in] i_token ��������scene-token�B
	    @return true�Ȃ猩�������Bfalse�Ȃ猩����Ȃ������B
	 */
	bool find_token(psyq::scene_token::shared_ptr const& i_token) const
	{
		return this->find_token_index(i_token.get()) < this->tokens_.size();
	}

	/** @brief scene-token���폜�B
	    @param[in] i_token �폜����scene-token�B
	 */
	void remove_token(psyq::scene_token::shared_ptr const& i_token)
	{
		std::size_t const a_index(this->find_token_index(i_token.get()));
		if (a_index < this->tokens_.size())
		{
			this->tokens_.at(a_index).swap(this->tokens_.back());
			this->tokens_.pop_back();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief render-target�ɕ`�悷��B
	    @param[in] i_target �`���render-target�B
	    @param[in] i_camera
	        �`��Ɏg��camera�BNULL�̏ꍇ��set_camera()�Ŏw�肳�ꂽcamera���g���B
	    @param[in] i_light
	        �`��Ɏg��light�BNULL�̏ꍇ��set_light()�Ŏw�肳�ꂽlight���g���B
	 */
	void draw(
		psyq::render_target const&           i_target,
		psyq_extern::scene_node const* const i_camera = NULL,
		psyq_extern::scene_unit const* const i_light = NULL)
	{
		PSYQ_ASSERT(i_target.is_drawing());

		// camera��ݒ�B
		psyq_extern::scene_node const* const a_camera(
			psyq_extern::set_camera(i_camera, this->camera_.get()));

		// light��ݒ�B
		psyq_extern::set_light(i_light, this->light_.get());

		// �`��B
		if (NULL != a_camera)
		{
			psyq_extern::draw_tokens(this->tokens_, i_target);
		}
	}

//.............................................................................
private:
	typedef std::vector<
		scene_token::shared_ptr,
		scene_event::allocator::rebind< scene_token::shared_ptr >::other >
			token_container;

	//-------------------------------------------------------------------------
	std::size_t find_token_index(scene_token const* const i_token) const
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

//.............................................................................
public:
	psyq::scene_camera::shared_ptr camera_;
	psyq::scene_token::shared_ptr  light_;

private:
	this_type::token_container tokens_;
};

#endif // !PSYQ_SCENE_SECTION_HPP_