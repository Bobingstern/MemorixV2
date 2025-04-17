#define DEV

#ifdef DEV

#include <time.h>
#include <ctime>
#include <chrono>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include "src/engine.hpp"

#define INPUT_SIZE 8192

enum InputCommands {
    // UCI
    GO          = 11,
    UCI         = 127,
    STOP        = 28,
    QUIT        = 29,
    ISREADY     = 113,
    POSITION    = 17,
    SETOPTION   = 96,
    UCINEWGAME  = 6,
    // Non-UCI
    EVAL        = 26,
    PRINT       = 112,
    PERFT       = 116
};

bool getInput(char *str) {

    memset(str, 0, INPUT_SIZE);

    if (fgets(str, INPUT_SIZE, stdin) == NULL)
        return false;

    str[strcspn(str, "\r\n")] = '\0';

    return true;
}

static int hashInput(char *str) {
  int hash = 0;
  int len = 1;
  while (*str && *str != ' ')
    hash ^= *(str++) ^ len++;
  return hash;
}
void setLimit(const char *str, const char *token, int *limit) {
  char *ptr = NULL;
  if ((ptr = strstr(str, token)))
      *limit = atoi(ptr + strlen(token));
}

void runPerft(Board &board, char *str){
  strtok(str, " ");
  char *d = strtok(NULL, " ");
  int depth = d ? atoi(d) : 5;
  perft(board, depth);
  fflush(stdout);
}

void go(Board &board, char *str){
  int movetime = 0;
  setLimit(str, "movetime",  &movetime);
  if (movetime == 0){
    setLimit(str, board.sideToMove == WHITE ? "wtime" : "btime", &movetime);
    movetime /= 20;
  }
  search(board, movetime);
}

// void printUint16(uint16_t b){
//   for (int i=15;i>=0;i--){
//     Serial.print((int)( (b >> i) & 0x01  ));
//   }
//   Serial.println("");
// }




int main() {

  Board board = Board();

  char st[] = "position startpos moves f2f4 f7f5 e1f2 e8f7";

  //board.uciPosition(st);
  //perft(board, 5);
  // printBoard(board);
  //search(board, 1000);
  
  char str[INPUT_SIZE];
  while (getInput(str)) {
    switch (hashInput(str)) {
      case GO         : go(board, str);fflush(stdout);  break;
      case UCI        : printf("id name MemorixV2\nid author Anik Patel\nuciok\n");fflush(stdout);         break;
      case ISREADY    : printf("readyok\n");fflush(stdout);      break;
      case POSITION   : board.uciPosition(str);printBoard(board); break;
      case EVAL  : printf("%d\n", evaluate(board, board.sideToMove)); break;
      case UCINEWGAME : board = Board();      break;
      case PERFT       : runPerft(board, str);         break;
      case QUIT       : return 0;
    }
  }
  return 0;

}


#endif