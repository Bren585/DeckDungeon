#pragma once
#include <vector>
#include <cassert>
#include "BLIB/string.h"

enum card_suit : unsigned int {
	spade,
	heart,
	club,
	diamond,

	max_suit,
	no_suit,
	any_suit
};

#define ace			1
#define jack		11
#define queen		12
#define king		13
#define max_face	14
#define no_face		15

class card {
private:
	card_suit		_suit;
	unsigned int	_face;
public:
	card(									) : _suit(no_suit	), _face(no_face) {}
	card(card_suit suit, unsigned int face	) : _suit(suit		), _face(face	) {}
	card(const card& o						) : _suit(o._suit	), _face(o._face) {}

	card_suit		suit() const { return _suit; }
	unsigned int	face() const { return _face; }

	int				id	() const { return (bool)*this ? _suit * king + _face - 1 : 52; }

	bool operator==(const card& o) const { return _suit == o._suit && _face == o._face; }
	explicit operator bool() const { return (_suit != no_suit && _face != no_face); }

	bool suit_is(card_suit		suit) const { return _suit == suit; }
	bool face_is(unsigned int	face) const { return _face == face; }

	int modifier() {
		switch (_face) {
		case ace:	return -1;
		case 2:		return  0;
		case 3:
		case 4:
		case 5:
		case 6:		return 1;
		case 7:
		case 8:
		case 9:
		case 10:	return 2;
		case jack:
		case queen:	return 3;
		case king:	return 4;
		default:	return 0;
		}
	}

	operator string() const { 
		string out;
		switch (_face) {
		case ace:	out = "ace";	break;
		case jack:	out = "jack";	break;
		case queen: out = "queen";	break;
		case king:	out = "king";	break;
		default:	out = string((int)_face);
		}
		
		out += " of ";

		switch (_suit) {
		case spade	: out += "spades";		break;
		case heart	: out += "hearts";		break;
		case club	: out += "clubs";		break;
		case diamond: out += "diamonds";	break;
		}

		return out;
	}

	string sprite_name() const {
		string out;
		switch (_suit) {
		case spade:		out = "S";	break;
		case heart:		out = "H";	break;
		case club:		out = "C";	break;
		case diamond:	out = "D";	break;
		default:					return "cards/Back_Blue.png";
		}

		switch (_face) {
		case ace:	out += "A";	break;
		case jack:	out += "J";	break;
		case queen: out += "Q";	break;
		case king:	out += "K";	break;
		case 10:	out += "T"; break;
		default:	out += string((int)_face);
		}

		return "cards/" + out + ".png";
	}
};

class pile {
private:
	std::vector<card> cards;

public:
	pile() {}
	pile(card card) : cards({ card }) {}
	pile(std::vector<card> cards) : cards(cards) {}
	pile(const pile& o) : cards(o.cards) {}

	size_t size() const { return cards.size(); }

	pile play(int index = -1) {
		if (index < 0) { index = (int)cards.size() - 1; }
		assert(index < cards.size());
		card out = cards[index];
		cards.erase(cards.begin() + index);
		return out;
	}

	pile remove(int index) { return play(index); }

	pile draw(int amount = 1) {
		pile out;
		if (amount == -1) { amount = (int)cards.size(); }
		for (int i = 0; i < amount; i++) {
			out.cards.push_back(cards.back());
			cards.pop_back();
		}
		return out;
	};

	pile empty() { return draw(-1); }

	void into(pile& dest) {
		while (!cards.empty()) {
			dest.cards.push_back(cards.back());
			cards.pop_back();
		}
	}

	void shuffle() {
		std::vector<card> temp;
		while (cards.size() > 0) {
			int i = rand() % cards.size();
			temp.push_back(cards[i]);
			cards.erase(cards.begin() + i);
		}
		cards = temp;
	}

	const card& operator[](int i) const { return cards[i]; }
	auto begin	() { return cards.begin	(); }
	auto end	() { return cards.end	(); }
	auto begin	() const { return cards.begin	(); }
	auto end	() const { return cards.end		(); }

	operator card() { assert(cards.size() == 1); return cards[0]; }

};

inline pile make_deck() {
	std::vector<card> cards;
	for (int s = 0; s < 4; s++) {
		for (unsigned int f = 1; f < 14; f++) {
			cards.push_back(card{ card_suit(s), f });
		}
	}
	return cards;
}

inline card rand_card() { return card((card_suit)(rand() % max_suit), rand() % king + 1); }