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
  void AddCard(Card* card);

  void ArrangeSets();
  void Evaluate(Combo& combo, bool is_natural);

  void Match(Hand& hand);
  void AddPoints(int points);

  void FindPatterns();
  void Show();

  void set_strategy(Strategy *strategy) { strategy_ = strategy; }

 private:
  void ShowHand();
  void ShowPatterns();
  void ShowSet(Set set, int p);
  void ShowCombos(const vector<Combo>& combos, const char* type);
  void ShowCombo(const Combo& combo);

  bool ThreeSuits();
  bool SixPairs();
  bool ThreeStraights();

  void Search();
  void GenerateCombos(int first, int middle, int last);
  void AddCombo(Pattern first, Pattern middle, Pattern last);

  Set GetUnusedCards();
  void SetInUse(Set set, bool in_use);
  bool IsInUse(Set set);

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

  Set Combine(Set set1, Set set2);

  bool IsRoyalFlush(Set flush);
  bool IsStraight(Set set);
  bool IsFlush(Set set);

  void SortFromLowToHigh(Set& cards);
  void SortFromHighToLow(Set& cards);

 private:
  Deck deck;
  Set cards;
  Set suits[NUM_SUITS];
  Set ranks[NUM_RANKS];
  vector<Pattern> patterns[NUM_PATTERNS];
  vector<Combo> combos;
  vector<Combo> naturals;
  Combo best;
  int points_;
  Strategy* strategy_;
};

#endif  // HAND_H_
