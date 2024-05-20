
import <chrono>;
import <iostream>;
import <algorithm>;
import random;
#include "matplotlib.hpp"

plot_matplotlib plot;

template <typename T>
void Birthday_Probability(
	int Ndays = 365,
	int TRIALS = 150000,
	int Npeople_with_same_birthday = 2,
	int N_people = 100,
	std::vector<T>& X = {},
	std::vector<T>& Y = {}
);

std::string utf8_encode(std::u8string const& s)
{
	return (const char*)(s.c_str());
}

std::ostream& operator <<
(
	std::ostream& o,
	std::u8string& s
	)
{
	o << (const char*)(s.c_str());
	return o;
}

int main() 
{  
	std::setlocale(LC_ALL, "en_US.utf8");

	int Ndays = 365; 
	int TRIALS = 1500000;
	int Npeople_with_same_birthday = 2;
	int N_people = 100;

	std::vector<double> X = { 0 }, Y = {0};
	
	Birthday_Probability(TRIALS, Ndays, Npeople_with_same_birthday, N_people, X, Y);

	plot.plot_somedata(X, Y, "", std::to_string(Npeople_with_same_birthday) + " people", "blue");
	plot.set_xlabel("Number of people");
	plot.set_ylabel("Percentage Change");
	plot.grid_on();
	std::u8string title = u8"Birthday problem";
	plot.set_title(utf8_encode(title));
	plot.show();

	return 0;
}
 

template <typename T>
void Birthday_Probability(
	int TRIALS,
	int Ndays,
	int Npeople_with_same_birthday,
	int N_people,
	std::vector<T>& X,
	std::vector<T>& Y
) {
	
	std::vector<short int> birthdays(Ndays);

	int successfulTrials;
	bool sharedBirthday;
	 
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

		double y = 100 * (double(successfulTrials) / TRIALS);
		X.push_back(people);
		Y.push_back(y);

		std::cout << std::setprecision(5) << std::fixed << " people " << std::setw(3)
			<< people << " chance "
			<< std::setw(9)
			<< y << " %"
			<< std::endl;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "\nTime difference = " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
} 