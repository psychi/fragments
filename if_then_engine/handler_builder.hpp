/// @file
/// @brief @copybrief psyq::if_then_engine::handler_builder
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_

/// @brief 文字列表で、条件式の識別値が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::evaluation::expression_key として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、挙動条件として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION "CONDITION"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)

/// @brief 文字列表で、NULL条件が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::dispatcher::handler::unit_condition_NULL
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL "NULL"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、ANY条件が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::dispatcher::handler::unit_condition_ANY
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY "ANY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、条件挙動関数の優先順位が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::dispatcher::handler::priority
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY "PRIORITY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)

/// @brief 文字列表で、条件挙動の種別として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、代入演算する条件挙動の種別として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT "STATUS_ASSIGNMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)

/// @brief 文字列表で、条件挙動の引数として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT "ARGUMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @brief 文字列表で、遅延種別をYIELDとして解析する文字列。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::accumulator::delay_YIELD
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD "YIELD"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD)

/// @brief 文字列表で、遅延種別をBLOCKとして解析する文字列。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::accumulator::delay_BLOCK
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK "BLOCK"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK)

/// @brief 文字列表で、遅延種別をNONBLOCKとして解析する文字列。
/// @details
/// psyq::if_then_engine::handler_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::accumulator::delay_NONBLOCK
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK "NONBLOCK"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        class handler_builder;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件挙動ハンドラを構築して登録する関数オブジェクト。
/// @details driver::extend_chunk の引数として使う。
class psyq::if_then_engine::handler_builder
{
    /// @brief this が指す値の型。
    private: typedef handler_builder this_type;

    //-------------------------------------------------------------------------
    /// @brief 条件挙動ハンドラの構築に使う文字列表の属性。
    private: template<typename template_relation_table>
    class table_attribute
    {
        public: table_attribute(template_relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)),
        condition_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)),
        priority_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)),
        kind_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)),
        argument_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT))
        {}

        bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 1 <= this->condition_.second
                && 1 <= this->priority_.second
                && 1 <= this->kind_.second
                && 1 <= this->argument_.second;
        }

        /// @brief 条件挙動ハンドラに対応する条件式の識別値の列番号と列数。
        public: typename template_relation_table::attribute key_;
        /// @brief 挙動条件の列番号と列数。
        public: typename template_relation_table::attribute condition_;
        /// @brief 条件挙動関数の呼び出し優先順位の列番号と列数。
        public: typename template_relation_table::attribute priority_;
        /// @brief 条件挙動関数の種類の列番号と列数。
        public: typename template_relation_table::attribute kind_;
        /// @brief 条件挙動関数の引数の列番号と列数。
        public: typename template_relation_table::attribute argument_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /// @copydoc this_type::register_handlers
    public: template<
        typename template_dispatcher,
        typename template_hasher,
        typename template_accumulator,
        typename template_relation_table>
    std::vector<
        typename template_dispatcher::handler::function_shared_ptr,
        typename template_dispatcher::allocator_type>
    operator()(
        /// [in,out] 条件挙動ハンドラを登録する条件挙動器。
        template_dispatcher& io_dispatcher,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動関数で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 条件挙動関数が記述されている psyq::string::relation_table 。
        /// 空の場合は条件挙動関数は構築されない。
        template_relation_table const& in_table)
    const
    {
        return this_type::register_handlers(
            io_dispatcher, io_hasher, io_accumulator, in_table);
    }

    /// @brief 文字列表から条件挙動ハンドラを構築し、条件挙動器へ登録する。
    /// @details
    /// in_table から driver::dispatcher::handler::function
    /// を構築し、それを弱参照する driver::dispatcher::handler を
    /// io_dispatcher へ登録する。
    /// @return 構築した driver::dispatcher::handler::function の強参照のコンテナ。
    public: template<
        typename template_dispatcher,
        typename template_hasher,
        typename template_accumulator,
        typename template_relation_table>
    static std::vector<
        typename template_dispatcher::handler::function_shared_ptr,
        typename template_dispatcher::allocator_type>
    register_handlers(
        /// [in,out] 条件挙動ハンドラを登録する条件挙動器。
        template_dispatcher& io_dispatcher,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動ハンドラの関数で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 条件挙動関数が記述されている psyq::string::relation_table 。
        /// 空の場合は条件挙動関数は構築されない。
        template_relation_table const& in_table)
    {
        std::vector<
            typename template_dispatcher::handler::function_shared_ptr,
            typename template_dispatcher::allocator_type>
                local_functions(io_dispatcher.get_allocator());

        // 文字列表の属性を取得する。
        typename this_type::table_attribute<template_relation_table> const
            local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(in_table.get_cells().empty());
            return local_functions;
        }

        // 文字列表を解析し、条件挙動ハンドラを登録する。
        auto const local_empty_key(
            io_hasher(typename template_hasher::argument_type()));
        auto const local_row_count(in_table.get_row_count());
        local_functions.reserve(local_row_count);
        for (
            typename template_relation_table::number i(0);
            i < local_row_count;
            ++i)
        {
            if (i == in_table.get_attribute_row())
            {
                continue;
            }

            // 条件式の識別値を取得する。
            auto const local_expression_key(
                io_hasher(in_table.find_cell(i, local_attribute.key_.first)));
            if (local_expression_key == local_empty_key)
            {
                // 条件式の識別値が正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }

            // 条件挙動関数の優先順位を取得する。
            typedef typename template_dispatcher::handler handler;
            typename handler::priority local_priority(
                PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT);
            if (!in_table.parse_cell(
                    local_priority, i, local_attribute.priority_.first, true))
            {
                // 優先順位として解析しきれなかった。
                PSYQ_ASSERT(false);
                continue;
            }

            // 条件挙動関数を構築し、条件挙動ハンドラを条件挙動器に登録する。
            auto local_function(
                this_type::build_function<handler>(
                    io_hasher, io_accumulator, in_table, i, local_attribute));
            auto const local_condition(
                this_type::build_condition<handler>(
                    in_table, i, local_attribute.condition_));
            PSYQ_ASSERT(local_condition != handler::INVALID_CONDITION);
            if (io_dispatcher.register_handler(
                    local_expression_key,
                    local_condition,
                    local_function,
                    local_priority))
            {
                local_functions.push_back(std::move(local_function));
            }
            else
            {
                // 条件挙動ハンドラの登録に失敗した。
                PSYQ_ASSERT(false);
            }
        }
        local_functions.shrink_to_fit();
        return local_functions;
    }

    //-------------------------------------------------------------------------
    /// @name 挙動条件の構築
    /// @{

    /// @brief 文字列表の行を解析し、挙動条件を構築する。
    /// @return
    /// 文字列表から構築した挙動条件。構築に失敗した場合は
    /// driver::dispatcher::handler::INVALID_CONDITION を返す。
    /// @tparam template_handler
    /// 構築した挙動条件を使う driver::dispatcher::handler 。
    public: template<typename template_handler, typename template_relation_table>
    static typename template_handler::condition build_condition(
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] in_table で解析する行の番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で挙動条件が記述されている属性の列番号と列数。
        typename template_relation_table::attribute const& in_attribute)
    {
        if (in_attribute.second < 2)
        {
            PSYQ_ASSERT(false);
        }
        else
        {
            auto const local_last_condition(
                this_type::parse_unit_condition<template_handler>(
                        in_table.find_cell(in_row_number, in_attribute.first)));
            auto const local_current_condition(
                this_type::parse_unit_condition<template_handler>(
                        in_table.find_cell(in_row_number, in_attribute.first + 1)));
            auto const local_condition(
                template_handler::make_condition(
                    local_current_condition, local_last_condition));
            if (local_condition != template_handler::INVALID_CONDITION)
            {
                return local_condition;
            }
            PSYQ_ASSERT(false);
        }
        return template_handler::INVALID_CONDITION;
    }

    /// @brief 文字列を解析し、単位条件を取得する。
    /// @return
    /// 文字列から取得した driver::dispatcher::handler::unit_condition 。
    /// 取得に失敗した場合は
    /// driver::dispatcher::handler::INVALID_CONDITION を返す。
    /// @tparam template_handler
    /// 構築した単位条件を使う driver::dispatcher::handler 。
    public: template<typename template_handler, typename template_string>
    static typename template_handler::unit_condition parse_unit_condition(
        /// [in] 解析する std::basic_string_view 互換の文字列。
        template_string const& in_string)
    {
        if (!in_string.empty())
        {
            typedef
                psyq::string::view<
                    typename template_string::value_type,
                    typename template_string::traits_type>
                string_view;
            auto const local_not(in_string.at(0) == '!');
            auto const local_string(
                string_view(in_string).substr(local_not? 1: 0));
            psyq::string::numeric_parser<bool> const local_parser(local_string);
            if (local_parser.is_completed())
            {
                if (local_parser.get_value())
                {
                    return local_not?
                        template_handler::unit_condition_NOT_TRUE:
                        template_handler::unit_condition_TRUE;
                }
                else
                {
                    return local_not?
                        template_handler::unit_condition_NOT_FALSE:
                        template_handler::unit_condition_FALSE;
                }
            }
            else if (
                local_string ==
                    PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL)
            {
                return local_not?
                    template_handler::unit_condition_NOT_NULL:
                    template_handler::unit_condition_NULL;
            }
            else if (
                !local_not
                && local_string ==
                    PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY)
            {
                return template_handler::unit_condition_ANY;
            }
        }
        return template_handler::INVALID_CONDITION;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 条件挙動関数の構築
    /// @{

    /// @brief 文字列表の行を解析し、状態値を代入演算する条件挙動関数を構築する。
    /// @return
    /// 状態値を代入演算する driver::dispatcher::handler::function 。
    /// 代入演算が記述されてない場合は、空となる。
    /// @tparam template_handler
    /// 構築した条件挙動関数を使う driver::dispatcher::handler 。
    public: template<
        typename template_handler,
        typename template_hasher,
        typename template_accumulator,
        typename template_relation_table>
    static typename template_handler::function_shared_ptr
    build_status_assignment_function(
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動関数から使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] in_table の解析する行の番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で代入演算が記述されている属性の列番号と列数。
        typename template_relation_table::attribute const& in_assignments)
    {
        if (in_assignments.second < 1)
        {
            return typename template_handler::function_shared_ptr();
        }

        // 状態値の代入演算のコンテナを構築する。
        std::vector<
            typename template_accumulator::reservoir::status_assignment,
            typename template_accumulator::allocator_type>
                local_assignments(io_accumulator.get_allocator());
        template_accumulator::reservoir::status_assignment::_build_container(
            local_assignments,
            io_hasher,
            in_table,
            in_row_number,
            in_assignments.first + 1,
            in_assignments.second - 1);
        return this_type::create_status_assignment_function<template_handler>(
            io_accumulator,
            this_type::parse_delay<template_accumulator>(
                typename template_relation_table::string::view(
                    in_table.find_cell(in_row_number, in_assignments.first))),
            local_assignments);
    }

    /// @brief 状態値を代入演算する条件挙動関数を構築する。
    /// @return
    /// 状態値を代入演算する driver::dispatcher::handler::function 。
    /// in_assignments が空の場合は空を返す。
    /// @tparam template_handler
    /// 構築した条件挙動関数を使う driver::dispatcher::handler 。
    public: template<
        typename template_handler,
        typename template_accumulator,
        typename template_assignment_container>
    static typename template_handler::function_shared_ptr
    create_status_assignment_function(
        /// [in,out] 条件挙動関数から使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] in_assignments の先頭要素の driver::accumulator::accumulate
        /// に渡す、 driver::accumulator::delay 。
        typename template_accumulator::delay const in_front_delay,
        /// [in] 条件挙動関数から呼び出す driver::accumulator::accumulate
        /// に渡す、 driver::reservoir::status_assignment のコンテナ。
        template_assignment_container const& in_assignments)
    {
        return std::begin(in_assignments) != std::end(in_assignments)?
            std::allocate_shared<typename template_handler::function>(
                in_assignments.get_allocator(),
                /// @todo io_accumulator を参照渡しするのは危険。対策を考えたい。
                [=, &io_accumulator](
                    typename template_handler::expression_key const&,
                    typename template_handler::evaluation const,
                    typename template_handler::evaluation const)
                {
                    io_accumulator.accumulate(in_assignments, in_front_delay);
                }):
            typename template_handler::function_shared_ptr();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析し、条件挙動関数を構築する。
    /// @return 構築した条件挙動関数。
    /// @tparam template_handler
    /// 構築した条件挙動関数を使う driver::dispatcher::handler 。
    private: template<
        typename template_handler,
        typename template_hasher,
        typename template_accumulator,
        typename template_relation_table>
    static typename template_handler::function_shared_ptr
    build_function(
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] in_table の解析する行の番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table の属性。
        typename this_type::table_attribute<template_relation_table> const&
            in_attribute)
    {
        // 挙動関数の種類を取得する。
        typename template_relation_table::string::view const local_kind_cell(
            in_table.find_cell(in_row_number, in_attribute.kind_.first));
        if (local_kind_cell
            == PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)
        {
            return this_type::build_status_assignment_function<template_handler>(
                io_hasher,
                io_accumulator,
                in_table,
                in_row_number,
                in_attribute.argument_);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename template_handler::function_shared_ptr();
        }
    }

    private: template<typename template_accumulator, typename template_string>
    static typename template_accumulator::delay parse_delay(
        template_string const& in_string)
    {
        if (in_string == PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_BLOCK)
        {
            return template_accumulator::delay_BLOCK;
        }
        if (in_string == PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_NONBLOCK)
        {
            return template_accumulator::delay_NONBLOCK;
        }
        PSYQ_ASSERT(in_string == PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_DELAY_YIELD);
        return template_accumulator::delay_YIELD;
    }

}; // class psyq::if_then_engine::handler_builder

#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_)
// vim: set expandtab:
