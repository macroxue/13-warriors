var None = 0, ThreeStraights = 1, ThreeFlushes = 2, SixPairs = 3, Dragon = 4;
var special_names = [
  'None', 'Three Straights', 'Three Flushes', 'Six Pairs', 'Dragon'
];
var special_points = [0, 6, 6, 6, 13];

class SpecialPattern {
  constructor(hand) {
    var hand_copy = hand.slice(0);
    if (this.dragon(hand_copy)) this.special = Dragon;
    else if (this.six_pairs(hand_copy)) this.special = SixPairs;
    else if (this.three_flushes(hand_copy)) this.special = ThreeFlushes;
    else if (this.three_straights(hand_copy)) this.special = ThreeStraights;
    else this.special = None;
  }

  get pattern() { return this.special; }
  get cards() { return this.hand; }

  is_straight(cards) {
    for (var i = 0; i < cards.length - 1; ++i)
      if (rank(cards[i]) + 1 != rank(cards[i + 1])) return false;
    return true;
  }

  dragon(hand) {
    hand.sort((a, b) => rank(a) - rank(b));
    if (this.is_straight(hand)) {
      this.hand = hand;
      return true;
    }
    return false;
  }

  six_pairs(hand) {
    hand.sort((a, b) => rank(a) - rank(b));
    var num_pairs = 0;
    for (var i = 0; i < 12; ++i) {
      if (rank(hand[i]) == rank(hand[i + 1])) {
        ++num_pairs;
        i += 1;
      }
    }
    if (num_pairs == 6) {
      this.hand = hand;
      return true;
    }
    return false;
  }

  is_flush(cards) {
    for (var i = 0; i < cards.length - 1; ++i)
      if (suit(cards[i]) != suit(cards[i + 1])) return false;
    return true;
  }

  three_flushes(hand) {
    hand.sort((a, b) => suit(a) - suit(b));
    if (this.is_flush(hand.slice(0,3)) && this.is_flush(hand.slice(3,8)) &&
        this.is_flush(hand.slice(8,13))) {
      this.hand = hand;
      return true;
    }
    if (this.is_flush(hand.slice(0,5)) && this.is_flush(hand.slice(5,8)) &&
        this.is_flush(hand.slice(8,13))) {
      this.hand = hand.slice(5,8).concat(hand.slice(0,5), hand.slice(8,13));
      return true;
    }
    if (this.is_flush(hand.slice(0,5)) && this.is_flush(hand.slice(5,10)) &&
        this.is_flush(hand.slice(10,13))) {
      this.hand = hand.slice(10,13).concat(hand.slice(0,5), hand.slice(5,10));
      return true;
    }
    return false;
  }

  remove_straight(buckets, length) {
    for (var i = 0; i < buckets.length; ++i)
      if (buckets[i].length != 0) break;
    for (var j = 1; j < length; ++j)
      if (i + j >= buckets.length || buckets[i + j].length == 0) return [];
    var wave = [];
    for (var j = 0; j < length; ++j)
      wave.push(buckets[i + j].pop());
    return wave;
  }

  three_straights_with_buckets(hand, num_rotated_aces) {
    var buckets = compute_rank_buckets(hand, num_rotated_aces);
    var front = this.remove_straight(buckets, 3);
    var center = this.remove_straight(buckets, 5);
    var back = this.remove_straight(buckets, 5);
    if (front.length && center.length && back.length) {
      this.hand = [].concat(front, center, back);
      return true;
    }
    // Recompute rank buckets since it may be destroyed.
    buckets = compute_rank_buckets(hand, num_rotated_aces);
    center = this.remove_straight(buckets, 5);
    front = this.remove_straight(buckets, 3);
    back = this.remove_straight(buckets, 5);
    if (front.length && center.length && back.length) {
      this.hand = [].concat(front, center, back);
      return true;
    }
    // Recompute rank buckets since it may be destroyed.
    buckets = compute_rank_buckets(hand, num_rotated_aces);
    center = this.remove_straight(buckets, 5);
    back = this.remove_straight(buckets, 5);
    front = this.remove_straight(buckets, 3);
    if (front.length && center.length && back.length) {
      this.hand = [].concat(front, center, back);
      return true;
    }
    return false;
  }

  three_straights(hand) {
    if (this.three_straights_with_buckets(hand, 0)) return true;

    var buckets = compute_rank_buckets(hand, 0);
    var num_aces = buckets[Ace].length;
    if (num_aces == 0) return false;

    for (var i = 0; i < num_aces; ++i) {
      if (this.three_straights_with_buckets(hand, i+1)) return true;
    }
    return false;
  }
};

function compute_rank_buckets(cards, num_rotated_aces) {
  var buckets = [[],[],[],[],[],[],[],[],[],[],[],[],[]];
  for (var card of cards) {
    buckets[rank(card)].push(card);
  }
  if (num_rotated_aces > 0) {
    // Shift the buckets to the right and convert Aces to Ones.
    buckets.unshift([]);
    for (var i = 0; i < num_rotated_aces; ++i)
      buckets[0].push(buckets[13].pop());
  }
  return buckets;
}

function random(end) { return Math.floor(Math.random() * end); }
function random_suit() { return random(4); }

function random_dragon() {
  var ranks = [...Array(13).keys()];
  return ranks.map((rank) => rank + random_suit() * 13);
}

function random_six_pairs() {
  var hand = [];
  var ranks = [...Array(13).keys()];
  shuffle(ranks);
  for (var rank of ranks.slice(0, 7)) {
    var suit = random_suit();
    hand.push(rank + suit * 13);
    suit = (suit + 1) % 4;
    hand.push(rank + suit * 13);
  }
  return hand.slice(0, 13);
}

function random_three_flushes() {
  var hand = [];
  var ranks = [...Array(13).keys()];
  shuffle(ranks);
  var suit = random_suit();
  hand = hand.concat(ranks.slice(0, 3).map((rank) => rank + suit * 13));
  shuffle(ranks);
  suit = (suit + 1) % 4;
  hand = hand.concat(ranks.slice(0, 5).map((rank) => rank + suit * 13));
  shuffle(ranks);
  suit = (suit + 1) % 4;
  hand = hand.concat(ranks.slice(0, 5).map((rank) => rank + suit * 13));
  return hand;
}

function random_three_straights() {
  var deck = [...Array(52).keys()];
  shuffle(deck);

  do {
    // With two Aces and two Ones, no guarantee for three random straights.
    var buckets = compute_rank_buckets(deck, 2);
    var hand = [];
    var pos = random(buckets.length - 3 + 1);
    for (var i = pos; i < pos + 3; ++i)
      if (buckets[i].length) hand.push(buckets[i].pop());
    var pos = random(buckets.length - 5 + 1);
    for (var i = pos; i < pos + 5; ++i)
      if (buckets[i].length) hand.push(buckets[i].pop());
    var pos = random(buckets.length - 5 + 1);
    for (var i = pos; i < pos + 5; ++i)
      if (buckets[i].length) hand.push(buckets[i].pop());
  } while (hand.length < 13);
  return hand;
}

function test_special_patterns() {
  var num_failures = 0;
  for (var i = 0; i < 1000; ++i) {
    var hand = random_dragon();
    var special = new SpecialPattern(hand);
    if (special.pattern < Dragon) {
      console.log('Mistake Dragon as ' + special_names[special.pattern]);
      ++num_failures;
    }

    var hand = random_six_pairs();
    var special = new SpecialPattern(hand);
    if (special.pattern < SixPairs) {
      console.log('Mistake Six Pairs as ' + special_names[special.pattern]);
      ++num_failures;
    }

    var hand = random_three_flushes();
    var special = new SpecialPattern(hand);
    if (special.pattern < ThreeFlushes) {
      console.log('Mistake Three Flushes as ' + special_names[special.pattern]);
      ++num_failures;
    }

    var hand = random_three_straights();
    var special = new SpecialPattern(hand);
    if (special.pattern < ThreeStraights) {
      console.log('Mistake Three Straights as ' + special_names[special.pattern]);
      ++num_failures;
    }
  }
  if (num_failures == 0) console.log('Passed');
}
