var Junk = 0, Pair = 1, TwoPairs = 2, Triple = 3, Straight = 4, Flush = 5,
    FullHouse = 6, Quadruple = 7, StraightFlush = 8, RoyalFlush = 9;
var pattern_names = [
  'junk', 'pair', 'two pairs', 'triple', 'straight', 'flush',
  'full house', 'quadruple', 'straight flush', 'royal flush'
];
var pattern_points = [
  //        Tr       FH Qu SF  RF
  [1, 1, 1, 3, 1, 1, 1, 1,  1,  1],  // front
  [1, 1, 1, 1, 1, 1, 2, 8, 10, 20],  // center
  [1, 1, 1, 1, 1, 1, 1, 4,  5, 10],  // back
];

var junk_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  1,  1,  2,  2,  4,  7, 15, 34],
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1],
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0]
];

var junk_value0 = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0, 20, 20, 21, 21, 22, 23, 24, 26, 30, 34, 42,  0],  //A?
  [ 0,  9,  9, 10, 10, 11, 11, 12, 13, 15, 18,  0,  0],  //K?
  [ 0,  5,  5,  5,  6,  6,  7,  7,  8,  8,  0,  0,  0]   //Q?
];

var one_pair_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [45, 47, 49, 51, 53, 56, 60, 64, 68, 73, 81, 89, 97],
  [ 2,  3,  3,  4,  5,  6,  8, 10, 12, 15, 18, 24, 33],
  [ 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  2,  2,  3]
];

var one_pair_value0 = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [96, 96, 96, 96, 96, 96, 97, 97, 97, 97, 98, 98,  0],  //A?
  [86, 86, 86, 86, 86, 87, 87, 88, 89, 89, 90,  0, 91],  //K?
  [76, 76, 77, 77, 77, 78, 78, 78, 79, 80,  0, 82, 83]   //Q?
];

var one_pair_value1 = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0, 30, 31, 31, 32, 32, 33, 33, 34, 35, 36,  0],  //A?
  [ 0,  0, 23, 23, 23, 23, 23, 24, 25, 25, 26,  0, 27],  //K?
  [ 0,  0, 17, 17, 18, 18, 18, 19, 19, 20,  0, 21, 22]   //Q?
];

var two_pair_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
  [ 0, 37, 37, 39, 41, 43, 46, 49, 54, 58, 62, 64, 64],
  [ 0,  3,  3,  4,  4,  5,  7,  8, 10, 11, 13, 14, 14]
];

var triple_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [99, 99, 99, 99, 99,100,100,100,100,100,100,100,100],
  [63, 66, 69, 71, 72, 72, 74, 74, 74, 75, 75, 75, 76],
  [12, 13, 13, 15, 16, 16, 16, 16, 16, 14, 15, 15, 15]
];

var straight_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
  [ 0,  0,  0, 77, 79, 81, 83, 85, 87, 88, 89, 91, 92],
  [ 0,  0,  0, 16, 18, 20, 22, 24, 26, 28, 31, 34, 37]
];

var flush_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
  [ 0,  0,  0,  0,  0, 93, 92, 92, 93, 94, 95, 97, 98],
  [ 0,  0,  0,  0,  0, 35, 37, 38, 38, 40, 44, 50, 61]
];

var flush_value2 = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0, 53, 54, 55, 56, 57, 59, 62, 65,  0],  //A?
  [ 0,  0,  0, 44, 44, 45, 46, 47, 48, 49, 52,  0,  0],  //K?
  [ 0,  0,  0, 41, 41, 41, 42, 42, 44, 45,  0,  0,  0]   //Q?
];

var full_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
  [98, 99, 99, 99, 99, 99, 99, 99,100,100,100,100,100],
  [65, 66, 69, 71, 73, 75, 78, 80, 82, 85, 88, 91, 94],
];

var quadruple_value = [
  //2   3   4   5   6   7   8   9   T   J   Q   K   A
  [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
  [100,100,100,100,100,100,100,100,100,100,100,100,100],
  [ 94, 94, 95, 95, 95, 96, 97, 97, 97, 97, 98, 98, 98]
];

class WaveEvaluator {
  constructor(cards, order) {
    this.cards = cards.slice(0);
    this.order = order;

    this.features = [];
    this.extract_features();
    this.win_percent = this.evaluate();
  }

  extract_features() {
    this.cards.sort((a, b) => rank(a) - rank(b));

    var num_junks = 0, num_pairs = 0, num_triples = 0;
    var length = this.cards.length;
    for (var i = 0; i < length; ++i) {
      var r = rank(this.cards[i]);
      if (i + 1 >= length || r != rank(this.cards[i + 1])) {
        this.features.push({type: Junk, rank: r});
        ++num_junks;
      } else if (i + 2 >= length || r != rank(this.cards[i + 2])) {
        this.features.push({type: Pair, rank: r});
        ++num_pairs;
        i += 1;
      } else if (i + 3 >= length || r != rank(this.cards[i + 3])) {
        this.features.push({type: Triple, rank: r});
        ++num_triples;
        i += 2;
      } else {
        this.features.push({type: Quadruple, rank: r});
        i += 3;
      }
    }
    if (num_junks == 5) {
      var high_rank = rank(this.cards[4]);
      var straight = false;
      if (rank(this.cards[0]) + 4 == rank(this.cards[4])) {
        straight = true;
      }
      // Special case for 2345A.
      if (rank(this.cards[0]) == Two && rank(this.cards[3]) == Five &&
          rank(this.cards[4]) == Ace) {
        straight = true;
        high_rank = Five;
      }

      var flush = true;
      for (var i = 0; i < length - 1; ++i) {
        if (suit(this.cards[i]) != suit(this.cards[i + 1])) {
          flush = false;
          break;
        }
      }

      if (straight && flush) {
        var type = high_rank == Ace ? RoyalFlush : StraightFlush;
        this.features = [{type: type, rank: high_rank}];
      } else if (straight) {
        this.features = [{type: Straight, rank: high_rank}];
      } else if (flush) {
        this.features.pop();
        this.features.push({type: Flush, rank: high_rank});
      }
    }

    this.features.sort((a, b) => a.type == b.type ?
                       b.rank - a.rank : b.type - a.type);

    if (num_pairs == 2) {
      this.features[0].type = TwoPairs;
    }
    if (num_triples == 1 && num_pairs == 1) {
      this.features[0].type = FullHouse;
    }
  }

  evaluate() {
    var r0 = this.rank;
    switch (this.pattern) {
      case RoyalFlush:
        return 100;
      case StraightFlush:
        return 100;
      case Quadruple:
        return quadruple_value[this.order][r0];
      case FullHouse:
        return full_value[this.order][r0];
      case Flush:
        var r1 = this.features[1].rank;
        if (this.order == Back && r0 >= Queen)
          return flush_value2[Ace - r0][r1];
        return flush_value[this.order][r0];
      case Straight:
        return straight_value[this.order][r0];
      case Triple:
        return triple_value[this.order][r0];
      case TwoPairs:
        return two_pair_value[this.order][r0];
      case Pair:
        var r1 = this.features[1].rank;
        if (this.order == Front && r0 >= Queen)
          return one_pair_value0[Ace - r0][r1];
        if (this.order == Center && r0 >= Queen)
          return one_pair_value1[Ace - r0][r1];
        return one_pair_value[this.order][r0];
      case Junk:
        var r1 = this.features[1].rank;
        if (this.order == Front && r0 >= Queen)
          return junk_value0[Ace - r0][r1];
        return junk_value[this.order][r0];
    }
  }

  get pattern() { return this.features[0].type; }
  get rank() { return this.features[0].rank; }
  get value() { return this.win_percent; }
  get points() { return pattern_points[this.order][this.pattern]; }
  get wave() {
    // Special case for 2345A.
    if ((this.pattern == Straight || this.pattern == StraightFlush) &&
        this.rank == Five) {
      return this.cards.slice(4, 5).concat(this.cards.slice(0, 4));
    }
    return this.cards;
  }

  is_smaller_than(wave) {
    var min_features = Math.min(this.features.length, wave.features.length);
    for (var i = 0; i < min_features; ++i) {
      if (this.features[i].type < wave.features[i].type) return true;
      if (this.features[i].type > wave.features[i].type) return false;
      if (this.features[i].rank < wave.features[i].rank) return true;
      if (this.features[i].rank > wave.features[i].rank) return false;
    }
    return false;
  }
};
