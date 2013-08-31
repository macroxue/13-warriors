#ifndef HAND_H_
#define HAND_H_

#include "combo.h"
#include "deck.h"
#include "pattern.h"

class Strategy;

class Hand {
 public:
  Hand();
  Hand(const char* arg);

  void DealFrom(Deck* deck);
  void Arrange(const Strategy& strategy);
  void Show();

  Combo best() { return best_; }

 private:
  void AddCard(Card* card);
  void Evaluate(const Strategy& strategy, Combo& combo);
  void FindPatterns();

  void ShowHand();
  void ShowPatterns();
  void ShowCombos(const vector<Combo>& combos, const char* type);

  bool ThreeSuits();
  bool SixPairs();
  bool ThreeStraights();

  void Search();
  void GenerateCombos(int first, int middle, int last);
  void AddCombo(Pattern first, Pattern middle, Pattern last);

  Set GetUnusedCards();

  void SortBySuit();
  void SortByRank();
  void FindFlushes();
  void FindMultiples();
  void FindTwoPairs();
  void FindFullHouses();
  void FindStraights();

  Combo TripleToPairs(Set triple);
  Combo PickFlushes(Set suit);
  Combo PickStraights(int r);

 private:
  Deck deck_;
  Set cards_;
  Set suits_[NUM_SUITS];
  Set ranks_[NUM_RANKS];
  Combo patterns_[NUM_PATTERNS];
  vector<Combo> combos_;
  vector<Combo> naturals_;
  Combo best_;
};

#endif  // HAND_H_
