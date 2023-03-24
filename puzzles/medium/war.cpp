#include <iostream>
#include <string>
#include <queue>
#include <algorithm>

using namespace std;

template <class T>
T pop(queue<T>& q) {
  T res = q.front();
  q.pop();
  return res;
}

struct Card {
  enum Suit : char {
    hearts = 'H',
    diamonds = 'D',
    spades = 'S',
    clubs = 'C'
  };
  Suit suit;
  string number;
  int value;

  void calcValue() {
    if (!isalpha(number.front())) {
      value = stoi(number);
    }
    else {
      switch (number.front()) {
      case 'J': value = 11; break;
      case 'Q': value = 12; break;
      case 'K': value = 13; break;
      case 'A': value = 14; break;
      default: throw invalid_argument("Invalid card");
      }
    }
  }
  friend istream& operator>>(istream& is, Card& card) {
    string name;
    is >> name;
    card.suit = (Suit)name.back();
    name.pop_back();
    card.number = name;
    card.calcValue();
    return is;
  }
  friend ostream& operator<<(ostream& os, Card& card) {
    return os << card.number << card.suit;
  }
};

enum HandResult : int {
  game_over_tie = 0,
  game_over_p1 = 1,
  game_over_p2 = 2,
  game_ongoing = 3
};

HandResult fight(queue<Card>& deck1, queue<Card>& deck2) {
  if (deck1.empty()) return game_over_p2;
  if (deck2.empty()) return game_over_p1;
  queue<Card> cards1, cards2;
  Card card1 = pop(deck1), card2 = pop(deck2);

  cerr << "Fighting " << card1 << " vs " << card2 << endl;

  while (card1.value == card2.value) {
    cerr << "Declaring war" << endl;
    cards1.push(card1);
    cards2.push(card2);
    for (int i = 0; i < 3; ++i) {
      if (deck1.empty() || deck2.empty()) return game_over_tie;
      cards1.push(pop(deck1));
      cards2.push(pop(deck2));
    }
    if (deck1.empty() || deck2.empty()) return game_over_tie;
    card1 = pop(deck1);
    card2 = pop(deck2);
    cerr << "Fighting " << card1 << " vs " << card2 << endl;
  }

  cards1.push(card1);
  cards2.push(card2);
  queue<Card>* winner;
  if (card1.value > card2.value) {
    winner = &deck1;
  }
  else {
    winner = &deck2;
  }

  do {
    winner->push(pop(cards1));
  } while (!cards1.empty());
  do {
    winner->push(pop(cards2));
  } while (!cards2.empty());
  return game_ongoing;
}

int main()
{
  int n; // the number of cards for player 1
  queue<Card> cards_p1;
  cin >> n; cin.ignore();
  for (int i = 0; i < n; i++) {
    Card c;
    cin >> c; cin.ignore();
    cards_p1.push(c);
  }

  int m; // the number of cards for player 2
  queue<Card> cards_p2;
  cin >> m; cin.ignore();
  for (int i = 0; i < m; i++) {
    Card c;
    cin >> c; cin.ignore();
    cards_p2.push(c);
  }

  cerr << "Consumed input" << endl;

  int num_turns = 0;
  while (true) {
    cerr << "Turn " << num_turns << endl;
    cerr << "P1 has " << cards_p1.size() << " cards" << endl;
    cerr << "P2 has " << cards_p2.size() << " cards" << endl;
    switch (fight(cards_p1, cards_p2)) {
    case game_over_tie:
      cout << "PAT" << endl;
      return 0;
    case game_over_p1:
      cout << "1 " << num_turns << endl;
      return 0;
    case game_over_p2:
      cout << "2 " << num_turns << endl;
      return 0;
    case game_ongoing:
      ++num_turns;
      break;
    }
  }
}
