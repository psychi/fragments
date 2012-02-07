#ifndef PSYQ_MEMORY_HPP_
#define PSYQ_MEMORY_HPP_

namespace psyq
{
	class memory;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// memory管理の基底class。
class psyq::memory
{
	typedef psyq::memory this_type;

	public:
	typedef std::shared_ptr< this_type > shared_ptr;
	typedef std::weak_ptr< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	virtual ~memory()
	{
		if (this_type::global_instance().expired())
		{
			this_type::global_instance().reset();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	    @param[in] i_offset    確保するmemoryのbyte単位の境界offset値。
	 */
	virtual void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*),
		std::size_t const i_offset = 0)
	{
		return operator new(i_size, i_alignment, i_offset);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
		@param[in] i_size   解放するmemoryのbyte単位の大きさ。
	 */
	virtual void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		operator delete(i_memory);
	}

	//-------------------------------------------------------------------------
	bool operator==(
		this_type const& i_right)
		const
	{
		return this->identity() == i_right.identity();
	}

	bool operator!=(
		this_type const& i_right)
		const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief 全体で使うmemory管理instanceを取得。
	 */
	static this_type::shared_ptr get_global()
	{
		this_type::shared_ptr a_global(this_type::global_instance().lock());
		if (nullptr == a_global.get())
		{
			// 全体で使うinstanceが未設定なら、新たに作る。
			a_global = std::make_shared< this_type >();
			this_type::global_instance() = a_global;
		}
		return a_global;
	}

	/** @brief 全体で使うmemory管理instanceを設定。
	    @param[in] i_global 全体で使うmemory管理instance。
	 */
	static void set_global(
		this_type::shared_ptr const& i_global)
	{
		if (nullptr != i_global.get())
		{
			this_type::global_instance() = i_global;
		}
	}

	//.........................................................................
	protected:
	//-------------------------------------------------------------------------
	/** @brief 等価性を識別するための値を取得。
	 */
	virtual void const* identity() const
	{
		return __FUNCTION__;
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	static this_type::weak_ptr& global_instance()
	{
		static this_type::weak_ptr s_global;
		return s_global;
	}
};

#endif // PSYQ_MEMORY_HPP_
