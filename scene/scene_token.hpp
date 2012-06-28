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

public:
	typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

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
