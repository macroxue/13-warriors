#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hand.h"
#include "score.h"
#include "strategy.h"

Hand::Hand()
  : points_(0) {}

Hand::Hand(const char* arg)
  : points_(0) {
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
      AddCard(deck.FindCard(suit, rank));
    }
  }
}

void Hand::DealFrom(Deck* deck) {
  for (int i = 0; i < 13; ++i) {
    cards.push_back(deck->DealOneCard());
  }
}

void Hand::AddCard(Card* card) {
  if (card->in_use) {
    fprintf(stderr, "Duplicate card: ");
    card->Show(stderr);
    fprintf(stderr, "\n");
    exit(-1);
  }
  cards.push_back(card);
  card->in_use = true;
}

void Hand::ArrangeSets() {
  if (cards.size() != 13) {
    fprintf(stderr, "Wrong number of cards: %ld\n", cards.size());
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

  if (naturals.empty() && combos.empty()) {
    ShowHand();
    ShowPatterns();
  }

  for (auto& natural : naturals) {
    Evaluate(natural, true);
    if (best.empty() || natural.score() > best.score()) {
      best = natural;
    }
  }
  for (auto& combo : combos) {
    Evaluate(combo, false);
    if (best.empty() || combo.score() > best.score()) {
      best = combo;
    }
  }
}

void Hand::Evaluate(Combo& combo, bool is_natural) {
  combo.set_is_natural(is_natural);
  if (is_natural) {
    combo.set_score(natural_points);
  } else {
    double score = 0;
    for (int i = 0; i < 3; i++) {
      int p = combo[i].first;
      Set set = combo[i].second;
      double win_prob = strategy_->GetWinningProbability(i, p, set);
      score += (2*win_prob - 1) * bonus[i][p];
    }
    combo.set_score(score);
  }
}

void Hand::Match(Hand& hand) {
  int points = 0;
  if (this->best.is_natural()) {
    if (!hand.best.is_natural()) {
      points = sweep_points;
    } else {
      // Tie.
    }
  } else {
    if (hand.best.is_natural()) {
      points = -sweep_points;
    } else {
      // Match sets one by one.
      int win_count = 0;
      for (int i = 0; i < 3; i++) {
        int p1 = this->best[i].first;
        int p2 = hand.best[i].first;
        const Set& set1 = this->best[i].second;
        const Set& set2 = hand.best[i].second;
        int result = Compare(set1, p1, set2, p2);
        win_count += result;
        if (result == 1) {
          points += bonus[i][p1];
        } else if (result == -1) {
          points -= bonus[i][p2];
        }
        strategy_->Update(i, set1, p1, set2, p2, result);
      }
      if (win_count == 3 || win_count == -3) {
        points *= 2;
      }
    }
  }
  this->AddPoints(points);
  hand.AddPoints(-points);
}

void Hand::AddPoints(int points) {
  points_ += points;
}

void Hand::FindPatterns() {
  FindFlushes();

  FindMultiples();
  FindTwoPairs();
  FindFullHouses();
  FindStraights();
}

void Hand::Show() {
  ShowHand();
  ShowCombo(best);
  ShowCombos(naturals, "NATURAL");
  ShowCombos(combos, "COMBO");
  // ShowPatterns();
  // printf("POINTS: %d\n", points_);
}

void Hand::ShowHand() {
  printf("FULL HAND:\t");
  // SortFromLowToHigh by suit.
  for (auto suit : suits) {
    for (auto card : suit) {
      card->Show();
    }
    if (suit.size()) {
      printf(", ");
    }
  }
  printf("\n\t\t");
  // SortFromLowToHigh by rank.
  for (auto rank : ranks) {
    for (auto card : rank) {
      card->Show();
    }
    if (rank.size()) {
      printf(", ");
    }
  }
  printf("\n");
}

void Hand::ShowPatterns() {
  for (int i = 0; i < NUM_PATTERNS; ++i) {
    if (patterns[i].size()) {
      printf("%s:\t", pattern_names[i]);
      if (strlen(pattern_names[i]) < 8) {
        printf("\t");
      }
      for (auto set : patterns[i]) {
        ShowSet(set, i);
      }
      printf("\n");
    }
  }
}

void Hand::ShowSet(Set set, int p) {
  for (int i = 0; i < set.size(); ++i) {
    if (set.size() > pattern_sizes[p] && i == 0) {
      printf("[ ");
    }
    set[i]->Show();
    if (set.size() > pattern_sizes[p] && i+1 == set.size() - pattern_sizes[p]) {
      printf("] ");
    }
  }
  printf(", ");
}

void Hand::ShowCombos(const vector<Combo>& combos, const char* type) {
  if (!combos.empty()) {
    printf("%s:", type);
  }
  for (const auto& combo : combos) {
    ShowCombo(combo);
  }
}

void Hand::ShowCombo(const Combo& combo) {
  printf("\t\t");
  for (auto set : combo) {
    printf("%s ", pattern_names[set.first]);
  }
  printf(", ");
  for (auto set : combo) {
    ShowSet(set.second, set.first);
  }
  printf("  %.2f\n", combo.score());
}

bool Hand::ThreeSuits() {
  bool three_suits = true;
  for (auto suit : suits) {
    if (suit.size() != 0 && suit.size() != 3 && suit.size() != 5) {
      three_suits = false;
      break;
    }
  }
  if (three_suits) {
    Combo natural;
    for (auto suit : suits) {
      if (!suit.empty()) {
        natural.push_back(make_pair(FLUSH, suit));
      }
    }
    naturals.push_back(natural);
  }
  return three_suits;
}

bool Hand::SixPairs() {
  if (patterns[PAIR].size() + patterns[TRIPLE].size()
      + patterns[FOUR_OF_A_KIND].size()*2 == 6) {
    Combo natural;
    for (auto set : patterns[PAIR]) {
      natural.push_back(make_pair(PAIR, set));
    }
    for (auto set : patterns[TRIPLE]) {
      natural.push_back(make_pair(TRIPLE, set));
    }
    for (auto set : patterns[FOUR_OF_A_KIND]) {
      natural.push_back(make_pair(FOUR_OF_A_KIND, set));
    }
    naturals.push_back(natural);
    return true;
  }
  return false;
}

bool Hand::ThreeStraights() {
  vector<Set> straights = patterns[STRAIGHT];
  straights.insert(straights.end(), patterns[STRAIGHT_FLUSH].begin(),
                   patterns[STRAIGHT_FLUSH].end());
  straights.insert(straights.end(), patterns[ROYAL_FLUSH].begin(),
                   patterns[ROYAL_FLUSH].end());

  if (straights.size() < 2) {
    return false;
  }
  SetInUse(cards, false);
  for (int l = 0; l < straights.size(); ++l) {
    SetInUse(straights[l], true);
    for (int m = 0; m < straights.size(); ++m) {
      if (IsInUse(straights[m]) ||
          Compare(straights[m], STRAIGHT, straights[l], STRAIGHT) == 1) {
        continue;
      }
      SetInUse(straights[m], true);

      Set unused_cards = GetUnusedCards();
      SortFromLowToHigh(unused_cards);
      if (IsStraight(unused_cards)) {
        Combo natural;
        natural.push_back(make_pair(STRAIGHT, unused_cards));
        natural.push_back(make_pair(STRAIGHT, straights[m]));
        natural.push_back(make_pair(STRAIGHT, straights[l]));
        naturals.push_back(natural);
        return true;
      }

      SetInUse(straights[m], false);
    }
    SetInUse(straights[l], false);
  }
  return false;
}

void Hand::Search() {
  SetInUse(cards, false);
  for (int last = NUM_PATTERNS-1; last >= 0; --last) {
    for (int middle = last; middle >= 0; --middle) {
      for (int first = middle; first >= 0; --first) {
        if (first != JUNK && first != PAIR && first != TRIPLE) {
          continue;
        }
        GenerateSets(first, middle, last);
      }
    }
  }
}

void Hand::GenerateSets(int first, int middle, int last) {
  for (int l = 0; l < patterns[last].size(); ++l) {
    SetInUse(patterns[last][l], true);

    for (int m = 0; m < patterns[middle].size(); ++m) {
      if (IsInUse(patterns[middle][m]) ||
          Compare(patterns[middle][m], middle, patterns[last][l], last) == 1) {
        continue;
      }
      SetInUse(patterns[middle][m], true);

      for (int f = 0; f < patterns[first].size(); ++f) {
        if (IsInUse(patterns[first][f]) ||
            Compare(patterns[first][f], first, patterns[middle][m], middle) == 1) {
          continue;
        }
        SetInUse(patterns[first][f], true);
        if (!Waste()) {
          AddCombo(first, patterns[first][f], middle, patterns[middle][m],
                   last, patterns[last][l]);
        }
        SetInUse(patterns[first][f], false);
      }

      if (first == JUNK && !Waste()) {
        AddCombo(first, Set(), middle, patterns[middle][m],
                 last, patterns[last][l]);
      }
      SetInUse(patterns[middle][m], false);
    }
    if (middle == JUNK && !Waste()) {
      AddCombo(first, Set(), middle, Set(), last, patterns[last][l]);
    }
    SetInUse(patterns[last][l], false);
  }
}

void Hand::AddCombo(int f, Set first, int m, Set middle, int l, Set last) {
  Set unused_cards = GetUnusedCards();
  SortFromHighToLow(unused_cards);
  int next = 0;

  // Special case when the first and the middle are junks.
  if (f == JUNK && m == JUNK) {
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
  if (f == JUNK && m == JUNK) {
    SortFromLowToHigh(first);
  }

  Combo combo;
  combo.push_back(make_pair(f, first));
  combo.push_back(make_pair(m, middle));
  combo.push_back(make_pair(l, last));
  bool worthy = true;
#if 0
  for (auto prev_combo : combos) {
    if (combo < prev_combo) {
      worthy = false;
      break;
    }
  }
#endif
  if (worthy) {
    combos.push_back(combo);
  }
}

Set Hand::GetUnusedCards() {
  Set unused_cards;
  for (auto card : cards) {
    if (!card->in_use) {
      unused_cards.push_back(card);
    }
  }
  return unused_cards;
}

void Hand::SetInUse(Set set, bool in_use) {
  for (auto card : set) {
    card->in_use = in_use;
  }
}

bool Hand::IsInUse(Set set) {
  for (auto card : set) {
    if (card->in_use) {
      return true;
    }
  }
  return false;
}

bool Hand::Waste() {
#if 0
  // TODO: Problem breaking aa bb cc dd 23456.
  for (auto pattern : patterns) {
    for (auto set : pattern) {
      bool in_use = false;
      for (auto card : set) {
        if (card->in_use) {
          in_use = true;
          break;
        }
      }
      if (!in_use) {
        return true;
      }
    }
  }
#endif
  return false;
}

int Hand::Compare(Set first, int p1, Set second, int p2) {
  if (p1 < p2)
    return -1;
  else if (p1 > p2)
    return 1;

  assert(first.size() == second.size());

  for (int i = first.size()-1; i >= 0; --i) {
    if (first[i]->rank < second[i]->rank) {
      return -1;
    } else if (first[i]->rank > second[i]->rank) {
      return 1;
    }
  }
  return 0;
}

void Hand::SortBySuit() {
  for (auto card : cards) {
    suits[card->suit].push_back(card);
  }
  for (auto& suit : suits) {
    SortFromLowToHigh(suit);
  }
}

void Hand::SortByRank() {
  for (auto card : cards) {
    ranks[card->rank].push_back(card);
  }
}

void Hand::FindFlushes() {
  for (auto suit : suits) {
    if (suit.size() < 5) {
      continue;
    }
    auto flushes = PickFlushes(suit);
    for (auto flush : flushes) {
      SortFromLowToHigh(flush);
      if (IsRoyalFlush(flush)) {
        patterns[ROYAL_FLUSH].push_back(flush);
      } else if (IsStraight(flush)) {
        //patterns[STRAIGHT_FLUSH].push_back(flush);
      } else {
        patterns[FLUSH].push_back(flush);
      }
    }
  }
}

void Hand::FindMultiples() {
  for (auto rank : ranks) {
    if (rank.size() == 2) {
      patterns[PAIR].push_back(rank);
    } else if (rank.size() == 3) {
      patterns[TRIPLE].push_back(rank);
    } else if (rank.size() == 4) {
      patterns[FOUR_OF_A_KIND].push_back(rank);
    }
  }
  if (SixPairs()) {
    return;
  }
  // Break triples into pairs.
  for (auto triple : patterns[TRIPLE]) {
    for (auto pair : TripleToPairs(triple)) {
      patterns[PAIR].push_back(pair);
    }
  }
}

vector<Set> Hand::TripleToPairs(Set triple) {
  vector<Set> pairs;
  pairs.push_back(Set{triple[0], triple[1]});
  pairs.push_back(Set{triple[0], triple[2]});
  pairs.push_back(Set{triple[1], triple[2]});
  return pairs;
}

void Hand::FindTwoPairs() {
  const auto& pairs = patterns[PAIR];
  for (int i = 0; i < pairs.size(); ++i) {
    for (int j = i+1; j < pairs.size(); ++j) {
      if (pairs[i].back()->rank < pairs[j].back()->rank) {
        patterns[TWO_PAIRS].push_back(Combine(pairs[i], pairs[j]));
      } else if (pairs[i].back()->rank > pairs[j].back()->rank) {
        patterns[TWO_PAIRS].push_back(Combine(pairs[j], pairs[i]));
      }
    }
  }
}

void Hand::FindFullHouses() {
  const auto& pairs = patterns[PAIR];
  const auto& triples = patterns[TRIPLE];
  for (int i = 0; i < pairs.size(); ++i) {
    for (int j = 0; j < triples.size(); ++j) {
      if (pairs[i][0]->rank != triples[j][0]->rank) {
        patterns[FULL_HOUSE].push_back(Combine(pairs[i], triples[j]));
      }
    }
  }
}

void Hand::FindStraights() {
  // Ace can be either the lowest or the highest.
  for (auto card : ranks[ACE]) {
    ranks[ONE].push_back(card);
  }
  for (int i = 0; i < NUM_RANKS-4; ++i) {
    bool is_straight = true;
    for (int j = i; j < i+5; ++j) {
      if (ranks[j].empty()) {
        is_straight = false;
        break;
      }
    }
    auto straights = PickStraights(i);
    for (auto straight : straights) {
      if (IsFlush(straight)) {
        if (!IsRoyalFlush(straight)) {
          patterns[STRAIGHT_FLUSH].push_back(straight);
        }
      } else {
        patterns[STRAIGHT].push_back(straight);
      }
    }
  }
  ranks[ONE].clear();
}

vector<Set> Hand::PickFlushes(Set suit) {
  vector<Set> flushes;
  for (int i = 0; i < suit.size()-4; ++i) {
    for (int j = i+1; j < suit.size()-3; ++j) {
      for (int k = j+1; k < suit.size()-2; ++k) {
        for (int l = k+1; l < suit.size()-1; ++l) {
          for (int m = l+1; m < suit.size(); ++m) {
            Set flush = { suit[i], suit[j], suit[k], suit[l], suit[m] };
            flushes.push_back(flush);
          }
        }
      }
    }
  }
  return flushes;
}

vector<Set> Hand::PickStraights(int r) {
  vector<Set> straights;
  for (int i = 0; i < ranks[r].size(); ++i) {
    for (int j = 0; j < ranks[r+1].size(); ++j) {
      for (int k = 0; k < ranks[r+2].size(); ++k) {
        for (int l = 0; l < ranks[r+3].size(); ++l) {
          for (int m = 0; m < ranks[r+4].size(); ++m) {
            Set straight = { ranks[r][i], ranks[r+1][j], ranks[r+2][k],
              ranks[r+3][l], ranks[r+4][m] };
            straights.push_back(straight);
          }
        }
      }
    }
  }
  return straights;
}

Set Hand::Combine(Set set1, Set set2) {
  Set set;
  set.insert(set.end(), set1.begin(), set1.end());
  set.insert(set.end(), set2.begin(), set2.end());
  return set;
}

bool Hand::IsRoyalFlush(Set flush) {
  return flush.front()->rank == TEN && flush.back()->rank == ACE;
}

bool Hand::IsStraight(Set set) {
  int num_cards = set.size();
  for (int i = 0; i < num_cards-1; ++i) {
    if (set[i]->rank + 1 != set[i+1]->rank) {
      return false;
    }
  }
  // The case of A2345 but sorted as 2345A.
  return set[num_cards-2]->rank + 1 == set[num_cards-1]->rank ||
    (set[0]->rank == TWO && set[num_cards-1]->rank == ACE);
}

bool Hand::IsFlush(Set set) {
  int suit = set[0]->suit;
  for (auto card : set) {
    if (card->suit != suit) {
      return false;
    }
  }
  return true;
}

void Hand::SortFromLowToHigh(Set& cards) {
  if (cards.empty()) {
    return;
  }
  for (int i = 0; i < cards.size()-1; ++i) {
    for (int j = i+1; j < cards.size(); ++j) {
      if (cards[i]->rank > cards[j]->rank) {
        swap(cards[i], cards[j]);
      }
    }
  }
}

void Hand::SortFromHighToLow(Set& cards) {
  if (cards.empty()) {
    return;
  }
  for (int i = 0; i < cards.size()-1; ++i) {
    for (int j = i+1; j < cards.size(); ++j) {
      if (cards[i]->rank < cards[j]->rank) {
        swap(cards[i], cards[j]);
      }
    }
  }
}

