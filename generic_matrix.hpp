#pragma once

#include "generic_vector.hpp"

#include "constexpr_list.hpp"

#include <array>
#include <type_traits>

/**
   Generic submatrix.

   Refers to another matrix with optionally reduced rows/columns set.
 */
template<
	typename TMatrix,
	typename RowList = ConstexprListSequenceT<size_t, TMatrix::rows()>,
	typename ColumnList = ConstexprListSequenceT<size_t, TMatrix::columns()>>
class GenericSubMatrix
{
public:
	using matrix_type = TMatrix;
	using value_type = typename matrix_type::value_type;
	using row_type = GenericSubVector<typename matrix_type::row_type, ColumnList>;
	using const_row_type = GenericSubVector<const typename matrix_type::row_type, ColumnList>;

	constexpr GenericSubMatrix(matrix_type& matrix)
		: m_matrix(matrix)
	{
	}

	static constexpr size_t columns() noexcept
	{
		return ColumnList::size;
	}

	static constexpr size_t rows() noexcept
	{
		return RowList::size;
	}

	static constexpr size_t dimension() noexcept
	{
		return std::max(ColumnList::size, RowList::size);
	}

	template<size_t ROW>
	constexpr row_type get() noexcept
	{
		return m_matrix.get<ConstexprListGetV<ROW, RowList>>();
	}

	template<size_t ROW>
	constexpr const_row_type get() const noexcept
	{
		return m_matrix.get<ConstexprListGetV<ROW, RowList>>();
	}

	template<size_t ROW, size_t COLUMN>
	constexpr value_type get() const noexcept
	{
		if constexpr (ROW < RowList::size && COLUMN < ColumnList::size)
		{
			return get<ROW>().get<COLUMN>();
		}
		else
		{
			if constexpr (ROW == COLUMN)
			{
				return row_type::IDENTITY_VALUE;
			}
			else
			{
				return row_type::ZERO_VALUE;
			}
		}
	}

	template<size_t ROW, size_t COLUMN>
	constexpr void set(value_type v) noexcept
	{
		if constexpr (ROW < RowList::size && COLUMN < ColumnList::size)
		{
			get<ROW>().set<COLUMN>(std::move(v));
		}
	}

	constexpr row_type operator[](size_t i) noexcept
	{
		return m_matrix[RowList::get(i)];
	}

	constexpr const_row_type operator[](size_t i) const noexcept
	{
		return m_matrix[RowList::get(i)];
	}

	template<size_t ROW, size_t COLUMN>
	constexpr GenericSubMatrix<
		matrix_type,
		ConstexprListEraseT<ROW, RowList>,
		ConstexprListEraseT<COLUMN, ColumnList>
	> subMatrix() const
	{
		return m_matrix;
	}

	constexpr value_type determinant() const noexcept
	{
		if constexpr (dimension() == 1)
		{
			return get<0, 0>();
		}
		else
		{
			return det<dimension() - 1>();
		}
	}

private:
	template<size_t I>
	constexpr value_type det() const noexcept
	{
		auto d = get<0, I>();
		if constexpr (I % 2)
		{
			d = -d;
		}

		d *= subMatrix<0, I>().determinant();

		if constexpr (I > 0)
		{
			d += det<I - 1>();
		}

		return d;
	}

	matrix_type& m_matrix;
};

/**
   Generic matrix of given type and dimensions
 */
template<typename T, size_t COLUMNS, size_t ROWS>
class GenericMatrix :
	public std::array<GenericVector<T, COLUMNS>, ROWS>
{
public:
	using value_type = T;
	using row_type = GenericVector<value_type, COLUMNS>;
	using row_index_list = ConstexprListSequenceT<size_t, ROWS>;
	using column_index_list = ConstexprListSequenceT<size_t, COLUMNS>;

	static constexpr size_t columns() noexcept
	{
		return COLUMNS;
	}

	static constexpr size_t rows() noexcept
	{
		return ROWS;
	}

	static constexpr size_t dimension() noexcept
	{
		return std::max(COLUMNS, ROWS);
	}

	template<size_t ROW>
	constexpr row_type& get() noexcept
	{
		return (*this)[ROW];
	}

	template<size_t ROW>
	constexpr const row_type& get() const noexcept
	{
		return (*this)[ROW];
	}

	template<size_t ROW, size_t COLUMN>
	constexpr value_type get() const noexcept
	{
		if constexpr (ROW < ROWS && COLUMN < COLUMNS)
		{
			return get<ROW>().get<COLUMN>();
		}
		else
		{
			if constexpr (ROW == COLUMN)
			{
				return row_type::IDENTITY_VALUE;
			}
			else
			{
				return row_type::ZERO_VALUE;
			}
		}
	}

	template<size_t ROW, size_t COLUMN>
	constexpr void set(value_type v) noexcept
	{
		if constexpr (ROW < ROWS && COLUMN < COLUMNS)
		{
			get<ROW>().set<COLUMN>(std::move(v));
		}
	}

	template<typename TBody>
	static constexpr void forEachRow(TBody body)
	{
		forEachRowImpl<0>(body);
	}

	template<typename TBody>
	static constexpr void forEach(TBody body)
	{
		forEachRow(
			[&](auto row)
			{
				row_type::forEachElement(
					[&](auto column)
					{
						body(row, column);
					});
			});
	}

	template<size_t ROW, size_t COLUMN>
	constexpr GenericSubMatrix<
		GenericMatrix,
		ConstexprListEraseT<ROW, row_index_list>,
		ConstexprListEraseT<COLUMN, column_index_list>
	> subMatrix() noexcept
	{
		return *this;
	}

	template<size_t ROW, size_t COLUMN>
	constexpr GenericSubMatrix<
		const GenericMatrix,
		ConstexprListEraseT<ROW, row_index_list>,
		ConstexprListEraseT<COLUMN, column_index_list>
	> subMatrix() const noexcept
	{
		return *this;
	}

	constexpr GenericMatrix<value_type, ROWS, COLUMNS> transpose() const
	{
		GenericMatrix<value_type, ROWS, COLUMNS> result;
		for (size_t r = 0; r != ROWS, ++r)
		{
			for (size_t c = 0; c != COLUMNS; ++c)
			{
				result[c][r] = (*this)[r][c];
			}
		}
		return result;
	}

	constexpr value_type determinant() const noexcept
	{
		return GenericSubMatrix<const GenericMatrix>(*this).determinant();
	}

	constexpr bool invert(GenericMatrix& out) const noexcept
	{
		auto det = determinant();
		if (det == row_type::ZERO_VALUE)
		{
			return false;
		}

		invert(out, det);
		return true;
	}

	constexpr GenericMatrix invert() const noexcept
	{
		auto det = determinant();
		assert(det != row_type::ZERO_VALUE);

		GenericMatrix out;
		invert(out, det);
		return out;
	}

	static constexpr GenericMatrix identity() noexcept
	{
		GenericMatrix result = {};
		for (size_t i = 0; i < ROWS && i < COLUMNS; ++i)
		{
			result[i][i] = row_type::IDENTITY_VALUE;
		}
		return result;
	}

	constexpr bool operator==(const GenericMatrix& rhs) const
	{
		for (size_t r = 0; r != ROWS; ++r)
		{
			for (size_t c = 0; c != COLUMNS; ++c)
			{
				if ((*this)[r][c] != rhs[r][c])
				{
					return false;
				}
			}
		}

		return true;
	}

	constexpr bool operator!=(const GenericMatrix& rhs) const
	{
		return !(*this == rhs);
	}

private:
	template<size_t ROW, typename TBody>
	static constexpr void forEachRowImpl(TBody& body)
	{
		if constexpr (ROW < ROWS)
		{
			body(std::integral_constant<size_t, ROW>());
			forEachRowImpl<ROW + 1>(body);
		}
	}

	constexpr void invert(GenericMatrix& out, value_type det) const noexcept
	{
		forEach(
			[&](auto row, auto column)
			{
				auto c = this->subMatrix<column.value, row.value>().determinant();
				if constexpr ((row.value + column.value) % 2)
				{
					c = -c;
				}
				auto& r = out[row.value];
				r[column.value] = c / det;
			});
	}
};

template<size_t I, typename T, size_t COLUMNS, size_t ROWS>
constexpr GenericVector<T, COLUMNS>& get(GenericMatrix<T, COLUMNS, ROWS>& v) noexcept
{
	static_assert(I < ROWS, "GenericMatrix row index out of bounds");
	return v[I];
}

template<size_t I, typename T, size_t COLUMNS, size_t ROWS>
constexpr const GenericVector<T, COLUMNS>& get(const GenericMatrix<T, COLUMNS, ROWS>& v) noexcept
{
	static_assert(I < ROWS, "GenericMatrix row index out of bounds");
	return v[I];
}

template<typename T, size_t COLUMNS, size_t ROWS, size_t N>
constexpr GenericMatrix<T, COLUMNS, ROWS> operator*(
	const GenericMatrix<T, N, ROWS>& lhs,
	const GenericMatrix<T, COLUMNS, N>& rhs)
{
	GenericMatrix<T, COLUMNS, ROWS> result;

	result.forEachRow(
		[&](auto row)
		{
			result[row.value] = lhs[row.value] * rhs;
		});

	return result;
}

template<typename T, size_t N>
constexpr GenericMatrix<T, N, N>& operator*=(
	GenericMatrix<T, N, N>& lhs,
	const GenericMatrix<T, N, N>& rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

template<typename T, size_t COLUMNS, size_t ROWS, size_t N>
constexpr std::enable_if_t<
	N <= ROWS,
	GenericVector<T, COLUMNS>
> operator*(
	const GenericVector<T, N>& lhs,
	const GenericMatrix<T, COLUMNS, ROWS>& rhs)
{
	GenericVector<T, COLUMNS> result;
	result.forEachElement(
		[&](auto column)
		{
			auto e = result.ZERO_VALUE;
			lhs.forEachElement(
				[&](auto row)
				{
					e += lhs[row.value] * rhs[row.value][column.value];
				});
			result[column.value] = e;
		});
	return result;
}

template<typename T, size_t COLUMNS, size_t ROWS, size_t N>
constexpr std::enable_if_t<
	N <= ROWS,
	GenericVector<T, N>&
> operator*=(
	GenericVector<T, N>& lhs,
	const GenericMatrix<T, COLUMNS, ROWS>& rhs)
{
	lhs = lhs * rhs;
	return lhs;
}

namespace std
{
template<size_t I, typename T, size_t COLUMNS, size_t ROWS>
struct tuple_element<I, BW::GenericMatrix<T, COLUMNS, ROWS>>
{
	using type = BW::GenericVector<T, COLUMNS>;
};

template<typename T, size_t COLUMNS, size_t ROWS>
struct tuple_size<BW::GenericMatrix<T, COLUMNS, ROWS>> :
	integral_constant<size_t, ROWS>
{
};
} // namespace std
