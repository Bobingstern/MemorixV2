#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "search.hpp"


#define INPUT_SIZE 16384

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
  #ifdef DEV
  	fflush(stdout);
  #endif
}

void go(Board &board, char *str){
  int movetime = 0;
  int depth = 0;
  setLimit(str, "movetime",  &movetime);
  setLimit(str, "depth",  &depth);
  if (movetime == 0 && depth == 0){
    setLimit(str, board.sideToMove == WHITE ? "wtime" : "btime", &movetime);
    movetime /= 20;
  }
  if (movetime == 0)
    search(board, 0, min(depth, (int)MAX_DEPTH));
  else
    search(board, movetime, 0);
  
}