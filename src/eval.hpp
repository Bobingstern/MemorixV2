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
  std::array<std::array<std::array<int, 2>, 32>, 4> mobilityBonus = {};
  // Singles
  std::array<int, 2> bishopPair;
};

Trace EvalTrace = {};

#endif

#ifdef DEV
  const int32_t pieceTypeValues[6] = {S(143, 212), S(640, 374), S(644, 394), S(960, 666), S(2089, 1230), S(0, 0), };
  
  const int32_t kingLineDanger[28] = {S(0, 0), S(0, 0), S(94, -45), S(59, 21), S(30, 50), S(9, 40), S(3, 41), S(-11, 46), S(-26, 54), S(-53, 51), S(-44, 45), S(-79, 54), S(-87, 57), S(-128, 65), S(-142, 62), S(-174, 60), S(-164, 51), S(-170, 42), S(-145, 29), S(-143, 22), S(-156, 18), S(-195, 6), S(-178, -4), S(-234, -23), S(-253, -29), S(-205, -53), S(-130, -32), S(-135, -34), };

  const int32_t pawnProtection[6] = {S(25, 17), S(16, 19), S(-2, 8), S(-6, 28), S(-2, 10), S(-78, 42), };
  
  const int32_t mobilityBonus[4][32] = {
    {S(-14, -38), S(-8, -15), S(10, 8), S(25, 6), S(33, 15), S(45, 25), S(59, 19), S(68, 28), S(80, 17), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-15, -27), S(-25, 5), S(17, 27), S(32, 28), S(46, 44), S(59, 51), S(69, 53), S(77, 58), S(86, 61), S(99, 51), S(119, 51), S(137, 39), S(210, 42), S(69, 15), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(66, -24), S(-69, 66), S(-24, 56), S(-10, 80), S(-4, 99), S(12, 95), S(7, 110), S(21, 114), S(34, 119), S(53, 120), S(79, 121), S(94, 119), S(94, 129), S(107, 126), S(120, 123), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-28, -13), S(-39, -61), S(8, -19), S(31, -52), S(39, -6), S(50, 0), S(52, 43), S(58, 60), S(55, 94), S(60, 110), S(63, 140), S(74, 135), S(78, 144), S(90, 152), S(86, 162), S(80, 177), S(94, 172), S(83, 184), S(77, 190), S(94, 196), S(79, 207), S(206, 116), S(189, 119), S(103, 142), S(163, 160), S(113, 153), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };

#else
  const int32_t pieceTypeValues[6] PROGMEM = {S(143, 212), S(640, 374), S(644, 394), S(960, 666), S(2089, 1230), S(0, 0), };
  const int32_t kingLineDanger[28] PROGMEM = {S(0, 0), S(0, 0), S(94, -45), S(59, 21), S(30, 50), S(9, 40), S(3, 41), S(-11, 46), S(-26, 54), S(-53, 51), S(-44, 45), S(-79, 54), S(-87, 57), S(-128, 65), S(-142, 62), S(-174, 60), S(-164, 51), S(-170, 42), S(-145, 29), S(-143, 22), S(-156, 18), S(-195, 6), S(-178, -4), S(-234, -23), S(-253, -29), S(-205, -53), S(-130, -32), S(-135, -34), };
  const int32_t mobilityBonus[4][32] PROGMEM = {
    {S(-14, -38), S(-8, -15), S(10, 8), S(25, 6), S(33, 15), S(45, 25), S(59, 19), S(68, 28), S(80, 17), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-15, -27), S(-25, 5), S(17, 27), S(32, 28), S(46, 44), S(59, 51), S(69, 53), S(77, 58), S(86, 61), S(99, 51), S(119, 51), S(137, 39), S(210, 42), S(69, 15), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(66, -24), S(-69, 66), S(-24, 56), S(-10, 80), S(-4, 99), S(12, 95), S(7, 110), S(21, 114), S(34, 119), S(53, 120), S(79, 121), S(94, 119), S(94, 129), S(107, 126), S(120, 123), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-28, -13), S(-39, -61), S(8, -19), S(31, -52), S(39, -6), S(50, 0), S(52, 43), S(58, 60), S(55, 94), S(60, 110), S(63, 140), S(74, 135), S(78, 144), S(90, 152), S(86, 162), S(80, 177), S(94, 172), S(83, 184), S(77, 190), S(94, 196), S(79, 207), S(206, 116), S(189, 119), S(103, 142), S(163, 160), S(113, 153), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };
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
int32_t evalMobility(Board &board, uint64_t area, bool color){
  int32_t eval = 0;
  for (int p=KNIGHT;p<=QUEEN;p++){
    uint64_t pbb = board.pieces(p, color);
    while (pbb != 0){
      uint64_t isol = rightBit(pbb);
      uint64_t b;
      if (p == BISHOP)
        b = bishop_attack(ntz(isol), board.occ() ^ board.pieces(QUEEN, color)); // Include xray
      else if (p == ROOK)
        b = rook_attack(ntz(isol), board.occ() ^ (board.pieces(ROOK, color) | board.pieces(QUEEN, color))); // Include xray
      else if (p == QUEEN)
        b = queen_attack(ntz(isol), board.occ());
      else
        b = knight_attack(ntz(isol));
      int mob = popcount(b & area);
      eval += mobilityBonus[p-1][mob];
      #ifdef TUNE
        EvalTrace.mobilityBonus[p-1][mob][color]++;
      #endif
      pbb &= ~isol;
    }

  }
  return eval;
}

// int32_t pawnProtectionBonus(Board &board, bool color){
//   int32_t eval = 0;
//   int cnt = 0;
//   uint64_t defended = pawn_attacks(board.pieces(PAWN, color), color);
//   printBitboard(defended);
//   for (int p=PAWN;p<=KING;p++){
//     cnt = popcount(defended & board.pieces(p, color));
//     eval += pawnProtection[p] * cnt;
//     #ifdef TUNE
//       EvalTrace.pawnProtection[p][color]+=cnt;
//     #endif
//   }
//   return eval;
  

// }
int evaluate(Board &board, bool color){
  #ifdef TUNE
    EvalTrace = {};
  #endif
  int32_t eval = evalMaterial(board) + board.psqt_value;

  // King Saftey
  eval += evalKingSaftey(board, WHITE) - evalKingSaftey(board, BLACK);


  // Bishop pair
  int32_t bpair = S(58, 76);
  eval += (board.material[BISHOP].C_[WHITE] >= 2 ? bpair : 0);
  eval -= (board.material[BISHOP].C_[BLACK] >= 2 ? bpair : 0);

  #ifdef TUNE
    EvalTrace.bishopPair[WHITE] += board.material[BISHOP].C_[WHITE] >= 2;
    EvalTrace.bishopPair[BLACK] += board.material[BISHOP].C_[BLACK] >= 2;
  #endif

  uint64_t mobilityArea[2] = {~(pawn_attacks(board.pieces(PAWN, BLACK), BLACK) | king_attack_bb(board.pieces(KING, WHITE))),
                              ~(pawn_attacks(board.pieces(PAWN, WHITE), WHITE) | king_attack_bb(board.pieces(KING, BLACK)))};

  eval += evalMobility(board, mobilityArea[0], WHITE);
  eval -= evalMobility(board, mobilityArea[1], BLACK);



  // Pawn Protection bonus
  //eval += pawnProtectionBonus(board, WHITE) - pawnProtectionBonus(board, BLACK);
  // Mobility
  //eval += evalMobility(board, KNIGHT, WHITE) + evalMobility(board, BISHOP, WHITE) + evalMobility(board, ROOK, WHITE) + evalMobility(board, QUEEN, WHITE);
  //eval -= evalMobility(board, KNIGHT, BLACK) + evalMobility(board, BISHOP, BLACK) + evalMobility(board, ROOK, BLACK) + evalMobility(board, QUEEN, BLACK);


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