#include <assert.h>

#include <algorithm>

#include "set.h"

Set::Set() {
  reserve(8);
}

Set::Set(std::initializer_list<Card*> cards)
  : vector<Card*>(cards) {
}

Set::Set(Set::const_iterator begin, Set::const_iterator end)
  : vector<Card*>(begin, end) {
}

void Set::Show(FILE *fp) {
  for (auto card : *this) {
    card->Show(fp);
  }
}

void Set::SortFromLowToHigh() {
  if (empty()) {
    return;
  }
  for (int i = 0; i < size()-1; ++i) {
    for (int j = i+1; j < size(); ++j) {
      if (at(i)->rank > at(j)->rank) {
        std::swap(at(i), at(j));
      }
    }
  }
}

void Set::SortFromHighToLow() {
  if (empty()) {
    return;
  }
  for (int i = 0; i < size()-1; ++i) {
    for (int j = i+1; j < size(); ++j) {
      if (at(i)->rank < at(j)->rank) {
        std::swap(at(i), at(j));
      }
    }
  }
}

bool Set::SortedFromLowToHigh() const {
  for (int i = 1; i < size(); ++i) {
    if (at(i-1)->rank > at(i)->rank) {
      return false;
    }
  }
  return true;
}

bool Set::SortedFromHighToLow() const {
  for (int i = 1; i < size(); ++i) {
    if (at(i-1)->rank < at(i)->rank) {
      return false;
    }
  }
  return true;
}

bool Set::IsStraight(bool three_cards) const {
  assert(SortedFromLowToHigh());
  if ((three_cards && size() != 3) || (!three_cards && size() != 5)) {
    return false;
  }
  for (int i = 0; i < size()-2; ++i) {
    if (at(i)->rank + 1 != at(i+1)->rank) {
      return false;
    }
  }
  // The case of A2345 but sorted as 2345A.
  return at(size()-2)->rank + 1 == back()->rank ||
    (front()->rank == TWO && back()->rank == ACE);
}

bool Set::IsFlush(bool three_cards) const {
  if ((three_cards && size() != 3) || (!three_cards && size() != 5)) {
    return false;
  }
  int suit = at(0)->suit;
  for (auto card : *this) {
    if (card->suit != suit) {
      return false;
    }
  }
  return true;
}

bool Set::IsStraightFlush() const {
  return IsFlush() && IsStraight();
}

bool Set::IsRoyalFlush() const {
  return IsStraightFlush() && front()->rank == TEN && back()->rank == ACE;
}

void Set::SetInUse(bool in_use) {
  for (auto card : *this) {
    card->in_use = in_use;
  }
}

bool Set::IsInUse() const {
  for (auto card : *this) {
    if (card->in_use) {
      return true;
    }
  }
  return false;
}

Set operator + (const Set& set1, const Set& set2) {
  Set set;
  set.insert(set.end(), set1.begin(), set1.end());
  set.insert(set.end(), set2.begin(), set2.end());
  return set;
}
