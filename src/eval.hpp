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

const int32_t pieceTypeValues[5] PROGMEM = {S(P_MG, P_EG), S(N_MG, N_EG), S(B_MG, B_EG), S(R_MG, R_EG), S(Q_MG, Q_EG)};


int fileOf(int sq) { return sq & 7; }
int rankOf(int sq) { return sq >> 3; }
int relativeRank(bool color, int rank) { return color == WHITE ? rank : RANK_8 - rank; }

int getPhase(int value){
  return (value * 256 + 12) / 24;
}

int32_t evalMaterial(Board &board){
  int32_t mat = 0;
  for (int i=PAWN;i<KING;i++){
    mat += (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[WHITE];
    mat -= (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[BLACK];
    
  }
  
  return mat;
}
int evaluate(Board &board, bool color){
  int32_t eval = evalMaterial(board) + board.psqt_value;
  int32_t phase = getPhase(board.PHASE_VALUE);
  eval = ( MgScore(eval) * phase + (EgScore(eval) * (256 - phase))) / 256;

  return color == WHITE ? eval : -eval;
}