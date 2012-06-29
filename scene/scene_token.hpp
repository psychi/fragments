#ifndef PSYQ_SCENE_TOKEN_HPP_
#define PSYQ_SCENE_TOKEN_HPP_

namespace psyq
{
	class texture_package;
	class shader_package;
	class scene_package;
	class scene_token;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::texture_package
{
	typedef psyq::texture_package this_type;

public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::shader_package
{
	typedef psyq::shader_package this_type;

public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_package
{
	typedef psyq::scene_package this_type;

//.............................................................................
public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

	//-------------------------------------------------------------------------
	static this_type::shared_ptr load(
		psyq::scene_event::string const& i_scene_path,
		psyq::scene_event::string const& i_shader_path,
		psyq::scene_event::string const& i_texture_path)
	{
		if (!i_scene_path.empty())
		{
			// textureをfileから読み込む。
			texture_package::shared_ptr a_texture;
			if (!i_texture_path.empty())
			{
				this_type::load_file< texture_package >(
					i_texture_path).swap(a_texture);
				if (NULL == a_texture.get())
				{
					// textureが読み込めなかった。
					PSYQ_ASSERT(false);
					return this_type::shared_ptr();
				}
			}

			// shaderをfileから読み込む。
			shader_package::shared_ptr a_shader;
			if (!i_shader_path.empty())
			{
				this_type::load_file< shader_package >(
					i_shader_path).swap(a_shader);
				if (NULL == a_shader.get())
				{
					// shaderが読み込めなかった。
					PSYQ_ASSERT(false);
					return this_type::shared_ptr();
				}
			}

			// sceneをfileから読み込む。
			this_type::shared_ptr const a_scene(
				this_type::load_file< this_type >(i_scene_path));
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
		return scene_package::shared_ptr();
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	template< typename t_value, typename t_string >
	static PSYQ_SHARED_PTR< t_value > load_file(t_string const& i_path)
	{
		i_path.length();
		return PSYQ_SHARED_PTR< t_value >(); // 未実装なので。
	}

//.............................................................................
private:
	psyq::shader_package::shared_ptr  shader_;
	psyq::texture_package::shared_ptr texture_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_token
{
	typedef psyq::scene_token this_type;

//.............................................................................
public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

//.............................................................................
public:
	psyq_extern::scene_unit                   scene_;
	psyq::scene_event::time_scale::shared_ptr time_scale_;
};

#endif // !PSYQ_SCENE_TOKEN_HPP_
