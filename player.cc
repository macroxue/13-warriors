#include "player.h"
#include "score.h"

Player::Player(int id)
  : id_(id), strategy_(NULL), points_(0), num_hands_(0), hand_(NULL) {
}

void Player::NewHand(Deck* deck) {
  delete hand_;
  hand_ = new Hand;
  hand_->DealFrom(deck);
  hand_->Arrange(*strategy_);

  ++num_hands_;
  if (num_hands_ % (100 * (id_+1)) == 0) {
    strategy_->UpdateWinningProbabilities();
  }
}

void Player::NewHand(const char* input) {
  delete hand_;
  hand_ = new Hand(input);
  hand_->Arrange(*strategy_);
}

void Player::Match(Player* other) {
  Combo c1 = this->hand()->best();
  Combo c2 = other->hand()->best();

  int match_points = 0;
  if (c1.is_natural()) {
    if (c2.is_natural()) {
      // Tie.
    } else {
      match_points = sweep_points;
    }
  } else if (c2.is_natural()) {
    match_points = -sweep_points;
  } else {
    // Match sets one by one.
    int win_count = 0;
    for (int i = 0; i < 3; i++) {
      int result = c1[i].Compare(c2[i]);
      win_count += result;
      if (result == 1) {
        match_points += bonus[i][c1[i].pattern()];
      } else if (result == -1) {
        match_points -= bonus[i][c2[i].pattern()];
      }
      this->strategy_->Update(i, c1[i], result);
      other->strategy_->Update(i, c2[i], -result);
    }
    if (win_count == 3 || win_count == -3) {
      match_points *= 2;
    }
  }
  this->points_ += match_points;
  other->points_ -= match_points;
}