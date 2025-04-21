#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "psqt.hpp"
#include "movegen.hpp"


#define getFlag(move) ((move & (0b1111000000000000)) >> 12)
#define getFrom(move) (move & (0b111111))
#define getTo(move) ((move & (0b0000111111000000)) >> 6)
#define bitRead(value, bit) (((value) >> (63-bit)) & 0x01)

enum PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ALL, TYPE_NB = 8
};

#define WHITE 0
#define BLACK 1

#define WHITE_BB 6
#define BLACK_BB 7

#define QUIET 0
#define DOUBLE_PAWN_PUSH 1
#define KING_CASTLE 2
#define QUEEN_CASTLE 3
#define CAPTURE 4
#define EP_CAPTURE 5
#define KNIGHT_PROMO 8
#define BISHOP_PROMO 9
#define ROOK_PROMO 10
#define QUEEN_PROMO 11
#define KNIGHT_PROMO_CAPTURE 12
#define BISHOP_PROMO_CAPTURE 13
#define ROOK_PROMO_CAPTURE 14
#define QUEEN_PROMO_CAPTURE 15

#define WHITE_OO_MOVE 0b0010000001000011
#define BLACK_OO_MOVE 0b0010111001111011
#define WHITE_OOO_MOVE 0b0011000101000011
#define BLACK_OOO_MOVE 0b0011111101111011
#define NO_MOVE 0;

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


struct MAT_C {
  uint8_t *C_ = nullptr;
  MAT_C(){
    C_ = new uint8_t[2];
    C_[WHITE] = 0; C_[BLACK] = 0;
  }
  MAT_C(uint8_t w, uint8_t b){
    C_ = new uint8_t[2];
    C_[WHITE] = w; C_[BLACK] = b;
  }
};

//
enum CastlingRights {
    WHITE_OO  = 1,
    WHITE_OOO = 2,
    BLACK_OO  = 4,
    BLACK_OOO = 8,

    OO  = WHITE_OO  | BLACK_OO,
    OOO = WHITE_OOO | BLACK_OOO,
    WHITE_CASTLE = WHITE_OO | WHITE_OOO,
    BLACK_CASTLE = BLACK_OO | BLACK_OOO,
    ALL_CASTLE = WHITE_CASTLE | BLACK_CASTLE
};

enum Square {
    H1_, G1_, F1_, E1_, D1_, C1_, B1_, A1_,
    H2_, G2_, F2_, E2_, D2_, C2_, B2_, A2_,
    H3_, G3_, F3_, E3_, D3_, C3_, B3_, A3_,
    H4_, G4_, F4_, E4_, D4_, C4_, B4_, A4_,
    H5_, G5_, F5_, E5_, D5_, C5_, B5_, A5_,
    H6_, G6_, F6_, E6_, D6_, C6_, B6_, A6_,
    H7_, G7_, F7_, E7_, D7_, C7_, B7_, A7_,
    H8_, G8_, F8_, E8_, D8_, C8_, B8_, A8_
};

enum File {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};

enum Rank {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

#ifdef DEV
  void printBitboard(uint64_t bb) {
    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 8; x++) {
        const uint8_t index = y * 8 + x;
        printf("%d", (int)bitRead(bb, index));
      }
      printf("\n");
    }
    printf("\n");
  }
#else
  void printBitboard(uint64_t bb) {
    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 8; x++) {
        const uint8_t index = y * 8 + x;
        Serial.print((int)bitRead(bb, index));
      }
      Serial.println("");
    }
    Serial.println("");
  }
#endif


void moveSet(uint16_t &move, uint8_t from, uint8_t to, bool doublePawnPush, bool capture, bool enPassant, int promo, uint8_t castle) {
  move = (uint16_t)from;
  move |= (uint16_t)to << 6;
  uint8_t flags = 0;
  if (doublePawnPush) {
    flags = 0b00000001;
  } else if (castle != 0) {
    flags = castle;
  } else if (capture) {
    if (promo != 0) {
      flags = 0b1100 + (promo - 1);
    } else {
      flags = 0b0100;
    }
  } else if (promo != 0) {
    flags = 0b1000 + (promo - 1);
  } else if (enPassant) {
    flags = 0b0101;
  }

  move |= (uint16_t)flags << 12;
}

uint16_t movePack(uint8_t from, uint8_t to, bool doublePawnPush, bool capture, bool enPassant, int promo, uint8_t castle) {
  uint16_t move = (uint16_t)0b0000000000000000;
  moveSet(move, from, to, doublePawnPush, capture, enPassant, promo, castle);
  return move;
}


// Utility functions
int chrType(char c){
  switch (c){
    case 'P': return 0;
    case 'N': return 1;
    case 'B': return 2;
    case 'R': return 3;
    case 'Q': return 4;
    case 'K': return 5;

    case 'p': return 6;
    case 'n': return 7;
    case 'b': return 8;
    case 'r': return 9;
    case 'q': return 10;
    case 'k': return 11;  
    case '.': return 12;
  }
  return -1;
}


int makeSquare(int file, int rank){
  return (7-file) + 8*rank;
}

int strToSq(char* str){
  return makeSquare(str[0]-'a', str[1]-'1');
}
void sqToStr(int sq, char *str) {
    str[1] = '1' + ((sq) / 8);
    str[0] = 'a' + (7-(int)((sq) % 8));
}
bool BeginsWith(const char *str, const char *token) {
  return strstr(str, token) == str;
}

const int phaseValue[6] = {0, 1, 1, 2, 4, 0};

class Board{
  public:
    uint64_t *bitboards = nullptr;
    uint8_t castleRights = ALL_CASTLE;
    uint16_t *moveHistory = nullptr;
    uint8_t *castleHistory = nullptr;
    int8_t *captureHistory = nullptr;
    int ply = 0; 
    bool sideToMove = WHITE;
    int PHASE_VALUE = 24;

    MAT_C *material = nullptr;

    int32_t psqt_value = 0;
    
    Board(){
      this->bitboards = new uint64_t[8];
      this->bitboards[PAWN] = 0x000000000000FF00ULL | 0x00FF000000000000ULL;
      this->bitboards[KNIGHT] = 0x0000000000000042ULL | 0x4200000000000000ULL;
      this->bitboards[BISHOP] = 0x0000000000000024ULL | 0x2400000000000000ULL;
      this->bitboards[ROOK] = 0x0000000000000081ULL | 0x8100000000000000ULL;
      this->bitboards[QUEEN] = 0x0000000000000010ULL | 0x1000000000000000ULL;
      this->bitboards[KING] = 0x000000000000008ULL | 0x0800000000000000ULL;

      this->bitboards[WHITE_BB] = 0xffffULL;
      this->bitboards[BLACK_BB] = 0xffff000000000000ULL;

      this->moveHistory = new uint16_t[32];
      this->castleHistory = new uint8_t[32]; //2 castle flags can be stored per 8 bits
      this->captureHistory = new int8_t[32];

      captureHistory[0] = -1;
      moveHistory[0] = 0;
      castleHistory[0] = ALL_CASTLE;

      // Material
      this->material = new MAT_C[6];
      this->material[PAWN].C_[WHITE] = 8; this->material[PAWN].C_[BLACK] = 8;
      this->material[KNIGHT].C_[WHITE] = 2; this->material[KNIGHT].C_[BLACK] = 2;
      this->material[BISHOP].C_[WHITE] = 2; this->material[BISHOP].C_[BLACK] = 2;
      this->material[ROOK].C_[WHITE] = 2; this->material[ROOK].C_[BLACK] = 2;
      this->material[QUEEN].C_[WHITE] = 1; this->material[QUEEN].C_[BLACK] = 1;

      for (int sq=0;sq<64;sq++){
        bool col = sqColor(sq);
        #ifdef DEV
          psqt_value += (col == WHITE ? 1 : -1) * (PSQT[sqType(sq)][ col == WHITE ? sq^56 : sq ]);
        #else
          psqt_value += (col == WHITE ? 1 : -1) * pgm_read_dword(&(PSQT[sqType(sq)][ col == WHITE ? sq^56 : sq ]));
        #endif
        
      }
    }
    // Get pieces of specific type and/or color
    uint64_t pieces(int type, bool color){
      return this->bitboards[type] & this->bitboards[color + 6];
    }
    uint64_t pieces(int type){
      return this->bitboards[type];
    }
    uint64_t pieces(bool color){
        return bitboards[color + 6];
    }
    // Utility function for square types and occupation
    bool sqColor(int sq){
      if ( (this->bitboards[WHITE_BB] & (0x1ULL << sq)) != 0 ){
        return WHITE;
      }
      return BLACK;
    }
    int sqType(int sq){
      uint64_t bb = 0x1ULL << sq;
      for (int i=0;i<6;i++){
        if ( (this->bitboards[i] & bb) != 0)
          return i;
      }
      return -1;
    }
    uint64_t occ(){
      return bitboards[WHITE_BB] | bitboards[BLACK_BB];
    }
    // Add and remove a piece
    void addPiece(int sq, int type, bool color){

      this->bitboards[type] |= 0x1ULL << sq;
      this->bitboards[6+color] |= 0x1ULL << sq;
      //printBitboard(bitboards[BLACK_BB]);
      this->material[type].C_[color]++;
      PHASE_VALUE += phaseValue[type];
      

      #ifdef DEV
        psqt_value += (color == WHITE ? 1 : -1) * (PSQT[type][ color == WHITE ? sq^56 : sq ]);
      #else
        psqt_value += (color == WHITE ? 1 : -1) * pgm_read_dword(&(PSQT[type][ color == WHITE ? sq^56 : sq ]));
      #endif
    }
    void addPiece(int sq, char t){
      int type = chrType(t);
      if (type > 11)
        return;
      this->bitboards[type%6] |= 0x1ULL << sq;
      this->bitboards[6+(type>=6)] |= 0x1ULL << sq;

      this->material[type % 6].C_[type > 5]++;
      PHASE_VALUE += phaseValue[type % 6];
      

      #ifdef DEV
        psqt_value += (sqColor(sq) == WHITE ? 1 : -1) * (PSQT[type % 6][ sqColor(sq) == WHITE ? sq^56 : sq ]);
      #else
        psqt_value += (sqColor(sq) == WHITE ? 1 : -1) * pgm_read_dword(&(PSQT[type % 6][ sqColor(sq) == WHITE ? sq^56 : sq ]));
      #endif
    }
    void removePiece(int sq){
      int type = sqType(sq);
      bool sqc = sqColor(sq);
      this->bitboards[type] &= ~(0x1ULL << sq);
      this->bitboards[sqc+6] &= ~(0x1ULL << sq);
      this->material[type].C_[sqc]--;

      PHASE_VALUE -= phaseValue[type];
      
      #ifdef DEV
        psqt_value -= (sqc == WHITE ? 1 : -1) * (PSQT[type][ sqc == WHITE ? sq^56 : sq ]);
      #else
        psqt_value -= (sqc == WHITE ? 1 : -1) * pgm_read_dword(&(PSQT[type][ sqc == WHITE ? sq^56 : sq ]));
      #endif
    }
    // Sliced diced cooked fen parsing
    void parseFen(const char *fen){
      for (int i=0;i<32;i++){
        captureHistory[i] = -1;
        moveHistory[i] = 0;
        castleHistory[i] = ALL_CASTLE;
      }
      for (int i=0;i<8;i++){
        bitboards[i] *= 0;
      }
      for (int i=PAWN;i<=KING;i++){
        this->material[i].C_[WHITE] = 0; this->material[i].C_[BLACK] = 0;
      }
      PHASE_VALUE = 0;
      psqt_value = 0;
      castleRights = 0;
      ply = 0;
      char c, *copy = strdup(fen);
      char *token = strtok(copy, " ");
      int sq = A8_;
      while ((c = *token++))
        switch (c) {
          case '/': sq -= 0; break;
          case '1' ... '8': sq -= c - '0'; break;
          default: { addPiece(sq--, c);};
        }

      this->sideToMove = *strtok(NULL, " ") == 'w' ? WHITE : BLACK;
      token = strtok(NULL, " ");
      while ((c = *token++)) {
        switch (c) {
          case 'K': castleRights |= WHITE_OO; break;
          case 'k': castleRights |= BLACK_OO; break;
          case 'Q': castleRights |= WHITE_OOO; break;
          case 'q': castleRights |= BLACK_OOO; break;
        }
      }
      castleHistory[ply] = castleRights;
      captureHistory[ply] = -1;
      moveHistory[ply] = 0;

      free(copy);
    }

    void uciPosition(char *str){
      //Board();
      parseFen(BeginsWith(str, "position fen") ? str + 13 : START_FEN);

      if ((str = strstr(str, "moves")) == NULL) return;
      char *move = strtok(str, " ");
      while ((move = strtok(NULL, " "))) {
          // Parse and make move
          ply = 0;
          castleHistory[0] = castleHistory[1];
          moveHistory[0] = moveHistory[1];
          captureHistory[0] = captureHistory[1];
          makeMove(parseMove(move));
      }
    }
    
    // Make and Unmake moves
    uint16_t parseMove(char *str){
      int from = strToSq(str);
      int to = strToSq(str+2);
      bool capture = (0x1ULL<<to) & occ();
      int type = sqType(from);
      int promo = str[4] == 'n' ? 1
                : str[4] == 'b' ? 2
                : str[4] == 'r' ? 3
                : str[4] == 'q' ? 4
                : 0;
      uint8_t castle = 0;
      if (type == KING)
        castle = (from == 3 && to == 1) ? KING_CASTLE
                : (from == 59 && to == 57) ? KING_CASTLE
                : (from == 3 && to == 5) ? QUEEN_CASTLE
                : (from == 59 && to == 61) ? QUEEN_CASTLE
                : 0;

      bool ep = (from - to == 9 || from - to == 7 || to - from == 9 || from - to == 7) && !capture && type == PAWN;
      return movePack(from, to, (from-to == 16 || to-from == 16) && type == PAWN, capture, ep, promo, castle);
    }
    char *moveToStr(uint16_t m){
      static char moveStr[6] = "";
      sqToStr(getFrom(m), moveStr);
      sqToStr(getTo(m), moveStr+2);
      switch (getFlag(m)){
          case (KNIGHT_PROMO): {
              moveStr[4] = 'n';break;
          }
          case (KNIGHT_PROMO_CAPTURE): {
                  moveStr[4] = 'n';break;
          }

          case (BISHOP_PROMO): {
              moveStr[4] = 'b';break;
          }
          case (BISHOP_PROMO_CAPTURE): {
                  moveStr[4] = 'b';break;
          }

          case (ROOK_PROMO): {
              moveStr[4] = 'r';break;
          }
          case (ROOK_PROMO_CAPTURE): {
                  moveStr[4] = 'r';break;
          }
          case (QUEEN_PROMO): {
              moveStr[4] = 'q';break;
          }
          case (QUEEN_PROMO_CAPTURE): {
                  moveStr[4] = 'q';break;
          }
          default:{
              moveStr[4] = ' ';
          }
      }
      return moveStr;

    }
    void makeNullMove(){
      ply++;
      moveHistory[ply] = NO_MOVE;
      castleHistory[ply] = castleRights;
      captureHistory[ply] = -1;
      if (ply > 0)
        castleHistory[ply] = castleHistory[ply-1];

      sideToMove = !sideToMove;
    }
    void unmakeNullMove(){
      captureHistory[ply] = -1;
      moveHistory[ply] = 0;
      castleHistory[ply] = 0;
      ply--;
      sideToMove = !sideToMove;
    }
    void makeMove(uint16_t move){
      ply++;
      moveHistory[ply] = move;
      castleHistory[ply] = castleRights;
      captureHistory[ply] = -1;
      if (ply > 0)
        castleHistory[ply] = castleHistory[ply-1];


      int from = getFrom(move);
      int to = getTo(move);
      uint8_t flag = getFlag(move);
      int type = sqType(from);
      bool color = sqColor(from);
      removePiece(from);


      if (from == 0 && type == ROOK && color == WHITE)
        castleHistory[ply] &= ~WHITE_OO;
      else if (from == 7 && type == ROOK && color == WHITE)
        castleHistory[ply] &= ~WHITE_OOO;
      else if (from == 56 && type == ROOK && color == BLACK)
        castleHistory[ply] &= ~BLACK_OO;
      else if (from == 63 && type == ROOK && color == BLACK)
        castleHistory[ply] &= ~BLACK_OOO;
      else if (type == KING && color == WHITE)
        castleHistory[ply] &= ~WHITE_CASTLE;
      else if (type == KING && color == BLACK)
        castleHistory[ply] &= ~BLACK_CASTLE;
      switch (flag){
        case CAPTURE:{
          
          switch (color){
            case (WHITE):{
              if (to == 56)
                castleHistory[ply] &= ~BLACK_OO;
              if (to == 63)
                castleHistory[ply] &= ~BLACK_OOO;
              break;
            }
            case (BLACK):{
              if (to == 0){
                castleHistory[ply] &= ~WHITE_OO;
              }
              if (to == 7)
                castleHistory[ply] &= ~WHITE_OOO;
              break;
            }
          }
          captureHistory[ply] = sqType(to); removePiece(to); break;
        }
        case EP_CAPTURE:{
          captureHistory[ply] = PAWN;
          switch (color){
            case WHITE: removePiece( to - from == 9 ? from+1 : from-1);break;
            case BLACK: removePiece( from - to == 9 ? from-1 : from+1);break;
          }
          break;
        }
        case KING_CASTLE: {
          switch (color){
            case WHITE: removePiece(0); addPiece(2, ROOK, WHITE); castleHistory[ply] &= ~WHITE_OO; break;
            case BLACK: removePiece(56); addPiece(58, ROOK, BLACK); castleHistory[ply] &= ~BLACK_OO; break;
          }
          break;
        }
        case QUEEN_CASTLE: {
          switch (color){
            case WHITE: removePiece(7); addPiece(4, ROOK, WHITE); castleHistory[ply] &= ~WHITE_OOO; break;
            case BLACK: removePiece(63); addPiece(60, ROOK, BLACK); castleHistory[ply] &= ~BLACK_OOO; break;
          }
          break;

        }
        case KNIGHT_PROMO: {
          addPiece(to, KNIGHT, color);break;
        }; 
        case BISHOP_PROMO: {
          addPiece(to, BISHOP, color);break;
        };
        case ROOK_PROMO: {
          addPiece(to, ROOK, color);break;
        };
        case QUEEN_PROMO: {
          addPiece(to, QUEEN, color);break;
        };

        case KNIGHT_PROMO_CAPTURE: {
          captureHistory[ply] = sqType(to); removePiece(to); addPiece(to, KNIGHT, color); break;
        }; 
        case BISHOP_PROMO_CAPTURE: {
          captureHistory[ply] = sqType(to); removePiece(to); addPiece(to, BISHOP, color);break;
        }; 
        case ROOK_PROMO_CAPTURE: {
          captureHistory[ply] = sqType(to); removePiece(to); addPiece(to, ROOK, color);break;
        }; 
        case QUEEN_PROMO_CAPTURE: {
          captureHistory[ply] = sqType(to); removePiece(to); addPiece(to, QUEEN, color);break;
        };
      }

      if (flag < KNIGHT_PROMO)
        addPiece(to, type, color);
      
      sideToMove = !sideToMove;
      //printf("From %d to %d flag %u color %d type %d\n", from, to, flag, color, type);
    }

    void unmakeMove(){
      uint16_t move = moveHistory[ply];
      int from = getFrom(move);
      int to = getTo(move);
      uint8_t flag = getFlag(move);
      int type = sqType(to);
      bool color = sqColor(to);

      removePiece(to);

      
      switch (flag){
        case CAPTURE:
          addPiece(to, captureHistory[ply], !color); break;
        case EP_CAPTURE: {
          switch (color){
            case WHITE: addPiece( to - from == 9 ? from+1 : from-1, PAWN, BLACK);break;
            case BLACK: addPiece( from - to == 9 ? from-1 : from+1, PAWN, WHITE);break;
          }
        }
        break;
        case KING_CASTLE: {
          switch (color){
            case WHITE: removePiece(2); addPiece(0, ROOK, WHITE); break;
            case BLACK: removePiece(58); addPiece(56, ROOK, BLACK); break;
          }
          break;
        }
        case QUEEN_CASTLE: {
          switch (color){
            case WHITE: removePiece(4); addPiece(7, ROOK, WHITE); break;
            case BLACK: removePiece(60); addPiece(63, ROOK, BLACK); break;
          }
          break;
        }
        case KNIGHT_PROMO ... QUEEN_PROMO: {
          addPiece(from, PAWN, color);break;
        }; 

        case KNIGHT_PROMO_CAPTURE ... QUEEN_PROMO_CAPTURE: {
          addPiece(to, captureHistory[ply], !color); addPiece(from, PAWN, color); break;
        }
      }
      if (flag < KNIGHT_PROMO)
        addPiece(from, type, color);
      
      
      captureHistory[ply] = -1;
      moveHistory[ply] = 0;
      castleHistory[ply] = 0;
      ply--;
      sideToMove = !sideToMove;
    }
};
