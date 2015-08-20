/// @file
/// @brief @copybrief psyq::if_then_engine::driver
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_DRIVER_HPP_
#define PSYQ_IF_THEN_ENGINE_DRIVER_HPP_

#include "../string/csv_table.hpp"
#include "../string/relation_table.hpp"
#include "./reservoir.hpp"
#include "./accumulator.hpp"
#include "./evaluator.hpp"
#include "./dispatcher.hpp"
#include "./behavior_chunk.hpp"
#include "./status_builder.hpp"
#include "./expression_builder.hpp"
#include "./behavior_builder.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        template<typename, typename, typename, typename> class driver;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief if-then規則による有限状態機械の駆動器。
/// @details ### 使い方の概略
/// - driver::driver で駆動器を構築する。
/// - driver::extend_chunk で、状態値と条件式と挙動関数を登録する。
/// - driver::progress を時間フレーム毎に呼び出す。
///   条件式の評価が変化していたら、挙動関数が呼び出される。
/// @tparam template_float     @copydoc reservoir::status_value::float_type
/// @tparam template_priority  @copydoc dispatcher::function_priority
/// @tparam template_hasher    @copydoc hasher
/// @tparam template_allocator @copydoc allocator_type
template<
    typename template_float = float,
    typename template_priority = std::int32_t,
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::if_then_engine::driver
{
    /// @brief thisが指す値の型。
    protected: typedef driver this_type;

    //-------------------------------------------------------------------------
    /// @brief 文字列からハッシュ値を生成する、ハッシュ関数オブジェクトの型。
    /// @details
    /// - std::hash 互換インターフェイスを持つこと。
    /// - hasher::argument_type
    ///   が文字列型で、以下に相当するメンバ関数を使えること。
    ///   @code
    ///   // brief 文字列の先頭位置を取得する。
    ///   hasher::argument_type::const_pointer hasher::argument_type::data() const;
    ///   // brief 文字列の要素数を取得する。
    ///   std::size_t hasher::argument_type::size() const;
    ///   @endcode
    public: typedef template_hasher hasher;
    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 駆動器で用いる状態貯蔵器の型。
    public: typedef
        psyq::if_then_engine::_private::reservoir<
            template_float,
            typename this_type::hasher::result_type,
            typename this_type::hasher::result_type,
            typename this_type::allocator_type>
        reservoir;
    /// @brief 駆動器で用いる状態変更器の型。
    public: typedef
        psyq::if_then_engine::_private::accumulator<
            typename this_type::reservoir>
        accumulator;
    /// @brief 駆動器で用いる条件評価器の型。
    public: typedef
        psyq::if_then_engine::_private::evaluator<
            typename this_type::reservoir,
            typename this_type::hasher::result_type>
        evaluator;
    /// @brief 駆動器で用いる条件挙動器の型。
    public: typedef
        psyq::if_then_engine::_private::dispatcher<
            typename this_type::evaluator, template_priority>
        dispatcher;

    //-------------------------------------------------------------------------
    /// @brief 駆動器で用いる条件挙動チャンクの型。
    private: typedef
        psyq::if_then_engine::_private::behavior_chunk<
            typename this_type::dispatcher>
        behavior_chunk;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 空の駆動器を構築する。
    public: driver(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_buckets,
        /// [in] 状態値辞書のバケット数。
        std::size_t const in_status_buckets,
        /// [in] 条件式辞書のバケット数。
        std::size_t const in_expression_buckets,
        /// [in] 条件挙動キャッシュの予約数。
        std::size_t const in_reserve_caches = 16,
        /// [in] ハッシュ関数オブジェクトの初期値。
        typename this_type::hasher in_hash_function = this_type::hasher(),
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type()):
    reservoir_(in_chunk_buckets, in_status_buckets, in_allocator),
    accumulator_(in_reserve_caches, in_allocator),
    evaluator_(in_chunk_buckets, in_expression_buckets, in_allocator),
    dispatcher_(
        in_expression_buckets,
        in_status_buckets,
        in_reserve_caches,
        in_allocator),
    behavior_chunks_(in_allocator),
    hash_function_(std::move(in_hash_function))
    {
        this->behavior_chunks_.reserve(in_chunk_buckets);
    }

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: driver(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    reservoir_(std::move(io_source.reservoir_)),
    accumulator_(std::move(io_source.accumulator_)),
    evaluator_(std::move(io_source.evaluator_)),
    dispatcher_(std::move(io_source.dispatcher_)),
    behavior_chunks_(std::move(io_source.behavior_chunks_)),
    hash_function_(std::move(io_source.hash_function_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->reservoir_ = std::move(io_source.reservoir_);
        this->accumulator_ = std::move(io_source.accumulator_);
        this->evaluator_ = std::move(io_source.evaluator_);
        this->dispatcher_ = std::move(io_source.dispatcher_);
        this->behavior_chunks_ = std::move(io_source.behavior_chunks_);
        this->hash_function_ = std::move(io_source.hash_function_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 駆動器を再構築する。
    public: void rebuild(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_buckets,
        /// [in] 状態値辞書のバケット数。
        std::size_t const in_status_buckets,
        /// [in] 条件式辞書のバケット数。
        std::size_t const in_expression_buckets)
    {
        this->reservoir_.rebuild(in_chunk_buckets, in_status_buckets);
        //this->accumulator_.shrink_to_fit();
        this->evaluator_.rebuild(in_chunk_buckets, in_expression_buckets);
        this->dispatcher_.shrink_to_fit();
        this->behavior_chunks_.shrink_to_fit();
        for (auto& local_behavior_chunk: this->behavior_chunks_)
        {
            local_behavior_chunk.functions_.shrink_to_fit();
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name チャンク
    /// @{

    /// @brief 状態値と条件式と挙動関数を、チャンクへ追加する。
    public: template<
        typename template_status_builder,
        typename template_expression_builder,
        typename template_behavior_builder>
    void extend_chunk(
        /// [in] 追加するチャンクの識別値。
        typename this_type::reservoir::chunk_key const& in_chunk_key,

        /// [in] 状態値を状態貯蔵器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 状態値を状態貯蔵器に登録する。
        /// void template_status_builder::operator()(
        ///     // [in,out] 状態値を登録する状態貯蔵器。
        ///     driver::reservoir& io_reservoir,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in] 状態値を登録するチャンクを表す識別値。
        ///     driver::reservoir::chunk_key const& in_chunk_key)
        /// const;
        /// @endcode
        template_status_builder const& in_status_builder,

        /// [in] 条件式を条件評価器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 条件式を条件評価器に登録する。
        /// void template_expression_builder::operator()(
        ///     // [in,out] 条件式を登録する条件評価器。
        ///     driver::evaluator& io_evaluator,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in] 条件式を登録するチャンクを表す識別値。
        ///     driver::reservoir::chunk_key const& in_chunk_key,
        ///     // [in] 条件式で使う状態貯蔵器。
        ///     driver::reservoir const& in_reservoir)
        /// const;
        /// @endcode
        template_expression_builder const& in_expression_builder,

        /// [in] 挙動関数を条件挙動器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 挙動関数を条件挙動器に登録する。
        /// // return
        /// // 条件挙動器に登録した挙動関数オブジェクトを指す、
        /// // スマートポインタのコンテナ。
        /// template<typename template_function_shared_ptr_container>
        /// template_function_shared_ptr_container template_behavior_builder::operator()(
        ///     // [in,out] 挙動関数を登録する条件挙動器。
        ///     driver::dispatcher& io_dispatcher,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in,out] 挙動関数で使う状態変更器。
        ///     driver::accumulator& io_accumulator)
        /// const;
        /// @endcode
        template_behavior_builder const& in_behavior_builder) 
    {
        in_status_builder(
            this->reservoir_, this->hash_function_, in_chunk_key);
        in_expression_builder(
            this->evaluator_,
            this->hash_function_,
            in_chunk_key,
            this->reservoir_);
        this_type::behavior_chunk::extend(
            this->behavior_chunks_,
            in_chunk_key,
            in_behavior_builder(
                this->dispatcher_, this->hash_function_, this->accumulator_));
    }

    /// @brief 状態値と条件式と挙動関数を、チャンクへ追加する。
    public: template<
        typename template_workspace_string,
        typename template_shared_ptr,
        typename template_string>
    void extend_chunk(
        /// [out] 文字列表の構築の作業領域として使う文字列。
        /// std::string 互換のインターフェイスを持つこと。
        template_workspace_string& out_workspace,
        /// [in] 文字列表の構築に使うフライ級文字列の生成器を指すスマートポインタ。
        /// - psyq::string::flyweight::factory::shared_ptr
        ///   互換のインターフェイスを持つこと。
        /// - 空のスマートポインタではないこと。
        template_shared_ptr const& in_string_factory,
        /// [in] 追加するチャンクの識別値。
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        /// [in] 状態値CSV文字列。
        template_string const& in_status_csv,
        /// [in] 状態値CSVの属性の行番号。
        std::size_t const in_status_attribute,
        /// [in] 条件式CSV文字列。
        template_string const& in_expression_csv,
        /// [in] 条件式CSVの属性の行番号。
        std::size_t const in_expression_attribute,
        /// [in] 条件挙動CSV文字列。
        template_string const& in_behavior_csv,
        /// [in] 条件挙動CSVの属性の行番号。
        std::size_t const in_behavior_attribute)
    {
        typedef
            psyq::string::csv_table<
                std::size_t,
                typename template_string::value_type,
                typename template_string::traits_type,
                typename template_shared_ptr::element_type::allocator_type>
            csv_table;
        typedef
            psyq::string::relation_table<
                std::size_t,
                typename template_string::value_type,
                typename template_string::traits_type,
                typename template_shared_ptr::element_type::allocator_type>
            relation_table;
        typedef
            psyq::if_then_engine::status_builder<relation_table>
            status_builder;
        typedef
            psyq::if_then_engine::expression_builder<relation_table>
            expression_builder;
        typedef
            psyq::if_then_engine::behavior_builder<
                relation_table, typename this_type::dispatcher>
            behavior_builder;
        this->extend_chunk(
            in_chunk_key,
            status_builder(
                relation_table(
                    csv_table(
                        out_workspace, in_string_factory, in_status_csv),
                    in_status_attribute)),
            expression_builder(
                relation_table(
                    csv_table(
                        out_workspace, in_string_factory, in_expression_csv),
                    in_expression_attribute)),
            behavior_builder(
                relation_table(
                    csv_table(
                        out_workspace, in_string_factory, in_behavior_csv),
                    in_behavior_attribute)));
    }


    /// @brief チャンクを削除する。
    public: void erase_chunk(
        /// [in] 削除するチャンクの識別値。
        typename this_type::reservoir::chunk_key const& in_chunk)
    {
        this->reservoir_.erase_chunk(in_chunk);
        this->evaluator_.erase_chunk(in_chunk);
        this_type::behavior_chunk::erase(this->behavior_chunks_, in_chunk);
    }

    /// @brief 条件式に対応する挙動関数を、チャンクへ追加する。
    /// @retval true  成功。
    /// @retval false 失敗。
    public: bool register_function(
        /// [in] 挙動関数を追加するチャンクの識別値。
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        /// [in] 評価に使う条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key,
        /// [in] 挙動関数を呼び出す条件。 dispatcher::make_condition で構築する。
        typename this_type::dispatcher::condition const in_condition,
        /// [in] 追加する挙動関数を指すスマートポインタ。
        typename this_type::dispatcher::function_shared_ptr in_function,
        /// [in] 挙動関数の呼び出し優先順位。優先順位の昇順に呼び出される。
        typename this_type::dispatcher::function_priority const in_priority =
            PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
    {
        // 挙動関数を条件挙動器へ登録する。
        auto const local_register_function(
            this->dispatcher_.register_function(
                in_expression_key, in_condition, in_function, in_priority));
        if (!local_register_function)
        {
            return false;
        }

        // 挙動関数を挙動関数チャンクへ追加する。
        this_type::behavior_chunk::extend(
            this->behavior_chunks_, in_chunk_key, std::move(in_function));
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 状態値
    /// @{

    /// @brief 状態貯蔵器を取得する。
    /// @return *this が持つ状態貯蔵器。
    public: typename this_type::reservoir const& get_reservoir() const
    {
        return this->reservoir_;
    }

    /// @copydoc reservoir::register_status(typename this_type::chunk_key const&, typename this_type::status_key const&, template_value const);
    public: template<typename template_value>
    bool register_status(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        typename this_type::reservoir::status_key const& in_status_key,
        template_value const in_value)
    {
        return this->reservoir_.register_status(
            in_chunk_key, in_status_key, in_value);
    }

    /// @copydoc reservoir::register_status(typename this_type::chunk_key const&, typename this_type::status_key const&, template_value const, std::size_t const);
    public: template<typename template_value>
    bool register_status(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        typename this_type::reservoir::status_key const& in_status_key,
        template_value const in_value,
        std::size_t const in_bit_width)
    {
        return this->reservoir_.register_status(
            in_chunk_key, in_status_key, in_value, in_bit_width);
    }

    /// @brief 状態を更新して、挙動関数を呼び出す。
    /// @details 基本的には、時間フレーム毎に呼び出すこと。
    public: void progress()
    {
        this->accumulator_._flush(this->reservoir_);
        this->dispatcher_._dispatch(this->evaluator_, this->reservoir_);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 駆動器で用いる状態貯蔵器。
    private: typename this_type::reservoir reservoir_;
    /// @brief 駆動器で用いる状態変更器。
    public: typename this_type::accumulator accumulator_;
    /// @brief 駆動器で用いる条件評価器。
    public: typename this_type::evaluator evaluator_;
    /// @brief 駆動器で用いる条件挙動器。
    public: typename this_type::dispatcher dispatcher_;
    /// @brief 駆動器で用いる条件挙動チャンクのコンテナ。
    private: typename this_type::behavior_chunk::container behavior_chunks_;
    /// @brief 駆動器で用いるハッシュ関数オブジェクト。
    public: typename this_type::hasher hash_function_;

}; // class psyq::if_then_engine::driver

#endif // !defined(PSYQ_IF_THEN_ENGINE_DRIVER_HPP_)
// vim: set expandtab:
