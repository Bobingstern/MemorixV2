#define DEV
#ifdef DEV
//#define TUNE

#include <time.h>
#include <ctime>
#include <chrono>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <random>
#include "src/engine.hpp"

#ifdef TUNE
#include "src/tuner.hpp"
#endif

int main() {

  Board board = Board();

  #ifdef TUNE
  // 0.084306
  intialize_tuner();
  load_data();
  computeOptimalK();
  tune();
  #endif
  
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