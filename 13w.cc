#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "player.h"

#if 0
  JUNK       PAIR  TWO_PAIRS     TRIPLE   STRAIGHT      FLUSH FULL_HOUSE FOUR_OF_A_KIND STRAIGHT_FLUSH ROYAL_FLUSH
0~0.33  0.47~0.97          -  0.99~1.00          -          -          -          -          -          -
0~0.01  0.01~0.33  0.36~0.64  0.63~0.71  0.75~0.90  0.93~0.97  0.96~1.00          -          -          -
     -  0.00~0.02  0.02~0.13  0.13~0.13  0.16~0.36  0.37~0.60  0.65~0.94  0.94~0.97  0.98~0.99       1.00
 * Never have Ace-high two pairs!!!
 50%                66%           75%                 80%               90%
 a pair             pair of 9     pair of J           pair of Q         pair of K
 9-high two-pairs   triple        straight            7-high straight   flush
 K-high flush       full-house    7-high full-house   9-high            K-high full-house
#endif

int main(int argc, char* argv[]) {
  int output_freq = 1000;
  char *input = NULL;
  bool play = false;
  int rounds = 1000;
  int seed = time(NULL);
  int c;
  while ((c = getopt(argc, argv, "f:i:pr:s:")) != -1) {
    switch (c) {
      case 'f': output_freq = atoi(optarg); break;
      case 'i': input = optarg; break;
      case 'p': play = true; break;
      case 'r': rounds = atoi(optarg); break;
      case 's': seed = atoi(optarg); break;
    }
  }

  printf("SEED:\t\t%d\n", seed);
  srand(seed);

  Strategy strategies[4] = { Strategy(ACE), Strategy(), Strategy(), Strategy() };
  Player players[4] = { Player(0, !play), Player(1), Player(2), Player(3) };
  for (int i = 0; i < 4; ++i) {
    players[i].set_strategy(&strategies[i]);
  }

  for (int r = 0; r < rounds; ++r) {
    Deck deck;
    deck.Shuffle();
    for (auto &player : players) {
      player.NewHand(&deck);
    }

    for (int i = 0; i < 3; ++i) {
      for (int j = i+1; j < 4; ++j) {
        players[i].Match(&players[j]);
      }
    }
    if ((!input && r % output_freq == 0) || play) {
      for (int i = 0; i < 4; ++i) {
        printf("%8s %d: %+3d/%+5d",
               (players[i].is_computer() ? "Computer" : "Human"),
               i+1, players[i].round_points(), players[i].points());
        players[i].hand()->best().Show();
      }
    }
  }
  if (input) {
    Player& me = players[0];
    me.NewHand(input);
    me.hand()->Show();
  } else {
    for (int i = 0; i < 4; ++i) {
      strategies[i].Show();
    }
  }
  return 0;
}

