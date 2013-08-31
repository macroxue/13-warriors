#include <assert.h>

#include "pattern.h"

Pattern::Pattern()
  : pattern_(JUNK) {
}

Pattern::Pattern(const Set& set, int pattern)
  : Set(set), pattern_(pattern) {
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

  assert(size() == p.size());

  for (int i = size()-1; i >= 0; --i) {
    if ((*this)[i]->rank < p[i]->rank) {
      return -1;
    } else if ((*this)[i]->rank > p[i]->rank) {
      return 1;
    }
  }
  return 0;
}
