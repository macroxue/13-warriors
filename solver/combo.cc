#include "combo.h"

Combo::Combo()
  : score_(0), type_(REGULAR) {
}

Combo::Combo(std::initializer_list<Pattern> patterns)
  : vector<Pattern>(patterns), score_(0), type_(REGULAR) {
}

void Combo::DetermineType() {
  type_ = REGULAR;

  int num_pairs = 0;
  for (auto p : *this) {
    if (p.pattern() == PAIR) {
      num_pairs += 1;
    } else if (p.pattern() == TRIPLE) {
      num_pairs += 1;
    } else if (p.pattern() == FOUR_OF_A_KIND) {
      num_pairs += 2;
    }
  }
  if (num_pairs == 6) {
    type_ = SIX_PAIRS;
    return;
  }

  if (size() == 3) {
    if (at(0).pattern() == JUNK && at(0).IsStraight(true) &&
        (at(1).pattern() == STRAIGHT || at(1).pattern() == STRAIGHT_FLUSH ||
         at(1).pattern() == ROYAL_FLUSH) &&
        (at(2).pattern() == STRAIGHT || at(2).pattern() == STRAIGHT_FLUSH ||
         at(2).pattern() == ROYAL_FLUSH)) {
      type_ = THREE_STRAIGHTS;
      return;
    }
    if (at(0).IsFlush(true) && at(1).IsFlush() && at(2).IsFlush()) {
      type_ = THREE_FLUSHES;
      return;
    }
  }
}

Combo Combo::operator + (const Combo& c) const {
  Combo combo(*this);
  combo.insert(combo.end(), c.begin(), c.end());
  return combo;
}

bool Combo::MaybeBetterThan(const Combo& c) const {
  return at(0).Compare(c[0]) == 1 ||
         at(1).Compare(c[1]) == 1 ||
         at(2).Compare(c[2]) == 1;
}

const char* Combo::CheckArrangement() const {
  if (size() == 3) {
    if (at(0).size() != 3) {
      return "The first set must have 3 cards";
    } else if (at(1).size() != 5) {
      return "The middle set must have 5 cards";
    } else if (at(2).size() != 5) {
      return "The last set must have 5 cards";
    } else if (at(0).Compare(at(1)) == 1) {
      return "The first two sets are out of order";
    } else if (at(1).Compare(at(2)) == 1) {
      return "The last two sets are out of order";
    } else {
      return NULL;
    }
  } else if (size() == 6) {
    // Must be six pairs.
    for (auto p : *this) {
      if (p.pattern() != PAIR) {
        return "Six sets must be all pairs";
      }
    }
    return NULL;
  } else {
    return "must be either three or six sets";
  }
}

void Combo::Show() const {
  printf("\t");
  if (IsNatural()) {
    printf("%s ", TypeName());
  } else {
    for (auto pattern : *this) {
      printf("%s ", pattern_names[pattern.pattern()]);
    }
  }
  printf(", ");
  for (auto pattern : *this) {
    pattern.Show();
  }
  printf("  %.2f\n", score());
}
