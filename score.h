#ifndef SCORE_H_
#define SCORE_H_

static const int bonus[3][NUM_PATTERNS] = {
  {1, 1, 1, 3, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 2, 8, 10, 20},
  {1, 1, 1, 1, 1, 1, 1, 4, 5, 10},
};

static const int natural_points = 6;

static const int sweep_points = 6;

#endif  // SCORE_H_
