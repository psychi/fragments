// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief 列挙型の一覧。
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

namespace Psyque
{
	//-------------------------------------------------------------------------
	/// @brief if-then規則で駆動する有限状態機械。
	namespace RulesEngine
	{
		//---------------------------------------------------------------------
		/// @brief 状態値の大小関係。
		enum class EStatusOrder: int8
		{
			Failed = -2, ///< 比較に失敗。
			Less,        ///< 左辺のほうが小さい。
			Equal,       ///< 左辺と右辺は等価。
			Greater,     ///< 左辺のほうが大きい。
		};

		//---------------------------------------------------------------------
		/// @brief 状態値を比較する演算子の種類。
		enum class EStatusComparison: uint8
		{
			Equal,        ///< 等価。
			NotEqual,     ///< 非等価。
			Less,         ///< 小なり。
			LessEqual,    ///< 以下。
			Greater,      ///< 大なり。
			GreaterEqual, ///< 以上。
		};

		//---------------------------------------------------------------------
		/// @brief Psyque::RulesEngine の管理者以外は、直接アクセス禁止。
		namespace _private {}

	} // namespace RulesEngine
} // namespace Psyque

// vim: set noexpandtab:
