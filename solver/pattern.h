#ifndef PATTERN_H_
#define PATTERN_H_

#include "set.h"

enum PatternName {
  JUNK, PAIR, TWO_PAIRS, TRIPLE, STRAIGHT, FLUSH, FULL_HOUSE,
  FOUR_OF_A_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH, NUM_PATTERNS
};

static const char* pattern_names[NUM_PATTERNS] = {
  "JUNK", "PAIR", "TWO_PAIRS", "TRIPLE", "STRAIGHT", "FLUSH", "FULL_HOUSE",
  "FOUR_OF_A_KIND", "STRAIGHT_FLUSH", "ROYAL_FLUSH"
};

static const int pattern_sizes[NUM_PATTERNS] = {
  0, 2, 4, 3, 5, 5, 5, 4, 5, 5
};

class Pattern : public Set {
 public:
  Pattern();
  Pattern(const Set& set);
  Pattern(const Set& set, int pattern);
  void Show() const;

  int Compare(const Pattern& p) const;

  Set set() const { return *this; }
  int pattern() const { return pattern_; }

 private:
  int pattern_;
};

#endif  // PATTERN_H_
