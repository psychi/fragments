#ifndef SOCIETY_HPP_
#define SOCIETY_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace society
{
  /// @brief 時間を表す型。
  typedef std::uint32_t time;
  enum: society::time {NULL_TIME = 0,};

  /// @brief  個人の識別値を表す型。
  typedef std::uint32_t person_key;
  enum: society::person_key {NULL_PERSON_KEY = 0,};

  /// @brief  家族の識別値を表す型。
  typedef std::uint32_t family_key;
  enum: society::person_key {NULL_FAMILY_KEY = 0,};

  class time_range;
  template<typename> class kinship;
  class person_body;
  class person_mind;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 期間。
class society::time_range
{
  public: explicit time_range(
    society::time const in_begin = society::NULL_TIME,
    society::time const in_end = society::NULL_TIME):
  begin_(in_begin)
  {
    if (in_begin == society::NULL_TIME)
    {
      assert(in_end == society::NULL_TIME);
      this->end_ = society::NULL_TIME;
    }
    else if (in_end == society::NULL_TIME || in_begin <= in_end)
    {
      this->end_ = in_end;
    }
    else
    {
      assert(false);
      this->begin_ = in_end;
      this->end_ = in_begin;
    }
  }

  /// @brief 開始済みか判定する。
  public: bool is_begin() const
  {
    return this->get_begin() != society::NULL_TIME;
  }

  /// @brief 継続中か判定する。
  public: bool is_continuing() const
  {
    return this->is_begin() && this->end_ == society::NULL_TIME;
  }

  /// @brief 終了済みか判定する。
  public: bool is_end() const
  {
    return this->is_begin() && this->end_ != society::NULL_TIME?
  }

  /// @brief 開始時間を取得する。
  public: society::time get_begin() const {return this->begin_;}

  /// @brief 終了時間を取得する。
  public: society::time get_end() const
  {
    return this->is_begin()? this->end_: society::NULL_TIME;
  }

  /// @brief 継続中の期間を終了する。
  /// @retval true  成功。
  /// @retval false 失敗。
  public: bool set_end(
    /// [in] 終了時間。
    society::time const in_end)
  {
    if (
      in_end == society::NULL_TIME
      || in_end < this->begin_
      || !this->is_continuing())
    {
      return false;
    }
    this->end_ = in_end;
    return true;
  }

  //---------------------------------------------------------------------------
  private:
  society::time begin_; ///< 開始時間。
  society::time end_;   ///< 終了時間。

}; // class society::time_range

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 親族関係。
template<typename template_allocator>
class society::kinship
{
  private: typedef kinship this_type;

  //---------------------------------------------------------------------------
  /// @brief 用いるメモリ割当子の型。
  public: typedef template_allocator allocator_type;

  //---------------------------------------------------------------------------
  /// @brief 個人ごとの親子関係。
  public: class person
  {
    private: typedef person this_type;

    public: typedef
      std::vector<society::family_key, template_allocator>
      family_history;

    public: person(
      /// [in] 出生時間。
      society::time const in_birth_time,
      /// [in] 自然的母の個人識別値。
      society::person_key const in_genitrix,
      /// [in] 自然的父の個人識別値。
      society::person_key const in_genitor)
    period_(in_birth_time),
    genitrix_(in_genitrix),
    genitor_(in_genitor)
    {}

    public:
    society::time_range period_;   ///< 存命期間。
    society::person_key genitrix_; ///< 自然的母の個人識別値。
    society::person_key genitor_;  ///< 自然的父の個人識別値。
    typename this_type::family_history parents_;   ///< 社会的親の履歴。
    typename this_type::family_history marriages_; ///< 婚姻の履歴。

  }; // class person

  //---------------------------------------------------------------------------
  /// @brief 社会的家族ごとの親子関係。
  public: class family
  {
    private: typedef family this_type;

    //-------------------------------------------------------------------------
    public: class child
    {
      public: child(
        society::time const in_begin_time,
        society::person_key const in_person):
      period_(in_begin_time),
      key_(in_person)
      {}

      public:
      society::time_range period_;
      society::person_key key_;
    };

    public: typedef
      std::vector<typename this_type::child, template_allocator>
      child_history;

    //-------------------------------------------------------------------------
    public: family(
      society::time const in_begin_time,
      society::person_key const in_master,
      society::person_key const in_spouse):
    period_(in_begin_time),
    master_(in_master),
    spouse_(in_spouse)
    {}

    public: bool is_child(society::person_key const in_person) const
    {
      for (auto& local_child: this->children_)
      {
        if (in_person == local_child.key_ && local_child.period_.is_continuing())
        {
          return true;
        }
      }
      return false;
    }

    //-------------------------------------------------------------------------
    public:
    society::time_range period_; ///< 期間。
    society::person_key master_; ///< 家長の個人識別値。
    society::person_key spouse_; ///< 家長の配偶者の個人識別値。
    typename this_type::child_history children_; ///< 子の履歴。

  }; // class family

  //---------------------------------------------------------------------------
  public: kinship(
    std::size_t const in_capacity,
    template_allocator const &in_allocator):
  families_(in_allocator),
  persons_(in_allocator),
  female_flags_(in_allocator),
  family_begin_(society::NULL_FAMILY_KEY + 1),
  person_begin_(society::NULL_PERSON_KEY + 1)
  {
    this->families_.reserve(in_capacity);
    this->persons_.reserve(in_capacity);
    this->female_flags_.reserve(in_capacity);
  }

  //---------------------------------------------------------------------------
  /// @brief 記録されている個人の数を取得する。
  public: std::size_t count_persons() const
  {
    return this->persons_.size();
  }

  /// @brief 先頭を指す個人識別値を取得する。
  public: society::person_key get_person_begin() const
  {
    return this->person_begin_;
  }

  /// @brief 末尾を指す個人識別値を取得する。
  public: society::person_key get_person_end() const
  {
    return static_cast<society::person_key>(
      this->get_person_begin() + this->count_persons());
  }

  //---------------------------------------------------------------------------
  /// @brief 記録されている家族の数を取得する。
  public: std::size_t count_families() const
  {
    return this->families_.size();
  }

  /// @brief 先頭を指す家族識別値を取得する。
  public: society::family_key get_family_begin() const
  {
    return this->person_begin_;
  }

  /// @brief 末尾を指す家族識別値を取得する。
  public: society::family_key get_family_end() const
  {
    return static_cast<society::family_key>(
      this->get_family_begin() + this->count_families());
  }

  //---------------------------------------------------------------------------
  /// @brief 個人が女性か判定する。
  /// @retval true in_person に対応する個人は女性。
  /// @retval true in_person に対応する個人は男性、もしくはいない。
  public: bool is_female(
    /// [in] 個人の識別値。
    society::person_key const in_person)
  const
  {
    return 0 < this->get_sex(in_person);
  }

  /// @brief 個人が男性か判定する。
  /// @retval true in_person に対応する個人は男性。
  /// @retval true in_person に対応する個人は女性、もしくはいない。
  public: bool is_male(
    /// [in] 個人の識別値。
    society::person_key const in_person)
  const
  {
    return this->get_sex(in_person) == 0;
  }

  /// @brief 個人の性別を取得する。
  /// @retval 正 女性。
  /// @retval 0  男性。
  /// @retval 負 in_person に対応する個人がいない。
  public: signed char find_sex(
    /// [in] 個人の識別値。
    society::person_key const in_person)
  const
  {
    if (this->get_person_begin() <= in_person)
    {
      auto const local_index(in_person - this->get_person_begin());
      if (local_index < this->count_persons())
      {
        return this->female_flags_.at(local_index)? 1: 0;
      }
    }
    return -1;
  }

  //---------------------------------------------------------------------------
  /// @brief 個人の親戚関係を取得する。
  /// @retval !=nullptr in_person に対応する個人の親戚関係を指すポインタ。
  /// @retval ==nullptr in_person に対応する個人がいない。
  public: typename this_type::person const* find_person(
    /// [in] 個人の識別値。
    society::person_key const in_person)
  const
  {
    return this_type::find_element(
      this->persons_, this->get_person_begin(), in_person);
  }

  /// @brief 人物の識別値を取得する。
  public: society::person_key find_person_key(
    typename this_type::person const* in_person)
  const
  {
    return this_type::find_element_key(
      this->persons_,
      this->get_person_begin(),
      in_person,
      society::NULL_PERSON_KEY);
  }

  /// @brief 個人の死亡を登録する。
  /// @retval true  成功。個人の死亡を登録した。
  /// @retval false 失敗。 in_person に対応する個人がいないか、すでに死んでいた。
  public: bool die_person(
    /// [in] 死亡時間。
    society::time const in_time,
    /// [in] 個人の識別値。
    society::person_key const in_person)
  {
    auto const local_person(
      const_cast<typename this_type::person*>(this->find_person(in_person)));
    /// @todo 家族との縁を終わらせること。
    return local_person != nullptr && local_person->period_.set_end(in_time);
  }

  //---------------------------------------------------------------------------
  /// @brief 家族の親戚関係を取得する。
  /// @retval !=nullptr in_family に対応する家族の親戚関係を指すポインタ。
  /// @retval ==nullptr in_family に対応する家族がいない。
  public: typename this_type::family const* find_family(
    /// [in] 家族の識別値。
    society::family_key const in_family)
  const
  {
    return this_type::find_element(
      this->families_, this->get_family_begin(), in_family);
  }

  /// @brief 家族の親戚関係を取得する。
  public: typename this_type::family const* find_family(
    /// [in] 配偶者#0の個人識別値。
    society::person_key const in_person_0,
    /// [in] 配偶者#1の個人識別値。
    society::person_key const in_person_1,
    /// [in] 家長となり得る配偶者の指定。
    /// 1であるビット位置に対応する配偶者が家長だった場合に検索対象とする。
    unsigned const in_master_flags = 3)
  const
  {
    auto const local_master_flags(in_master_flags & 3);
    if (local_master_flags != 0)
    {
      auto const local_spouse(
        this->find_person(
          local_master_flags == 3?
            (in_spouse_0 != society::NULL_PERSON_KEY? in_person_0: in_person_1):
            (local_master_flags == 1? in_person_0: in_person_1)));
      if (local_spouse != nullptr)
      {
        for (
          auto i(local_spouse->marriages_.rbegin());
          i != local_spouse->marriages_.rend();
          ++i)
        {
          auto const local_family(this->find_family(*i));
          if (local_family != nullptr)
          {
            if (
              ( (local_master_flags & 1) != 0
                && local_family->master_ == in_spouse_0
                && local_family->spouse_ == in_spouse_1)
              || (
                (local_master_flags & 2) != 0
                && local_family->master_ == in_spouse_1
                && local_family->spouse_ == in_spouse_0))
            {
              return local_family;
            }
          }
        }
      }
    }
    return nullptr;
  }

  /// @brief 家族の識別値を取得する。
  public: society::family_key find_family_key(
    typename this_type::family const* in_family)
  const
  {
    return this_type::find_element_key(
      this->families_,
      this->get_family_begin(),
      in_family,
      society::NULL_FAMILY_KEY);
  }

  /// @brief 家長側の配偶者を取得する。
  public: society::person_key find_master_spouse(
    society::person_key const in_person)
  const
  {
    auto const local_person(this->find_person(in_person));
    if (local_person != nullptr)
    {
      for (
        auto i(local_person->marriages_.rbegin());
        i != local_person->marriages_.rend();
        ++i)
      {
        auto const local_family(this->find_family(*i));
        if (local_family == nullptr)
        {
          assert(false);
        }
        else if(local_family->period_.is_continuing())
        {
          if (local_family->master_ == in_person)
          {
            return in_person;
          }
          else
          {
            assert(local_family->spouse_ == in_person);
            return local_family->master_;
          }
        }
      }
    }
    return society::NULL_PERSON_KEY;
  }

  /// @brief 現在結婚している配偶者を数える。
  /// @retval 0     in_person に対応する個人がいない。
  /// @retval 1     in_person に対応する個人は、現在結婚してない。
  /// @retval 2     in_person に対応する個人は、家長の配偶者として結婚している。
  /// @retval 3以上 in_person に対応する個人は、家長として結婚している。戻り値は配偶者の数+2。
  public: std::size_t count_marriage_level(
    /// [in] 配偶者を数える個人の識別値。
    society::person_key const in_person)
  const
  {
    auto const local_person(this->find_person(in_person));
    if (local_person == nullptr)
    {
      return 0;
    }
    std::size_t local_level(1);
    for (auto local_marriage: local_person->marriages_)
    {
      auto const local_family(this->find_family(local_marriage));
      if (local_family != nullptr && local_family->period_.is_continuing())
      {
        if (local_family->master_ != in_person)
        {
          assert(local_level == 1);
          return 2;
        }
        local_level = (local_level < 3? local_level + 1: 3);
      }
    }
    return local_level;
  }

  public: society::family const* marriage(
    /// [in] 時間。
    society::time const in_time,
    /// [in] 結婚して家長となる個人の識別値。
    society::person_key const in_master,
    /// [in] 結婚して家長となる個人の識別値。
    society::person_key const in_spouse,
    /// [in] 家長が結婚可能な配偶者の最大数。
    std::size_t const in_max = 1)
  {
    if (in_time != society::NULL_TIME && 0 < in_max)
    {
      auto const local_level(this->count_marriage_level(in_master));
      if (
        0 < local_level
        && local_level != 2
        && local_level < in_max + 2
        && this->find_master_spouse(in_spouse) == society::NULL_PERSON_KEY)
      {
        this->families_.emplace_back(in_time, in_master, in_spouse);
        return;
      }
    }
    return nullptr;
  }

  //---------------------------------------------------------------------------
  /// @brief 家族内に生まれた子を登録する。
  /// @retval !=society::NULL_PERSON_KEY 成功。登録した個人の識別値。
  /// @retval ==society::NULL_PERSON_KEY 失敗。登録できなかった。
  public: society::person_key make_child(
    /// [in] 子の出生時間。
    society::time const in_time,
    /// [in] 子が女性かどうか。
    bool const in_female,
    /// [in] 子の出自家族の識別値。
    society::person_key const in_family)
  {
    auto const local_family(this->find_family(in_family));
    if (local_family == nullptr)
    {
      return society::NULL_PERSON_KEY;
    }
    society::person_key local_genitrix;
    society::person_key local_genitor;
    if (this->is_female(local_family->master_))
    {
      local_genitrix = local_family->master_;
      local_genitor = local_family->spouse_;
    }
    else
    {
      local_genitrix = local_family->spouse_;
      local_genitor = local_family->master_;
    }
    return this->make_child(
      in_time, in_female, in_family, local_genitrix, local_genitor);
  }

  /// @copydoc make_child
  public: society::person_key make_child(
    /// [in] 子の出生時間。
    society::time const in_time,
    /// [in] 子が女性かどうか。
    bool const in_female,
    /// [in] 子の出自家族の識別値。
    society::family_key const in_family,
    /// [in] 子の自然的母の個人識別値。
    society::person_key const in_genitrix,
    /// [in] 子の自然的父の個人識別値。
    society::person_key const in_genitor)
  {
    auto const local_family(this->find_family(in_family));
    if (local_family != nullptr)
    {
      auto const local_child(
        this->make_child(in_time, in_female, in_genitrix, in_genitor));
      if (local_child != society::NULL_PERSON_KEY
          && this_type::connect_parent_and_child(
            in_time,
            in_family,
            const_cast<typename this_type::family&>(*local_family),
            local_child,
            this->persons_.back()))
      {
        return local_child;
      }
      assert(local_child == society::NULL_PERSON_KEY);
    }
    return society::NULL_PERSON_KEY;
  }

  /// @brief 家族外で生まれた子を登録する。
  /// @copydetails make_child
  public: society::person_key make_child(
    /// [in] 子の出生時間。
    society::time const in_time,
    /// [in] 子が女性かどうか。
    bool const in_female,
    /// [in] 子の自然的母の個人識別値。
    society::person_key const in_genitrix,
    /// [in] 子の自然的父の個人識別値。
    society::person_key const in_genitor)
  {
    if (std::numeric_limits<society::person_key>::max() <= this->count_persons())
    {
      assert(false);
      return society::NULL_PERSON_KEY;
    }
    if (in_time == society::NULL_TIME
        || !this->is_female(in_genitrix)
        || !this->is_male(in_genitor))
    {
      return society::NULL_PERSON_KEY;
    }
    this->persons_.emplace_back(in_time, in_genitrix, in_genitor);
    this->female_flags_.emplace_back(in_female);
    return static_cast<society::person_key>(
      this->get_person_begin() + this->count_persons() - 1);
  }

  /// @brief 親子の縁を結ぶ。
  public: bool connect_child(
    /// [in] 時間。
    society::time const in_time,
    /// [in] 子を引き取る家族の識別値。
    society::family_key const in_family,
    /// [in] 引き取る子の個人識別番号。子は、すでに親との縁が切れていること。
    society::person_key const in_child)
  {
    auto const local_family(this->find_family(in_family));
    if (local_family == nullptr)
    {
      return false;
    }
    auto const local_child(this->find_person(in_child));
    return local_child != nullptr && this_type::connect_parent_and_child(
      in_time,
      in_family,
      const_cast<typename this_type::family&>(*local_family),
      in_child,
      const_cast<typename this_type::person&>(*local_child));
    }
  }

  /// @brief 親子の縁を切る。
  /// @retval !=society::NULL_FAMILY_KEY 成功。縁を切った家族の識別値。
  /// @retval ==society::NULL_FAMILY_KEY 失敗。縁を切る家族がいない。
  public: society::family_key disconnect_child(
    /// [in] 時間。
    society::time const in_time,
    /// [in] 縁を切る子の個人識別番号。
    society::person_key const in_child)
  {
    auto const local_person(this->find_person(in_child));
    if (local_person != nullptr && !local_person->parents_.empty())
    {
      // 親家族を取得する。
      auto const local_parent_family(
        this->find_family(local_person->parents_.back()));
      if (local_parent_family != nullptr)
      {
        // 親家族から子を検索し、関係を終わらせる。
        for (auto& local_child: local_parent_family->childs_)
        {
          if (in_child == local_child.key_)
          {
            auto& local_period(
              const_cast<society::time_range&>(local_child.period_));
            assert(local_period.is_begin());
            return local_period.set_end(in_time)?
              local_person->parents_.back(): society::NULL_FAMILY_KEY;
          }
        }
      }
      assert(false);
    }
    return society::NULL_FAMILY_KEY;
  }

  //---------------------------------------------------------------------------
  private: template<typename template_container>
  static typename template_container::const_pointer find_element(
      template_container const& in_container,
      std::size_t const in_offset,
      std::size_t const in_key)
  {
    if (in_offset <= in_person)
    {
      auto const local_index(in_key - in_offset);
      if (local_index < in_container.size())
      {
        return &in_container.at(local_index);
      }
    }
    return nullptr;
  }

  private: template<typename template_container, typename template_key>
  static template_key find_element_key(
    template_container const& in_container,
    template_key const in_offset,
    typename template_container::const_pointer const in_element,
    template_key const in_null_key)
  {
    if (in_element != nullptr && !in_container.empty())
    {
      auto const local_index(std::distance(&in_container.front(), in_element));
      if (local_index < in_container.size())
      {
        return static_cast<template_key>(in_offset + local_index);
      }
    }
    return in_null_key;
  }

  private: static bool connect_parent_and_child(
    society::time const in_time,
    society::family_key const in_family_key,
    typename this_type::family& io_family,
    society::person_key const in_child_key,
    typename this_type::person& io_child)
  {
    if (in_time != society::NULL_TIME)
    {
      for (auto local_family: io_child.parents_)
      {
      }
      if (!io_family.is_child(in_child_key))
      {
        io_family.children_.emplace_back(in_time, in_child_key);
        io_child.parents_.emplace_back(in_family_key);
        return true;
      }
    }
    return false;
  }

  //---------------------------------------------------------------------------
  private:
  /// @brief 家族の親子関係のコンテナ。
  std::vector<typename this_type::family, template_allocator> families_;
  /// @brief 個人の親子関係のコンテナ。
  std::vector<typename this_type::person, template_allocator> persons_;
  /// @brief 個人の性別のコンテナ。
  std::vector<bool, template_allocator> female_flags_;
  /// @brief 家族コンテナの先頭要素の識別値。
  society::family_key family_begin_;
  /// @brief 個人コンテナの先頭要素の識別値。
  society::person_key person_begin_;

}; // class society::kinship

#endif // !defined(SOCIETY_HPP_)
// vim: set expandtab shiftwidth=2:
