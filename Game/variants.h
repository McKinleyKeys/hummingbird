//
//  variants.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 7/1/22.
//

#pragma once
#ifndef variants_h
#define variants_h

enum Variant: short
{
	CLASSIC,
//	ENERGY_CELLS,
	EXPLODING_KNIGHTS,
	COMPULSION,
	COMPULSION_AND_BACKSTABBING,
//	PETRIFICATION,
	FORCED_CHECK,
	FORCED_CHECK_AND_BACKSTABBING,
	LOSER,
	KING_OF_THE_HILL,
	KING_OF_THE_HILL_AND_COMPULSION,
//	THREE_CHECK,
//	FOG_OF_WAR,
	UNRECOGNIZED_VARIANT,
};
#define VARIANT_COUNT 9

#define FOR_EACH_VARIANT(F) \
	F((Variant)0) \
	F((Variant)1) \
	F((Variant)2) \
	F((Variant)3) \
	F((Variant)4) \
	F((Variant)5) \
	F((Variant)6) \
	F((Variant)7) \
	F((Variant)8) \
//	F((Variant)9) \
//	F((Variant)10) \
//	F((Variant)11) \

namespace Variants
{

/// Returns whether `v` has the rule that players can capture their own pieces (not including their own king).
constexpr bool has_friendly_fire_enabled(Variant v)
{
	return
		v == COMPULSION_AND_BACKSTABBING ||
		v == FORCED_CHECK_AND_BACKSTABBING;
}

/// Returns whether `v` has the rule that players must make a capture if they can.
constexpr bool has_forced_capture_enabled(Variant v)
{
	return
		v == COMPULSION ||
		v == COMPULSION_AND_BACKSTABBING ||
		v == LOSER ||
		v == KING_OF_THE_HILL_AND_COMPULSION;
}

/// Returns whether `v` has the rule that players must put their opponent in check if they can.
constexpr bool has_forced_check_enabled(Variant v)
{
	return v == FORCED_CHECK || v == FORCED_CHECK_AND_BACKSTABBING;
}

/// Returns whether checkmate constitutes a win for `v`.
constexpr bool has_win_by_checkmate(Variant v)
{
	return !(v == LOSER);
}

/// Returns whether checks are ignored in `v`.
constexpr bool has_check_disabled(Variant v)
{
	return v == LOSER;
}

/// Returns whether `v` has additional ways to win the game on top of checkmating.
constexpr bool has_alternative_winning_condition(Variant v)
{
	return
		v == EXPLODING_KNIGHTS ||
		v == KING_OF_THE_HILL ||
		v == KING_OF_THE_HILL_AND_COMPULSION;
}

constexpr bool has_king_of_the_hill(Variant v)
{
	return v == KING_OF_THE_HILL || v == KING_OF_THE_HILL_AND_COMPULSION;
}

/// Returns whether `v` allows winning by capturing the opponent's king.
constexpr bool has_win_by_king_capture(Variant v)
{
	return v == EXPLODING_KNIGHTS;
}

/// Returns whether `v` allows for moves that are "destructive". A move is destructive if it removes information from the board without storing that information in itself. A game that is using a destructive variant will save the destroyed information whenever a destructive move is made so that it can later undo that move.
constexpr bool has_destructive_moves(Variant v)
{
	return v == EXPLODING_KNIGHTS;
}

} // namespace Variants

//#define INSTANTIATE_TEMPLATE_CLASS_WITH_VARIANT(_, VARIANT, CLASS_NAME) template class CLASS_NAME<(Variant)VARIANT>;
//#define INSTANTIATE_TEMPLATE_CLASS_WITH_ALL_VARIANTS(CLASS_NAME) BOOST_PP_REPEAT_FROM_TO(0, VARIANT_COUNT, INSTANTIATE_TEMPLATE_CLASS_WITH_VARIANT, CLASS_NAME)

//template<template<Variant V> typename CLASS, class T>
//struct force_instantiation;
//
//template<template<Variant V> typename CLASS, int... VALUES>
//struct force_instantiation<CLASS, std::integer_sequence<int, VALUES...>> {
//	std::tuple<CLASS<(Variant)VALUES>...> unused;
//};
//
//template<template<Variant V> typename CLASS>
//struct force_instantiation_all {
//	force_instantiation<CLASS, std::make_integer_sequence<int, 12>> unused;
//};

#endif /* variants_h */
