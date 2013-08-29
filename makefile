all: 13w

13w: 13w.cc card.h combo.cc combo.h deck.cc deck.h hand.cc hand.h \
		pattern.h score.h set.h strategy.cc strategy.h
	g++ -O3 -std=c++0x -o 13w $(filter %.cc,$^)
