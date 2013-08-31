#ifndef PLAYER_H_
#define PLAYER_H_

#include "hand.h"
#include "strategy.h"

class Player {
 public:
  Player(int number);
  void NewHand(Deck* deck);
  void NewHand(const char* input);
  void Match(Player* other);

  int points() const { return points_; }
  Hand* hand() { return hand_; }

  void set_strategy(Strategy *strategy) { strategy_ =  strategy; }

 private:
  int   id_;
  Strategy* strategy_;
  int   points_;
  int   num_hands_;
  Hand* hand_;
};

#endif  // PLAYER_H_
