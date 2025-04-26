#define SEARCHINFO
#include "src/engine.hpp"

#define MAX_MESSAGE 256

// static int hashInput(char *str) {
//   int hash = 0;
//   int len = 1;
//   while (*str && *str != ' ')
//     hash ^= *(str++) ^ len++;
//   return hash;
// }
// void setLimit(const char *str, const char *token, int *limit) {
//     char *ptr = NULL;
//     if ((ptr = strstr(str, token)))
//         *limit = atoi(ptr + strlen(token));
// }

// void go(Board &board, char *str){
//   int movetime = 0;
//   setLimit(str, "movetime",  &movetime);
//   if (movetime == 0)
//     setLimit(str, board.sideToMove == WHITE ? "wtime" : "btime", &movetime);
//   search(board, movetime);
// }


Board board = Board();
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing MemorixV2");

  Board board = Board();

//   char str[] = "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves f3h5 g6h5 a2a4";

//   board.uciPosition(str);
//   printBoard(board);
//   //printBitboard(rook_attack(56, 0));
//   perft(board, 1);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  static char str[MAX_MESSAGE];
  static unsigned char index = 0;
  char inch;
  
  while (Serial.available() > 0) {
    inch = Serial.read();
    if (inch == '\r') {
      // ----
      switch (hashInput(str)) {
        case GO         : go(board, str);fflush(stdout);  break;
        case UCI        : Serial.print("id name MemorixV2 \nid author Anik Patel\nuciok\n");         break;
        case ISREADY    : Serial.print("readyok\n");fflush(stdout);      break;
        case POSITION   : board.uciPosition(str);printBoard(board); break;
        case EVAL       : Serial.println(evaluate(board, board.sideToMove)); break;
        case UCINEWGAME : board = Board();      break;
        case PERFT      : runPerft(board, str);         break;
        case QUIT       : return 0;
      }

      //---
      str[0] = 0;
      index = 0;
    } else {        
      if (index < MAX_MESSAGE-1) {
        str[index++] = inch;
        str[index] = 0;
      }
    }
  }

}
