#ifndef COMBO_H_
#define COMBO_H_

#include "pattern.h"
#include "set.h"

class Combo : public vector<Pattern> {
 public:
  Combo();
  Combo(Pattern first, Pattern middle, Pattern last);

  void Show() const;

  double score() const { return score_; }
  bool is_natural() const { return is_natural_; }

  void set_score(double score) { score_ = score; }
  void set_is_natural(bool is_natural) { is_natural_ = is_natural; }

 private:
  double score_;
  bool is_natural_;
};

#endif  // COMBO_H_
