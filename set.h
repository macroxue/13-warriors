#ifndef SET_H_
#define SET_H_

#include <vector>
using namespace std;

#include "card.h"

class Set : public vector<Card*> {
 public:
  Set();
  Set(std::initializer_list<Card*> cards);

  void SortFromLowToHigh();
  void SortFromHighToLow();

  bool SortedFromLowToHigh();

  bool IsFlush();
  bool IsStraight();
  bool IsRoyalFlush();

  void SetInUse(bool in_use);
  bool IsInUse();
};

Set operator + (const Set& set1, const Set& set2);

#endif  // SET_H_
