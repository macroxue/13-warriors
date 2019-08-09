var suit_names = [
  '<font color="black">♠</font>',
  '<font color="red">♥</font>',
  '<font color="black">♣</font>',
  '<font color="red">♦</font>',
];
var rank_names = [
  '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A' ];
var Two = 0, Three = 1, Four = 2, Five = 3, Six = 4, Seven = 5, Eight = 6,
    Nine = 7, Ten = 8, Jack = 9, Queen = 10, King = 11, Ace = 12;

var fold_points = 3;

var wave_names = [ 'front', 'center', 'back' ];
var Front = 0, Center = 1, Back = 2;

var level_def = [
  {ai_handicap: 3, ai_claim: 0, auto: 1, claim_aid: 1, peek: 1}, // Jack
  {ai_handicap: 2, ai_claim: 0, auto: 1, claim_aid: 1, peek: 1}, // Queen
  {ai_handicap: 1, ai_claim: 1, auto: 1, claim_aid: 1, peek: 1}, // King
  {ai_handicap: 0, ai_claim: 1, auto: 0, claim_aid: 0, peek: 0}  // Ace
];
var level = King - Jack;

var total_points = 0, ai_total_points = 0;
var hand = [], ai_hand = [];
var sorted_by_rank = false;
var wave_sizes = [ 3, 5, 5 ];
var wave_values = [ 0, 0, 0 ];
var waves = [ [], [], [] ], ai_waves = [];
var ai_eval = [];
var membership = {};
var active_wave = Front;
var saved_waves = [];

function suit(card) { return Math.floor(card / 13); }
function rank(card) { return card % 13; }

function shuffle(array) {
  for (var i = 0; i < array.length; ++i) {
    var pos = Math.floor(Math.random() * (array.length - i));
    var item = array[pos];
    array[pos] = array[array.length - 1 - i];
    array[array.length - 1 - i] = item;
  }
}

function deal_hand() {
  select_level();

  var deck = [...Array(52).keys()];
  shuffle(deck);
  ai_hand = deck.slice(0, 13);
  for (var wave in wave_names) {
    for (var i = 0; i < wave_sizes[wave]; ++i) {
      clear_card('ai_' + wave_names[wave] + '_', i);
    }
  }
  ai_waves = (new HandOptimizer(ai_hand, level_def[level].ai_handicap)).waves;
  ai_eval = [new WaveEvaluator(ai_waves[Front], Front),
    new WaveEvaluator(ai_waves[Center], Center),
    new WaveEvaluator(ai_waves[Back], Back)];

  hand = deck.slice(13, 26);
  sort_hand(false);
  membership = {};
  for (var wave in waves) {
    waves[wave] = [];
    redraw_wave(wave);
  }
  redraw_hand();
  saved_waves = [];

  set_inner_html('load', '&nbsp;Load: ');
  hide_element('score_diff');
  disable_element('level');
  hide_element('deal');
  show_element('show');
  hide_element('load');
  hide_element('special');
  show_element('claim');
  enable_element('claim');
  if (level_def[level].claim_aid && (new SpecialPattern(hand)).pattern == None)
    hide_element('claim');
  if (level_def[level].auto) {
    show_element('auto');
    hide_element('fold');
  } else {
    hide_element('auto');
    show_element('fold');
    enable_element('fold');

    var f = ai_eval[Front].value / 100.0;
    var c = ai_eval[Center].value / 100.0;
    var b = ai_eval[Back].value / 100.0;
    var win3 = f*c*b;
    var win2 = f*c*(1-b) + f*(1-c)*b + (1-f)*c*b;
    var win1 = f*(1-c)*(1-b) + (1-f)*c*(1-b) + (1-f)*(1-c)*b;
    var win0 = (1-f)*(1-c)*(1-b);
    if (win3*6 + win2*1 + win1*-1 + win0*-6 < -fold_points)
      ai_fold_hand();
  }
}

function sort_hand(toggle) {
  if (toggle) sorted_by_rank = !sorted_by_rank;
  if (sorted_by_rank) {
    hand.sort(function(a, b) { return rank(a) - rank(b); });
  } else {
    hand.sort(function(a, b) {
      return suit(a) == suit(b) ? rank(a) - rank(b) : suit(a) - suit(b);
    });
  }
  redraw_hand();
}

function redraw_hand() {
  for (var card in hand) {
    set_card('', card, hand[card]);
    if (membership[hand[card]] == null) {
      enable_card('', card);
    } else {
      disable_card('', card);
    }
  }
}

function activate_wave(wave) {
  set_background(wave_names[active_wave], 'darkgreen');
  set_background(wave_names[wave], 'orange');
  active_wave = wave;
}

function move_card(index) {
  var card = hand[index];
  if (membership[card] != null) {
    var wave = membership[card];
    for (var i in waves[wave]) {
      if (waves[wave][i] == card) {
        undo_card(membership[card], i);
        break;
      }
    }
    return;
  }

  disable_card('', index);
  membership[card] = active_wave;
  waves[active_wave].push(card);
  waves[active_wave].sort(function(a, b) { return rank(a) - rank(b); });
  redraw_wave(active_wave);

  if (waves[active_wave].length == wave_sizes[active_wave]) {
    for (var wave in waves) {
      if (waves[wave].length < wave_sizes[wave]) {
        activate_wave(wave);
        break;
      }
    }
  }
}

function fill_wave(wave) {
  activate_wave(wave);
  for (var i = waves[wave].length; i < wave_sizes[wave]; ++i) {
    for (var index in hand) {
      if (membership[hand[index]] == null) {
        move_card(index);
        break;
      }
    }
  }
}

function move_all_cards() {
  for (var index in hand) {
    move_card(i);
  }
}

function undo_all_cards() {
  for (var wave in waves) {
    clear_wave(wave);
  }
}

function clear_wave(wave) {
  for (var i = waves[wave].length - 1; i >= 0; --i) {
    undo_card(wave, i);
  }
  redraw_wave(wave);
}

function undo_card(wave, index) {
  if (waves[wave].length <= index) {
    redraw_wave(wave);
    return;
  }

  for (var card in hand) {
    if (hand[card] == waves[wave][index]) {
      enable_card('', card);
      delete membership[hand[card]];
      break;
    }
  }
  waves[wave].splice(index, 1);
  redraw_wave(wave);
}

function redraw_wave(wave) {
  var name = wave_names[wave];
  if (waves[wave].length < wave_sizes[wave]) {
    set_inner_html(name + '_pattern', '&nbsp;');
    wave_values[wave] = 0;
  } else {
    var eval = new WaveEvaluator(waves[wave], wave);
    set_inner_html(name + '_pattern',
                   pattern_names[eval.pattern] + ': ' + eval.value + '%');
    wave_values[wave] = eval.value;
    // The wave could be resorted.
    waves[wave] = eval.wave;
  }
  var sum = wave_values.reduce((a, b) => a + b, 0);
  set_inner_html('average', sum ? Math.floor(sum / 3) + '%' : '');

  for (var i = 0; i < wave_sizes[wave]; ++i) {
    if (i < waves[wave].length)
      set_card(name + '_', i, waves[wave][i]);
    else
      clear_card(name + '_', i);
  }
  activate_wave(wave);
}

function set_card(prefix, index, card) {
  set_inner_html(prefix + 'suit_' + index, suit_names[suit(card)]);
  set_inner_html(prefix + 'rank_' + index, rank_names[rank(card)]);
}

function clear_card(prefix, index) {
  set_inner_html(prefix + 'suit_' + index, '?');
  set_inner_html(prefix + 'rank_' + index, '?');
}

function enable_card(prefix, index) {
  var id = prefix + 'card_' + index;
  set_background(id, 'white');
}

function disable_card(prefix, index) {
  var id = prefix + 'card_' + index;
  set_background(id, 'silver');
}

function set_inner_html(id, value) {
  document.getElementById(id).innerHTML = value;
}

function set_background(id, color) {
  document.getElementById(id).style.backgroundColor = color;
}

function show_element(id, display = 'inline') {
  document.getElementById(id).style.display = display;
}

function hide_element(id) {
  document.getElementById(id).style.display = 'none';
}

function enable_element(id) {
  document.getElementById(id).disabled = false;
}

function disable_element(id) {
  document.getElementById(id).disabled = true;
}

function show_alert(message) {
  set_inner_html('alert_message', message);
  show_element('alert');
}

function save_waves() {
  for (var wave in waves) {
    if (waves[wave].length < wave_sizes[wave]) {
      show_alert('Finish all three waves before saving them.');
      return;
    }
  }
  for (var number in saved_waves) {
    var duplicate = true;
    for (var card in membership) {
      if (membership[card] != saved_waves[number][card]) {
        duplicate = false;
        break;
      }
    }
    if (duplicate) {
      show_alert('Waves already saved as #' + (Number(number) + 1));
      return;
    }
  }
  var number = saved_waves.length;
  saved_waves.push({});
  for (var card in membership) {
    saved_waves[number][card] = membership[card];
  }

  var button = document.createElement('BUTTON');
  button.innerHTML = Math.floor(number + 1);
  button.className = 'load';
  button.onclick = function() { load_waves(number) };
  document.getElementById('load').appendChild(button);
  show_element('load');
}

function load_waves(number) {
  for (var wave in waves) {
    waves[wave] = [];
  }
  for (var card in saved_waves[number]) {
    membership[card] = saved_waves[number][card];
    waves[membership[card]].push(card);
  }
  for (var wave in waves) {
    redraw_wave(wave);
  }
  redraw_hand();
}

function select_level() {
  level = document.getElementById('level').value;
}

function peek_ai_card(index) {
  if (!level_def[level].peek) return;

  var wave = Math.floor((index + 2) / 5);
  var card = wave == Front ? index : (index + 2) % 5;
  var prefix = 'ai_' + wave_names[wave] + '_';
  set_card(prefix, card, ai_hand[index]);
  setTimeout(function() { clear_card(prefix, card); }, 500);
}

function show_special_message(message, bottom, color) {
  show_element('special');
  set_inner_html('special', message);
  document.getElementById('special').style.bottom = bottom;
  document.getElementById('special').style.color = color;
}

function fold_hand() {
  show_special_message('Fold', '0%', 'silver');
  update_scores(0, fold_points);
  on_hand_finished();
}

function ai_fold_hand() {
  show_special_message('Fold', '74%', 'silver');
  update_scores(fold_points, 0);
  on_hand_finished();
}

function show_hand() {
  if (ai_claim()) return;

  for (var wave in waves) {
    if (waves[wave].length < wave_sizes[wave]) {
      show_alert('Finish all three waves before matching against AI.');
      return;
    }
  }

  var eval = [new WaveEvaluator(waves[Front], Front),
              new WaveEvaluator(waves[Center], Center),
              new WaveEvaluator(waves[Back], Back)];
  if (eval[Center].is_smaller_than(eval[Front])) {
    show_alert('The center wave should not be smaller than the front wave.');
    return;
  }
  if (eval[Back].is_smaller_than(eval[Center])) {
    show_alert('The back wave should not be smaller than the center wave.');
    return;
  }

  var points = 0, ai_points = 0;
  var wins = 0, ai_wins = 0;
  for (var wave in ai_waves) {
    for (var card in ai_waves[wave]) {
      set_card('ai_' + wave_names[wave] + '_', card, ai_waves[wave][card]);
    }
    var result = '';
    if (ai_eval[wave].is_smaller_than(eval[wave])) {
      var p = pattern_points[wave][eval[wave].pattern];
      points += p;
      result = '+' + p;
      ++wins;
    } else if (eval[wave].is_smaller_than(ai_eval[wave])) {
      var p = pattern_points[wave][ai_eval[wave].pattern];
      ai_points += p;
      result = '-' + p;
      ++ai_wins;
    } else {
      result = '0';
    }
    set_inner_html(wave_names[wave] + '_pattern', result);
  }

  // Double points for a sweeping win.
  if (wins == 3) points *= 2;
  else if (ai_wins == 3) ai_points *= 2;
  set_inner_html('average', (wins == 3 || ai_wins == 3) ? 'x2' : 'x1');

  update_scores(points, ai_points);
  on_hand_finished();
}

function update_scores(points, ai_points) {
  var sound = document.getElementById(points >= ai_points ? 'win' : 'lose');
  sound.volume = Math.min(1.0, Math.abs(points - ai_points) / 10 + 0.1);
  sound.play();

  total_points += points;
  ai_total_points += ai_points;
  set_inner_html('score', 'You ' + total_points + '-' + ai_total_points + ' AI');
  set_inner_html('score_diff', '&nbsp;&nbsp;+' + points + ' ' + '+' + ai_points);
  show_element('score_diff', 'block');
  animate_score_diff(-5);
}

function animate_score_diff(offset) {
  document.getElementById('score_diff').style.top = offset + 'px';
  if (offset > -25) {
    setTimeout(function() { animate_score_diff(offset - 2); }, 100);
  } else {
    hide_element('score_diff');
  }
}

function on_hand_finished() {
  enable_element('level');
  show_element('deal');
  hide_element('show');
  disable_element('fold');
  disable_element('claim');
}

function claim() {
  var special = new SpecialPattern(hand);
  if (special.pattern == None) {
    show_alert('No six pairs, three straights, three flushes or dragon.');
    return;
  }

  hand = special.cards;
  redraw_hand();

  show_special_message(special_names[special.pattern], '0%', 'gold');
  update_scores(special_points[special.pattern], 0);
  on_hand_finished();
}

function ai_claim() {
  if (!level_def[level].ai_claim) return false;

  var special = new SpecialPattern(ai_hand);
  if (special.pattern == None) {
    return false;
  }

  ai_hand = special.cards;
  for (var index = 0; index < 13; ++index) {
    var wave = Math.floor((index + 2) / 5);
    var card = wave == Front ? index : (index + 2) % 5;
    var prefix = 'ai_' + wave_names[wave] + '_';
    set_card(prefix, card, ai_hand[index]);
  }

  show_special_message(special_names[special.pattern], '74%', 'gold');
  update_scores(0, special_points[special.pattern]);
  on_hand_finished();
  return true;
}

function optimize_hand() {
  waves = (new HandOptimizer(hand)).waves;
  for (var wave in waves) {
    for (var card in waves[wave]) {
      membership[waves[wave][card]] = wave;
    }
    redraw_wave(wave);
  }
  redraw_hand();
}
