#include <assert.h>

#include "pattern.h"

Pattern::Pattern()
  : pattern_(JUNK) {
}

Pattern::Pattern(const Set& set)
  : Set(set) {
  SortFromLowToHigh();

  if (IsRoyalFlush()) {
    pattern_ = ROYAL_FLUSH;
  } else if (IsStraightFlush()) {
    pattern_ = STRAIGHT_FLUSH;
  } else if (IsFlush()) {
    pattern_ = FLUSH;
  } else if (IsStraight()) {
    pattern_ = STRAIGHT;
  } else {
    Set ranks[NUM_RANKS];
    for (const auto& card : *this) {
      ranks[card->rank].push_back(card);
    }
    pattern_ = JUNK;
    for (int r = NUM_RANKS-1; r >= TWO && pattern_ == JUNK; --r) {
      if (ranks[r].size() == 4) {
        *this = Pattern(ranks[r], FOUR_OF_A_KIND);
      } else if (ranks[r].size() == 3) {
        *this = Pattern(ranks[r], TRIPLE);
      }
    }
    for (int r = NUM_RANKS-1; r >= TWO && pattern_ <= TRIPLE; --r) {
      if (ranks[r].size() == 2) {
        if (pattern_ == TRIPLE) {
          *this = Pattern(ranks[r] + this->set(), FULL_HOUSE);
        } else if (pattern_ == PAIR) {
          *this = Pattern(ranks[r] + this->set(), TWO_PAIRS);
        } else if (pattern_ == JUNK) {
          *this = Pattern(ranks[r], PAIR);
        }
      }
    }
    for (int r = NUM_RANKS-1; r >= TWO && size() < set.size(); --r) {
      if (ranks[r].size() == 1) {
        *this = Pattern(ranks[r] + this->set(), pattern_);
      }
    }
  }
}

Pattern::Pattern(const Set& set, int pattern)
  : Set(set), pattern_(pattern) {
  if ((pattern == STRAIGHT || pattern == STRAIGHT_FLUSH) &&
      set.front()->rank == TWO && set.back()->rank == ACE) {
    // Move ACE to the front so it becomes ONE.
    auto ace = back();
    pop_back();
    insert(begin(), ace);
  }
}

void Pattern::Show() const {
  int i = 0;
  for (auto card : *this) {
    if (size() > pattern_sizes[pattern_] && i == 0) {
      printf("[ ");
    }
    card->Show();
    if (size() > pattern_sizes[pattern_] &&
        i+1 == size() - pattern_sizes[pattern_]) {
      printf("] ");
    }
    i++;
  }
  printf(", ");
}

int Pattern::Compare(const Pattern& p) const {
  if (pattern() < p.pattern())
    return -1;
  else if (pattern() > p.pattern())
    return 1;

  int s1 = size() - 1;
  int s2 = p.size() - 1;

  for (int i = s1, j = s2; i >= 0 && j >= 0; --i, --j) {
    if (at(i)->rank < p[j]->rank) {
      return -1;
    } else if (at(i)->rank > p[j]->rank) {
      return 1;
    }
  }
  return 0;
}
