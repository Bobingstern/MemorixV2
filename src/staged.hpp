#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "board.hpp"


#define getQuietStage(s) ((s & (0b1)))
#define getPieceStage(s) ((s & (0b1110)) >> 1)
#define getCaptureStage(s) ((s & (0b1110000)) >> 4)
#define getPromoStage(s) ((s & (0b1110000000)) >> 7)
#define getCastleStage(s) ((s & (0b110000000000)) >> 10)

int fileOf(int sq) { return sq & 7; }
int rankOf(int sq) { return sq >> 3; }
int relativeRank(bool color, int rank) { return color == WHITE ? rank : RANK_8 - rank; }

template <typename T> bool sgn(T val) {
    return T(0) > val;
}

uint16_t stagePack(bool isQuiet, uint16_t pieceType, uint16_t captureType, uint16_t promo, uint16_t castle){
  // (2 castle bit if promo)(3 bits promo type)(3 bits capture type)(3 bits piece type)(is quiet)
  // 4 bits wasted
  uint16_t p = (uint16_t)isQuiet;
  p |= pieceType << 1;
  p |= captureType << 4;
  p |= promo << 7;
  p |= castle << 10;
  return p;
}


#ifdef DEV
  void printBoard(Board &board) {
    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 8; x++) {
        const uint8_t index = y * 8 + x;
        if (bitRead(board.pieces(PAWN, WHITE), index)) {
          printf("P");
        } else if (bitRead(board.pieces(PAWN, BLACK), index)) {
          printf("p");
        } else if (bitRead(board.pieces(KNIGHT, WHITE), index)) {
          printf("N");
        } else if (bitRead(board.pieces(KNIGHT, BLACK), index)) {
          printf("n");
        } else if (bitRead(board.pieces(BISHOP, WHITE), index)) {
          printf("B");
        } else if (bitRead(board.pieces(BISHOP, BLACK), index)) {
          printf("b");
        } else if (bitRead(board.pieces(ROOK, WHITE), index)) {
          printf("R");
        } else if (bitRead(board.pieces(ROOK, BLACK), index)) {
          printf("r");
        } else if (bitRead(board.pieces(QUEEN, WHITE), index)) {
          printf("Q");
        } else if (bitRead(board.pieces(QUEEN, BLACK), index)) {
          printf("q");
        } else if (bitRead(board.pieces(KING, WHITE), index)) {
          printf("K");
        } else if (bitRead(board.pieces(KING, BLACK), index)) {
          printf("k");
        } else {
          printf(".");
        }
        printf(" ");
      }
      printf("\n");
    }
    printf("\n");
  }

#else
  void printBoard(Board &board) {
    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 8; x++) {
        const uint8_t index = y * 8 + x;
        if (bitRead(board.pieces(PAWN, WHITE), index)) {
          Serial.print("P");
        } else if (bitRead(board.pieces(PAWN, BLACK), index)) {
          Serial.print("p");
        } else if (bitRead(board.pieces(KNIGHT, WHITE), index)) {
          Serial.print("N");
        } else if (bitRead(board.pieces(KNIGHT, BLACK), index)) {
          Serial.print("n");
        } else if (bitRead(board.pieces(BISHOP, WHITE), index)) {
          Serial.print("B");
        } else if (bitRead(board.pieces(BISHOP, BLACK), index)) {
          Serial.print("b");
        } else if (bitRead(board.pieces(ROOK, WHITE), index)) {
          Serial.print("R");
        } else if (bitRead(board.pieces(ROOK, BLACK), index)) {
          Serial.print("r");
        } else if (bitRead(board.pieces(QUEEN, WHITE), index)) {
          Serial.print("Q");
        } else if (bitRead(board.pieces(QUEEN, BLACK), index)) {
          Serial.print("q");
        } else if (bitRead(board.pieces(KING, WHITE), index)) {
          Serial.print("K");
        } else if (bitRead(board.pieces(KING, BLACK), index)) {
          Serial.print("k");
        } else {
          Serial.print(".");
        }
        Serial.print(" ");
      }
      Serial.print("\n");
    }
    Serial.print("\n");
  }
#endif



#ifdef DEV
  void printType(int t){
    switch (t){
      case PAWN: printf("Pawn "); break;
      case KNIGHT: printf("Knight "); break;
      case BISHOP: printf("Bishop "); break;
      case ROOK: printf("Rook "); break;
      case QUEEN: printf("Queen "); break;
      case KING: printf("King "); break;
    }
  }
  void printMaterial(Board &b){
    for (int i=PAWN;i<KING;i++){
      printType(i); printf("White: "); printf("%d ", b.material[i].C_[WHITE]); printf(" Black: "); printf("%d\n", b.material[i].C_[BLACK]);
    }
  }
#else
  void printType(int t){
    switch (t){
      case PAWN: Serial.print("Pawn "); break;
      case KNIGHT: Serial.print("Knight "); break;
      case BISHOP: Serial.print("Bishop "); break;
      case ROOK: Serial.print("Rook "); break;
      case QUEEN: Serial.print("Queen "); break;
      case KING: Serial.print("King "); break;
    }
  }
  void printMaterial(Board &b){
    for (int i=PAWN;i<KING;i++){
      printType(i); Serial.print("White: "); Serial.print(b.material[i].C_[WHITE]); Serial.print(" Black: "); Serial.println(b.material[i].C_[BLACK]);
    }
  }

#endif


class StagedMoveHandler{
  public:
    uint16_t stage = stagePack(true, PAWN, 0, 0, 0);
    uint64_t movementBB;
    uint8_t currentPiece;
    bool color;
    Board *board = nullptr;

    StagedMoveHandler(Board *b, bool c){
      board = b;
      color = c;
      movementBB = 0ULL;
      currentPiece = 0;
    }
    void reset(){
      stage = stagePack(true, PAWN, 0, 0, 0);
      movementBB = 0;
      currentPiece = 0;
    }
    uint64_t getOpposing(){
      return board->pieces(QUEEN - getCaptureStage(stage), !color);
    }
    uint64_t generateType(int x, bool isQuiet, int type){
      uint64_t toAnd = isQuiet ? ~board->occ() : getOpposing();
      switch (type) {
        case PAWN:
          return generatePawn(BB(x), isQuiet, false);
        case KNIGHT:
          return knight_attack(x) & toAnd;
        case BISHOP:
          return bishop_attack(x, board->occ()) & toAnd;
        case ROOK:
          return rook_attack(x, board->occ()) & toAnd;
        case QUEEN:
          return queen_attack(x, board->occ()) & toAnd;
        case KING:
          return king_attack(x) & toAnd;
      }
    }
    void setAttackBB(int from, bool isQuiet, int type){
      movementBB = generateType(from, isQuiet, type);
    }
    
    uint64_t getCurrentPiece(){
      uint64_t x = board->pieces(getPieceStage(stage), color);
      for (uint8_t i=0;i<currentPiece;i++){
        x &= ~rightBit(x);
      }
      return rightBit(x);
    }
    void nextPieceStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage) + 1, getCaptureStage(stage), getPromoStage(stage), getCastleStage(stage));
    }
    void nextCaptureStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage), getCaptureStage(stage) + 1, getPromoStage(stage), getCastleStage(stage));
    }
    void nextPromoStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage), getCaptureStage(stage), getPromoStage(stage) + 1, getCastleStage(stage));
    }
    void nextCastleStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage), getCaptureStage(stage), getPromoStage(stage), getCastleStage(stage) + 1);
    }
    void resetPromoStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage), getCaptureStage(stage), 0, getCastleStage(stage));
    }
    void resetCaptureStage(){
      stage = stagePack(getQuietStage(stage), getPieceStage(stage), 0, getPromoStage(stage), getCastleStage(stage));
    }
    
    void setQuiet(){
      stage = stagePack(false, PAWN, PAWN, 0, getCastleStage(stage));
    }
    bool isSqAttacked(int sq){
      uint64_t result = 0ULL;
      result |= knight_attack(sq) & board->pieces(KNIGHT, !color);
      result |= bishop_attack(sq, board->occ()) & (board->pieces(BISHOP, !color) | board->pieces(QUEEN, !color));
      result |= rook_attack(sq, board->occ()) & (board->pieces(ROOK, !color) | board->pieces(QUEEN, !color));
      result |= king_attack(sq) & board->pieces(KING, !color);
      result |= generatePawn(BB(sq), false, true) & board->pieces(PAWN, !color);
      return result > 0;
    }
    bool inCheck(){
      return isSqAttacked(ntz(board->pieces(KING, color)));
    }
    bool canOO(){
      return ((board->castleHistory[board->ply] & (color == WHITE ? WHITE_OO : BLACK_OO)) != 0 &&
          (color == WHITE ? (BB(G1_) | BB(F1_)) & board->occ() : (BB(G8_) | BB(F8_)) & board->occ()) == 0 && 
          !(isSqAttacked(color == WHITE ? G1_ : G8_) || isSqAttacked(color == WHITE ? F1_ : F8_)) && (BB((int)(color == WHITE ? H1_ : H8_)) & board->pieces(ROOK, color)) != 0) &&
          !inCheck();
    }
    bool canOOO(){
      // printBitboard(BB((int)(color == WHITE ? A1_ : A8_)));
      // printBitboard(board->pieces(ROOK, color));
      return ((board->castleHistory[board->ply] & (color == WHITE ? WHITE_OOO : BLACK_OOO)) != 0 &&
          (color == WHITE ? (BB(B1_) | BB(C1_)| BB(D1_)) & board->occ() : (BB(B8_) | BB(C8_)| BB(D8_)) & board->occ()) == 0 && 
          !(isSqAttacked(color == WHITE ? C1_ : C8_) || isSqAttacked(color == WHITE ? D1_ : D8_)) && (BB((int)(color == WHITE ? A1_ : A8_)) & board->pieces(ROOK, color)) != 0) &&
          !inCheck();
    }
    uint64_t updatePieceUntilFill(){
      // Basically we cycle over the piece stages until we get a non empty one
      // THis is better than doing it recusrively cuz it saves RAM
      uint64_t x = getCurrentPiece();
      while (x == 0){
        // So this one is empty
        // Let try the next piece 
        // However, if we're on the 11th piece (max 10 of each piece is possible if all pawns promote to say a rook for example)
        // Then we need to move on to the next piece type and reset currentPiece
        if (currentPiece >= 11){
          currentPiece = 0;
          nextPieceStage();
          // BUT! If this is the last piece type (king), there were done and there are no more valid moves
          if (getPieceStage(stage) > KING){
            return 0;
          }
        }
        else {
          currentPiece ++;
        }
        x = getCurrentPiece();
      }
      return x;
    }
    uint16_t getMove(int from, bool isQuiet){
      int to = ntz(rightBit(movementBB));
      if (getPieceStage(stage) == PAWN && (to < 8 || to > 55)){
        nextPromoStage();
      }
      int pr = getPromoStage(stage);
      bool isEP = !isQuiet && getPieceStage(stage) == PAWN && (abs(to - from) == 9 || abs(from - to) == 7) && ((BB(to) & board->occ()) == 0);
      if (pr >= 4 || pr == 0){
        movementBB &= ~rightBit(movementBB);
        resetPromoStage();
      }
      // Are we out of moves? Lets shift now
      if (movementBB == 0){
        if (isQuiet)
          currentPiece ++;
        else {
          nextCaptureStage();
          if (getCaptureStage(stage) >= KING){
            resetCaptureStage();
            currentPiece ++;
          }
        }
      }
      return movePack(from, to, (getPieceStage(stage) == PAWN && abs(from - to) == 16), !isQuiet && !isEP, isEP, pr, 0);
    }
    uint16_t nuxt(bool isQuiet){
      // Who are we on?
      uint64_t x = updatePieceUntilFill();
      if (x == 0){
        // There are no more pieces to generate for
        return 0;
      }
      int from = ntz(x);
      // We are guartuneed to have a piece to generate for here
      // Lets make sure we don't already have a populated movement bb
      // if we dont, lets make one
      if (movementBB == 0){
        setAttackBB(from, isQuiet, getPieceStage(stage));
      }
      // Ok, now we have a movement bb to generate with
      // Is it empty? That would mean no valid moves are available for that piece and we should move on to the next piece
      if (movementBB == 0){
        if (isQuiet)
          currentPiece ++;
        else {
          nextCaptureStage();
          if (getCaptureStage(stage) >= KING){
            resetCaptureStage();
            currentPiece ++;
          }
        }
        return 0;
      }
      // Alright, now we definetely have a moveset, and a valid piece
      // Lets make the move and pop it from the movement bb
      return getMove(from, isQuiet);
    }
    uint16_t nextMove(){
      uint16_t m = 0;
      // Check castles
      if (getCastleStage(stage) == 0){
        nextCastleStage();
        if (canOO()){
          return color == WHITE ? WHITE_OO_MOVE : BLACK_OO_MOVE;
        }
      }
      else if (getCastleStage(stage) == 1){
        nextCastleStage();
        if (canOOO()){
          return color == WHITE ? WHITE_OOO_MOVE : BLACK_OOO_MOVE;
        }
      }
      while (m == 0 && getQuietStage(stage) == 1){
        m = nuxt(true);
        if (getPieceStage(stage) > KING){
          stage = stagePack(false, PAWN, 0, 0, getCastleStage(stage));
          break;
        }
      }
      // No quiet moves cuz 0
      while (m == 0){
        m = nuxt(false);
        if (getPieceStage(stage) > KING){
          break;
        }
      }
      return m;
    }
    
    uint64_t generatePawn(uint64_t x, bool isQuiet, bool checkcheck){
      int up = color == WHITE ? NORTH : SOUTH;
      int left = color == WHITE ? WEST : EAST;
      int right = -left;

      if (isQuiet){
        uint64_t m = shift(x, up) & ~board->occ();
        if ((x & (color == WHITE ? rank2BB : rank7BB)) != 0)
          m |= shift(m, up) & ~board->occ();
        return m;
      }
      else {
        uint64_t bb = (shift(x, up+left) | shift(x, up+right)) & (checkcheck ? board->pieces(!color) : getOpposing());
        int me = ntz(x);
        int to = getTo(board->moveHistory[board->ply]);
        
        if (getFlag(board->moveHistory[board->ply]) == DOUBLE_PAWN_PUSH && rankOf(me) == rankOf(to) && (abs(me - to) == 1) && getCaptureStage(stage) == PAWN){
            bb |= shift(BB(to), up) & ~board->occ();
            // printBoard(*board);
            // printBitboard(bb);
            //Serial.print("En passant possible after ");
            //Serial.println(board->moveToStr(board->moveHistory[board->ply]));
            //enPassantTarget = shift(BB(getTo(board->moveHistory[board->ply])), up);
            //Serial.println("Possible en passant");
        }
        // else {
        //     bb &= board->pieces(!color);
        // }
        return bb;
      }
      return 0;
    }

};



