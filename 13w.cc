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
  1, 2, 4, 3, 5, 5, 5, 4, 5, 5
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

class Hand {
 public:
  Hand() {
    for (int c = 0; c < 52; c++) {
      all_cards[c].suit = c/13;
      all_cards[c].rank = 13 - c%13;
      all_cards[c].in_use = false;
    }
    GenerateRandomHand();
    //DragonHand();
    FindPatterns();
    ShowHand();
    ShowPatterns();

    Natural();
    Search();
  }

  void GenerateRandomHand() {
    bool selected[52];
    for (int c = 0; c < 52; c++) {
      selected[c] = false;
    }
    for (int h = 0; h < 13; h++) {
      int c;
      do c = rand() % 52;
      while (selected[c]);
      selected[c] = true;
      cards.push_back(&all_cards[c]);
    }
  }

  void DragonHand() {
    for (int h = 0; h < 13; h++) {
      cards.push_back(&all_cards[h]);
    }
  }

  void FindPatterns() {
    SortBySuit();
    SortByRank();

    FindFlushes();

    FindMultiples();
    FindTwoPairs();
    FindFullHouses();
    FindStraights();
  }

  void ShowHand() {
    printf("FULL HAND:\t");
    // Sort by suit.
    for (auto suit : suits) {
      for (auto card : suit) {
        card->Show();
      }
      if (suit.size()) {
        printf(", ");
      }
    }
    printf("\n\t\t");
    // Sort by rank.
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
          ShowSet(set);
        }
        printf("\n");
      }
    }
  }

  void ShowSet(Set set) {
    for (auto card : set) {
      card->Show();
    }
    printf(", ");
  }

  void Natural() {
    if (patterns[PAIR].size() == 6) {
      printf("NATURAL:\t*** 6 PAIRS ***\n");
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
    }
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
            printf("\t\t");
            ShowSet(patterns[first][f]);
            ShowSet(patterns[middle][m]);
            ShowSet(patterns[last][l]);
            printf("\n");
          }
          SetInUse(patterns[first][f], false);
        }

        if (first == JUNK && !Waste()) {
          printf("\t\tJUNK , ");
          ShowSet(patterns[middle][m]);
          ShowSet(patterns[last][l]);
          printf("\n");
        }
        SetInUse(patterns[middle][m], false);
      }
      if (middle == JUNK && !Waste()) {
        printf("\t\tJUNK , JUNK , ");
        ShowSet(patterns[last][l]);
        printf("\n");
      }
      SetInUse(patterns[last][l], false);
    }
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
        SortFlush(flush);
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

  void SortFlush(Set& flush) {
    for (int i = 0; i < flush.size()-1; ++i) {
      for (int j = i+1; j < flush.size(); ++j) {
        if (flush[i]->rank > flush[j]->rank) {
          swap(flush[i], flush[j]);
        }
      }
    }
    for (int i = 0; i < flush.size()-1; ++i) {
      assert(flush[i]->rank < flush[i+1]->rank);
    }
  }

 private:
  Card all_cards[52];
  Set cards;
  Set suits[NUM_SUITS];
  Set ranks[NUM_RANKS];
  vector<Set> patterns[NUM_PATTERNS];
};

int main(int argc, char* argv[])
{
  int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
  printf("SEED:\t\t%d\n", seed);
  srand(seed);
  Hand hand;
  return 0;
}

