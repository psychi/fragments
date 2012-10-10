#ifndef PSYQ_SCENE_TOKEN_HPP_
#define PSYQ_SCENE_TOKEN_HPP_

//#include <psyq/scene/event_line.hpp>

namespace psyq
{
	class texture_package;
	class shader_package;
	class scene_package;
	template< typename, typename > class scene_token;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::texture_package
{
	public: typedef psyq::texture_package this_type;

	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::shader_package
{
	public: typedef psyq::shader_package this_type;

	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_package
{
	public: typedef psyq::scene_package this_type;

	//-------------------------------------------------------------------------
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

	//-------------------------------------------------------------------------
	/** @brief fileを読み込んでscene-packageを構築。
	    @param[in] i_allocator    使用するmemory割当子。
	    @param[in] i_scene_path   scene-fileのpath名。
	    @param[in] i_shader_path  shared-fileのpath名。
	    @param[in] i_texture_path texture-fileのpath名。
	 */
	public: template< typename t_allocator, typename t_string >
	static this_type::shared_ptr make(
		t_allocator const& i_allocator,
		t_string const&    i_scene_path,
		t_string const&    i_shader_path,
		t_string const&    i_texture_path)
	{
		if (!i_scene_path.empty())
		{
			// textureをfileから読み込む。
			psyq::texture_package::shared_ptr a_texture;
			if (!i_texture_path.empty())
			{
				typename t_allocator::template
					rebind< psyq::texture_package >::other a_allocator(
						i_allocator);
				this_type::read_file(
					a_allocator, i_texture_path).swap(a_texture);
				if (NULL == a_texture.get())
				{
					// textureが読み込めなかった。
					PSYQ_ASSERT(false);
					return this_type::shared_ptr();
				}
			}

			// shaderをfileから読み込む。
			psyq::shader_package::shared_ptr a_shader;
			if (!i_shader_path.empty())
			{
				typename t_allocator::template
					rebind< psyq::shader_package >::other a_allocator(
						i_allocator);
				this_type::read_file(
					a_allocator, i_shader_path).swap(a_shader);
				if (NULL == a_shader.get())
				{
					// shaderが読み込めなかった。
					PSYQ_ASSERT(false);
					return this_type::shared_ptr();
				}
			}

			// sceneをfileから読み込む。
			typename t_allocator::template
				rebind< psyq::scene_package >::other a_allocator(i_allocator);
			this_type::shared_ptr const a_scene(
				this_type::read_file(a_allocator, i_scene_path));
			if (NULL != a_scene.get())
			{
				a_scene->shader_.swap(a_shader);
				a_scene->texture_.swap(a_texture);
				return a_scene;
			}
			else
			{
				PSYQ_ASSERT(false);
			}
		}
		return this_type::shared_ptr();
	}

	//-------------------------------------------------------------------------
	private: template< typename t_allocator, typename t_string >
	static PSYQ_SHARED_PTR< typename t_allocator::value_type > read_file(
		t_allocator&    io_allocator,
		t_string const& i_path)
	{
		io_allocator.max_size();
		i_path.length();

		// 未実装なので。
		return PSYQ_SHARED_PTR< typename t_allocator::value_type >();
	}

	//-------------------------------------------------------------------------
	private: psyq::shader_package::shared_ptr  shader_;
	private: psyq::texture_package::shared_ptr texture_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_hash, typename t_real >
class psyq::scene_token
{
	public: typedef psyq::scene_token< t_hash, t_real > this_type;

	//-------------------------------------------------------------------------
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef t_real real;

	//-------------------------------------------------------------------------
	public: psyq_extern::scene_unit scene_;
	public: typename psyq::event_line< t_hash, t_real >::scale::shared_ptr
		time_scale_;
};

#endif // !PSYQ_SCENE_TOKEN_HPP_
