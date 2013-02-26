#ifndef PSYQ_VECTOR_MAP_HPP_
#define PSYQ_VECTOR_MAP_HPP_

namespace psyq
{
    /// @cond
    template<typename, typename> class vector_map;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_vector_type,
    typename template_compare_type =
std::less<typename template_vector_type::value_type::first_type>>
class psyq::vector_map:
    protected template_vector_type
{
    public: typedef psyq::vector_map<
        template_vector_type, template_compare_type>
            self;
    public: typedef template_vector_type super;

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

    public: vector_map()
    :
        super()
    {
        // pass
    }

    public: void reserve(
        typename super::size_type in_capacity)
    {
        this->super::capacity();
    }
/*
    std::pair<typename super::iterator, bool> insert(
        typename super::value_type const& in_value)
    {
        typename super::iterator const local_position(
            this->lower_bound(in_value.first));
        if (local_position != this->end() &&
            in_value.first == local_position->first)
        {
            return std::make_pair(local_position, false);
        }

        typename super::size_type const local_last_size(this->size());
        typename super::size_type const local_insert_index(
            std::distance(this->begin(), local_position));
        this->super::resize(local_last_size + 1);
        for (
            typename super::size_type i = local_last_size;
            local_insert_index < i;
            --i)
        {
            std::swap(this->super::at(i), this->super::at(i - 1));
        }
        
    }
*/
    public: typename super::const_iterator lower_bound(
        typename self::key_type const& in_key)
    const
    {
        return std::lower_bound(
            this->begin(), this->end(), in_key, self::compare());
    }

    //-------------------------------------------------------------------------
    private: struct compare:
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
            key_type const&   in_left,
            typename template_vector_type::value_type const& in_right)
        const
        {
            return this->operator()(in_left, in_right.second);
        }

        bool operator()(
            typename template_vector_type::value_type const& in_left,
            key_type const&   in_right)
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

#endif // !PSYQ_VECTOR_MAP_HPP_
