import sys
import math

"""
Let's go back to basics with this simple card game: war!

Your goal is to write a program which finds out which player 
is the winner for a given card distribution of the "war" game.
"""

# Auto-generated code below aims at helping you parse
# the standard input according to the problem statement.

n = int(input())  # the number of cards for player 1
cards_player_1 = [input() for i in range(n)]  # the n cards of player 1
m = int(input())  # the number of cards for player 2
cards_player_2 = [input() for i in range(m)]  # the m cards of player 2

card_vals = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A']

end = False
rounds = 0

while not end:
    rounds += 1
    print('beginning round ' + str(rounds), file=sys.stderr)
    end = True

    p1_card = cards_player_1[0]
    p1_card_val = card_vals.index(p1_card[:-1])
    cards_player_1.remove(p1_card)

    p2_card = cards_player_2[0]
    p2_card_val = card_vals.index(p2_card[:-1])
    cards_player_2.remove(p2_card)

    if p1_card_val == p2_card_val:
        # war
        won = False

        while not won:
            p1_war_cards = [[p1_card]]
            p2_war_cards = [[p2_card]]
            try:
                p1_war_cards.append(cards_player_1[:3])
                [cards_player_1.remove(i) for i in p1_war_cards[-1]]
                p1_card = cards_player_1[0]
                p1_card_val = card_vals.index(p1_card[:-1])
                cards_player_1.remove(p1_card)
            except IndexError:
                print('PAT')
                while True:
                    pass

            try:
                p2_war_cards.append(cards_player_2[:3])
                [cards_player_2.remove(i) for i in p2_war_cards[-1]]
                p2_card = cards_player_2[0]
                p2_card_val = card_vals.index(p2_card[:-1])
                cards_player_2.remove(p2_card)
            except IndexError:
                print('PAT')
                while True:
                    pass

            if p1_card_val > p2_card_val:
                won = True
                cards_player_1.append(p1_war_cards[0])
                p1_war_cards.remove(p1_war_cards[0])
                cards_player_1 += p1_war_cards[0]
                p1_war_cards.remove(p1_war_cards[0])
                cards_player_1.append(p2_war_cards[0])
                p2_war_cards.remove(p2_war_cards[0])
                cards_player_1 += p2_war_cards[0]
                p2_war_cards.remove(p2_war_cards[0])
                for i in range(len(p1_war_cards)):
                    [cards_player_1.append(j) for j in p1_war_cards[i]]
                    [cards_player_1.append(j) for j in p2_war_cards[i]]
            elif p1_card_val < p2_card_val:
                won = True
                cards_player_2.append(p1_war_cards[0])
                p1_war_cards.remove(p1_war_cards[0])
                cards_player_2 += p1_war_cards[0]
                p1_war_cards.remove(p1_war_cards[0])
                cards_player_2.append(p2_war_cards[0])
                p2_war_cards.remove(p2_war_cards[0])
                cards_player_2 += p2_war_cards[0]
                p2_war_cards.remove(p2_war_cards[0])
                for i in range(len(p2_war_cards)):
                    [cards_player_2.append(j) for j in p1_war_cards[i]]
                    [cards_player_2.append(j) for j in p2_war_cards[i]]

    elif p1_card_val > p2_card_val:
        cards_player_1.append(p1_card)
        cards_player_1.append(p2_card)

    else:
        cards_player_2.append(p1_card)
        cards_player_2.append(p2_card)

    if cards_player_2 and not cards_player_1:
        print('2 ' + str(rounds))

    elif cards_player_1 and not cards_player_2:
        print('1 ' + str(rounds))

    else:
        end = False
