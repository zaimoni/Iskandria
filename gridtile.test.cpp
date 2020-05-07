#include "gridtile.hpp"
#include "gridspace.hpp"

#include "test_driver.h"

int main(int argc, char* argv[])
{	// parse options
	STRING_LITERAL_TO_STDOUT("starting main\n");

	typename iskandria::grid::cartesian<2>::coord_type dest;
	const decltype(dest) origin(7);	// for offset code 3 this is the center of a 16x16 voxel square
	const decltype(dest) quadrant_1(8);	// this test series is meant to be on an exact diagonal
	const decltype(dest) quadrant_3(6);
	bool all_tests_ok = true;

	INC_INFORM("origin: ");
	INC_INFORM(origin[0]);
	INC_INFORM(",");
	INFORM(origin[1]);

	INC_INFORM("quad 1: ");
	INC_INFORM(quadrant_1[0]);
	INC_INFORM(",");
	INFORM(quadrant_1[1]);

	INC_INFORM("quad 3: ");
	INC_INFORM(quadrant_3[0]);
	INC_INFORM(",");
	INFORM(quadrant_3[1]);

	// test series
#pragma start_copy identity_rotation
	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_copy
#pragma substitute quadrant_3 for quadrant_1 in identity_rotation
	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::N, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_substitute

#pragma start_copy origin_rotation
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_copy
#pragma for F in iskandria::compass::E,iskandria::compass::SE,iskandria::compass::S,iskandria::compass::SW,iskandria::compass::W,iskandria::compass::NW
#pragma substitute $F for iskandria::compass::NE in origin_rotation
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	dest = origin;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == origin);

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_1[0], quadrant_1[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(quadrant_3[0], quadrant_3[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
#pragma end_substitute
#pragma done

	// reject diagonal rotations
	dest = quadrant_1;
#pragma start_copy reject_diagonal
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::NE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_copy
#pragma for F in iskandria::compass::SE,iskandria::compass::SW,iskandria::compass::NW
#pragma substitute $F for iskandria::compass::NE in reject_diagonal
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::SE, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::SW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::NW, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
#pragma end_substitute
#pragma done

	// reject E/w for partial-offsets
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(!iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	// verify that S i.e 180 degrees is period 2
	const auto Q1_S0(zaimoni::make<decltype(dest)>(6, 6));
	const auto Q1_S1(zaimoni::make<decltype(dest)>(7, 6));
	const auto Q1_S2(zaimoni::make<decltype(dest)>(6, 7));
	const auto Q1_S3(zaimoni::make<decltype(dest)>(7, 7));
	const auto Q3_S0(zaimoni::make<decltype(dest)>(8, 8));
	const auto Q3_S1(zaimoni::make<decltype(dest)>(9, 8));
	const auto Q3_S2(zaimoni::make<decltype(dest)>(8, 9));
	const auto Q3_S3(zaimoni::make<decltype(dest)>(9, 9));

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S2);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 1, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S2);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 2, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::S, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	// \todo verify that E i.e 270 degrees is period 4
	// \todo verify that W i.e 90 degrees is period 4
	const auto Q1_E0(zaimoni::make<decltype(dest)>(8, 6));
	const auto Q1_E3(zaimoni::make<decltype(dest)>(8, 7));
	const auto Q1_W0(zaimoni::make<decltype(dest)>(6, 8));
	const auto Q1_W3(zaimoni::make<decltype(dest)>(7, 8));
	const auto Q3_E0(zaimoni::make<decltype(dest)>(6, 8));
	const auto Q3_E3(zaimoni::make<decltype(dest)>(6, 9));
	const auto Q3_W0(zaimoni::make<decltype(dest)>(8, 6));
	const auto Q3_W3(zaimoni::make<decltype(dest)>(9, 6));

	dest = quadrant_1;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q1_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_1);

	dest = quadrant_3;
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E0);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 0, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::E, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_W3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_S3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == Q3_E3);
	SUCCEED_OR_DIE(iskandria::grid::exact_rotate(origin[0], origin[1], 3, iskandria::compass::W, dest[0], dest[1]));
	SUCCEED_OR_DIE(dest == quadrant_3);

	STRING_LITERAL_TO_STDOUT("tests finished\n");
}
