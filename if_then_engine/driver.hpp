/// @file
/// @brief @copybrief psyq::if_then_engine::driver
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_DRIVER_HPP_
#define PSYQ_IF_THEN_ENGINE_DRIVER_HPP_

#ifndef PSYQ_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT
#define PSYQ_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT 64
#endif // !defined(PSYQ_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)

#include "../string/csv_table.hpp"
#include "../string/relation_table.hpp"
#include "./reservoir.hpp"
#include "./accumulator.hpp"
#include "./evaluator.hpp"
#include "./dispatcher.hpp"
#include "./handler_chunk.hpp"
#include "./handler_builder.hpp"
#include "./status_builder.hpp"
#include "./expression_builder.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        template<typename, typename, typename, typename, typename> class driver;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief if-then規則による有限状態機械の駆動器。
/// @par 使い方の概略
/// - driver::driver で駆動器を構築する。
/// - driver::extend_chunk で、状態値と条件式と条件挙動関数を登録する。
///   - 状態値の登録のみしたい場合は driver::register_status を呼び出す。
///   - 条件式の登録のみしたい場合は driver::evaluator_ に対して
///     evaluator::register_expression を呼び出す。
///   - 条件挙動関数の登録のみしたい場合は driver::register_handler を呼び出す。
/// - driver::accumulator_ に対して
///   accumulator::accumulate を呼び出し、状態値の変更を予約する。
/// - driver::progress
///   を時間フレーム毎に呼び出す。状態値の変更と条件式の評価が行われ、
///   挙動条件に合致する条件挙動関数が呼び出される。
/// @tparam template_unsigned  @copydoc reservoir::status_value::unsigned_type
/// @tparam template_float     @copydoc reservoir::status_value::float_type
/// @tparam template_priority  @copydoc dispatcher::handler::priority
/// @tparam template_hasher    @copydoc hasher
/// @tparam template_allocator @copydoc allocator_type
template<
    typename template_unsigned = std::uint64_t,
    typename template_float = float,
    typename template_priority = std::int32_t,
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::if_then_engine::driver
{
    /// @brief this が指す値の型。
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
            template_unsigned,
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
    /// @brief チャンクの識別値を表す型。
    public: typedef typename this_type::reservoir::chunk_key chunk_key;

    //-------------------------------------------------------------------------
    /// @brief 駆動器で用いる条件挙動チャンクの型。
    private: typedef
        psyq::if_then_engine::_private::handler_chunk<
            typename this_type::dispatcher>
        handler_chunk;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 空の駆動器を構築する。
    public: driver(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_count,
        /// [in] 状態値辞書のバケット数。
        std::size_t const in_status_count,
        /// [in] 条件式辞書のバケット数。
        std::size_t const in_expression_count,
        /// [in] キャッシュの予約数。
        std::size_t const in_cache_capacity =
            PSYQ_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT,
        /// [in] 文字列ハッシュ関数オブジェクトの初期値。
        typename this_type::hasher in_hash_function = this_type::hasher(),
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type()):
    reservoir_(in_chunk_count, in_status_count, in_allocator),
    accumulator_(in_cache_capacity, in_allocator),
    evaluator_(in_chunk_count, in_expression_count, in_allocator),
    dispatcher_(
        in_status_count,
        in_expression_count,
        in_cache_capacity,
        in_allocator),
    handler_chunks_(in_allocator),
    hash_function_(std::move(in_hash_function))
    {
        this->handler_chunks_.reserve(in_chunk_count);
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
    handler_chunks_(std::move(io_source.handler_chunks_)),
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
        this->handler_chunks_ = std::move(io_source.handler_chunks_);
        this->hash_function_ = std::move(io_source.hash_function_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 駆動器を再構築する。
    public: void rebuild(
        /// [in] チャンク辞書のバケット数。
        std::size_t const in_chunk_count,
        /// [in] 状態値辞書のバケット数。
        std::size_t const in_status_count,
        /// [in] 条件式辞書のバケット数。
        std::size_t const in_expression_count,
        /// [in] キャッシュの予約数。
        std::size_t const in_cache_capacity =
            PSYQ_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)
    {
        this->reservoir_.rebuild(in_chunk_count, in_status_count);
        //this->accumulator_.rebuild(in_cache_capacity);
        this->evaluator_.rebuild(in_chunk_count, in_expression_count);
        this->dispatcher_.rebuild(
            in_status_count, in_expression_count, in_cache_capacity);
        this->handler_chunks_.shrink_to_fit();
        for (auto& local_handler_chunk: this->handler_chunks_)
        {
            local_handler_chunk.shrink_to_fit();
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name チャンク
    /// @{

    /// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
    public: template<
        typename template_status_builder,
        typename template_expression_builder,
        typename template_handler_builder,
        typename template_relation_table>
    void extend_chunk(
        /// [in] 追加するチャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,

        /// [in] 状態値を状態貯蔵器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 文字列表を解析して状態値を構築し、状態貯蔵器に登録する。
        /// void template_status_builder::operator()(
        ///     // [in,out] 状態値を登録する状態貯蔵器。
        ///     driver::reservoir& io_reservoir,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in] 状態値を登録するチャンクを表す識別値。
        ///     driver::reservoir::chunk_key const& in_chunk_key,
        ///     // [in] 解析する文字列表。
        ///     template_relation_table const& in_table)
        /// const;
        /// @endcode
        template_status_builder const& in_status_builder,
        /// [in] 状態値が記述されている psyq::string::relation_table 。
        /// 文字列表が空の場合は、状態値を追加しない。
        template_relation_table const& in_status_table,

        /// [in] 条件式を条件評価器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
        /// void template_expression_builder::operator()(
        ///     // [in,out] 条件式を登録する条件評価器。
        ///     driver::evaluator& io_evaluator,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in] 条件式を登録するチャンクを表す識別値。
        ///     driver::evaluator::chunk_key const& in_chunk_key,
        ///     // [in] 条件式で使う状態貯蔵器。
        ///     driver::reservoir const& in_reservoir,
        ///     // [in] 解析する文字列表。
        ///     template_relation_table const& in_table)
        /// const;
        /// @endcode
        template_expression_builder const& in_expression_builder,
        /// [in] 条件式が記述されている psyq::string::relation_table 。
        /// 文字列表が空の場合は、条件式を追加しない。
        template_relation_table const& in_expression_table,

        /// [in] 条件挙動ハンドラを条件挙動器に登録する関数オブジェクト。
        /// 以下に相当するメンバ関数を使えること。
        /// @code
        /// // brief 文字列表から条件挙動ハンドラを構築し、条件挙動器に登録する。
        /// // return
        /// // 条件挙動器に登録した条件挙動ハンドラに対応する関数を指す、
        /// // スマートポインタのコンテナ。
        /// template<typename template_function_shared_ptr_container>
        /// template_function_shared_ptr_container template_handler_builder::operator()(
        ///     // [in,out] 条件挙動ハンドラを登録する条件挙動器。
        ///     driver::dispatcher& io_dispatcher,
        ///     // [in,out] 文字列から識別値を生成する関数オブジェクト。
        ///     driver::hasher& io_hasher,
        ///     // [in,out] 条件挙動ハンドラで使う状態変更器。
        ///     driver::accumulator& io_accumulator,
        ///     // [in] 解析する文字列表。
        ///     template_relation_table const& in_table)
        /// const;
        /// @endcode
        template_handler_builder const& in_handler_builder,
        /// [in] 条件挙動ハンドラが記述されている psyq::string::relation_table 。
        /// 文字列表が空の場合は、条件挙動ハンドラを追加しない。
        template_relation_table const& in_handler_table)
    {
        in_status_builder(
            this->reservoir_,
            this->hash_function_,
            in_chunk_key,
            in_status_table);
        in_expression_builder(
            this->evaluator_,
            this->hash_function_,
            in_chunk_key,
            this->reservoir_,
            in_expression_table);
        this_type::handler_chunk::extend(
            this->handler_chunks_,
            in_chunk_key,
            in_handler_builder(
                this->dispatcher_,
                this->hash_function_,
                this->accumulator_,
                in_handler_table));
    }

    /// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
    public: template<
        typename template_workspace_string,
        typename template_shared_ptr,
        typename template_string>
    void extend_chunk(
        /// [out] 文字列表の構築の作業領域として使う文字列。
        /// std::basic_string 互換のインタフェイスを持つこと。
        template_workspace_string& out_workspace,
        /// [in] 文字列表の構築に使うフライ級文字列の生成器を指すスマートポインタ。
        /// - psyq::string::flyweight::factory::shared_ptr
        ///   互換のインタフェイスを持つこと。
        /// - 空ではないこと。
        template_shared_ptr const& in_string_factory,
        /// [in] 追加するチャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 状態値CSV文字列。空文字列の場合は、状態値を追加しない。
        template_string const& in_status_csv,
        /// [in] 状態値CSVの属性の行番号。
        std::size_t const in_status_attribute,
        /// [in] 条件式CSV文字列。空文字列の場合は、条件式を追加しない。
        template_string const& in_expression_csv,
        /// [in] 条件式CSVの属性の行番号。
        std::size_t const in_expression_attribute,
        /// [in] 条件挙動CSV文字列。空文字列の場合は、条件挙動ハンドラを追加しない。
        template_string const& in_handler_csv,
        /// [in] 条件挙動CSVの属性の行番号。
        std::size_t const in_handler_attribute)
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
        this->extend_chunk(
            in_chunk_key,
            psyq::if_then_engine::status_builder<relation_table>(),
            relation_table(
                csv_table(out_workspace, in_string_factory, in_status_csv),
                in_status_csv.empty()?
                    relation_table::INVALID_NUMBER: in_status_attribute),
            psyq::if_then_engine::expression_builder<relation_table>(),
            relation_table(
                csv_table(out_workspace, in_string_factory, in_expression_csv),
                in_expression_csv.empty()?
                    relation_table::INVALID_NUMBER: in_expression_attribute),
            psyq::if_then_engine::handler_builder<
                relation_table, typename this_type::dispatcher>(),
            relation_table(
                csv_table(out_workspace, in_string_factory, in_handler_csv),
                in_handler_csv.empty()?
                    relation_table::INVALID_NUMBER: in_handler_attribute));
    }

    /// @brief チャンクを削除する。
    public: void erase_chunk(
        /// [in] 削除するチャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key)
    {
        this->reservoir_.erase_chunk(in_chunk_key);
        this->evaluator_.erase_chunk(in_chunk_key);
        this_type::handler_chunk::erase(this->handler_chunks_, in_chunk_key);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 条件挙動
    /// @{

    /// @brief 条件式に対応する条件挙動関数を登録して強参照する。
    /// @sa
    /// this_type::progress で、 in_expression_key に対応する条件式の評価が変化し
    /// in_condition と合致すると、 in_function の指す条件挙動関数が呼び出される。
    /// @sa
    /// in_function の指す条件挙動関数が解体されると、
    /// in_function に対応する dispatcher::handler は自動的に削除される。
    /// 明示的に削除するには dispatcher::unregister_handler を使う。
    /// @retval true
    /// 成功。 in_function の指す条件挙動関数を弱参照する
    /// dispatcher::handler を構築して this_type::dispatcher_
    /// に登録し、登録した条件挙動関数の強参照をチャンクに追加した。
    /// @retval false
    /// 失敗。条件挙動関数は登録されず、
    /// 条件挙動関数の強参照はチャンクに追加されなかった。
    /// - in_condition が dispatcher::handler::INVALID_CONDITION だと失敗する。
    /// - in_function が空か、空の関数を指していると失敗する。
    /// - in_expression_key と対応する dispatcher::handler に
    ///   in_function が既に登録されていると、失敗する。
    public: bool register_handler(
        /// [in] 条件挙動関数を保持させるチャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] in_function の指す条件挙動関数に対応する条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key,
        /// [in] in_function の指す条件挙動関数を呼び出す挙動条件。
        /// dispatcher::handler::make_condition から作る。
        typename this_type::dispatcher::handler::condition const in_condition,
        /// [in] 登録する条件挙動関数を指すスマートポインタ。
        /// in_expression_key に対応する条件式の評価が変化して
        /// in_condition に合致すると、呼び出される。
        typename this_type::dispatcher::handler::function_shared_ptr in_function,
        /// [in] in_function の指す条件挙動関数の呼び出し優先順位。
        /// 昇順に呼び出される。
        typename this_type::dispatcher::handler::priority const in_priority =
            PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
    {
        // 条件挙動関数を条件挙動器へ登録する。
        auto const local_register_handler(
            this->dispatcher_.register_handler(
                in_expression_key, in_condition, in_function, in_priority));
        if (local_register_handler)
        {
            // 条件挙動関数を条件挙動チャンクへ追加する。
            this_type::handler_chunk::extend(
                this->handler_chunks_, in_chunk_key, std::move(in_function));
        }
        return local_register_handler;
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

    /// @brief 状態値を登録する。
    /// @sa
    /// - 登録した状態値を取得するには、 this_type::get_reservoir から
    ///   reservoir::find_status を呼び出す。
    /// - 状態値の変更は this_type::accumulator_ から
    ///   accumulator::accumulate を呼び出して行う。
    /// - this_type::erase_chunk で、登録した状態値をチャンク毎に削除できる。
    /// @retval true  成功。状態値を登録した。
    /// @retval false 失敗。状態値は登録されなかった。
    /// - in_status_key に対応する状態値がすでに登録されていると失敗する。
    public: template<typename template_value>
    bool register_status(
        /// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 登録する状態値の識別値。
        typename this_type::reservoir::status_key const& in_status_key,
        /// [in] 登録する状態値の初期値。以下の型の値を登録できる。
        /// - bool 型。
        /// - C++ 組み込み整数型。
        /// - C++ 組み込み浮動小数点数型。
        template_value const in_value)
    {
        return this->reservoir_.register_status(
            in_chunk_key, in_status_key, in_value);
    }

    /// @brief 整数型の状態値を登録する。
    /// @sa
    /// - 登録した状態値を取得するには、 this_type::get_reservoir から
    ///   reservoir::find_status を呼び出す。
    /// - 登録した状態値を書き換えるには、 this_type::accumulator_ から
    ///   accumulator::accumulate を呼び出す。
    /// - this_type::erase_chunk で、登録した状態値をチャンク毎に削除できる。
    /// @retval true  成功。状態値を登録した。
    /// @retval false 失敗。状態値は登録されなかった。
    /// - in_status_key に対応する状態値がすでに登録されていると失敗する。
    /// - in_value のビット幅が in_bit_width を超えていると失敗する。
    /// - reservoir::status_chunk::BLOCK_BIT_WIDTH より
    ///   in_bit_width が大きいと失敗する。
    /// - in_bit_width が2未満だと失敗する。
    ///   1ビットの値は論理型として登録すること。
    public: template<typename template_value>
    bool register_status(
        /// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] 登録する状態値の識別値。
        typename this_type::reservoir::status_key const& in_status_key,
        /// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
        template_value const in_value,
        /// [in] 登録する状態値のビット幅。
        std::size_t const in_bit_width)
    {
        return this->reservoir_.register_status(
            in_chunk_key, in_status_key, in_value, in_bit_width);
    }

    /// @brief 状態値を更新し、条件式を評価して、条件挙動関数を呼び出す。
    /// @details 基本的には、時間フレーム毎に呼び出すこと。
    public: void progress()
    {
        this->accumulator_._flush(this->reservoir_);
        this->dispatcher_._dispatch(this->reservoir_, this->evaluator_);
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
    private: typename this_type::handler_chunk::container handler_chunks_;
    /// @brief 駆動器で用いる文字列ハッシュ関数オブジェクト。
    public: typename this_type::hasher hash_function_;

}; // class psyq::if_then_engine::driver

#endif // !defined(PSYQ_IF_THEN_ENGINE_DRIVER_HPP_)
// vim: set expandtab:
