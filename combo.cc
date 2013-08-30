#include "combo.h"

Combo::Combo()
  : score_(0), is_natural_(false) {
}

Combo::Combo(Pattern first, Pattern middle, Pattern last)
  : score_(0), is_natural_(false) {
  push_back(first);
  push_back(middle);
  push_back(last);
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
