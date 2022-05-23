#include "mxws.hpp"
#include <iostream>
#include <chrono>
#include <numbers>
#include <array>

void Birthday_Probability(
	int Ndays = 365,
	int TRIALS = 1500000,
	int Npeople_with_same_birthday = 2,
	int N_people = 100
);

int main()
{
	int Ndays = 365;
	int TRIALS = 1500000;
	int Npeople_with_same_birthday = 2;
	int N_people = 100;

	Birthday_Probability(TRIALS, Ndays, Npeople_with_same_birthday, N_people);

	return 0;
}

void Birthday_Probability(
	int TRIALS,
	int Ndays,
	int Npeople_with_same_birthday,
	int N_people
) {

	std::vector<short int> birthdays(Ndays);

	int successfulTrials;
	bool sharedBirthday;

	mxws rng;

	auto begin = std::chrono::steady_clock::now();

	std::cout <<
		"Number of days " << Ndays << std::endl <<
		"Number of Trials " << TRIALS << std::endl <<
		"Number of people with the same birthday " << Npeople_with_same_birthday << std::endl <<
		"Number of people " << N_people << std::endl << std::endl;

	std::cout << "The chance that, in a set of N"
		<< " randomly chosen people, at least " << Npeople_with_same_birthday << " people"
		<< " will share a birthday is " << std::endl;

	for (auto people = Npeople_with_same_birthday; people <= N_people; ++people) {

		successfulTrials = 0;
		for (auto i = 0; i < TRIALS; ++i) {

			std::ranges::fill(birthdays, 0);
			sharedBirthday = false;

			for (auto j = 0; j < people; ++j) {
				// if the given birthday is shared (has more than one person)
				// then we have a shared birthday, stop checking
				if (++birthdays[rng(Ndays - 1ull)] == Npeople_with_same_birthday) {
					sharedBirthday = true;
					break;
				}
			}
			if (sharedBirthday) ++successfulTrials;
		}

		std::cout << std::setprecision(5) << std::fixed << " people " << std::setw(3)
			<< people << " chance "
			<< std::setw(9)
			<< 100 * (double(successfulTrials) / TRIALS) << " %"
			<< std::endl;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "\nTime difference = " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
}