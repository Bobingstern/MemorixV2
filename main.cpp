#define DEV
//#define SEARCHINFO
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
  // 0.0855748379 (5m)
  // 0.0860449140 (5m)
  // 0.0854098400 (5m)
  // 0.0854858463 (10m)
  // position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
  intialize_tuner();
  load_data();
  //computeOptimalK(); // 2.228
  tune();
  #endif  
  char str[INPUT_SIZE];
  while (getInput(str)) {
    switch (hashInput(str)) {
      case GO         : go(board, str);fflush(stdout);  break;
      case UCI        : printf("id name MemorixV2\nid author Anik Patel\nuciok\n");fflush(stdout);         break;
      case ISREADY    : printf("readyok\n");fflush(stdout);      break;
      case POSITION   : board.uciPosition(str);printBoard(board); break;
      case EVAL       : printf("%d\n", evaluate(board, board.sideToMove)); break;
      case UCINEWGAME : board = Board();      break;
      case PERFT      : runPerft(board, str);         break;
      case QUIT       : return 0;
    }
  }
  return 0;

}


#endif