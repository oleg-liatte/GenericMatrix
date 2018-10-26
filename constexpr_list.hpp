#pragma once

#include <stddef.h> // for size_t

/**
   Compile-time list of values
 */
template<typename T, T... Vs>
struct ConstexprList
{
	using type = T;
	static constexpr size_t size = sizeof...(Vs);
	static constexpr bool empty = size == 0;

	static constexpr T get(size_t i) noexcept
	{
		static const T arr[] = {Vs...};
		return arr[i];
	}
};

/**
   Get value from list
 */
template<size_t I, typename IL>
struct ConstexprListGet
{
};

template<size_t I, typename IL>
static constexpr auto ConstexprListGetV = ConstexprListGet<I, IL>::value;

template<typename T, T V, T... Vs>
struct ConstexprListGet<0, ConstexprList<T, V, Vs...>>
{
	static constexpr T value = V;
};

template<size_t I, typename T, T V, T... Vs>
struct ConstexprListGet<I, ConstexprList<T, V, Vs...>> :
	ConstexprListGet<I - 1, ConstexprList<T, Vs...>>
{
};

/**
   Concatenate two value lists
 */
template<typename IL1, typename IL2>
struct ConstexprListCat
{
};

template<typename IL1, typename IL2>
using ConstexprListCatT = typename ConstexprListCat<IL1, IL2>::type;

template<typename T, T... Vs1, T... Vs2>
struct ConstexprListCat<ConstexprList<T, Vs1...>, ConstexprList<T, Vs2...>>
{
	using type = ConstexprList<T, Vs1..., Vs2...>;
};

/**
   Get head of list: range of [0, End).
 */
template<size_t End, typename IL>
struct ConstexprListHead
{
};

template<size_t End, typename IL>
using ConstexprListHeadT = typename ConstexprListHead<End, IL>::type;

template<size_t End, typename HeadList, typename TailList>
struct ConstexprListHeadShift
{
};

template<size_t End, typename HeadList, typename TailList>
struct ConstexprListHeadImpl :
	ConstexprListHeadShift<End, HeadList, TailList>
{
};

template<typename HeadList, typename TailList>
struct ConstexprListHeadImpl<0, HeadList, TailList>
{
	using type = HeadList;
};

template<size_t End, typename T, T... Head, T V, T... Tail>
struct ConstexprListHeadShift<End, ConstexprList<T, Head...>, ConstexprList<T, V, Tail...>> :
	ConstexprListHeadImpl<End - 1, ConstexprList<T, Head..., V>, ConstexprList<T, Tail...>>
{
};

template<size_t End, typename T, T... Vs>
struct ConstexprListHead<End, ConstexprList<T, Vs...>> :
	ConstexprListHeadImpl<End, ConstexprList<T>, ConstexprList<T, Vs...>>
{
};

/**
   Get tail of list: range of [Begin, <size of list>).
 */
template<size_t Begin, typename IL>
struct ConstexprListTail
{
};

template<size_t Begin, typename IL>
using ConstexprListTailT = typename ConstexprListTail<Begin, IL>::type;

template<size_t Begin, typename HeadList, typename TailList>
struct ConstexprListTailShift
{
};

template<size_t Begin, typename HeadList, typename TailList>
struct ConstexprListTailImpl :
	ConstexprListTailShift<Begin, HeadList, TailList>
{
};

template<typename HeadList, typename TailList>
struct ConstexprListTailImpl<0, HeadList, TailList>
{
	using type = TailList;
};

template<size_t Begin, typename T, T... Head, T V, T... Tail>
struct ConstexprListTailShift<Begin, ConstexprList<T, Head...>, ConstexprList<T, V, Tail...>> :
	ConstexprListTailImpl<Begin - 1, ConstexprList<T, Head..., V>, ConstexprList<T, Tail...>>
{
};

template<size_t Begin, typename T, T... Vs>
struct ConstexprListTail<Begin, ConstexprList<T, Vs...>> :
	ConstexprListTailImpl<Begin, ConstexprList<T>, ConstexprList<T, Vs...>>
{
};

/**
   Get range of values of [Begin, End).
 */
template<size_t Begin, size_t End, typename IL>
struct ConstexprListRange :
	ConstexprListHead<End - Begin, ConstexprListTailT<Begin, IL>>
{
};

template<size_t Begin, size_t End, typename IL>
using ConstexprListRangeT = typename ConstexprListRange<Begin, End, IL>::type;

/**
   Erase single value from list.

   Returns value list with corresponding value erased.
 */
template<size_t I, typename IL>
struct ConstexprListErase
{
};

template<size_t I, typename IL>
using ConstexprListEraseT = typename ConstexprListErase<I, IL>::type;

template<size_t I, typename HeadList, typename TailList>
struct ConstexprListEraseImpl
{
};

template<typename T, T... Head, T V, T... Tail>
struct ConstexprListEraseImpl<0, ConstexprList<T, Head...>, ConstexprList<T, V, Tail...>>
{
	using type = ConstexprList<T, Head..., Tail...>;
};

template<size_t I, typename T, T... Head, T V, T... Tail>
struct ConstexprListEraseImpl<I, ConstexprList<T, Head...>, ConstexprList<T, V, Tail...>> :
	ConstexprListEraseImpl<I - 1, ConstexprList<T, Head..., V>, ConstexprList<T, Tail...>>
{
};

template<size_t I, typename T, T... Vs>
struct ConstexprListErase<I, ConstexprList<T, Vs...>> :
	ConstexprListEraseImpl<I, ConstexprList<T>, ConstexprList<T, Vs...>>
{
};

/**
   Generate ConstexprList of Size startting from Start
 */
template<typename T, size_t Size, T Start = 0>
struct ConstexprListSequence
{
	using type = ConstexprListCatT<
		ConstexprList<T, Start>,
		typename ConstexprListSequence<T, Size - 1, Start + 1>::type>;
};

template<typename T, T Start>
struct ConstexprListSequence<T, 0, Start>
{
	using type = ConstexprList<T>;
};

template<typename T, size_t Size, T Start = 0>
using ConstexprListSequenceT = typename ConstexprListSequence<T, Size, Start>::type;
