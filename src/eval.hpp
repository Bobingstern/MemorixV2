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

#ifdef TUNE
struct Trace {
  int phase, eval, phase_value = 0;
  double result = 0.5;
  bool sideToMove = 0;
  std::array<std::array<int, 2>, 6> pieceTypeValues = {};
  std::array<std::array<int, 2>, 28> kingLineDanger = {};
  std::array<std::array<std::array<int, 2>, 64>, 6> PSQT = {};
  std::array<std::array<int, 2>, 5> mobilities = {};
  // Singles
  std::array<int, 2> bishopPair;
};

Trace EvalTrace = {};

#endif

#ifdef DEV
  const int32_t pieceTypeValues[6] = {S(124, 202), S(560, 363), S(567, 410), S(802, 714), S(1588, 1401), S(0, 0), };
  const int32_t kingLineDanger[28] =  {S(0, 0), S(0, 0), S(72, -27), S(36, 26), S(18, 49), S(-1, 42), S(-3, 40), S(-14, 45), S(-28, 53), S(-50, 49), S(-39, 43), S(-71, 52), S(-75, 53), S(-118, 63), S(-131, 61), S(-155, 57), S(-148, 49), S(-152, 41), S(-128, 29), S(-130, 23), S(-136, 19), S(-176, 9), S(-163, 0), S(-202, -17), S(-212, -23), S(-159, -46), S(-130, -32), S(-135, -34), };
  const int32_t mobilities[5] = {S(10, 2), S(10, 5), S(8, 4), S(5, 1), S(-5, -1), };
#else
  const int32_t pieceTypeValues[6] PROGMEM = {S(124, 202), S(560, 363), S(567, 410), S(802, 714), S(1588, 1401), S(0, 0), };
  const int32_t kingLineDanger[28] PROGMEM =  {S(0, 0), S(0, 0), S(72, -27), S(36, 26), S(18, 49), S(-1, 42), S(-3, 40), S(-14, 45), S(-28, 53), S(-50, 49), S(-39, 43), S(-71, 52), S(-75, 53), S(-118, 63), S(-131, 61), S(-155, 57), S(-148, 49), S(-152, 41), S(-128, 29), S(-130, 23), S(-136, 19), S(-176, 9), S(-163, 0), S(-202, -17), S(-212, -23), S(-159, -46), S(-130, -32), S(-135, -34), };
#endif


int32_t getValue(const int32_t *arr, int i){
  #ifdef DEV
    return arr[i];
  #else
    return (int32_t)pgm_read_dword(arr+i); // Reading from flash, cooked activity but its necessary
  #endif
}


uint64_t popcount(uint64_t x) {
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
      #ifdef TUNE
        EvalTrace.pieceTypeValues[i][WHITE] += board.material[i].C_[WHITE];
        EvalTrace.pieceTypeValues[i][BLACK] += board.material[i].C_[BLACK];
      #endif
    #else
      mat += (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[WHITE];
      mat -= (int32_t)pgm_read_dword(pieceTypeValues+i) * board.material[i].C_[BLACK];
    #endif
  }

  #ifdef TUNE
  // PSQT
  for (int sq=0;sq<64;sq++){
    if (board.sqType(sq) != -1){
      bool col = board.sqColor(sq);
      EvalTrace.PSQT[board.sqType(sq)][col == WHITE ? sq ^ 56 : sq][col]++;
    }
  }
  #endif

  return mat;
}

int32_t evalKingSaftey(Board &board, bool color){
  // Based off Weiss
  int32_t eval = 0;
  uint64_t safeline = rank1BB << (relativeRank(color, RANK_1) * 8);
  int count = popcount(~safeline & queen_attack(ntz(board.pieces(KING, color)), board.pieces(color) | board.pieces(PAWN)));
  eval += getValue(kingLineDanger, count);

  #ifdef TUNE
    EvalTrace.kingLineDanger[count][color]++;
  #endif

  return eval;
}
int32_t evalMobility(Board &board, int type, bool color){
}
int evaluate(Board &board, bool color){
  #ifdef TUNE
    EvalTrace = {};
  #endif
  int32_t eval = evalMaterial(board) + board.psqt_value;

  // King Saftey
  eval += evalKingSaftey(board, WHITE) - evalKingSaftey(board, BLACK);


  // Bishop pair
  int32_t bpair = S(50, 66);
  eval += (board.material[BISHOP].C_[WHITE] >= 2 ? bpair : 0);
  eval -= (board.material[BISHOP].C_[BLACK] >= 2 ? bpair : 0);

  #ifdef TUNE
    EvalTrace.bishopPair[WHITE] += board.material[BISHOP].C_[WHITE] >= 2;
    EvalTrace.bishopPair[BLACK] += board.material[BISHOP].C_[BLACK] >= 2;
  #endif

  // Mobility
  //eval += evalMobility(board, KNIGHT, WHITE) + evalMobility(board, BISHOP, WHITE) + evalMobility(board, ROOK, WHITE) + evalMobility(board, QUEEN, WHITE);
  //eval -= evalMobility(board, KNIGHT, BLACK) + evalMobility(board, BISHOP, BLACK) + evalMobility(board, ROOK, BLACK) + evalMobility(board, QUEEN, BLACK);

  //printf("%d %d\n", MgScore(eval), EgScore(eval));

  int32_t phase = getPhase(board.PHASE_VALUE);
  eval = ( MgScore(eval) * phase + (EgScore(eval) * (256 - phase))) / 256;

  #ifdef TUNE
    EvalTrace.phase = phase;
    EvalTrace.phase_value = board.PHASE_VALUE;
    EvalTrace.eval = eval;
    EvalTrace.sideToMove = board.sideToMove;
  #endif

  return (color == WHITE ? eval : -eval) + 18; // Tempo too
}