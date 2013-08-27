#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

using namespace std;

enum Suit { SPADE, HEART, CLUB, DIAMOND, NUM_SUITS };
enum Rank {
  ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
  JACK, QUEEN, KING, ACE, NUM_RANKS
};

const char suit_symbol[] = "SHCD";
const char rank_symbol[] = "123456789TJQKA";

enum PatternName {
  JUNK, PAIR, TWO_PAIRS, TRIPLE, STRAIGHT, FLUSH, FULL_HOUSE,
  FOUR_OF_A_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH, NUM_PATTERNS
};

const char* pattern_names[NUM_PATTERNS] = {
  "JUNK", "PAIR", "TWO_PAIRS", "TRIPLE", "STRAIGHT", "FLUSH", "FULL_HOUSE",
  "FOUR_OF_A_KIND", "STRAIGHT_FLUSH", "ROYAL_FLUSH"
};

const int pattern_sizes[NUM_PATTERNS] = {
  0, 2, 4, 3, 5, 5, 5, 4, 5, 5
};

struct Card {
  int  suit;
  int  rank;
  bool in_use;

  void Show() {
    printf("%c%c ", suit_symbol[suit], rank_symbol[rank]);
  }
};

typedef vector<Card*> Set;
class Combo : public vector<pair<int,Set>> {
 public:
  bool operator < (const Combo& c) {
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
};

struct Deck {
  Deck() : top(0) {
    for (int i = 0; i < 52; ++i) {
      cards[i].suit = i/13;
      cards[i].rank = 13 - i%13;
      cards[i].in_use = false;
    }
  }

  void Shuffle() {
    top = 0;
    for (int i = 0; i < 52; ++i) {
      swap(cards[i], cards[rand() % 52]);
    }
  }

  Card* DealOneCard() {
    return &cards[top++];
  }

  Card cards[52];
  int  top;
};

class Hand {
 public:
  Hand() {}

  void DealFrom(Deck* deck) {
    for (int i = 0; i < 13; ++i) {
      cards.push_back(deck->DealOneCard());
    }
  }

  void ArrangeSets() {
    SortBySuit();
    SortByRank();
    ShowHand();
    FindPatterns();

    if (Natural()) {
      return;
    } else {
      ShowPatterns();
      Search();
      ShowCombos();
    }
  }

  void FindPatterns() {
    FindFlushes();

    FindMultiples();
    FindTwoPairs();
    FindFullHouses();
    FindStraights();
  }

  void ShowHand() {
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

  void ShowPatterns() {
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

  void ShowSet(Set set, int p) {
    for (int i = 0; i < set.size(); ++i) {
      if (i == pattern_sizes[p]) {
        printf("[ ");
      }
      set[i]->Show();
      if (i >= pattern_sizes[p] && i == set.size()-1) {
        printf("] ");
      }
    }
    printf(", ");
  }

  void ShowCombos() {
    for (auto combo : combos) {
      int f = combo[0].first;
      int m = combo[1].first;
      int l = combo[2].first;
      printf("\t\t");
      printf("%s %s %s , ", pattern_names[f], pattern_names[m], pattern_names[l]);
      ShowSet(combo[0].second, f);
      ShowSet(combo[1].second, m);
      ShowSet(combo[2].second, l);
      printf("\n");
    }
  }

  bool Natural() {
    if (patterns[PAIR].size() == 6) {
      printf("NATURAL:\t*** 6 PAIRS ***\n");
      return true;
    }

    bool three_suits = true;
    for (auto suit : suits) {
      if (suit.size() != 0 && suit.size() != 3 && suit.size() != 5) {
        three_suits = false;
        break;
      }
    }
    if (three_suits) {
      printf("NATURAL:\t*** 3 SUITS ***\n");
      return true;
    }

    return false;
  }

  void Search() {
    printf("SETS:");
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

  void GenerateSets(int first, int middle, int last) {
    for (int l = 0; l < patterns[last].size(); ++l) {
      SetInUse(patterns[last][l], true);

      for (int m = 0; m < patterns[middle].size(); ++m) {
        if (IsInUse(patterns[middle][m]) ||
            !InOrder(patterns[middle][m], middle, patterns[last][l], last)) {
          continue;
        }
        SetInUse(patterns[middle][m], true);

        for (int f = 0; f < patterns[first].size(); ++f) {
          if (IsInUse(patterns[first][f]) ||
              !InOrder(patterns[first][f], first, patterns[middle][m], middle)) {
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

  void AddCombo(int f, Set first, int m, Set middle, int l, Set last) {
    Set unused_cards = GetUnusedCards();
    SortFromHighToLow(unused_cards);
    int next = 0;

    // Special case when the first and the middle are junks.
    if (first.empty() && middle.empty()) {
      swap(unused_cards[0], unused_cards[3]);
    }

    for (int i = first.size(); i < 3; ++i) {
      first.push_back(unused_cards[next++]);
    }
    for (int i = middle.size(); i < 5; ++i) {
      middle.push_back(unused_cards[next++]);
    }
    for (int i = last.size(); i < 5; ++i) {
      last.push_back(unused_cards[next++]);
    }

    Combo combo;
    combo.push_back(make_pair(f, first));
    combo.push_back(make_pair(m, middle));
    combo.push_back(make_pair(l, last));

    bool worthy = true;
    for (auto prev_combo : combos) {
      if (combo < prev_combo) {
        worthy = false;
        break;
      }
    }
    if (worthy) {
      combos.push_back(combo);
    }
  }

  Set GetUnusedCards() {
    Set unused_cards;
    for (auto card : cards) {
      if (!card->in_use) {
        unused_cards.push_back(card);
      }
    }
    return unused_cards;
  }

  void SetInUse(Set set, bool in_use) {
    for (auto card : set) {
      card->in_use = in_use;
    }
  }

  bool IsInUse(Set set) {
    for (auto card : set) {
      if (card->in_use) {
        return true;
      }
    }
    return false;
  }

  bool Waste() {
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
    return false;
  }

  bool InOrder(Set first, int p1, Set second, int p2) {
    if (p1 < p2)
      return true;
    else if (p1 > p2)
      return false;

    for (int i = pattern_sizes[p1]-1; i >= 0; --i) {
      if (first[i]->rank < second[i]->rank) {
        return true;
      } else if (first[i]->rank > second[i]->rank) {
        return false;
      }
    }
    return true;
  }

  void SortBySuit() {
    for (auto card : cards) {
      suits[card->suit].push_back(card);
    }
  }

  void SortByRank() {
    for (auto card : cards) {
      ranks[card->rank].push_back(card);
    }
  }

  void FindFlushes() {
    for (auto suit : suits) {
      if (suit.size() < 5) {
        continue;
      }
      auto flushes = PickFlushes(suit);
      for (auto flush : flushes) {
        SortFromLowToHigh(flush);
        if (IsFlushRoyal(flush)) {
          patterns[ROYAL_FLUSH].push_back(flush);
        } else if (IsFlushStraight(flush)) {
          //patterns[STRAIGHT_FLUSH].push_back(flush);
        } else {
          patterns[FLUSH].push_back(flush);
        }
      }
    }
  }

  void FindMultiples() {
    for (auto rank : ranks) {
      switch (rank.size()) {
        case 2:
          patterns[PAIR].push_back(rank);
          break;
        case 3:
          patterns[TRIPLE].push_back(rank);
          break;
        case 4:
          patterns[FOUR_OF_A_KIND].push_back(rank);
          break;
      }
    }
  }

  void FindTwoPairs() {
    const auto& pairs = patterns[PAIR];
    for (int i = 0; i < pairs.size(); ++i) {
      for (int j = i+1; j < pairs.size(); ++j) {
        patterns[TWO_PAIRS].push_back(Combine(pairs[i], pairs[j]));
      }
    }
  }

  void FindFullHouses() {
    const auto& pairs = patterns[PAIR];
    const auto& triples = patterns[TRIPLE];
    for (int i = 0; i < pairs.size(); ++i) {
      for (int j = 0; j < triples.size(); ++j) {
        patterns[FULL_HOUSE].push_back(Combine(pairs[i], triples[j]));
      }
    }
  }

  void FindStraights() {
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
        if (IsStraightFlush(straight)) {
          patterns[STRAIGHT_FLUSH].push_back(straight);
        } else {
          patterns[STRAIGHT].push_back(straight);
        }
      }
    }
    ranks[ONE].clear();
  }

  vector<Set> PickFlushes(Set suit) {
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

  vector<Set> PickStraights(int r) {
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

  Set Combine(Set set1, Set set2) {
    Set set;
    set.insert(set.end(), set1.begin(), set1.end());
    set.insert(set.end(), set2.begin(), set2.end());
    return set;
  }

  bool IsFlushRoyal(Set flush) {
    return flush.front()->rank == TEN && flush.back()->rank == ACE;
  }

  bool IsFlushStraight(Set flush) {
    // The case of A2345 but sorted as 2345A.
    if (flush[0]->rank == TWO &&
        flush[3]->rank == FIVE &&
        flush[4]->rank == ACE) {
      return true;
    }
    // Other cases.
    return flush.front()->rank + 4 == flush.back()->rank;
  }

  bool IsStraightFlush(Set straight) {
    int suit = straight[0]->suit;
    for (auto card : straight) {
      if (card->suit != suit) {
        return false;
      }
    }
    return true;
  }

  void SortFromLowToHigh(Set& cards) {
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

  void SortFromHighToLow(Set& cards) {
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

 private:
  Card all_cards[52];
  Set cards;
  Set suits[NUM_SUITS];
  Set ranks[NUM_RANKS];
  vector<Set> patterns[NUM_PATTERNS];
  vector<Combo> combos;
};

int main(int argc, char* argv[])
{
  int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
  printf("SEED:\t\t%d\n", seed);
  srand(seed);
  Deck deck;
  deck.Shuffle();

  Hand hands[4];
  for (int i = 0; i < 4; i++) {
    hands[i].DealFrom(&deck);
    hands[i].ArrangeSets();
  }

  return 0;
}

