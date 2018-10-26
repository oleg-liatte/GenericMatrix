#include "generic_matrix.hpp"
#include <catch.hpp>
#include <atomic>
#include <intrin.h>

namespace
{
using Matrix = GenericMatrix<int, 2, 2>;
using Vector = Matrix::row_type;

// rotation to 90 deg CW
constexpr Matrix s_cw =
{
	0, -1,
	1, 0
};

// rotation to 90 deg CCW
constexpr Matrix s_ccw =
{
	0, 1,
	-1, 0
};
} // namespace

TEST_CASE("GenericMatrix")
{
	Matrix m =
	{
		1, -2,
		2, 3
	};

	CHECK(m.get<0, 0>() == 1);
	CHECK(m.get<0, 1>() == -2);
	CHECK(m.get<1, 0>() == 2);
	CHECK(m.get<1, 1>() == 3);

	CHECK(m[0][0] == 1);
	CHECK(m[0][1] == -2);
	CHECK(m[1][0] == 2);
	CHECK(m[1][1] == 3);

	m.set<1, 0>(4);
	CHECK(m.get<1, 0>() == 4);
	CHECK(m[1][0] == 4);

	m[1][0] = 2;
	CHECK(m.get<1, 0>() == 2);
	CHECK(m[1][0] == 2);

	CHECK(m.determinant() == 7);
}

TEST_CASE("GenericMatrix inversion")
{
	auto d = s_ccw.determinant();
	CHECK(d == 1);

	auto inv = s_ccw.invert();
	CHECK(inv == s_cw);
}

TEST_CASE("GenericMatrix multiplication")
{
	auto m = s_ccw * s_cw;
	CHECK(m == Matrix::identity());
}

TEST_CASE("GenericMatrix vector transform")
{
	auto v = Vector{1, 0};
	auto vt = v * s_ccw;
	CHECK(vt == Vector{0, 1});
}

TEST_CASE("GenericMatrix point transform: origin")
{
	GenericMatrix<int, 2, 3> ccw =
	{
		0, 1,
		-1, 0,
		0, 0
	};

	Vector v = {0, 1};

	auto v2 = v * ccw;

	CHECK(v2 == Vector{-1, 0});
}

TEST_CASE("GenericMatrix point transform: pivot")
{
	using M = GenericMatrix<int, 3, 3>;
	using V = GenericVector<int, 3>;
	M ccw =
	{
		0, 1, 0,
		-1, 0, 0,
		0, 0, 1
	};

	M pivot =
	{
		1, 0, 0,
		0, 1, 0,
		1, 2, 1
	};

	auto ccwAroundPivot = pivot.invert() * ccw * pivot;

	V v = {1, 1, 1};

	v *= ccwAroundPivot;
	CHECK(v == V{2, 2, 1});

	v *= ccwAroundPivot;
	CHECK(v == V{1, 3, 1});

	v *= ccwAroundPivot;
	CHECK(v == V{0, 2, 1});

	v *= ccwAroundPivot;
	CHECK(v == V{1, 1, 1});
}
