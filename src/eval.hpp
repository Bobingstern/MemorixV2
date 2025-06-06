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

const int32_t bishopPair = S(14, 87);

#ifdef DEV
  const int32_t pieceTypeValues[6] = {S(65, 183), S(344, 473), S(304, 513), S(555, 816), S(1166, 1442), S(0, 0), };
  
  const int32_t kingLineDanger[28] = {S(0, 0), S(0, 0), S(30, 13), S(22, 47), S(9, 45), S(8, 29), S(6, 27), S(0, 33), S(-6, 34), S(-18, 34), S(-14, 31), S(-30, 37), S(-39, 35), S(-53, 36), S(-64, 36), S(-72, 35), S(-83, 31), S(-83, 24), S(-94, 19), S(-105, 13), S(-115, 9), S(-130, -1), S(-139, -11), S(-188, -26), S(-184, -42), S(-206, -54), S(-130, -32), S(-135, -34), };

  const int32_t mobilityBonus[4][32] = {
    {S(-156, -22), S(-143, 19), S(-133, 40), S(-125, 47), S(-122, 54), S(-119, 61), S(-112, 56), S(-106, 60), S(-95, 45), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-112, -49), S(-120, -12), S(-99, 14), S(-94, 32), S(-90, 46), S(-88, 59), S(-86, 71), S(-86, 78), S(-85, 82), S(-80, 82), S(-70, 80), S(-37, 64), S(-26, 74), S(-18, 42), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-224, -5), S(-239, 44), S(-213, 43), S(-209, 78), S(-205, 99), S(-203, 115), S(-202, 121), S(-195, 128), S(-188, 134), S(-176, 136), S(-166, 138), S(-156, 137), S(-155, 144), S(-143, 132), S(-95, 111), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-336, -211), S(-302, -116), S(-284, -87), S(-272, -28), S(-263, 31), S(-260, 64), S(-260, 87), S(-261, 114), S(-260, 133), S(-260, 152), S(-257, 162), S(-256, 171), S(-255, 179), S(-253, 182), S(-252, 186), S(-253, 189), S(-255, 187), S(-253, 185), S(-250, 183), S(-250, 183), S(-241, 178), S(-231, 162), S(-251, 178), S(-227, 142), S(-192, 123), S(-95, 125), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };

  const int32_t passers[4] = {S(-18, 28), S(0, 53), S(29, 104), S(91, 21), };

  const int32_t passersDefended[4] = {S(10, 20), S(35, 33), S(91, 55), S(195, 60), };

  const int32_t kingAttacks[4] = {S(11, -2), S(20, 0), S(27, -4), S(16, 21), };

#else
  const int32_t pieceTypeValues[6] PROGMEM = {S(65, 183), S(344, 473), S(304, 513), S(555, 816), S(1166, 1442), S(0, 0), };
  
  const int32_t kingLineDanger[28] PROGMEM = {S(0, 0), S(0, 0), S(30, 13), S(22, 47), S(9, 45), S(8, 29), S(6, 27), S(0, 33), S(-6, 34), S(-18, 34), S(-14, 31), S(-30, 37), S(-39, 35), S(-53, 36), S(-64, 36), S(-72, 35), S(-83, 31), S(-83, 24), S(-94, 19), S(-105, 13), S(-115, 9), S(-130, -1), S(-139, -11), S(-188, -26), S(-184, -42), S(-206, -54), S(-130, -32), S(-135, -34), };
  
  const int32_t mobilityBonus[4][32] PROGMEM = {
    {S(-156, -22), S(-143, 19), S(-133, 40), S(-125, 47), S(-122, 54), S(-119, 61), S(-112, 56), S(-106, 60), S(-95, 45), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-112, -49), S(-120, -12), S(-99, 14), S(-94, 32), S(-90, 46), S(-88, 59), S(-86, 71), S(-86, 78), S(-85, 82), S(-80, 82), S(-70, 80), S(-37, 64), S(-26, 74), S(-18, 42), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-224, -5), S(-239, 44), S(-213, 43), S(-209, 78), S(-205, 99), S(-203, 115), S(-202, 121), S(-195, 128), S(-188, 134), S(-176, 136), S(-166, 138), S(-156, 137), S(-155, 144), S(-143, 132), S(-95, 111), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), },
    {S(-40, -35), S(-336, -211), S(-302, -116), S(-284, -87), S(-272, -28), S(-263, 31), S(-260, 64), S(-260, 87), S(-261, 114), S(-260, 133), S(-260, 152), S(-257, 162), S(-256, 171), S(-255, 179), S(-253, 182), S(-252, 186), S(-253, 189), S(-255, 187), S(-253, 185), S(-250, 183), S(-250, 183), S(-241, 178), S(-231, 162), S(-251, 178), S(-227, 142), S(-192, 123), S(-95, 125), S(128, 199), S(0, 0), S(0, 0), S(0, 0), S(0, 0), }
  };

  const int32_t passers[4] PROGMEM = {S(-18, 28), S(0, 53), S(29, 104), S(91, 21), };

  const int32_t passersDefended[4] PROGMEM = {S(10, 20), S(35, 33), S(91, 55), S(195, 60), };

  const int32_t kingAttacks[4] PROGMEM = {S(11, -2), S(20, 0), S(27, -4), S(16, 21), };

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