#ifndef PLAYER_H_
#define PLAYER_H_

#include "hand.h"
#include "strategy.h"

class Player {
 public:
  Player(int id, bool is_computer = true);

  void NewHand(Deck* deck);
  void NewHand(const char* input);
  void Match(Player* other);

  int id() const { return id_; }
  bool is_computer() const { return is_computer_; }
  int points() const { return points_; }
  int round_points() const { return points_ - prev_points_; }
  Hand* hand() { return hand_; }

  void set_strategy(Strategy *strategy) { strategy_ = strategy; }

 private:
  void Arrange();

  int   id_;
  bool  is_computer_;
  int   points_;
  int   prev_points_;
  int   num_hands_;
  Hand* hand_;
  Strategy* strategy_;
};

#endif  // PLAYER_H_
