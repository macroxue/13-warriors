#include "combo.h"

Combo::Combo()
  : score_(0), is_natural_(false) {
}

bool Combo::operator < (const Combo& c) {
  int lose_count = 0;
  for (int i = 0; i < 3; ++i) {
    lose_count += (*this)[i].first < c[i].first;
  }
  int win_count = 0;
  for (int i = 0; i < 3; ++i) {
    win_count += (*this)[i].first > c[i].first;
  }
  return lose_count > 0 && win_count == 0;
}
