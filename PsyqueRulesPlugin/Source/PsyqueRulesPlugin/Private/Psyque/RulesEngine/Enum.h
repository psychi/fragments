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
		/// @brief 状態値の型の種別。
		enum class EStatusKind: int8
		{
			Empty    =  0, ///< 空。
			Bool     =  1, ///< 論理値。
			Unsigned =  2, ///< 符号なし整数。
			Signed   = -2, ///< 符号あり整数。
			Float    = -1, ///< 浮動小数点数。
		};

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
		/// @brief 状態値を代入する演算子の種類。
		UENUM(BlueprintType)
		enum class EStatusAssignment: uint8
		{
			Copy, ///< 単純代入。
			Add,  ///< 加算代入。
			Sub,  ///< 減算代入。
			Mul,  ///< 乗算代入。
			Div,  ///< 除算代入。
			Mod,  ///< 除算の余りの代入。
			Or,   ///< 論理和の代入。
			Xor,  ///< 排他的論理和の代入。
			And,  ///< 論理積の代入。
		};

		//---------------------------------------------------------------------
		/// @brief 状態変更の予約系列と遅延方法。
		/// @details
		///   1度の _private::TAccumulator::_flush で状態変更を適用する際に、
		///   1つの状態値に対して異なる予約系列から複数回の状態変更がある場合の、
		///   2回目以降の状態変更の遅延方法を決める。
		///   EAccumulationDelay::Follow と
		///   EAccumulationDelay::Yield の使用を推奨する。
		enum class EAccumulationDelay: uint8
		{
			/// 予約系列を切り替えず、
			/// 直前の状態変更の予約と同じタイミングで状態変更を適用する。
			Follow,

			/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
			/// で、対象となる状態値が既に変更されていた場合、
			/// 同じ予約系列の状態変更の適用が次回以降の
			/// _private::TAccumulator::_flush まで遅延する。
			Yield,

			/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
			/// で、対象となる状態値が既に変更されていた場合、
			/// 以後にある全ての状態変更の適用が次回以降の
			/// _private::TAccumulator::_flush まで遅延する。
			/// @warning
			///   EAccumulationDelay::Block を
			///   _private::TAccumulator::Accumulate に何度も渡すと、
			///   状態変更の予約がどんどん蓄積する場合があるので、注意すること。
			Block,

			/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
			/// で、対象となる状態値が既に変更されていた場合でも、
			/// 遅延せずに状態変更を適用する。
			/// @warning
			///   EAccumulationDelay::Nonblock を
			///   _private::TAccumulator::Accumulate に渡すと、
			///   それ以前の状態変更が無視されることになるので、注意すること。
			Nonblock,
		};

		//---------------------------------------------------------------------
		/// @brief Psyque::RulesEngine の管理者以外は、直接アクセス禁止。
		namespace _private {}

	} // namespace RulesEngine
} // namespace Psyque

// vim: set noexpandtab:
