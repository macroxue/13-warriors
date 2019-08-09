#ifndef COMBO_H_
#define COMBO_H_

#include "pattern.h"
#include "set.h"

class Combo : public vector<Pattern> {
 public:
  Combo();
  Combo(std::initializer_list<Pattern> patterns);

  void DetermineType();
  bool IsNatural() const { return type_ != REGULAR; }
  const char* TypeName() const {
    static const char* type_names[NUM_TYPES] = {
      "REGULAR",
      "SIX PAIRS",
      "THREE STRAIGHTS",
      "THREE FLUSHES",
    };
    return type_names[type_];
  }

  bool MaybeBetterThan(const Combo& c) const;
  const char* CheckArrangement() const;

  void Show() const;

  Combo operator + (const Combo& c) const;

  enum Type {
    REGULAR,
    SIX_PAIRS,
    THREE_STRAIGHTS,
    THREE_FLUSHES,
    NUM_TYPES
  };

  // Getters.
  double score() const { return score_; }
  Type type() const { return type_; }

  // Setters.
  void set_score(double score) { score_ = score; }
  void set_type(Type type) { type_ = type; }

 private:
  double score_;
  Type   type_;
};

#endif  // COMBO_H_
