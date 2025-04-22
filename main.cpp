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
  //0.0855748379
  // position startpos moves d2d4 d7d6 e2e4 g7g6 g2g3 d6d5 e4d5 d8d5 g1f3 g8f6 b1c3 d5e6 f1e2 f8g7 d4d5 e6f5 d1d3 e8g8 d3f5 c8f5 f3d4 f5g4 e1g1 g4e2 d4e2 e7e5 d5e6
  intialize_tuner();
  load_data();
  //computeOptimalK();
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