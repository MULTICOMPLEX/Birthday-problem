
#include "mxws.hpp"
#include <iostream>
#include <chrono>
#include <numbers>


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
  short int birthdays[Ndays] = {};
  int successfulTrials;
  bool sharedBirthday;
  const int Npeople_with_same_birthday = 2;

  mxws rng;

  auto begin = std::chrono::steady_clock::now();

  for (int people = 2; people < 101; ++people) {

    successfulTrials = 0;
    for (int i = 0; i < TRIALS; ++i) {
      for (int j = 0; j < Ndays; birthdays[j++] = 0); // set days all to 0
      sharedBirthday = false;
      for (int j = 0; j < people; ++j) {
        // if the given birthday is shared (has more than one person)
        // then we have a shared birthday, stop checking
        if (++birthdays[rng(Ndays-1)] > Npeople_with_same_birthday - 1) {
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
