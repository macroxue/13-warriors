#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hand.h"
#include "score.h"
#include "strategy.h"

Hand::Hand()
  : points_(0), strategy_(NULL) {}

Hand::Hand(const char* arg)
  : points_(0), strategy_(NULL) {
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
}

void Hand::DealFrom(Deck* deck) {
  for (int i = 0; i < 13; ++i) {
    cards_.push_back(deck->DealOneCard());
  }
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

void Hand::ArrangeSets() {
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
    Evaluate(natural, true);
    if (best_.empty() || natural.score() > best_.score()) {
      best_ = natural;
    }
  }
  for (auto& combo : combos_) {
    Evaluate(combo, false);
    if (best_.empty() || combo.score() > best_.score()) {
      best_ = combo;
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
      double win_prob = strategy_->GetWinningProbability(i, combo[i]);
      score += (2*win_prob - 1) * bonus[i][combo[i].pattern()];
    }
    combo.set_score(score);
  }
}

void Hand::Match(Hand& hand) {
  int points = 0;
  if (best_.is_natural()) {
    if (!hand.best_.is_natural()) {
      points = sweep_points;
    } else {
      // Tie.
    }
  } else {
    if (hand.best_.is_natural()) {
      points = -sweep_points;
    } else {
      // Match sets one by one.
      int win_count = 0;
      for (int i = 0; i < 3; i++) {
        auto& p1 = best_[i];
        auto& p2 = hand.best_[i];
        int result = p1.Compare(p2);
        win_count += result;
        if (result == 1) {
          points += bonus[i][p1.pattern()];
        } else if (result == -1) {
          points -= bonus[i][p2.pattern()];
        }
        strategy_->Update(i, p1, p2, result);
      }
      if (win_count == 3 || win_count == -3) {
        points *= 2;
      }
    }
  }
  AddPoints(points);
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
  best_.Show();
  ShowCombos(naturals_, "NATURAL");
  ShowCombos(combos_, "COMBO");
  // ShowPatterns();
  // printf("POINTS: %d\n", points_);
}

void Hand::ShowHand() {
  printf("FULL HAND:\t");
  // SortFromLowToHigh by suit.
  for (auto suit : suits_) {
    for (auto card : suit) {
      card->Show();
    }
    if (suit.size()) {
      printf(", ");
    }
  }
  printf("\n\t\t");
  // SortFromLowToHigh by rank.
  for (auto rank : ranks_) {
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
    if (patterns_[i].size()) {
      printf("%s:\t", pattern_names[i]);
      if (strlen(pattern_names[i]) < 8) {
        printf("\t");
      }
      for (auto pattern : patterns_[i]) {
        pattern.Show();
      }
      printf("\n");
    }
  }
}

void Hand::ShowCombos(const vector<Combo>& combos, const char* type) {
  if (!combos.empty()) {
    printf("%s:", type);
  }
  for (const auto& combo : combos) {
    combo.Show();
  }
}

bool Hand::ThreeSuits() {
  bool three_suits = true;
  for (auto suit : suits_) {
    if (suit.size() != 0 && suit.size() != 3 && suit.size() != 5) {
      three_suits = false;
      break;
    }
  }
  if (three_suits) {
    Combo natural;
    for (auto suit : suits_) {
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
    Combo natural;
    for (auto pattern : patterns_[PAIR]) {
      natural.push_back(pattern);
    }
    for (auto pattern : patterns_[TRIPLE]) {
      natural.push_back(pattern);
    }
    for (auto pattern : patterns_[FOUR_OF_A_KIND]) {
      natural.push_back(pattern);
    }
    naturals_.push_back(natural);
    return true;
  }
  return false;
}

bool Hand::ThreeStraights() {
  vector<Pattern> straights = patterns_[STRAIGHT];
  straights.insert(straights.end(), patterns_[STRAIGHT_FLUSH].begin(),
                   patterns_[STRAIGHT_FLUSH].end());
  straights.insert(straights.end(), patterns_[ROYAL_FLUSH].begin(),
                   patterns_[ROYAL_FLUSH].end());

  if (straights.size() < 2) {
    return false;
  }
  SetInUse(cards_, false);
  for (int l = 0; l < straights.size(); ++l) {
    SetInUse(straights[l], true);
    for (int m = 0; m < straights.size(); ++m) {
      if (IsInUse(straights[m]) || straights[m].Compare(straights[l]) == 1) {
        continue;
      }
      SetInUse(straights[m], true);

      Set unused_cards = GetUnusedCards();
      SortFromLowToHigh(unused_cards);
      if (IsStraight(unused_cards)) {
        Combo natural;
        natural.push_back(Pattern(unused_cards, STRAIGHT));
        natural.push_back(straights[m]);
        natural.push_back(straights[l]);
        naturals_.push_back(natural);
        return true;
      }

      SetInUse(straights[m], false);
    }
    SetInUse(straights[l], false);
  }
  return false;
}

void Hand::Search() {
  SetInUse(cards_, false);
  for (int last = NUM_PATTERNS-1; last >= 0; --last) {
    for (int middle = last; middle >= 0; --middle) {
      for (int first = middle; first >= 0; --first) {
        if (first != JUNK && first != PAIR && first != TRIPLE) {
          continue;
        }
        GenerateCombos(first, middle, last);
      }
    }
  }
}

void Hand::GenerateCombos(int first, int middle, int last) {
  for (int l = 0; l < patterns_[last].size(); ++l) {
    SetInUse(patterns_[last][l], true);

    for (int m = 0; m < patterns_[middle].size(); ++m) {
      if (IsInUse(patterns_[middle][m]) ||
          patterns_[middle][m].Compare(patterns_[last][l]) == 1) {
        continue;
      }
      SetInUse(patterns_[middle][m], true);

      for (int f = 0; f < patterns_[first].size(); ++f) {
        if (IsInUse(patterns_[first][f]) ||
            patterns_[first][f].Compare(patterns_[middle][m]) == 1) {
          continue;
        }
        SetInUse(patterns_[first][f], true);
        AddCombo(patterns_[first][f], patterns_[middle][m], patterns_[last][l]);
        SetInUse(patterns_[first][f], false);
      }

      if (first == JUNK) {
        AddCombo(Pattern(), patterns_[middle][m], patterns_[last][l]);
      }
      SetInUse(patterns_[middle][m], false);
    }
    if (middle == JUNK) {
      AddCombo(Pattern(), Pattern(), patterns_[last][l]);
    }
    SetInUse(patterns_[last][l], false);
  }
}

void Hand::AddCombo(Pattern first, Pattern middle, Pattern last) {
  Set unused_cards = GetUnusedCards();
  SortFromHighToLow(unused_cards);
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
    SortFromLowToHigh(first);
  }

  combos_.push_back(Combo(first, middle, last));
}

Set Hand::GetUnusedCards() {
  Set unused_cards;
  for (auto card : cards_) {
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

void Hand::SortBySuit() {
  for (auto card : cards_) {
    suits_[card->suit].push_back(card);
  }
  for (auto& suit : suits_) {
    SortFromLowToHigh(suit);
  }
}

void Hand::SortByRank() {
  for (auto card : cards_) {
    ranks_[card->rank].push_back(card);
  }
}

void Hand::FindFlushes() {
  for (auto suit : suits_) {
    if (suit.size() < 5) {
      continue;
    }
    auto flushes = PickFlushes(suit);
    for (auto flush : flushes) {
      SortFromLowToHigh(flush);
      if (IsRoyalFlush(flush)) {
        patterns_[ROYAL_FLUSH].push_back(flush);
      } else if (IsStraight(flush)) {
        //patterns[STRAIGHT_FLUSH].push_back(flush);
      } else {
        patterns_[FLUSH].push_back(flush);
      }
    }
  }
}

void Hand::FindMultiples() {
  for (auto rank : ranks_) {
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
  for (auto triple : patterns_[TRIPLE]) {
    for (auto pair : TripleToPairs(triple)) {
      patterns_[PAIR].push_back(pair);
    }
  }
}

void Hand::FindTwoPairs() {
  const auto& pairs = patterns_[PAIR];
  for (int i = 0; i < pairs.size(); ++i) {
    for (int j = i+1; j < pairs.size(); ++j) {
      if (pairs[i].back()->rank < pairs[j].back()->rank) {
        Pattern two_pairs(Combine(pairs[i], pairs[j]), TWO_PAIRS);
        patterns_[TWO_PAIRS].push_back(two_pairs);
      } else if (pairs[i].back()->rank > pairs[j].back()->rank) {
        Pattern two_pairs(Combine(pairs[j], pairs[i]), TWO_PAIRS);
        patterns_[TWO_PAIRS].push_back(two_pairs);
      }
    }
  }
}

void Hand::FindFullHouses() {
  const auto& pairs = patterns_[PAIR];
  const auto& triples = patterns_[TRIPLE];
  for (int i = 0; i < pairs.size(); ++i) {
    for (int j = 0; j < triples.size(); ++j) {
      if (pairs[i][0]->rank != triples[j][0]->rank) {
        Pattern full_house(Combine(pairs[i], triples[j]), FULL_HOUSE);
        patterns_[FULL_HOUSE].push_back(full_house);
      }
    }
  }
}

void Hand::FindStraights() {
  // Ace can be either the lowest or the highest.
  for (auto card : ranks_[ACE]) {
    ranks_[ONE].push_back(card);
  }
  for (int i = 0; i < NUM_RANKS-4; ++i) {
    bool is_straight = true;
    for (int j = i; j < i+5; ++j) {
      if (ranks_[j].empty()) {
        is_straight = false;
        break;
      }
    }
    auto straights = PickStraights(i);
    for (auto straight : straights) {
      if (IsFlush(straight)) {
        if (!IsRoyalFlush(straight)) {
          patterns_[STRAIGHT_FLUSH].push_back(straight);
        }
      } else {
        patterns_[STRAIGHT].push_back(straight);
      }
    }
  }
  ranks_[ONE].clear();
}

vector<Pattern> Hand::TripleToPairs(Set triple) {
  vector<Pattern> pairs;
  pairs.push_back(Pattern(Set{triple[0], triple[1]}, PAIR));
  pairs.push_back(Pattern(Set{triple[0], triple[2]}, PAIR));
  pairs.push_back(Pattern(Set{triple[1], triple[2]}, PAIR));
  return pairs;
}

vector<Pattern> Hand::PickFlushes(Set suit) {
  vector<Pattern> flushes;
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

vector<Pattern> Hand::PickStraights(int r) {
  vector<Pattern> straights;
  for (int i = 0; i < ranks_[r].size(); ++i) {
    for (int j = 0; j < ranks_[r+1].size(); ++j) {
      for (int k = 0; k < ranks_[r+2].size(); ++k) {
        for (int l = 0; l < ranks_[r+3].size(); ++l) {
          for (int m = 0; m < ranks_[r+4].size(); ++m) {
            Set straight = { ranks_[r][i], ranks_[r+1][j], ranks_[r+2][k],
              ranks_[r+3][l], ranks_[r+4][m] };
            straights.push_back(Pattern(straight, STRAIGHT));
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

