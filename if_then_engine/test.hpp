/** @file
    @brief 
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_TEST_HPP_
#define PSYQ_IF_THEN_ENGINE_TEST_HPP_

#include "./driver.hpp"
#include "../string/storage.hpp"
#include "../static_deque.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace tesv
{
    typedef psyq::string::flyweight<char> flyweight_string;
    typedef tesv::flyweight_string::view string_view;
    typedef tesv::flyweight_string::factory::hash string_hash;
    typedef psyq::string::storage<tesv::string_view::value_type> string_storage;
    typedef float float32_t;
    typedef std::allocator<void*> void_allocator;
    typedef
        psyq::string::csv_table<
            tesv::string_view::value_type,
            tesv::string_view::traits_type,
            tesv::void_allocator>
        csv_table;
    typedef
        psyq::string::relation_table<
            tesv::string_view::value_type,
            tesv::string_view::traits_type,
            tesv::void_allocator>
        relation_table;

    namespace asset
    {
        class efficacy_property;
        class item_property;
        class item_efficacy;
        class cooking_recipi;
        class food_efficacy;
    } // namespace item
} // namespace tesv

#define TESV_ITEM_PROPERTY_TABLE_COLUMN_KEY "KEY"
#define TESV_ITEM_PROPERTY_TABLE_COLUMN_WEIGHT "WEIGHT"
#define TESV_ITEM_PROPERTY_TABLE_COLUMN_PRICE "PRICE"
#define TESV_ITEM_PROPERTY_TABLE_COLUMN_CATEGORY "CATEGORY"
#define TESV_FOOD_EFFECT_TABLE_COLUMN_KEY "KEY"
#define TESV_FOOD_EFFECT_TABLE_COLUMN_HEALTH "HEALTH"
#define TESV_FOOD_EFFECT_TABLE_COLUMN_STAMINA "STAMINA"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#ifdef TABLE_ATTRIBUTE
class table_attribute
{
    public:
    explicit table_attribute(RELATION_TABLE const& in_table) PSYQ_NOEXCEPT:
    ATTRIBUTE_0(in_table.find_attribute(#ATTRIBUTE_0)),
    ATTRIBUTE_1(in_table.find_attribute(#ATTRIBUTE_1)),
    ATTRIBUTE_2(in_table.find_attribute(#ATTRIBUTE_2)),
    ATTRIBUTE_3(in_table.find_attribute(#ATTRIBUTE_3))
    {}
    RELATION_TABLE ::attribute ATTRIBUTE_0;
    RELATION_TABLE ::attribute ATTRIBUTE_1;
    RELATION_TABLE ::attribute ATTRIBUTE_2;
    RELATION_TABLE ::attribute ATTRIBUTE_3;
};
#endif

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class tesv::asset::item_property
{
    private: typedef item_property this_type;

    public: typedef std::vector<this_type, tesv::void_allocator> container;

    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: explicit table_attribute(tesv::relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(in_table.find_attribute(TESV_ITEM_PROPERTY_TABLE_COLUMN_KEY)),
        weight_(in_table.find_attribute(TESV_ITEM_PROPERTY_TABLE_COLUMN_WEIGHT)),
        price_(in_table.find_attribute(TESV_ITEM_PROPERTY_TABLE_COLUMN_PRICE)),
        category_(in_table.find_attribute(TESV_ITEM_PROPERTY_TABLE_COLUMN_CATEGORY))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 1 <= this->weight_.second
                && 1 <= this->price_.second
                && 1 <= this->category_.second;
        }

        public: tesv::relation_table::attribute key_;
        public: tesv::relation_table::attribute weight_;
        public: tesv::relation_table::attribute price_;
        public: tesv::relation_table::attribute category_;
    };

    //-------------------------------------------------------------------------
    public: static tesv::string_view csv_string()
    {
        return
            "WEIGHT, PRICE, CATEGORY,    KEY,\n"
            "   0.2,     3, raw meat,    chicken breast,\n"
            "   0.1,     0, raw meat,    clam meat,\n"
            "   0.2,     3, raw meat,    dog meat,\n"
            "   3  ,     6, raw meat,    freash meat,\n"
            "   1  ,     3, raw meat,    horker meat,\n"
            "   2  ,     3, raw meat,    horse meat,\n"
            "   1  ,     3, raw meat,    leg of goat,\n"
            "   3  ,     6, raw meat,    mammoth snout,\n"
            "   0.1,     3, raw meat,    mudcrab legs,\n"
            "   0.2,     3, raw meat,    pheasant breast,\n"
            "   0.2,     4, raw meat,    raw beaf,\n"
            "   0.1,     2, raw meat,    raw rabbit leg,\n"
            "   0.1,     3, raw meat,    salmon meat,\n"
            "   2  ,     4, raw meat,    venison,\n"
            "   0.25,    2, vegetables,  cabbage,\n"
            "   0.1,     1, vegetables,  carrot,\n"
            "   0.2,     1, vegetables,  gourd,\n"
            "   0.1,     3, vegetables,  green apple,\n"
            "   0.1,     1, vegetables,  leek,\n"
            "   0.1,     1, vegetables,  potato,\n"
            "   0.1,     3, vegetables,  red apple,\n"
            "   0.1,     4, vegetables,  tomato,\n"
            "   0.1,     1, baking food, butter,\n"
            "   1  ,     2, baking food, jug of milk,\n"
            "   0.5,     1, baking food, sack of flour,\n"
            "";
    }

    /** @brief CSV文字列を解析し、アイテム特質のコンテナを構築する。
        @param[in] in_string_factory
            構築に使うライト級文字列生成器を指すスマートポインタ。
            必ず空ではないこと。
        @param[in] in_csv_string 解析するCSV文字列。
        @param[in] in_allocator  解析する文字列表のアイテム特質の属性。
        @return 構築したアイテム特質のコンテナ。
     */
    public: static this_type::container build(
        tesv::flyweight_string::factory::shared_ptr const& in_string_factory,
        tesv::string_view const& in_csv_string,
        tesv::void_allocator const& in_allocator)
    {
        // CSV文字列から文字列表を構築する。
        this_type::container local_container(in_allocator);
        tesv::string_storage local_workspace;
        tesv::relation_table local_table(
            tesv::csv_table(local_workspace, in_string_factory, in_csv_string),
            0);
        this_type::table_attribute const local_attribute(local_table);
        if (!local_attribute.is_valid())
        {
            // 文字列表の属性が、アイテム特質として適切でなかった。
            PSYQ_ASSERT(false);
            return local_container;
        }

        // アイテム特質のコンテナを構築する。
        auto const local_row_count(local_table.get_row_count());
        local_container.reserve(local_row_count);
        this_type local_instance;
        for (auto i(local_row_count); 0 < i; --i)
        {
            auto const local_row(i - 1);
            if (local_row != local_table.get_attribute_row()
                && this_type::build_instance(
                    local_instance, local_table, local_row, local_attribute))
            {
                local_container.push_back(local_instance);
            }
        }
        local_container.shrink_to_fit();
        return local_container;
    }

    //-------------------------------------------------------------------------
    private: item_property() PSYQ_NOEXCEPT {}

    /** @brief 文字列表を解析し、アイテム特質を構築する。
        @param[out] out_instance 構築したアイテム特質の出力先。
        @param[in] in_table      解析する文字列表。
        @param[in] in_row_index  解析する文字列表の行番号。
        @param[in] in_attribute  解析する文字列表のアイテム特質の属性。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool build_instance(
        this_type& out_instance,
        tesv::relation_table const& in_table,
        std::size_t const in_row_index,
        this_type::table_attribute const& in_attribute)
    {
        // アイテム識別名を取得する。
        out_instance.key_ = in_table.find_cell(
            in_row_index, in_attribute.key_.first);
        if (out_instance.key_.empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // カテゴリ識別名を取得する。
        out_instance.category_ = in_table.find_cell(
            in_row_index, in_attribute.category_.first);
        if (out_instance.category_.empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 重量を取得する。
        if (!in_table.parse_cell(
                out_instance.weight_,
                in_row_index,
                in_attribute.weight_.first,
                false))
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 価格を取得する。
        if (!in_table.parse_cell(
                out_instance.price_,
                in_row_index,
                in_attribute.price_.first,
                false))
        {
            PSYQ_ASSERT(false);
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief アイテム識別名。
    public: tesv::flyweight_string key_;
    /// @brief カテゴリ識別名。
    public: tesv::flyweight_string category_;
    /// @brief アイテムの重量。
    public: tesv::float32_t weight_;
    /// @brief アイテムの価格。
    public: std::uint32_t price_;

}; // class tesv::asset::item_property

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 料理のレシピ。
 */
class tesv::asset::cooking_recipi
{
    private: typedef cooking_recipi this_type;

    public: enum: std::uint8_t
    {
        MAX_INGREDIENTS = 4, ///< レシピで使う食材の最大数。
    };

    public: typedef
        psyq::static_deque<tesv::flyweight_string, this_type::MAX_INGREDIENTS>
        ingredient_container;

    public: static tesv::string_view csv_string()
    {
        return
            "KEY,                      INGREDIENT,\n"
            "apple cabbage stew,       cabbage,         red apple,          salt pile,\n"
            "beef stew,                carrot,          garlic,             raw beef,    salt pile,\n"
            "cabbage potate soup,      cabbage,         leek,               potate,      salt pile,\n"
            "cabbage soup,             cabbage,         salt pile,\n"
            "clam chawder,             clam meat,       potate,             jug of milk, butter,\n"
            "cooked beef,              raw beef,        salt pile,\n"
            "elsweyr fondue,           ale,             eidar cheese wheel, moon sugar,\n"
            "grilled chicken breast,   chiken breast,   salt pile,\n"
            "horker and ash yam stew,  ash yam,         horker meat,        garlic,\n"
            "horker loaf,              horker meat,     salt pile,\n"
            "horker stew,              garlic,          horker meat,        lavender,    tomato,\n"
            "horse haunch,             horse meat,      salt pile,\n"
            "leg of goat roast,        leg of goat,     salt pile,\n"
            "mammoth staek,            mammoth snout,   salt pile,\n"
            "pheasant roast,           pheasant breast, salt pile,\n"
            "potate soup,              potate,          salt pile,\n"
            "rabbit haunch,            raw rabbit leg,  salt pile,\n"
            "salmon steak,             salmon meat,     salt pile,\n"
            "steamed mudcrab legs,     mudclab legs,    butter,\n"
            "tomato soup,              garlic,          leek,               salt pile,   tomato,\n"
            "vegetable soup,           cabbage,         leek,               potate,      tomato,\n"
            "venison chop,             salt pile,       venison,\n"
            "venison stew,             leek,            potate,             salt pile,   venison,\n"
            "";
    }

    public: tesv::flyweight_string key_;
    public: this_type::ingredient_container ingredients_;

}; // class tesv::asset::cooking_recipi

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 食料の効能。
 */
class tesv::asset::food_efficacy
{
    private: typedef food_efficacy this_type;

    /// @brief 食料使用時の回復効果。
    public: class recovery
    {
        /// @brief 即効性の回復量。
        public: std::uint32_t point_;
        /// @brief 時限制の回復秒数。
        public: std::uint32_t time_;
        /// @brief 時限制の単位時間あたりの回復量。
        public: std::uint32_t time_point_;
    };

    public: typedef std::vector<this_type, tesv::void_allocator> container;

    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: explicit table_attribute(tesv::relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(in_table.find_attribute(TESV_FOOD_EFFECT_TABLE_COLUMN_KEY)),
        health_(in_table.find_attribute(TESV_FOOD_EFFECT_TABLE_COLUMN_HEALTH)),
        stamina_(in_table.find_attribute(TESV_FOOD_EFFECT_TABLE_COLUMN_STAMINA))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 3 <= this->health_.second
                && 3 <= this->stamina_.second;
        }

        public: tesv::relation_table::attribute key_;
        public: tesv::relation_table::attribute health_;
        public: tesv::relation_table::attribute stamina_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    public: static tesv::string_view csv_string()
    {
        return
            "HEALTH,,,   STAMINA,,,  KEY,\n"
            "10,  ,    , 15,  ,    , apple cabbage stew,\n"
            "10,  ,    ,   ,  ,    , apple Pie,\n"
            " 2,  ,    ,   ,  ,    , Ash Hopper leg,\n"
            " 2,  ,    ,   ,  ,    , Ash Hopper Meet,\n"
            " 1,  ,    ,   ,  ,    , Ash Yam,\n"
            " 5,  ,    ,   ,  ,    , Baked Potatoes,\n"
            "  ,  ,    ,   , 2, 720, Beef stew,\n"
            " 2,  ,    ,   ,  ,    , Boar meat,\n"
            " 2,  ,    ,   ,  ,    , Bread,\n"

            " 1,  ,    ,   ,  ,    , horker meat,\n"
            "15, 1, 720, 15,  ,    , horker stew,\n"

            "  , 1, 720,   , 1, 720, vegetable Soup,\n"
            " 2,  ,    ,   ,  ,    , venison,\n"
            " 5,  ,    ,   ,  ,    , venison Chop,\n"
            "  , 1, 720, 15, 1, 720, venison stew,\n"
            "";
    }

    /** @brief CSV文字列を解析し、食料効能のコンテナを構築する。
        @param[in] in_string_factory
            構築に使うライト級文字列生成器を指すスマートポインタ。
            必ず空ではないこと。
        @param[in] in_csv_string 解析するCSV文字列。
        @param[in] in_allocator  解析する文字列表の食料効能の属性。
        @return 構築した食料効能のコンテナ。
     */
    public: static this_type::container build(
        tesv::flyweight_string::factory::shared_ptr const& in_string_factory,
        tesv::string_view const& in_csv_string,
        tesv::void_allocator const& in_allocator)
    {
        // CSV文字列から文字列表を構築する。
        this_type::container local_container(in_allocator);
        tesv::string_storage local_workspace;
        tesv::relation_table local_table(
            tesv::csv_table(local_workspace, in_string_factory, in_csv_string),
            0);
        this_type::table_attribute const local_attribute(local_table);
        if (!local_attribute.is_valid())
        {
            // 文字列表の属性が、食料アイテム特質として適切でなかった。
            PSYQ_ASSERT(false);
            return local_container;
        }

        // 食料効能のコンテナを構築する。
        auto const local_row_count(local_table.get_row_count());
        local_container.reserve(local_row_count);
        this_type local_instance;
        for (auto i(local_row_count); 0 < i; --i)
        {
            auto const local_row(i - 1);
            if (local_row != local_table.get_attribute_row()
                && this_type::build_instance(
                    local_instance, local_table, local_row, local_attribute))
            {
                local_container.push_back(local_instance);
            }
        }
        local_container.shrink_to_fit();
        return local_container;
    }

    //-------------------------------------------------------------------------
    private: food_efficacy() PSYQ_NOEXCEPT {}

    /** @brief 文字列表を解析し、食料効能を構築する。
        @param[out] out_instance 構築した食料効能の出力先。
        @param[in] in_table      解析する文字列表。
        @param[in] in_row_index  解析する文字列表の行番号。
        @param[in] in_attribute  解析する文字列表の食料効能の属性。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool build_instance(
        this_type& out_instance,
        tesv::relation_table const& in_table,
        std::size_t const in_row_index,
        this_type::table_attribute const& in_attribute)
    {
        // アイテム識別名を取得する。
        out_instance.key_ = in_table.find_cell(
            in_row_index, in_attribute.key_.first);
        if (out_instance.key_.empty())
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // ヘルス回復量を取得する。
        auto const local_build_health(
            this_type::build_recovery(
                out_instance.health_,
                in_table,
                in_row_index,
                in_attribute.health_.first));
        if (!local_build_health)
        {
            return false;
        }

        // スタミナ回復量を取得する。
        return this_type::build_recovery(
            out_instance.stamina_,
            in_table,
            in_row_index,
            in_attribute.stamina_.first);
    }

    /** @brief 文字列表を解析し、食料の回復特質を構築する。
        @param[out] out_instance 構築した回復特質の出力先。
        @param[in] in_table        解析する文字列表。
        @param[in] in_row_index    解析する文字列表の行番号。
        @param[in] in_column_index 解析する文字列表の列番号。
        @retval true  成功。
        @retval false 失敗。
     */
    private: static bool build_recovery(
        this_type::recovery& out_recovery,
        tesv::relation_table const& in_table,
        std::size_t const in_row_index,
        std::size_t const in_column_index)
    {
        // 即効性の回復量を取得する。
        out_recovery.point_ = 0;
        if (!in_table.parse_cell(
                out_recovery.point_, in_row_index, in_column_index, true))
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 時限制の回復量を取得する。
        out_recovery.time_point_ = 0;
        if (!in_table.parse_cell(
                out_recovery.time_point_, in_row_index, in_column_index + 1, true))
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 時限制の回復時間を取得する。
        out_recovery.time_ = 0;
        if (!in_table.parse_cell(
                out_recovery.time_, in_row_index, in_column_index + 2, true))
        {
            PSYQ_ASSERT(false);
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief アイテム識別名。
    public: tesv::flyweight_string key_;
    /// @brief 使用時のヘルス回復量。
    public: this_type::recovery health_;
    /// @brief 使用時のスタミナ回復料。
    public: this_type::recovery stamina_;

}; // class tesv::asset::food_efficacy

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void if_then_engine()
    {
        std::uint64_t local_uint64(10);
        std::int8_t local_int8(-1);
        auto const local_mult(local_uint64 * local_int8);

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

        auto const local_item_properties(
            tesv::asset::item_property::build(
                local_string_factory,
                tesv::asset::item_property::csv_string(),
                tesv::void_allocator()));
        auto const local_food_potencies(
            tesv::asset::food_efficacy::build(
                local_string_factory,
                tesv::asset::food_efficacy::csv_string(),
                tesv::void_allocator()));

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
            "KEY         , CONDITION,,,,,, PRIORITY,       KIND, ARGUMENT\n"
            "expression_0, TRUE,,,, TRUE,, 9, STATUS_ASSIGNMENT, status_unsigned, :=, 1, status_unsigned, +=, STATUS:status_unsigned\n"
            "expression_1, TRUE,,,, TRUE,, 8, STATUS_ASSIGNMENT, status_unsigned, +=, 1\n"
            "expression_2, TRUE,,,, TRUE,, 7, STATUS_ASSIGNMENT, status_unsigned, -=, 1\n"
            "expression_3, TRUE,,,, TRUE,, 6, STATUS_ASSIGNMENT, status_unsigned, *=, 1\n"
            "expression_4, TRUE,,,, TRUE,, 5, STATUS_ASSIGNMENT, status_unsigned, /=, 1\n"
            "expression_5, TRUE,,,, TRUE,, 4, STATUS_ASSIGNMENT, status_unsigned, %=, 1\n"
            "expression_6, TRUE,,,, TRUE,, 3, STATUS_ASSIGNMENT, status_unsigned, |=, 1\n"
            "expression_7, TRUE,,,, TRUE,, 2, STATUS_ASSIGNMENT, status_unsigned, ^=, 0\n"
            "expression_8, TRUE,,,, TRUE,, 1, STATUS_ASSIGNMENT, status_unsigned, &=, 0\n"
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
                driver::reservoir::status_value(
                    static_cast<driver::reservoir::status_value::unsigned_type>(
                        10u))));
        //PSYQ_ASSERT(!local_driver.extend_chunk(0, 0, nullptr));
        local_driver.rebuild(1024, 1024, 1024);

        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_bool")).compare(
                    driver::reservoir::status_value::comparison_EQUAL,
                    true));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_unsigned")).compare(
                    driver::reservoir::status_value::comparison_EQUAL,
                    10u));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_signed")).compare(
                    driver::reservoir::status_value::comparison_EQUAL,
                    -20));
        PSYQ_ASSERT(
            0 < local_driver.reservoir_.extract_status(
                local_driver.hash_function_("status_float")).compare(
                    driver::reservoir::status_value::comparison_GREATER_EQUAL,
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
