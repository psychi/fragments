/** @file
    @brief 
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_TEST_HPP_
#define PSYQ_SCENARIO_ENGINE_TEST_HPP_

#include "./driver.hpp"
#include "./state_builder.hpp"
#include "./expression_builder.hpp"
#include "./behavior_builder.hpp"
#include "../string/storage.hpp"
#include "../string/csv_table.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void scenario_engine()
    {
        // シナリオ駆動機を構築する。
        typedef psyq::scenario_engine::driver<> driver;
        driver local_driver(16, 16, 16);

        // 文字列表の構築に使うフライ級文字列生成器を構築する。
        typedef psyq::string::relation_table<char> relation_table;
        auto const local_string_factory(
            std::allocate_shared<typename relation_table::string::factory>(
                relation_table::allocator_type(),
                PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT,
                relation_table::allocator_type()));

        // 状態値CSV文字列を構築する。
        typedef psyq::string::csv_table<char> csv_table;
        csv_table::string::view const local_csv_state(
            "KEY,            KIND,      VALUE,\n"
            "state_bool,     BOOL,       TRUE,\n"
            "state_unsigned, UNSIGNED_7,   10,\n"
            "state_signed,   SIGNED_13,   -20,\n"
            "state_float,    FLOAT,      1.25,\n"
            "");

        // 条件式CSV文字列を構築する。
        csv_table::string::view const local_csv_expression(
            "KEY,          LOGIC, KIND,             ELEMENT,\n"
            "expression_0, AND,   STATE_COMPARISON, state_bool,     ==, FALSE,\n"
            "expression_1, AND,   STATE_COMPARISON, state_unsigned, <=, 10,\n"
            "expression_2, AND,   STATE_COMPARISON, state_signed,   >=, -20,\n"
            "expression_3, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_4, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_5, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_6, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_7, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_8, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_9, OR,    SUB_EXPRESSION,   expression_0, TRUE, expression_1, FALSE,\n"
            "");

        // 条件挙動CSV文字列を構築する。
        csv_table::string::view const local_csv_behavior(
            "KEY         , CONDITION, PRIORITY, KIND, ARGUMENT\n"
            "expression_0, TRUE,      9,       STATE, state_unsigned, :=, 1\n"
            "expression_1, TRUE,      8,       STATE, state_unsigned, +=, 1\n"
            "expression_2, TRUE,      7,       STATE, state_unsigned, -=, 1\n"
            "expression_3, TRUE,      6,       STATE, state_unsigned, *=, 1\n"
            "expression_4, TRUE,      5,       STATE, state_unsigned, /=, 1\n"
            "expression_5, TRUE,      4,       STATE, state_unsigned, %=, 1\n"
            "expression_6, TRUE,      3,       STATE, state_unsigned, |=, 1\n"
            "expression_7, TRUE,      2,       STATE, state_unsigned, ^=, 0\n"
            "expression_8, TRUE,      1,       STATE, state_unsigned, &=, 0\n"
            "");

        // シナリオ駆動機に条件挙動を登録する。
        auto const local_chunk_key(local_driver.hash_function_("chunk_0"));
        psyq::string::storage<csv_table::string::view::value_type>
            local_csv_workspace;
        local_driver.extend_chunk(
            local_chunk_key,
            psyq::scenario_engine::state_builder<relation_table>(
                relation_table(
                    csv_table(
                        local_csv_workspace,
                        local_csv_state,
                        local_string_factory),
                    0)),
            psyq::scenario_engine::expression_builder<relation_table>(
                relation_table(
                    csv_table(
                        local_csv_workspace,
                        local_csv_expression,
                        local_string_factory),
                    0)),
            psyq::scenario_engine::behavior_builder<
                relation_table, driver::dispatcher>(
                    relation_table(
                        csv_table(
                            local_csv_workspace,
                            local_csv_behavior,
                            local_string_factory),
                        0)));
        PSYQ_ASSERT(
            local_driver.reservoir_.register_value(
                local_chunk_key,
                local_driver.hash_function_("10"),
                driver::reservoir::state_value(10u)));
        PSYQ_ASSERT(!local_driver.extend_chunk(0, 0, nullptr));
        local_driver.shrink_to_fit();
        PSYQ_ASSERT(
            true == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_bool")).get_bool());
        PSYQ_ASSERT(
            10 == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_unsigned")).get_unsigned());
        PSYQ_ASSERT(
            -20 == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_signed")).get_signed());
        PSYQ_ASSERT(
            1.25 <= *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_float")).get_float());


        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_bool"), false);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_unsigned"), 10);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_signed"), -20);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), true);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), 0x20u);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), -10);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), 1.25f);
        auto const local_float_state(
            local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_float")));

        local_driver.progress();
        local_driver.erase_chunk(local_chunk_key);
    }
}

#endif // defined(PSYQ_SCENARIO_ENGINE_TEST_HPP_)
// vim: set expandtab:
