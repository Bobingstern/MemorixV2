#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MATE 31000
#define INFINITE MATE + 1
#define NO_SCORE MATE + 2
#define TBWIN 30000
#define TBWIN_IN_MAX TBWIN - 999,

#ifdef TUNE
struct Trace {
  int phase, eval, phase_value = 0;
  double result = 0.5;
  bool sideToMove = 0;
  std::array<std::array<int, 2>, 6> pieceTypeValues = {};
  std::array<std::array<int, 2>, 28> kingLineDanger = {};
  std::array<std::array<std::array<int, 2>, 64>, 6> PSQT = {};
  std::array<std::array<std::array<int, 2>, 32>, 4> mobilityBonus = {};
  std::array<std::array<int, 2>, 4> passers = {};
  std::array<std::array<int, 2>, 4> passersDefended = {};
  std::array<std::array<int, 2>, 4> kingAttacks = {};
  // Singles
  std::array<int, 2> bishopPair;
};

Trace EvalTrace = {};

#endif

const int32_t bishopPair = S(13, 86);

#ifdef DEV
  const int32_t pieceTypeValues[6] = {S(64, 182), S(343, 470), S(303, 510), S(551, 809), S(1165, 1420), S(0, 0), };
  
  const int32_t kingLineDanger[28] = {S(0, 0), S(0, 0), S(30, 10), S(22, 47), S(11, 45), S(9, 28), S(6, 27), S(0, 33), S(-4, 33), S(-17, 35), S(-15, 32), S(-30, 38), S(-37, 35), S(-50, 36), S(-61, 35), S(-71, 35), S(-79, 30), S(-83, 25), S(-89, 19), S(-101, 13), S(-112, 9), S(-127, 0), S(-141, -10), S(-191, -25), S(-184, -39), S(-238, -49), S(-130, -32), S(-135, -34), };

  const int32_t mobilityBonus[4][32] = {
    {S(-161, -19), S(-145, 21), S(-136, 40), S(-128, 46), S(-124, 52), S(-121, 59), S(-114, 54), S(-108, 58), S(-97, 42), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-112, -48), S(-120, -11), S(-100, 14), S(-95, 31), S(-91, 44), S(-89, 58), S(-87, 69), S(-87, 76), S(-87, 80), S(-82, 81), S(-72, 78), S(-41, 62), S(-38, 70), S(-34, 37), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-237, 14), S(-239, 42), S(-213, 44), S(-209, 78), S(-206, 97), S(-204, 114), S(-203, 120), S(-196, 126), S(-189, 133), S(-177, 135), S(-167, 136), S(-157, 136), S(-155, 142), S(-144, 130), S(-92, 109), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-339, -201), S(-300, -130), S(-286, -115), S(-279, -18), S(-266, 31), S(-266, 69), S(-265, 87), S(-265, 114), S(-265, 136), S(-264, 153), S(-261, 164), S(-261, 175), S(-259, 181), S(-258, 187), S(-256, 188), S(-257, 191), S(-258, 189), S(-258, 187), S(-254, 183), S(-257, 188), S(-251, 188), S(-234, 163), S(-265, 188), S(-218, 131), S(-131, 92), S(-71, 112), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };

  const int32_t passers[4] = {S(-19, 28), S(-1, 52), S(25, 105), S(84, 20), };

  const int32_t passersDefended[4] = {S(11, 20), S(34, 34), S(94, 50), S(232, 43), };

  const int32_t kingAttacks[4] = {S(12, -3), S(21, 0), S(26, -4), S(16, 21), };

#else
  const int32_t pieceTypeValues[6] PROGMEM = {S(70, 200), S(341, 461), S(300, 501), S(545, 788), S(1144, 1381), S(0, 0), };
  
  const int32_t kingLineDanger[28] PROGMEM = {S(0, 0), S(0, 0), S(26, 7), S(18, 46), S(7, 46), S(5, 29), S(2, 30), S(-2, 34), S(-7, 35), S(-19, 34), S(-17, 31), S(-32, 39), S(-39, 35), S(-49, 36), S(-59, 35), S(-69, 35), S(-73, 29), S(-78, 25), S(-84, 20), S(-95, 14), S(-106, 11), S(-124, 0), S(-139, -7), S(-185, -26), S(-179, -38), S(-233, -51), S(-130, -32), S(-135, -34), };

  const int32_t mobilityBonus[4][32] PROGMEM = {
    {S(-164, -16), S(-149, 20), S(-139, 37), S(-131, 42), S(-127, 47), S(-124, 54), S(-117, 48), S(-111, 52), S(-101, 35), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-116, -43), S(-123, -12), S(-103, 12), S(-98, 28), S(-94, 41), S(-91, 54), S(-90, 64), S(-90, 71), S(-89, 75), S(-85, 75), S(-75, 71), S(-46, 57), S(-41, 63), S(-41, 32), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-240, -7), S(-241, 34), S(-215, 36), S(-211, 73), S(-208, 93), S(-206, 110), S(-205, 117), S(-198, 123), S(-191, 130), S(-179, 132), S(-169, 133), S(-159, 132), S(-156, 137), S(-146, 126), S(-94, 106), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-332, -179), S(-301, -152), S(-289, -112), S(-283, -20), S(-270, 32), S(-270, 69), S(-269, 87), S(-269, 113), S(-269, 135), S(-268, 152), S(-265, 162), S(-264, 173), S(-263, 180), S(-262, 185), S(-260, 187), S(-260, 189), S(-262, 187), S(-261, 184), S(-258, 181), S(-261, 185), S(-255, 185), S(-238, 159), S(-269, 183), S(-223, 128), S(-118, 76), S(-47, 90), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };

  const int32_t kingAttacks[4] PROGMEM = {S(12, -3), S(20, 0), S(26, -3), S(16, 20), };

  const int32_t passers[4] PROGMEM = {S(-19, 28), S(-1, 52), S(25, 105), S(84, 20), };

  const int32_t passersDefended[4] PROGMEM = {S(11, 20), S(34, 34), S(94, 50), S(232, 43), };

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
    mat += getValue(pieceTypeValues, i) * board.material[i].C_[WHITE];
    mat -= getValue(pieceTypeValues, i) * board.material[i].C_[BLACK];
    #ifdef TUNE
      EvalTrace.pieceTypeValues[i][WHITE] += board.material[i].C_[WHITE];
      EvalTrace.pieceTypeValues[i][BLACK] += board.material[i].C_[BLACK];
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
int32_t evalMobility(Board &board, uint64_t area, bool color, uint64_t full[2]){
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
      full[color] |= b;
      int mob = popcount(b & area);
      int kingThreats = popcount(b & king_attack_bb(board.pieces(KING, !color)));

      #ifdef DEV
        eval += mobilityBonus[p-1][mob];
      #else
        eval += pgm_read_dword(&(mobilityBonus[p-1][mob]));
      #endif
      // Also consider king attacks
      eval += getValue(kingAttacks, p-1) * kingThreats;

      #ifdef TUNE
        EvalTrace.mobilityBonus[p-1][mob][color]++;
        EvalTrace.kingAttacks[p-1][color] += kingThreats;
      #endif
      pbb &= ~isol;
    }
  }
  return eval;
}

int32_t evalPawns(Board &board, bool color){
  int32_t eval = 0;
  uint64_t pawns = board.pieces(PAWN, color);
  while (pawns){
    int sq = ntz(rightBit(pawns));
    if (relativeRank(color, rankOf(sq)) > 2){
      uint64_t walkable = color == WHITE ? fileHBB << sq : fileABB >> (63 - sq);
      // No blockers and adjacent pawns and not doubled up means we're a passed pawn
      if (!( walkable & ~BB(sq) & (board.pieces(PAWN, color) | board.pieces(PAWN, !color) | pawn_attacks( board.pieces(PAWN, !color), !color))   )){
        eval += getValue(passers, relativeRank(color, rankOf(sq))-3);
        // Bonus if we're defended by another fellow pawn
        if (pawn_attacks(board.pieces(PAWN, color), color) & BB(sq)){
          eval += getValue(passersDefended, relativeRank(color, rankOf(sq))-3);
          #ifdef TUNE
          EvalTrace.passersDefended[relativeRank(color, rankOf(sq))-3][color]++;
          #endif
        }
        #ifdef TUNE
        EvalTrace.passers[relativeRank(color, rankOf(sq))-3][color]++;
        #endif
      }
    }
    pawns &= ~rightBit(pawns);
  }
  return eval;
}
int evaluate(Board &board, bool color){
  #ifdef TUNE
    EvalTrace = {};
  #endif
  int32_t eval = evalMaterial(board) + board.psqt_value;

  // King Saftey
  eval += evalKingSaftey(board, WHITE) - evalKingSaftey(board, BLACK);


  // Bishop pair
  
  eval += (board.material[BISHOP].C_[WHITE] >= 2 ? bishopPair : 0);
  eval -= (board.material[BISHOP].C_[BLACK] >= 2 ? bishopPair : 0);

  #ifdef TUNE
    EvalTrace.bishopPair[WHITE] += board.material[BISHOP].C_[WHITE] >= 2;
    EvalTrace.bishopPair[BLACK] += board.material[BISHOP].C_[BLACK] >= 2;
  #endif

  uint64_t mobilityArea[2] = {~(pawn_attacks(board.pieces(PAWN, BLACK), BLACK) | king_attack_bb(board.pieces(KING, WHITE))),
                              ~(pawn_attacks(board.pieces(PAWN, WHITE), WHITE) | king_attack_bb(board.pieces(KING, BLACK)))};

  uint64_t mobility[2] = {0, 0};
  eval += evalMobility(board, mobilityArea[0], WHITE, mobility);
  eval -= evalMobility(board, mobilityArea[1], BLACK, mobility);


  //Pawn stuff

  eval += evalPawns(board, WHITE) - evalPawns(board, BLACK);
  

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