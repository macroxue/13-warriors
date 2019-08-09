#ifndef SET_H_
#define SET_H_

#include <vector>
using namespace std;

#include "card.h"

class Set : public vector<Card*> {
 public:
  Set();
  Set(std::initializer_list<Card*> cards);
  Set(Set::const_iterator begin, Set::const_iterator end);

  void Show(FILE *fp = stdout);

  void SortFromLowToHigh();
  void SortFromHighToLow();

  bool SortedFromLowToHigh() const;
  bool SortedFromHighToLow() const;

  vector<int> Runs() const;

  bool IsStraight(bool three_cards = false) const;
  bool IsFlush(bool three_cards = false) const;
  bool IsStraightFlush() const;
  bool IsRoyalFlush() const;

  void SetInUse(bool in_use);
  bool IsInUse() const;
};

Set operator + (const Set& set1, const Set& set2);

#endif  // SET_H_
