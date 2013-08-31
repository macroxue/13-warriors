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

void Combo::Show() const {
  printf("\t\t");
  for (auto pattern : *this) {
    printf("%s ", pattern_names[pattern.pattern()]);
  }
  printf(", ");
  for (auto pattern : *this) {
    pattern.Show();
  }
  printf("  %.2f\n", score());
}
