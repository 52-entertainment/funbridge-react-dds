#include <stdbool.h>
#include "dds.h"
#include <stdio.h>
#include <string.h>
#include <emscripten/emscripten.h>

int suit_map[128];
int rank_map[128];
int player_map[128];

EMSCRIPTEN_KEEPALIVE int dds_init()
{
  suit_map['S'] = 0;
  suit_map['H'] = 1;
  suit_map['D'] = 2;
  suit_map['C'] = 3;
  suit_map['N'] = 4;

  rank_map['2'] = 2;
  rank_map['3'] = 3;
  rank_map['4'] = 4;
  rank_map['5'] = 5;
  rank_map['6'] = 6;
  rank_map['7'] = 7;
  rank_map['8'] = 8;
  rank_map['9'] = 9;
  rank_map['T'] = 10;
  rank_map['J'] = 11;
  rank_map['Q'] = 12;
  rank_map['K'] = 13;
  rank_map['A'] = 14;

  player_map['N'] = 0;
  player_map['E'] = 1;
  player_map['S'] = 2;
  player_map['W'] = 3;

  SetResources(40, 1);
  return 0;
}

void translate_card(const char *card, int *dds_suit, int *dds_rank)
{
  if (strlen(card) != 2)
  {
    dds_suit[0] = 0;
    dds_rank[0] = 0;
    return;
  }
  char rank = card[0];
  char suit = card[1];

  dds_suit[0] = suit_map[(int)suit];
  dds_rank[0] = rank_map[(int)rank];
}

EMSCRIPTEN_KEEPALIVE char *do_dds_solve_board(const char *contract, const char *trick_leader, const char *card1, const char *card2, const char *card3, const char *pbn_remain_cards)
{
  struct dealPBN dpbn;
  dpbn.first = player_map[(int)trick_leader[0]];

  char suit = contract[1];
  dpbn.trump = suit_map[(int)suit];

  translate_card(card1, dpbn.currentTrickSuit + 0, dpbn.currentTrickRank + 0);
  translate_card(card2, dpbn.currentTrickSuit + 1, dpbn.currentTrickRank + 1);
  translate_card(card3, dpbn.currentTrickSuit + 2, dpbn.currentTrickRank + 2);

  memcpy(dpbn.remainCards, pbn_remain_cards, strlen(pbn_remain_cards) + 1);

  struct futureTricks ft;
  SolveBoardPBN(dpbn, 0, 3, 1, &ft, 0);

  char *json_result = (char *)malloc(1024);
  strcpy(json_result, "{");

  for (int i = 0; i < ft.cards; ++i)
  {
    char card[16];
    snprintf(card, sizeof(card), "\"%c%c\":%d,", "23456789TJQKA"[ft.rank[i] - 2], "SHDC"[ft.suit[i]], ft.score[i]);
    strcat(json_result, card);

    for (int j = ft.rank[i] - 1; j >= 2; --j)
    {
      if ((ft.equals[i] & (1 << j)) > 0)
      {
        snprintf(card, sizeof(card), "\"%c%c\":%d,", "23456789TJQKA"[j - 2], "SHDC"[ft.suit[i]], ft.score[i]);
        strcat(json_result, card);
      }
    }
  }

  json_result[strlen(json_result) - 1] = '}';
  return json_result;
}
