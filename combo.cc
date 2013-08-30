#include "combo.h"

Combo::Combo()
  : score_(0), is_natural_(false) {
}

Combo::Combo(std::initializer_list<Pattern> patterns)
  : vector<Pattern>(patterns), score_(0), is_natural_(false) {
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
