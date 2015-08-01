/** @file
    @brief @copybrief psyq::if_then_engine::driver
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_DRIVER_HPP_
#define PSYQ_IF_THEN_ENGINE_DRIVER_HPP_

#include "../string/csv_table.hpp"
#include "../string/relation_table.hpp"
#include "./reservoir.hpp"
#include "./modifier.hpp"
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
/** @brief if-then規則による有限状態機械の駆動器。

    ### 使い方の概略
    - driver::driver で駆動器を構築する。
    - driver::extend_chunk で、状態値と条件式と挙動関数を登録する。
    - driver::progress を時間フレーム毎に呼び出す。
      条件式の評価が変化していたら、挙動関数が呼び出される。

    @tparam template_float     @copydoc reservoir::status_value::float_type
    @tparam template_priority  @copydoc dispatcher::function_priority
    @tparam template_hasher    @copydoc hasher
    @tparam template_allocator @copydoc allocator_type
 */
template<
    typename template_float = float,
    typename template_priority = std::int32_t,
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::if_then_engine::driver
{
    /// @brief thisが指す値の型。
    private: typedef driver this_type;

    /** @brief 文字列からハッシュ値を生成する、ハッシュ関数オブジェクトの型。

        - std::hash 互換インターフェイスを持つこと。
        - hasher::argument_type
          が文字列型で、以下に相当するメンバ関数を使えること。
          @code
          // @brief 文字列の先頭位置を取得する。
          hasher::argument_type::const_pointer hasher::argument_type::data() const;
          // @brief 文字列の要素数を取得する。
          std::size_t hasher::argument_type::size() const;
          @endcode
     */
    public: typedef template_hasher hasher;

    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

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
        psyq::if_then_engine::_private::modifier<
            typename this_type::reservoir>
        modifier;

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

    /// @brief 駆動器で用いる条件挙動チャンクの型。
    private: typedef
        psyq::if_then_engine::_private::behavior_chunk<
            typename this_type::dispatcher>
        behavior_chunk;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の駆動器を構築する。
        @param[in] in_reserve_chunks      チャンクの予約数。
        @param[in] in_reserve_statuses    状態値の予約数。
        @param[in] in_reserve_expressions 条件式の予約数。
        @param[in] in_reserve_caches      条件挙動キャッシュの予約数。
        @param[in] in_hash_function       ハッシュ関数オブジェクトの初期値。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: driver(
        std::size_t const in_reserve_chunks,
        std::size_t const in_reserve_statuses,
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_caches = 16,
        typename this_type::hasher in_hash_function = this_type::hasher(),
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    :
    reservoir_(in_reserve_chunks, in_reserve_statuses, in_allocator),
    modifier_(in_reserve_caches, in_allocator),
    evaluator_(in_reserve_chunks, in_reserve_expressions, in_allocator),
    dispatcher_(
        in_reserve_expressions,
        in_reserve_statuses,
        in_reserve_caches,
        in_allocator),
    behavior_chunks_(in_allocator),
    hash_function_(std::move(in_hash_function))
    {
        this->behavior_chunks_.reserve(in_reserve_chunks);
    }

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: driver(this_type&& io_source):
    reservoir_(std::move(io_source.reservoir_)),
    modifier_(std::move(io_source.modifier_)),
    evaluator_(std::move(io_source.evaluator_)),
    dispatcher_(std::move(io_source.dispatcher_)),
    behavior_chunks_(std::move(io_source.behavior_chunks_)),
    hash_function_(std::move(io_source.hash_function_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->reservoir_ = std::move(io_source.reservoir_);
        this->modifier_ = std::move(io_source.modifier_);
        this->evaluator_ = std::move(io_source.evaluator_);
        this->dispatcher_ = std::move(io_source.dispatcher_);
        this->behavior_chunks_ = std::move(io_source.behavior_chunks_);
        this->hash_function_ = std::move(io_source.hash_function_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 駆動器を再構築し、メモリ領域を必要最小限にする。
     */
    public: void rebuild(
        std::size_t const in_chunk_buckets,
        std::size_t const in_status_buckets,
        std::size_t const in_expression_buckets)
    {
        this->reservoir_.rebuild(in_chunk_buckets, in_status_buckets);
        //this->modifier_.shrink_to_fit();
        this->evaluator_.rebuild(in_chunk_buckets, in_expression_buckets);
        this->dispatcher_.shrink_to_fit();
        this->behavior_chunks_.shrink_to_fit();
        for (auto& local_behavior_chunk: this->behavior_chunks_)
        {
            local_behavior_chunk.functions_.shrink_to_fit();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 進行を更新する。

        基本的には、フレーム毎に更新すること。
     */
    public: void progress()
    {
        this->modifier_._modify(this->reservoir_);
        this->dispatcher_._dispatch(this->evaluator_, this->reservoir_);
    }

    //-------------------------------------------------------------------------
    /// @name チャンク
    //@{
    /** @brief 状態値と条件式と挙動関数を、チャンクへ追加する。
        @param[out] out_workspace
            文字列表の構築の作業領域として使う文字列。
            std::string 互換のインターフェイスを持つこと。
        @param[in] in_string_factory
            文字列表の構築に使うフライ級文字列の生成器を指すスマートポインタ。
            - psyq::string::flyweight::factory::shared_ptr
              互換のインターフェイスを持つこと。
            - 空のスマートポインタではないこと。
        @param[in] in_chunk_key            追加するチャンクの識別値。
        @param[in] in_status_csv            状態値CSV文字列。
        @param[in] in_status_attribute      状態値CSVの属性の行番号。
        @param[in] in_expression_csv       条件式CSV文字列。
        @param[in] in_expression_attribute 条件式CSVの属性の行番号。
        @param[in] in_behavior_csv         条件挙動CSV文字列。
        @param[in] in_behavior_attribute   条件挙動CSVの属性の行番号。
     */
    public: template<
        typename template_workspace_string,
        typename template_shared_ptr,
        typename template_string>
    void extend_chunk(
        template_workspace_string& out_workspace,
        template_shared_ptr const& in_string_factory,
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        template_string const& in_status_csv,
        typename template_string::size_type const& in_status_attribute,
        template_string const& in_expression_csv,
        typename template_string::size_type const& in_expression_attribute,
        template_string const& in_behavior_csv,
        typename template_string::size_type const& in_behavior_attribute)
    {
        typedef
            psyq::string::csv_table<
                typename template_string::value_type,
                typename template_string::traits_type,
                typename template_shared_ptr::element_type::allocator_type>
            csv_table;
        typedef
            psyq::string::relation_table<
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

    /** @brief 状態値と条件式と挙動関数を、チャンクへ追加する。
        @param[in] in_chunk_key 追加するチャンクの識別値。
        @param[in] in_status_builder
            状態値を状態貯蔵器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 状態値を状態貯蔵器に登録する。
            // @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     状態値を登録するチャンクを表す識別値。
            void template_status_builder::operator()(
                driver::reservoir& io_reservoir,
                driver::hasher& io_hasher,
                driver::reservoir::chunk_key const& in_chunk_key)
            const;
            @endcode
        @param[in] in_expression_builder
            条件式を条件評価器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 条件式を条件評価器に登録する。
            // @param[in,out] io_evaluator 条件式を登録する条件評価器。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     条件式を登録するチャンクを表す識別値。
            // @param[in] in_reservoir     条件式で使う状態貯蔵器。
            void template_expression_builder::operator()(
                driver::evaluator& io_evaluator,
                driver::hasher& io_hasher,
                driver::reservoir::chunk_key const& in_chunk_key,
                driver::reservoir const& in_reservoir)
            const;
            @endcode
        @param[in] in_behavior_builder
            挙動関数を条件挙動器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 挙動関数を条件挙動器に登録する。
            // @param[in,out] io_dispatcher 挙動関数を登録する条件挙動器。
            // @param[in,out] io_hasher     文字列から識別値を生成する関数オブジェクト。
            // @param[in,out] io_modifier   挙動関数で使う状態変更器。
            // @return
            //     条件挙動器に登録した挙動関数オブジェクトを指す、
            //     スマートポインタのコンテナ。
            template<typename template_function_shared_ptr_container>
            template_function_shared_ptr_container
            template_behavior_builder::operator()(
                driver::dispatcher& io_dispatcher,
                driver::hasher& io_hasher,
                driver::modifier& io_modifier)
            const;
            @endcode
     */
    public: template<
        typename template_status_builder,
        typename template_expression_builder,
        typename template_behavior_builder>
    void extend_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        template_status_builder const& in_status_builder,
        template_expression_builder const& in_expression_builder,
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
                this->dispatcher_, this->hash_function_, this->modifier_));
    }

    /** @brief 条件式に対応する挙動関数を、チャンクへ追加する。
        @param[in] in_chunk_key      挙動関数を追加するチャンクの識別値。
        @param[in] in_expression_key 評価に用いる条件式の識別値。
        @param[in] in_condition
            挙動関数を呼び出す条件。 dispatcher::make_condition で構築する。
        @param[in] in_function
            追加する挙動関数を指すスマートポインタ。
        @param[in] in_priority
            挙動関数の呼び出し優先順位。優先順位の昇順に呼び出される。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool extend_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        typename this_type::evaluator::expression_key const& in_expression_key,
        typename this_type::dispatcher::condition const in_condition,
        typename this_type::dispatcher::function_shared_ptr const& in_function,
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

    /** @brief チャンクを削除する。
        @param[in] in_chunk 削除するチャンクのキー。
     */
    public: void erase_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk)
    {
        this->reservoir_.erase_chunk(in_chunk);
        this->evaluator_.erase_chunk(in_chunk);
        this_type::behavior_chunk::erase(this->behavior_chunks_, in_chunk);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief 駆動器で用いる状態貯蔵器。
    public: typename this_type::reservoir reservoir_;
    /// @brief 駆動器で用いる状態変更器。
    public: typename this_type::modifier modifier_;
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
