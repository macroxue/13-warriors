#include <assert.h>

#include "set.h"

Set::Set() {
}

Set::Set(std::initializer_list<Card*> cards)
  : vector<Card*>(cards) {
}

void Set::SortFromLowToHigh() {
  if (empty()) {
    return;
  }
  for (int i = 0; i < size()-1; ++i) {
    for (int j = i+1; j < size(); ++j) {
      if ((*this)[i]->rank > (*this)[j]->rank) {
        std::swap((*this)[i], (*this)[j]);
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
      if ((*this)[i]->rank < (*this)[j]->rank) {
        std::swap((*this)[i], (*this)[j]);
      }
    }
  }
}

bool Set::SortedFromLowToHigh() {
  for (int i = 1; i < size(); ++i) {
    if ((*this)[i-1]->rank > (*this)[i]->rank) {
      return false;
    }
  }
  return true;
}

bool Set::IsFlush() {
  int suit = (*this)[0]->suit;
  for (auto card : *this) {
    if (card->suit != suit) {
      return false;
    }
  }
  return true;
}

bool Set::IsStraight() {
  assert(SortedFromLowToHigh());
  int num_cards = size();
  for (int i = 0; i < num_cards-1; ++i) {
    if ((*this)[i]->rank + 1 != (*this)[i+1]->rank) {
      return false;
    }
  }
  // The case of A2345 but sorted as 2345A.
  return (*this)[num_cards-2]->rank + 1 == (*this)[num_cards-1]->rank ||
    ((*this)[0]->rank == TWO && (*this)[num_cards-1]->rank == ACE);
}

bool Set::IsRoyalFlush() {
  return IsFlush() && IsStraight() && front()->rank == TEN && back()->rank == ACE;
}

void Set::SetInUse(bool in_use) {
  for (auto card : *this) {
    card->in_use = in_use;
  }
}

bool Set::IsInUse() {
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
