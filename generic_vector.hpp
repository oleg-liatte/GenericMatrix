#pragma once

#include <cstdmf/constexpr_list.hpp>

#include <array>
#include <type_traits>

namespace BW
{
/**
   Generic subvector.

   Refers to another vector with optionally reduced elements set.
 */
template<typename TVector, typename TIndexList>
class GenericSubVector
{
public:
	using index_list = TIndexList;
	using vector_type = TVector;
	using value_type = std::conditional_t<
		std::is_const_v<vector_type>,
		const typename vector_type::value_type,
		typename vector_type::value_type>;

	static constexpr value_type ZERO_VALUE = 0;
	static constexpr value_type IDENTITY_VALUE = 1;

	constexpr GenericSubVector(vector_type& vector)
		: m_vector(vector)
	{
	}

	constexpr size_t size() const noexcept
	{
		return index_list::size;
	}

	template<size_t I>
	constexpr value_type get() const noexcept
	{
		if constexpr (I < index_list::size)
		{
			return m_vector.get<ConstexprListGetV<I, index_list>>();
		}
		else
		{
			return ZERO_VALUE;
		}
	}

	template<size_t I>
	constexpr void set(value_type v) noexcept
	{
		if constexpr (I < index_list::size)
		{
			m_vector.set<ConstexprListGetV<I, index_list>>(std::move(v));
		}
	}

	constexpr value_type& operator[](size_t i) noexcept
	{
		return m_vector[index_list::get(i)];
	}

	constexpr const value_type& operator[](size_t i) const noexcept
	{
		return m_vector[index_list::get(i)];
	}

	template<typename IL>
	constexpr operator GenericSubVector<value_type, IL>() const
	{
		return m_vector;
	}

private:
	vector_type& m_vector;
};

/**
   Generic vector of given type and size.
 */
template<typename T, size_t N>
class GenericVector :
	public std::array<T, N>
{
public:
	using value_type = T;

	static constexpr value_type ZERO_VALUE = 0;
	static constexpr value_type IDENTITY_VALUE = 1;

	template<size_t I>
	constexpr value_type get() const noexcept
	{
		if constexpr (I < N)
		{
			return (*this)[I];
		}
		else
		{
			return ZERO_VALUE;
		}
	}

	template<size_t I>
	constexpr void set(value_type v) noexcept
	{
		if constexpr (I < N)
		{
			(*this)[I] = std::move(v);
		}
	}

	template<typename TBody>
	static constexpr void forEachElement(TBody body)
	{
		forEachElementImpl<0>(body);
	}

	constexpr operator GenericVector<value_type, N - 1> &()
	{
		return reinterpret_cast<GenericVector<value_type, N - 1> &>(*this);
	}

	constexpr operator const GenericVector<value_type, N - 1> &() const
	{
		return reinterpret_cast<const GenericVector<value_type, N - 1> &>(*this);
	}

	constexpr bool operator==(const GenericVector& rhs) const
	{
		for (size_t i = 0; i != N; ++i)
		{
			if ((*this)[i] != rhs[i])
			{
				return false;
			}
		}

		return true;
	}

	constexpr bool operator!=(const GenericVector& rhs) const
	{
		return !(*this == rhs);
	}

private:
	template<size_t I, typename TBody>
	static constexpr void forEachElementImpl(TBody& body)
	{
		if constexpr (I < N)
		{
			body(std::integral_constant<size_t, I>());
			forEachElementImpl<I + 1>(body);
		}
	}
};

template<size_t I, typename T, size_t N>
constexpr T& get(GenericVector<T, N>& v) noexcept
{
	static_assert(I < N, "GenericVector index out of bounds");
	return v[I];
}

template<size_t I, typename T, size_t N>
constexpr const T& get(const GenericVector<T, N>& v) noexcept
{
	static_assert(I < N, "GenericVector index out of bounds");
	return v[I];
}
} // namespace BW

namespace std
{
template<size_t I, typename T, size_t N>
struct tuple_element<I, BW::GenericVector<T, N>>
{
	using type = T;
};

template<typename T, size_t N>
struct tuple_size<BW::GenericVector<T, N>> :
	integral_constant<size_t, N>
{
};
} // namespace std
