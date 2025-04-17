#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MATE 31000
#define INFINITE MATE + 1
#define NO_SCORE MATE + 2
#define TBWIN 30000
#define TBWIN_IN_MAX TBWIN - 999,

enum PieceValue {
    P_MG =  104, P_EG =  204,
    N_MG =  420, N_EG =  632,
    B_MG =  427, B_EG =  659,
    R_MG =  569, R_EG = 1111,
    Q_MG = 1485, Q_EG = 1963
};

#ifdef DEV
  const int32_t pieceTypeValues[6] = {S(P_MG, P_EG), S(N_MG, N_EG), S(B_MG, B_EG), S(R_MG, R_EG), S(Q_MG, Q_EG), 0};
  const int32_t kingLineDanger[28] = {
    S(  0,  0), S(  0,  0), S( 15,  0), S( 11, 21),
    S(-16, 35), S(-25, 30), S(-29, 29), S(-37, 38),
    S(-48, 41), S(-67, 43), S(-67, 40), S(-80, 43),
    S(-85, 43), S(-97, 44), S(-109, 46), S(-106, 41),
    S(-116, 41), S(-123, 37), S(-126, 34), S(-131, 29),
    S(-138, 28), S(-155, 26), S(-149, 23), S(-172,  9),
    S(-148, -8), S(-134,-26), S(-130,-32), S(-135,-34)
  };
#else
  const int32_t pieceTypeValues[5] PROGMEM = {S(P_MG, P_EG), S(N_MG, N_EG), S(B_MG, B_EG), S(R_MG, R_EG), S(Q_MG, Q_EG)};
  const int32_t kingLineDanger[28] PROGMEM = {
    S(  0,  0), S(  0,  0), S( 15,  0), S( 11, 21),
    S(-16, 35), S(-25, 30), S(-29, 29), S(-37, 38),
    S(-48, 41), S(-67, 43), S(-67, 40), S(-80, 43),
    S(-85, 43), S(-97, 44), S(-109, 46), S(-106, 41),
    S(-116, 41), S(-123, 37), S(-126, 34), S(-131, 29),
    S(-138, 28), S(-155, 26), S(-149, 23), S(-172,  9),
    S(-148, -8), S(-134,-26), S(-130,-32), S(-135,-34)
  };
#endif


int32_t getValue(const int32_t *arr, int i){
  #ifdef DEV
    return arr[i];
  #else
    return (int32_t)pgm_read_dword(arr+i); // Reading from flash, cooked activity but its necessary
  #endif
}


uint64_t popcount(uint64_t x)
{
  uint64_t m1 = 0x5555555555555555ull;
  uint64_t m2 = 0x3333333333333333ull;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Full;
  uint64_t h01 = 0x0101010101010101ull;

  x -= (x >> 1) & m1;
  x = (x & m2) + ((x >> 2) & m2);
  x = (x + (x >> 4)) & m4;

  return (x * h01) >> 56;
}

int getPhase(int value){
  return (value * 256 + 12) / 24;
}

int32_t evalMaterial(Board &board){
  int32_t mat = 0;
  for (int i=PAWN;i<KING;i++){
    #ifdef DEV
      mat += (int32_t)pieceTypeValues[i] * board.material[i].C_[WHITE];
      mat -= (int32_t)pieceTypeValues[i] * board.material[i].C_[BLACK];
    #else
      mat += (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[WHITE];
      mat -= (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[BLACK];
    #endif
  }
  return mat;
}

int32_t evalKingSaftey(Board &board, bool color){
  // Based off Weiss
  int32_t eval = 0;
  uint64_t safeline = rank1BB << (relativeRank(color, RANK_1) * 8);
  int count = popcount(~safeline & queen_attack(ntz(board.pieces(KING, color)), board.pieces(color) | board.pieces(PAWN)));
  eval += getValue(kingLineDanger, count);
  return eval;
  // //int32_t kingShieldValues[2] = {S(33, -10), S(25, -7)}; // Doesnt even need to be in progmem
  // if (board.pieces(color) & (color == WHITE ? 0xC3d7ULL : 0xd3c7000000000000ULL)){
  //   uint64_t shield = 0;
  //   if (color == WHITE)
  //     shield = 0x700ULL << 5 * (fileOf(ntz(board.pieces(KING, color))) > 2);
  //   else
  //     shield = 0xe0000000000000ULL >> 5 * (fileOf(ntz(board.pieces(KING, color))) > 5);

  //   eval += popcount(shield & board.pieces(PAWN, color)) * S(33, -10);
  //   eval += popcount(shift(shield, color == WHITE ? NORTH : SOUTH) & board.pieces(PAWN, color)) * S(25, -7);
  // }
  // return eval;


}
int evaluate(Board &board, bool color){
  int32_t eval = evalMaterial(board) + board.psqt_value;

  // King Saftey
  eval += evalKingSaftey(board, WHITE) - evalKingSaftey(board, BLACK);


  // Bishop pair
  eval += (board.material[BISHOP].C_[WHITE] >= 2 ? S(29, 84) : 0);
  eval -= (board.material[BISHOP].C_[BLACK] >= 2 ? S(29, 84) : 0);

  int32_t phase = getPhase(board.PHASE_VALUE);
  eval = ( MgScore(eval) * phase + (EgScore(eval) * (256 - phase))) / 256;

  return (color == WHITE ? eval : -eval) + 18; // Tempo too
}