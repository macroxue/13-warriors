#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hand.h"
#include "score.h"
#include "strategy.h"

Hand::Hand() {
}

Hand::Hand(const char* arg) {
  int suit = -1;
  for (; *arg; ++arg) {
    int c = toupper(*arg);
    char* s = strchr(const_cast<char*>(suit_symbols), c);
    char* r = strchr(const_cast<char*>(rank_symbols), c);
    if (s) {
      suit = s - suit_symbols;
    } else if (r) {
      if (suit == -1) {
        fprintf(stderr, "Missing suit symbol\n");
        exit(-1);
      }
      int rank = r - rank_symbols;
      AddCard(deck_.FindCard(suit, rank));
    }
  }
  cards_.SortFromLowToHigh();
}

void Hand::DealFrom(Deck* deck) {
  for (int i = 0; i < 13; ++i) {
    cards_.push_back(deck->DealOneCard());
  }
  cards_.SortFromLowToHigh();
}

void Hand::AddCard(Card* card) {
  if (card->in_use) {
    fprintf(stderr, "Duplicate card: ");
    card->Show(stderr);
    fprintf(stderr, "\n");
    exit(-1);
  }
  cards_.push_back(card);
  card->in_use = true;
}

void Hand::Arrange(const Strategy& strategy) {
  if (cards_.size() != 13) {
    fprintf(stderr, "Wrong number of cards: %ld\n", cards_.size());
    exit(-1);
  }
  SortBySuit();
  SortByRank();
  if (ThreeSuits()) {
    // TODO: Bonus like royal flush should be kept.
    //return;
  }
  FindPatterns();
  if (ThreeStraights()) {
    // TODO: Bonus like royal flush should be kept.
    //return;
  }

  Search();

  if (naturals_.empty() && combos_.empty()) {
    ShowHand();
    ShowPatterns();
  }

  for (auto& natural : naturals_) {
    natural.set_is_natural(true);
    Evaluate(strategy, natural);
    if (best_.empty() || natural.score() > best_.score()) {
      best_ = natural;
    }
  }
  for (auto& combo : combos_) {
    combo.set_is_natural(false);
    Evaluate(strategy, combo);
    if (best_.empty() || combo.score() > best_.score()) {
      best_ = combo;
    }
  }
}

void Hand::Evaluate(const Strategy& strategy, Combo& combo) {
  if (combo.is_natural()) {
    combo.set_score(natural_points);
  } else {
    double score = 0;
    for (int i = 0; i < 3; i++) {
      double win_prob = strategy.GetWinningProbability(i, combo[i]);
      score += (2*win_prob - 1) * bonus[i][combo[i].pattern()];
    }
    combo.set_score(score);
  }
}

void Hand::FindPatterns() {
  FindFlushes();

  FindMultiples();
  FindTwoPairs();
  FindFullHouses();
  FindStraights();
}

void Hand::Show() const {
  ShowHand();
  best_.Show();
  ShowCombos(naturals_, "NATURAL");
  ShowCombos(combos_, "COMBO");
  // ShowPatterns();
  // printf("POINTS: %d\n", points_);
}

void Hand::ShowHand() const {
  printf("FULL HAND:\t");
  // Sort by suit.
  for (const auto& suit : suits_) {
    for (const auto& card : suit) {
      card->Show();
    }
    if (suit.size()) {
      printf(", ");
    }
  }
  printf("\n\t\t");
  // Sort by rank.
  for (const auto& rank : ranks_) {
    for (const auto& card : rank) {
      card->Show();
    }
    if (rank.size()) {
      printf(", ");
    }
  }
  printf("\n");
}

void Hand::ShowPatterns() const {
  for (int i = 0; i < NUM_PATTERNS; ++i) {
    if (patterns_[i].size()) {
      printf("%s:\t", pattern_names[i]);
      if (strlen(pattern_names[i]) < 8) {
        printf("\t");
      }
      for (const auto& pattern : patterns_[i]) {
        pattern.Show();
      }
      printf("\n");
    }
  }
}

void Hand::ShowCombos(const vector<Combo>& combos, const char* type) const {
  if (!combos.empty()) {
    printf("%s:", type);
  }
  for (const auto& combo : combos) {
    combo.Show();
  }
}

bool Hand::ThreeSuits() {
  bool three_suits = true;
  for (const auto& suit : suits_) {
    if (suit.size() != 0 && suit.size() != 3 && suit.size() != 5) {
      three_suits = false;
      break;
    }
  }
  if (three_suits) {
    Combo natural;
    for (const auto& suit : suits_) {
      if (!suit.empty()) {
        natural.push_back(Pattern(suit, FLUSH));
      }
    }
    naturals_.push_back(natural);
  }
  return three_suits;
}

bool Hand::SixPairs() {
  if (patterns_[PAIR].size() + patterns_[TRIPLE].size()
      + patterns_[FOUR_OF_A_KIND].size()*2 == 6) {
    naturals_.push_back(patterns_[PAIR] + patterns_[TRIPLE]
                        + patterns_[FOUR_OF_A_KIND]);
    return true;
  }
  return false;
}

bool Hand::ThreeStraights() {
  Combo straights = patterns_[STRAIGHT] + patterns_[STRAIGHT_FLUSH]
    + patterns_[ROYAL_FLUSH];

  if (straights.size() < 2) {
    return false;
  }
  cards_.SetInUse(false);
  for (int m = 0; m < straights.size()-1; ++m) {
    straights[m].SetInUse(true);
    for (int l = m+1; l < straights.size(); ++l) {
      if (straights[l].IsInUse()) {
        continue;
      }
      straights[l].SetInUse(true);

      Set unused_cards = GetUnusedCards();
      unused_cards.SortFromLowToHigh();
      if (unused_cards.IsStraight()) {
        naturals_.push_back(Combo{Pattern(unused_cards, STRAIGHT),
                            straights[m], straights[l]});
        return true;
      }

      straights[l].SetInUse(false);
    }
    straights[m].SetInUse(false);
  }
  return false;
}

void Hand::Search() {
  cards_.SetInUse(false);
  for (int last = NUM_PATTERNS-1; last >= 0; --last) {
    for (int middle = last; middle >= 0; --middle) {
      for (int first = middle; first >= 0; --first) {
        if (first == JUNK || first == PAIR || first == TRIPLE) {
          GenerateCombos(first, middle, last);
        }
      }
    }
  }
}

void Hand::GenerateCombos(int first, int middle, int last) {
  for (auto& p3 : patterns_[last]) {
    p3.SetInUse(true);

    for (auto& p2 : patterns_[middle]) {
      if (p2.IsInUse() || p2.Compare(p3) == 1) {
        continue;
      }
      p2.SetInUse(true);

      for (auto& p1 : patterns_[first]) {
        if (p1.IsInUse() || p1.Compare(p2) == 1) {
          continue;
        }
        p1.SetInUse(true);
        AddCombo(p1, p2, p3);
        p1.SetInUse(false);
      }

      if (first == JUNK) {
        AddCombo(Pattern(), p2, p3);
      }
      p2.SetInUse(false);
    }
    if (middle == JUNK) {
      AddCombo(Pattern(), Pattern(), p3);
    }
    p3.SetInUse(false);
  }
}

void Hand::AddCombo(Pattern first, Pattern middle, Pattern last) {
  Set unused_cards = GetUnusedCards();
  int next = 0;

  // Special case when the first and the middle are junks.
  if (first.pattern() == JUNK && middle.pattern() == JUNK) {
    swap(unused_cards[0], unused_cards[3]);
  }

  for (int i = first.size(); i < 3; ++i) {
    first.insert(first.begin(), unused_cards[next++]);
  }
  for (int i = middle.size(); i < 5; ++i) {
    middle.insert(middle.begin(), unused_cards[next++]);
  }
  for (int i = last.size(); i < 5; ++i) {
    last.insert(last.begin(), unused_cards[next++]);
  }

  // Special case when the first and the middle are junks.
  if (first.pattern() == JUNK && middle.pattern() == JUNK) {
    first.SortFromLowToHigh();
  }

  combos_.push_back(Combo({first, middle, last}));
}

Set Hand::GetUnusedCards() const {
  Set unused_cards;
  for (int i = cards_.size()-1; i >= 0; --i) {
    if (!cards_[i]->in_use) {
      unused_cards.push_back(cards_[i]);
    }
  }
  assert(unused_cards.SortedFromHighToLow());
  return unused_cards;
}

void Hand::SortBySuit() {
  for (const auto& card : cards_) {
    suits_[card->suit].push_back(card);
  }
  for (const auto& suit : suits_) {
    assert(suit.SortedFromLowToHigh());
  }
}

void Hand::SortByRank() {
  for (const auto& card : cards_) {
    ranks_[card->rank].push_back(card);
  }
}

void Hand::FindFlushes() {
  for (const auto& suit : suits_) {
    if (suit.size() < 5) {
      continue;
    }
    auto flushes = PickFlushes(suit);
    for (const auto& flush : flushes) {
      assert(flush.SortedFromLowToHigh());
      if (flush.IsStraight()) {
        if (flush.IsRoyalFlush()) {
          patterns_[ROYAL_FLUSH].push_back(Pattern(flush, ROYAL_FLUSH));
        } else {
          patterns_[STRAIGHT_FLUSH].push_back(Pattern(flush, STRAIGHT_FLUSH));
        }
      } else {
        patterns_[FLUSH].push_back(flush);
      }
    }
  }
}

void Hand::FindMultiples() {
  for (const auto& rank : ranks_) {
    if (rank.size() == 2) {
      patterns_[PAIR].push_back(Pattern(rank, PAIR));
    } else if (rank.size() == 3) {
      patterns_[TRIPLE].push_back(Pattern(rank, TRIPLE));
    } else if (rank.size() == 4) {
      patterns_[FOUR_OF_A_KIND].push_back(Pattern(rank, FOUR_OF_A_KIND));
    }
  }
  if (SixPairs()) {
    return;
  }
  // Break triples into pairs.
  for (const auto& triple : patterns_[TRIPLE]) {
    for (const auto& pair : TripleToPairs(triple)) {
      patterns_[PAIR].push_back(pair);
    }
  }
}

void Hand::FindTwoPairs() {
  for (const auto& pair1 : patterns_[PAIR]) {
    for (const auto& pair2 : patterns_[PAIR]) {
      if (pair1[0]->rank < pair2[0]->rank) {
        patterns_[TWO_PAIRS].push_back(Pattern(pair1 + pair2, TWO_PAIRS));
      }
    }
  }
}

void Hand::FindFullHouses() {
  for (const auto& pair : patterns_[PAIR]) {
    for (const auto& triple : patterns_[TRIPLE]) {
      if (pair[0]->rank != triple[0]->rank) {
        patterns_[FULL_HOUSE].push_back(Pattern(pair + triple, FULL_HOUSE));
      }
    }
  }
}

void Hand::FindStraights() {
  // Ace can be either the lowest or the highest.
  for (const auto& card : ranks_[ACE]) {
    ranks_[ONE].push_back(card);
  }
  for (int i = 0; i < NUM_RANKS-4; ++i) {
    auto straights = PickStraights(i);
    for (const auto straight : straights) {
      if (!straight.IsFlush()) {
        patterns_[STRAIGHT].push_back(straight);
      }
    }
  }
  ranks_[ONE].clear();
}

Combo Hand::TripleToPairs(const Set& triple) const {
  return { Pattern({triple[0], triple[1]}, PAIR),
           Pattern({triple[0], triple[2]}, PAIR),
           Pattern({triple[1], triple[2]}, PAIR) };
}

Combo Hand::PickFlushes(const Set& suit) const {
  Combo flushes;
  for (int i = 0; i < suit.size()-4; ++i) {
    for (int j = i+1; j < suit.size()-3; ++j) {
      for (int k = j+1; k < suit.size()-2; ++k) {
        for (int l = k+1; l < suit.size()-1; ++l) {
          for (int m = l+1; m < suit.size(); ++m) {
            Set flush = { suit[i], suit[j], suit[k], suit[l], suit[m] };
            flushes.push_back(Pattern(flush, FLUSH));
          }
        }
      }
    }
  }
  return flushes;
}

Combo Hand::PickStraights(int r) const {
  Combo straights;
  for (auto c : ranks_[r]) {
    for (auto c1 : ranks_[r+1]) {
      for (auto c2 : ranks_[r+2]) {
        for (auto c3 : ranks_[r+3]) {
          for (auto c4 : ranks_[r+4]) {
            Set straight = { c, c1, c2, c3, c4 };
            straights.push_back(Pattern(straight, STRAIGHT));
          }
        }
      }
    }
  }
  return straights;
}

