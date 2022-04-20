#include "mxws.hpp"
#include <iostream>
#include <chrono>
#include <numbers>
#include <array>


void Birthday_Probability();

int main()
{
  Birthday_Probability();
  return 0;
}

void Birthday_Probability()
{
  const int Ndays = 365;
  const int TRIALS = 1500000;
  std::array <short int, Ndays> birthdays = {};
  int successfulTrials;
  bool sharedBirthday;
  const int Npeople_with_same_birthday = 2;
  const int N_people = 100;

  mxws rng;

  auto begin = std::chrono::steady_clock::now();

  for (int people = Npeople_with_same_birthday; people <= N_people; ++people) { 

    successfulTrials = 0;
    for (int i = 0; i < TRIALS; ++i) {

      birthdays.fill(0); // set days all to 0
      sharedBirthday = false;
      
      for (int j = 0; j < people; ++j) {
        // if the given birthday is shared (has more than one person)
        // then we have a shared birthday, stop checking
        if (++birthdays[rng(Ndays - 1)] == Npeople_with_same_birthday) {
          sharedBirthday = true;
          break;
        }
      }
      if (sharedBirthday) ++successfulTrials;
    }

    std::cout << "The chance that, in a set of " << std::setw(3) << std::left << people
      << " randomly chosen people, at least " << Npeople_with_same_birthday << " people"
      << " will share a birthday is " << std::setw(9) << std::setprecision(5) << std::fixed
      << 100 * (double(successfulTrials) / double(TRIALS)) << " %"
      << std::endl;
  }
  auto end = std::chrono::steady_clock::now();
  std::cout << "\nTime difference = " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
}