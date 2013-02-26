#ifndef PSYQ_VECTOR_MAP_HPP_
#define PSYQ_VECTOR_MAP_HPP_

namespace psyq
{
    /// @cond
    namespace closed
    {
        template<typename, typename> class vector_map_base;
    }
    template<typename, typename> class vector_map;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_vector_type,
    typename template_compare_type>
class psyq::closed::vector_map_base:
    protected template_vector_type
{
    public: typedef psyq::closed::vector_map_base<
        template_vector_type, template_compare_type>
            self;

    protected: typedef template_vector_type super;

    public: typedef template_vector_type base;

    public: typedef typename super::value_type::first_type key_type;
    static_assert(std::is_const<key_type>::value, "key_type is not const.");

    public: typedef typename super::value_type::second_type mapped_type;

    public: typedef template_compare_type key_compare;

    public: using super::value_type;
    public: using super::allocator_type;
    public: using super::size_type;
    public: using super::difference_type;
    public: using super::reference;
    public: using super::const_reference;
    public: using super::pointer;
    public: using super::const_pointer;
    public: using super::iterator;
    public: using super::const_iterator;
    public: using super::reverse_iterator;
    public: using super::const_reverse_iterator;

    public: using super::get_allocator;
    public: using super::begin;
    public: using super::end;
    public: using super::rbegin;
    public: using super::rend;
    public: using super::empty;
    public: using super::size;
    public: using super::max_size;
    public: using super::reserve;
    public: using super::capacity;
    public: using super::clear;
    public: using super::erase;
    public: using super::pop_back;

    protected: void move_from_back(
        typename super::size_type in_index)
    {
        if (this->empty())
        {
            PSYQ_ASSERT(false);
            return;
        }
        for (
            typename super::size_type i(this->size() - 1);
            in_index < i;
            --i)
        {
            self::swap_value(this->super::at(i), this->super::at(i - 1));
        }
    }

    protected: static void swap_value(
        typename super::value_type& io_left,
        typename super::value_type& io_right)
    {
        typedef std::remove_const<key_type>::type mutable_key;
        std::swap(
            const_cast<mutable_key&>(io_left.first),
            const_cast<mutable_key&>(io_right.first));
        std::swap(io_left.second, io_right.second);
    }

    //-------------------------------------------------------------------------
    public: typename super::iterator lower_bound(
        typename self::key_type const& in_key)
    {
        return std::lower_bound(
            this->begin(), this->end(), in_key, self::compare());
    }

    public: typename super::const_iterator lower_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::lower_bound(
            this->begin(), this->end(), in_key, self::compare());
    }

    public: typename super::iterator upper_bound(
        typename self::key_type const& in_key)
    {
        return std::upper_bound(
            this->begin(), this->end(), in_key, self::compare());
    }

    public: typename super::const_iterator upper_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::upper_bound(
            this->begin(), this->end(), in_key, self::compare());
    }

    //-------------------------------------------------------------------------
    protected: struct compare:
        public template_compare_type
    {
        bool operator()(
            typename template_vector_type::value_type const& in_left,
            typename template_vector_type::value_type const& in_right)
        const
        {
            return this->operator()(in_left.first, in_right.second);
        }

        bool operator()(
            key_type const&                                  in_left,
            typename template_vector_type::value_type const& in_right)
        const
        {
            return this->operator()(in_left, in_right.second);
        }

        bool operator()(
            typename template_vector_type::value_type const& in_left,
            key_type const&                                  in_right)
        const
        {
            return this->operator()(in_left.first, in_right);
        }

        bool operator()(
            key_type const& in_left,
            key_type const& in_right)
        const
        {
            return this->template_compare_type::operator()(in_left, in_right);
        }
    };
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_vector_type,
    typename template_compare_type =
        std::less<typename template_vector_type::value_type::first_type>>
class psyq::vector_map:
    public psyq::closed::vector_map_base<
        template_vector_type, template_compare_type>
{
    public: typedef psyq::vector_map<
        template_vector_type, template_compare_type>
            self;
    private: typedef psyq::closed::vector_map_base<
        template_vector_type, template_compare_type>
            super;

    public: vector_map()
    :
        super()
    {
        // pass
    }

    mapped_type& at(
        key_type const& in_key)
    {
        return const_cast<mapped_type&>(
            const_cast<const self*>(this)->at(in_key));
    }

    mapped_type const& at(
        key_type const& in_key) const
    {
        super::const_iterator const local_position(this->lower_bound(in_key));
        PSYQ_ASSERT(
            local_position != this->end() && local_position->first == in_key);
        return local_position->second;
    }

    mapped_type& operator[](
        key_type const& in_key)
    {
        super::iterator const local_position(this->lower_bound(in_key));
        if (local_position != this->end() && local_position->first == in_key)
        {
            return local_position->second;
        }
    }

    public: std::pair<typename self::iterator, bool> insert(
        typename self::value_type const& in_value)
    {
        self::value_type local_value(in_value);
        return this->insert(local_value);
    }

    public: std::pair<typename self::iterator, bool> insert(
        typename self::value_type& in_value)
    {
        typename super::iterator const local_position(
            this->lower_bound(in_value.first));
        if (local_position != this->end() &&
            in_value.first == local_position->first)
        {
            return std::make_pair(local_position, false);
        }

        typename super::size_type const local_insert_index(
            std::distance(this->begin(), local_position));
        this->super::push_back(std::make_pair(key_type(), mapped_type()));
        super::swap_value(this->super::back(), in_value);
        this->super::move_from_back(local_insert_index);
        return std::make_pair(this->begin() + local_insert_index, true);
    }

    public: template<typename template_iterator_type>
    void insert(
        template_iterator_type const& in_begin,
        template_iterator_type const& in_end)
    {
        for (
            template_iterator_type i(in_begin);
            i != in_end;
            ++i)
        {
            this->insert(*i);
        }
    }

    public: void swap(
        self& io_target)
    {
        this->super::swap(io_target);
    }
};

#endif // !PSYQ_VECTOR_MAP_HPP_
