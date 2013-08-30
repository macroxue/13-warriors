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
  void Arrange();
  void Show();
  void Match(Hand& hand);

  void set_strategy(Strategy *strategy) { strategy_ = strategy; }

 private:
  void AddCard(Card* card);
  void Evaluate(Combo& combo, bool is_natural);
  void AddPoints(int points);
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

  vector<Pattern> TripleToPairs(Set triple);
  vector<Pattern> PickFlushes(Set suit);
  vector<Pattern> PickStraights(int r);

 private:
  Deck deck_;
  Set cards_;
  Set suits_[NUM_SUITS];
  Set ranks_[NUM_RANKS];
  vector<Pattern> patterns_[NUM_PATTERNS];
  vector<Combo> combos_;
  vector<Combo> naturals_;
  Combo best_;
  int points_;
  Strategy* strategy_;
};

#endif  // HAND_H_
