#include "combo.h"

Combo::Combo()
  : score_(0), is_natural_(false) {
}

Combo::Combo(std::initializer_list<Pattern> patterns)
  : vector<Pattern>(patterns), score_(0), is_natural_(false) {
}

Combo Combo::operator + (const Combo& c) const {
  Combo combo(*this);
  combo.insert(combo.end(), c.begin(), c.end());
  return combo;
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
  for (auto pattern : *this) {
    printf("%s ", pattern_names[pattern.pattern()]);
  }
  printf(", ");
  for (auto pattern : *this) {
    pattern.Show();
  }
  printf("  %.2f\n", score());
}
