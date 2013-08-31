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

  bool SortedFromLowToHigh() const;
  bool SortedFromHighToLow() const;

  bool IsFlush() const;
  bool IsStraight() const;
  bool IsRoyalFlush() const;

  void SetInUse(bool in_use);
  bool IsInUse() const;
};

Set operator + (const Set& set1, const Set& set2);

#endif  // SET_H_
