/** @file
    @brief 
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_TEST_HPP_
#define PSYQ_IF_THEN_ENGINE_TEST_HPP_

#include "./driver.hpp"
#include "../string/storage.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void if_then_engine()
    {
        // 駆動機を構築する。
        typedef psyq::if_then_engine::driver<> driver;
        driver local_driver(256, 256, 256);

        // 文字列表の構築に使うフライ級文字列生成器を構築する。
        typedef psyq::string::flyweight<char> flyweight_string;
        auto const local_string_factory(
            std::allocate_shared<typename flyweight_string::factory>(
                flyweight_string::allocator_type(),
                PSYQ_STRING_FLYWEIGHT_FACTORY_CAPACITY_DEFAULT,
                flyweight_string::allocator_type()));

        // 状態値CSV文字列を構築する。
        flyweight_string::view const local_csv_status(
            "KEY,             KIND,      VALUE,\n"
            "status_bool,     BOOL,       TRUE,\n"
            "status_unsigned, UNSIGNED_7,   10,\n"
            "status_signed,   SIGNED_13,   -20,\n"
            "status_float,    FLOAT,      1.25,\n"
            "");

        // 条件式CSV文字列を構築する。
        flyweight_string::view const local_csv_expression(
            "KEY,          LOGIC, KIND,              ELEMENT,\n"
            "expression_0, AND,   STATUS_COMPARISON, status_bool,     ==, FALSE,\n"
            "expression_1, AND,   STATUS_COMPARISON, status_unsigned, <=, 10,\n"
            "expression_2, AND,   STATUS_COMPARISON, status_signed,   >=, STATUS:status_unsigned,\n"
            "expression_3, AND,   STATUS_COMPARISON, status_unsigned, ==, 30,\n"
            "expression_4, AND,   STATUS_COMPARISON, status_unsigned, ==, 40,\n"
            "expression_5, AND,   STATUS_COMPARISON, status_unsigned, ==, 50,\n"
            "expression_6, AND,   STATUS_COMPARISON, status_unsigned, ==, 60,\n"
            "expression_7, AND,   STATUS_COMPARISON, status_unsigned, ==, 70,\n"
            "expression_8, AND,   STATUS_COMPARISON, status_unsigned, ==, 80,\n"
            "expression_9, OR,    SUB_EXPRESSION,   expression_0, TRUE, expression_1, FALSE,\n"
            "");

        // 条件挙動CSV文字列を構築する。
        flyweight_string::view const local_csv_behavior(
            "KEY         , CONDITION,,,,,, PRIORITY, KIND, ARGUMENT\n"
            "expression_0, TRUE,,,,TRUE,,  9,      STATUS, status_unsigned, :=, 1, status_unsigned, +=, STATUS:status_unsigned\n"
            "expression_1, TRUE,,,,TRUE,,  8,      STATUS, status_unsigned, +=, 1\n"
            "expression_2, TRUE,,,,TRUE,,  7,      STATUS, status_unsigned, -=, 1\n"
            "expression_3, TRUE,,,,TRUE,,  6,      STATUS, status_unsigned, *=, 1\n"
            "expression_4, TRUE,,,,TRUE,,  5,      STATUS, status_unsigned, /=, 1\n"
            "expression_5, TRUE,,,,TRUE,,  4,      STATUS, status_unsigned, %=, 1\n"
            "expression_6, TRUE,,,,TRUE,,  3,      STATUS, status_unsigned, |=, 1\n"
            "expression_7, TRUE,,,,TRUE,,  2,      STATUS, status_unsigned, ^=, 0\n"
            "expression_8, TRUE,,,,TRUE,,  1,      STATUS, status_unsigned, &=, 0\n"
            "");

        // 状態値と条件式と条件挙動を、駆動器に登録する。
        auto const local_chunk_key(local_driver.hash_function_("chunk_0"));
        psyq::string::storage<flyweight_string::view::value_type>
            local_workspace_string;
        local_driver.extend_chunk(
            local_workspace_string,
            local_string_factory,
            local_chunk_key,
            local_csv_status,
            0,
            local_csv_expression,
            0,
            local_csv_behavior,
            0);
        PSYQ_ASSERT(
            local_driver.reservoir_.register_status(
                local_chunk_key,
                local_driver.hash_function_("10"),
                driver::reservoir::status(
                    static_cast<driver::reservoir::status::unsigned_type>(
                        10u))));
        //PSYQ_ASSERT(!local_driver.extend_chunk(0, 0, nullptr));
        local_driver.shrink_to_fit();

        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_bool")).compare(
                    driver::reservoir::status::comparison_EQUAL,
                    true));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_unsigned")).compare(
                    driver::reservoir::status::comparison_EQUAL,
                    10u));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_signed")).compare(
                    driver::reservoir::status::comparison_EQUAL,
                    -20));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_float")).compare(
                    driver::reservoir::status::comparison_GREATER_EQUAL,
                    1.25));
        local_driver.progress();

        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_bool"), false);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_unsigned"), 10);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_signed"), -20);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_float"), true);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_float"), 0x20u);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_float"), -10);
        local_driver.reservoir_.assign_status(
            local_driver.hash_function_("status_float"), 1.25f);
        auto const local_float_status(
            local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_float")));

        local_driver.progress();
        local_driver.erase_chunk(local_chunk_key);
    }
}

#endif // defined(PSYQ_IF_THEN_ENGINE_TEST_HPP_)
// vim: set expandtab:
