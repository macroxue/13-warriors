#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "hand.h"
#include "strategy.h"

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
  char *input = NULL;
  int rounds = 30;
  int update_cycle = 30;
  int seed = time(NULL);
  int c;
  while ((c = getopt(argc, argv, "c:i:r:s:")) != -1) {
    switch (c) {
      case 'c': update_cycle = atoi(optarg); break;
      case 'i': input = optarg; break;
      case 'r': rounds = atoi(optarg); break;
      case 's': seed = atoi(optarg); break;
    }
  }

  Strategy strategy;
  strategy.Learn(seed, rounds, update_cycle);
  strategy.ShowWinningProbabilities();
  if (input) {
    Deck deck;
    Hand hand(input);
    hand.set_strategy(&strategy);
    hand.Arrange();
    hand.Show();
  }
  return 0;
}

