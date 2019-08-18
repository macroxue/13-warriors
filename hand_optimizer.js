class HandOptimizer {
  constructor(hand, handicap = 0) {
    this.hand = hand.slice(0);
    this.handicap = handicap;

    this.top_waves = [];
    this.optimize_waves();
  }

  optimize_waves() {
    this.hand.sort((a, b) => rank(a) - rank(b));
    this.threshold = 0;
    this.optimize_back();
  }

  optimize_back() {
    for (var b1 = 0; b1 < 9; ++b1) {
      for (var b2 = b1 + 1; b2 < 10; ++b2) {
        for (var b3 = b2 + 1; b3 < 11; ++b3) {
          for (var b4 = b3 + 1; b4 < 12; ++b4) {
            for (var b5 = b4 + 1; b5 < 13; ++b5) {
              this.back = new WaveEvaluator(
                [this.hand[b1], this.hand[b2], this.hand[b3],
                 this.hand[b4], this.hand[b5]], Back);
              if (this.back.pattern == Junk) continue;
              if (200 + this.back.value < this.threshold) continue;

              var bits = (1<<b1) + (1<<b2) + (1<<b3) + (1<<b4) + (1<<b5);
              this.optimize_center(bits);
            }
          }
        }
      }
    }
  }

  optimize_center(bits) {
    out:
    for (var c1 = 0; c1 < 9; ++c1) {
      if ((1 << c1) & bits) continue;
      for (var c2 = c1 + 1; c2 < 10; ++c2) {
        if ((1 << c2) & bits) continue;
        for (var c3 = c2 + 1; c3 < 11; ++c3) {
          if ((1 << c3) & bits) continue;
          for (var c4 = c3 + 1; c4 < 12; ++c4) {
            if ((1 << c4) & bits) continue;
            for (var c5 = c4 + 1; c5 < 13; ++c5) {
              if ((1 << c5) & bits) continue;

              this.center = new WaveEvaluator(
                [this.hand[c1], this.hand[c2], this.hand[c3],
                 this.hand[c4], this.hand[c5]], Center);
              if (this.back.is_smaller_than(this.center)) break out;
              if (100 + this.center.value + this.back.value < this.threshold)
                continue;

              var center_bits = (1<<c1) + (1<<c2) + (1<<c3) + (1<<c4) + (1<<c5);
              this.optimize_front(bits + center_bits);
            }
          }
        }
      }
    }
  }

  optimize_front(bits) {
    out:
    for (var f1 = 0; f1 < 11; ++f1) {
      if ((1 << f1) & bits) continue;
      for (var f2 = f1 + 1; f2 < 12; ++f2) {
        if ((1 << f2) & bits) continue;
        for (var f3 = f2 + 1; f3 < 13; ++f3) {
          if ((1 << f3) & bits) continue;

          this.front = new WaveEvaluator(
            [this.hand[f1], this.hand[f2], this.hand[f3]], Front);
          if (this.center.is_smaller_than(this.front)) break out;

          var sum_value = expected_points(this.front, this.center, this.back);
          var duplicate = false;
          for (var pos = 0; pos < this.top_waves.length; ++pos) {
            if (sum_value < this.top_waves[pos][0]) break;
            if (sum_value == this.top_waves[pos][0]) {
              duplicate = true;
              break;
            }
          }
          if (duplicate) continue;
          if (pos == 0 && this.top_waves.length == this.handicap + 1) continue;

          this.top_waves.splice(pos, 0, [sum_value].concat(
                this.front.wave, this.center.wave, this.back.wave));
          if (this.top_waves.length > this.handicap + 1)
            this.top_waves.shift();
          this.threshold = this.top_waves[0][0];
        }
      }
    }
  }

  get waves() {
    var waves = this.top_waves[0].slice(1);
    return [waves.slice(0, 3), waves.slice(3, 8), waves.slice(8, 13)];
  }

  get points() {
    return this.top_waves[0][0] / 100;
  }
};

function expected_points(front_eval, center_eval, back_eval) {
  var f = front_eval.value / 100.0;
  var c = center_eval.value / 100.0;
  var b = back_eval.value / 100.0;
  var fp = front_eval.points;
  var cp = center_eval.points;
  var bp = back_eval.points;
  var win3 = f*c*b*2*(fp+cp+bp);
  var win2 = f*c*(1-b)*(fp+cp-bp) + f*(1-c)*b*(fp-cp+bp) + (1-f)*c*b*(-fp+cp+bp);
  var win1 = f*(1-c)*(1-b)*(fp-cp-bp) + (1-f)*c*(1-b)*(-fp+cp-bp) +
             (1-f)*(1-c)*b*(-fp-cp+bp);
  var win0 = (1-f)*(1-c)*(1-b)*2*(-fp-cp-bp);
  return Math.floor(100 * (win3 + win2 + win1 + win0));
}

