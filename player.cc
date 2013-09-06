#include "player.h"
#include "score.h"

Player::Player(int id, bool is_computer)
  : id_(id), is_computer_(is_computer), points_(0), prev_points_(0),
    num_hands_(0), hand_(NULL), strategy_(NULL) {
}

void Player::NewHand(Deck* deck) {
  prev_points_ = points_;
  delete hand_;
  hand_ = new Hand;
  hand_->DealFrom(deck);
  Arrange();
}

void Player::NewHand(const char* input) {
  prev_points_ = points_;
  delete hand_;
  hand_ = new Hand(input);
  Arrange();
}

void Player::Arrange() {
  hand_->Arrange(*strategy_);
  if (!is_computer()) {
    hand_->ReadArrangement();
  }
}

void Player::Match(Player* other) {
  Combo c1 = this->hand()->best();
  Combo c2 = other->hand()->best();

  int match_points = 0;
  if (c1.IsNatural()) {
    if (c2.IsNatural()) {
      // Tie.
    } else {
      match_points = sweep_points;
    }
  } else if (c2.IsNatural()) {
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
      if (this->is_computer_) {
        this->strategy_->Update(i, c1[i], result);
      }
      if (other->is_computer_) {
        other->strategy_->Update(i, c2[i], -result);
      }
    }
    if (win_count == 3 || win_count == -3) {
      match_points *= 2;
    }
  }
  this->points_ += match_points;
  other->points_ -= match_points;
}
