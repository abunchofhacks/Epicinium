//usr/bin/g++ src/build/quicktest.cpp -o bin/.quicktest && bin/.quicktest; exit

#include <cstdio>
#include <iostream>
#include <string>
#include <regex>
#include <cmath>


int main(int argc, char* argv[])
{
	std::vector<std::pair<int, int>> scores = {
		{45,0},
		{64,0},
		{100,0},
		{18,0},
		{50,0},
		{0,57},
		{69,0},
		{0,14},
		{44,0},
		{0,72},
		{0,0},
		{0,91},
		{34,0},
		{0,100},
		{0,100},
		{46,0},
		{0,100},
		{100,0},
		{0,59},
		{0,87},
		{0,19},
		{0,24},
		{44,0},
		{0,34},
		{41,0},
		{0,100},
		{67,0},
		{45,0},
		{17,0},
		{39,0},
		{100,0},
		{100,0},
		{54,0},
		{1,0},
		{49,0},
		{0,100},
		{17,0},
		{43,0},
		{67,0},
		{51,0},
		{0,32},
		{43,0},
		{0,114},
		{0,0},
		{0,93},
		{0,87},
		{28,0},
		{34,0},
		{37,0},
		{32,0},
		{48,0},
		{0,0},
		{0,55},
		{0,0},
		{98,0},
		{0,0},
		{54,0},
		{100,0},
		{0,57},
		{62,0},
		{0,79},
		{0,0},
		{0,41},
		{99,0},
		{0,0},
		{0,0},
		{19,0},
		{71,0},
		{90,0},
		{0,100},
		{57,0},
	};

	double eloAverage = 50;
	double eloSpread = 20;

	double eloPlayer1 = eloAverage;
	double eloPlayer2 = eloAverage;
	std::vector<double> eloBaseline(100, eloAverage);

	constexpr int NUM_ITERATIONS = 10000;

	for (int t = 0; t < NUM_ITERATIONS; t++)
	{
		for (const std::pair<int, int>& match : scores)
		{
			int scorePlayer1 = std::get<0>(match);
			int scorePlayer2 = std::get<1>(match);

			int hasWon1 = (scorePlayer1 > scorePlayer2)
				|| (scorePlayer1 == scorePlayer2 && (rand() % 2 == 0));
			int hasWon2 = 1 - hasWon1;

			double K1 = 0.2 * eloSpread;
			double K2 = K1;
			double Kstar = 0.25 * (K1 + K2);
			double Kbase = Kstar;

			double expected1 = 1 / (1 + pow(10.0,
				(eloPlayer2 - eloPlayer1) / eloSpread));
			double expected2 = 1.0 - expected1;

			double newEloPlayer1 = eloPlayer1 + K1 * (hasWon1 - expected1);
			double newEloPlayer2 = eloPlayer2 + K2 * (hasWon2 - expected2);

			for (int b = 0; b <= 99; b++)
			{
				if (hasWon1)
				{
					double expectedb = 1 / (1 + pow(10.0,
						(eloBaseline[b] - eloPlayer1) / eloSpread));
					newEloPlayer1 += 0.01 * Kstar
						* ((scorePlayer1 > b) - expectedb);
					eloBaseline[b] = eloBaseline[b] + 0.01 * Kbase
						* (expectedb - (scorePlayer1 > b));
				}
				else if (hasWon2)
				{
					double expectedb = 1 / (1 + pow(10.0,
						(eloBaseline[b] - eloPlayer2) / eloSpread));
					newEloPlayer2 += 0.01 * Kstar
						* ((scorePlayer2 > b) - expectedb);
					eloBaseline[b] = eloBaseline[b] + 0.01 * Kbase
						* (expectedb - (scorePlayer2 > b));
				}
			}

			// This adds inflation.
			//if (hasWon1 && newEloPlayer1 < eloPlayer1)
			//{
			//	newEloPlayer1 = eloPlayer1;
			//}
			//if (hasWon2 && newEloPlayer2 < eloPlayer2)
			//{
			//	newEloPlayer2 = eloPlayer2;
			//}

			eloPlayer1 = newEloPlayer1;
			eloPlayer2 = newEloPlayer2;

			if (t == NUM_ITERATIONS - 1)
			{
				std::cout << "" << scorePlayer1 << " vs " << scorePlayer2
					<< " (expected " << ((int) (expected1 * 100)) << "%)"
					<< "  ==>  "
					<< "P1: " << ((int) eloPlayer1) << ", "
					<< "P2: " << ((int) eloPlayer2) << ", "
					<< "0: " << ((int) eloBaseline[0]) << ", "
					<< "50: " << ((int) eloBaseline[50]) << ", "
					<< "99: " << ((int) eloBaseline[99])
					<< std::endl;
			}
		}

		if ((t % 10) == 0)
		{
			std::cout << t
				<< "  ///  "
				<< "P1: " << ((int) eloPlayer1) << ", "
				<< "P2: " << ((int) eloPlayer2) << ", "
				<< "0: " << ((int) eloBaseline[0]) << ", "
				<< "50: " << ((int) eloBaseline[50]) << ", "
				<< "99: " << ((int) eloBaseline[99])
				<< std::endl;
		}
	}

	std::cout << std::endl
		<< "P1: " << ((int) eloPlayer1) << ", "
		<< "P2: " << ((int) eloPlayer2)
		<< std::endl;

	for (int b = 0; b <= 99; b++)
	{
		if (b % 10 == 0)
		{
			std::cout << std::endl;
		}
		std::cout << "\t " << b << ": " << ((int) eloBaseline[b]);
	}
	std::cout << std::endl;

	return 0;
}
