var suit_names = [
  '<font color="black">♠</font>',
  '<font color="red">♥</font>',
  '<font color="black">♣</font>',
  '<font color="red">♦</font>',
];

var Two = 0, Three = 1, Four = 2, Five = 3, Six = 4, Seven = 5, Eight = 6,
    Nine = 7, Ten = 8, Jack = 9, Queen = 10, King = 11, Ace = 12;
var rank_names = [
  '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A' ];

var Front = 0, Center = 1, Back = 2;
var wave_names = [ 'front', 'center', 'back' ];
var wave_sizes = [ 3, 5, 5 ];

var level_def = [
  {ai_handicap: 6, ai_claim: 0, auto: 1, claim_aid: 1, fold: 0, peek: 1}, // Jack
  {ai_handicap: 4, ai_claim: 0, auto: 1, claim_aid: 1, fold: 0, peek: 1}, // Queen
  {ai_handicap: 2, ai_claim: 1, auto: 1, claim_aid: 1, fold: 1, peek: 1}, // King
  {ai_handicap: 0, ai_claim: 1, auto: 0, claim_aid: 0, fold: 1, peek: 0}  // Ace
];
var level = King - Jack;

var fold_points = 3;

var total_points = 0, ai_total_points = 0;
var hand = [], ai_hand = [];
var sorted_by_rank = false;
var waves = [ [], [], [] ], ai_waves = [];
var wave_values = [ 0, 0, 0 ], ai_eval = [];
var membership = {};  // which wave a card belongs to
var active_wave = Front;
var saved_waves = [];

var key_map = {
  'A': function() { click_element('auto'); },
  'C': function() { click_element('clear'); },
  'D': function() { click_element('deal'); },
  'F': function() { click_element('fold'); },
  'M': function() { click_element('claim'); },
  'S': function() { click_element('sort'); },
  'V': function() { click_element('save'); },
  'W': function() { click_element('show'); },

  '1': function() { click_element('load_1'); },
  '2': function() { click_element('load_2'); },
  '3': function() { click_element('load_3'); },
  '4': function() { click_element('load_4'); },

  'O': function() { click_element('ok'); },
  'Y': function() { click_element('yes'); },
  'N': function() { click_element('no'); },
  '': function() { hide_element('alert'); hide_element('confirm'); }
};

function initialize() {
  document.addEventListener('keyup', function(e) {
    for (var key in key_map) {
      if (key.charCodeAt(0) == e.keyCode) {
        key_map[key].apply();
        e.preventDefault();
        return;
      }
    }
  });
  deal_hand();
}

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

function select_level() {
  level = document.getElementById('level').value;
}

function deal_hand() {
  select_level();

  var deck = [...Array(52).keys()];
  shuffle(deck);
  ai_hand = deck.slice(0, 13);
  for (var wave in wave_names) {
    for (var i = 0; i < wave_sizes[wave]; ++i) {
      face_down('ai_' + wave_names[wave] + '_', i);
    }
  }

  hand = deck.slice(13, 26);
  sort_hand(false);
  membership = {};
  for (var wave in waves) {
    waves[wave] = [];
    redraw_wave(wave);
  }
  sort_hand(false);
  membership = {};
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
  hide_element('auto');
  if (level_def[level].claim_aid && (new SpecialPattern(hand)).pattern == None) {
    hide_element('claim');
    if (level_def[level].auto) show_element('auto');
  }
  if (level_def[level].fold) {
    show_element('fold');
    enable_element('fold');
  } else {
    hide_element('fold');
  }

  setTimeout(function() {
    var ai = (new HandOptimizer(ai_hand, level_def[level].ai_handicap));
    ai_waves = ai.waves;
    ai_eval = [new WaveEvaluator(ai_waves[Front], Front),
               new WaveEvaluator(ai_waves[Center], Center),
               new WaveEvaluator(ai_waves[Back], Back)];
    if (level_def[level].fold && ai.points < -fold_points)
      ai_fold_hand();
  }, 0);
}

function sort_hand(toggle) {
  if (toggle) sorted_by_rank = !sorted_by_rank;
  if (sorted_by_rank) {
    hand.sort((a, b) => rank(a) == rank(b) ? suit(a) - suit(b) : rank(a) - rank(b));
  } else {
    hand.sort((a, b) => suit(a) == suit(b) ? rank(a) - rank(b) : suit(a) - suit(b));
  }
  redraw_hand();
}

function redraw_hand() {
  for (var index in hand) {
    face_up('', index, hand[index]);
  }
  for (index = hand.length; index < 13; ++index) {
    clear_card('', index);
  }
}

function activate_wave(wave) {
  set_background(wave_names[active_wave], 'darkgreen');
  set_background(wave_names[wave], 'orange');
  active_wave = wave;
}

function move_card(index) {
  if (index >= hand.length) return;

  var card = hand[index];
  hand.splice(index, 1);
  redraw_hand();

  membership[card] = active_wave;
  waves[active_wave].push(card);
  waves[active_wave].sort((a, b) => rank(a) - rank(b));
  redraw_wave(active_wave);

  if (waves[active_wave].length == wave_sizes[active_wave]) {
    for (var wave in waves) {
      if (hand.length > 0 && waves[wave].length + hand.length == wave_sizes[wave]) {
        fill_wave(wave);
        break;
      }
      if (waves[wave].length < wave_sizes[wave]) {
        activate_wave(wave);
        break;
      }
    }
  }
}

// The browser fires touchstart--touchend--mousedown--mouseup for a short touch
// and touchstart--touchend for a long touch.
var long_press_ms = 250;
var touch_start_time = 0, touch_end_time = 0;
var mouse_down_time = 0, mouse_up_time = 0;

function on_touch_start(index) {
  touch_start_time = (new Date()).getTime();
}

function on_touch_end(index) {
  touch_end_time = (new Date()).getTime();

  if (index >= hand.length) return;
  if (touch_end_time - touch_start_time < long_press_ms) {
    move_card(index);
  } else {
    fill_wave(active_wave, index);
  }
}

function on_mouse_down(index) {
  mouse_down_time = (new Date()).getTime();

  // Ignore mousedown if it is too close to touchend,
  if (mouse_down_time - touch_end_time < long_press_ms * 2)
    mouse_down_time = 0;
}

function on_mouse_up(index) {
  // Ignore mouseup if mousedown is ignored.
  if (mouse_down_time == 0) return;
  mouse_up_time = (new Date()).getTime();

  if (index >= hand.length) return;
  if (mouse_up_time - mouse_down_time < long_press_ms) {
    move_card(index);
  } else {
    fill_wave(active_wave, index);
  }
}

function undo_wave_card(wave, index) {
  if (waves[wave].length <= index) {
    redraw_wave(wave);
    return;
  }

  var card = waves[wave][index];
  delete membership[card];
  hand.push(card);
  sort_hand(false);

  waves[wave].splice(index, 1);
  redraw_wave(wave);
}

function fill_wave(wave, start_index = 0) {
  activate_wave(wave);
  while (start_index < hand.length && waves[wave].length < wave_sizes[wave]) {
    move_card(start_index);
  }
}

function undo_all_cards() {
  for (var wave in waves) {
    clear_wave(wave);
  }
}

function clear_wave(wave) {
  while (waves[wave].length > 0) {
    undo_wave_card(wave, 0);
  }
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
      face_up(name + '_', i, waves[wave][i]);
    else
      clear_card(name + '_', i);
  }
  activate_wave(wave);
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
  var id = Math.floor(number + 1);
  button.innerHTML = '<span class="hotkey">' + id + '</span>';
  button.className = 'load';
  button.id = 'load_' + id;
  button.onclick = function() { load_waves(number) };
  document.getElementById('load').appendChild(button);
  show_element('load');
}

function load_waves(number) {
  waves = [ [], [], [] ];
  for (var card in saved_waves[number]) {
    membership[card] = saved_waves[number][card];
    waves[membership[card]].push(card);
  }
  for (var wave in waves) {
    redraw_wave(wave);
  }
  hand = [];
  redraw_hand();
}

function peek_ai_card(index) {
  if (!level_def[level].peek) return;
  if (is_element_active('deal')) return;

  var wave = Math.floor((index + 2) / 5);
  var card = wave == Front ? index : (index + 2) % 5;
  var prefix = 'ai_' + wave_names[wave] + '_';
  face_up(prefix, card, ai_hand[index]);
  setTimeout(() => face_down(prefix, card), 500);
}

function fold_hand() {
  show_confirm('Really want to fold your hand?',
               function() {
                 show_special_message('Fold', '0%', 'silver');
                 update_scores(0, fold_points);
                 on_hand_finished();
               });
}

function ai_fold_hand() {
  show_special_message('Fold', '74%', 'silver');
  update_scores(fold_points, 0);
  on_hand_finished();
}

function optimize_hand() {
  hand = hand.concat(waves[Front], waves[Center], waves[Back]);
  waves = (new HandOptimizer(hand)).waves;
  for (var wave in waves) {
    for (var card of waves[wave]) {
      membership[card] = wave;
    }
    redraw_wave(wave);
  }
  hand = [];
  redraw_hand();
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
    for (var i in ai_waves[wave]) {
      face_up('ai_' + wave_names[wave] + '_', i, ai_waves[wave][i]);
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

function claim() {
  var special = new SpecialPattern(hand.concat(waves[Front], waves[Center],
                                               waves[Back]));
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
    face_up(prefix, card, ai_hand[index]);
  }

  show_special_message(special_names[special.pattern], '74%', 'gold');
  update_scores(0, special_points[special.pattern]);
  on_hand_finished();
  return true;
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
    setTimeout(() => animate_score_diff(offset - 2), 100);
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

/////////////////////// Lower level functions ///////////////////////////////

function face_up(prefix, index, card) {
  set_inner_html(prefix + 'suit_' + index, suit_names[suit(card)]);
  set_inner_html(prefix + 'rank_' + index, rank_names[rank(card)]);
  set_background(prefix + 'card_' + index, 'white');
}

function face_down(prefix, index) {
  var mesh = 'linear-gradient(45deg, #dca 12%, transparent 0, transparent 88%, #dca 0), linear-gradient(135deg, transparent 37%, #a85 0, #a85 63%, transparent 0), linear-gradient(45deg, transparent 37%, #dca 0, #dca 63%, transparent 0) #753';
  set_inner_html(prefix + 'suit_' + index, '&nbsp;');
  set_inner_html(prefix + 'rank_' + index, '&nbsp;');
  set_background(prefix + 'card_' + index, 'white', mesh, '10px 10px');
}

function clear_card(prefix, index) {
  set_inner_html(prefix + 'suit_' + index, '&nbsp;');
  set_inner_html(prefix + 'rank_' + index, '&nbsp;');
  set_background(prefix + 'card_' + index, 'green');
}

function show_special_message(message, bottom, color) {
  show_element('special');
  set_inner_html('special', message);
  document.getElementById('special').style.bottom = bottom;
  document.getElementById('special').style.color = color;
}

function show_alert(message) {
  set_inner_html('alert_message', message);
  show_element('alert');
}

function show_confirm(message, action) {
  set_inner_html('confirm_message', message);
  document.getElementById('yes').onclick = function() {
    hide_element('confirm');
    action.apply();
  };
  show_element('confirm');
}

function set_inner_html(id, value) {
  document.getElementById(id).innerHTML = value;
}

function set_background(id, color, pattern = '', size = 0) {
  var element = document.getElementById(id);
  element.style.background = pattern;
  element.style.backgroundColor = color;
  element.style.backgroundSize = size;
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

function is_element_active(id) {
  var element = document.getElementById(id);
  return element && !element.disabled && element.style.display != 'none';
}

function click_element(id) {
  if (!is_element_active(id)) return;

  var element = document.getElementById(id);
  if (typeof element.onclick == 'function') element.onclick.apply();
  if (typeof element.onclick == 'string') eval(element.onclick);
}
