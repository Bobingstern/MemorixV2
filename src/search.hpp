#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "staged.hpp"
#include "eval.hpp"

#define MAX_DEPTH 8

struct PV {
    int length = 0;
    uint16_t line[MAX_DEPTH+3] = {};
};

struct History {
  int eval[MAX_DEPTH+3] = {0};
  int ply = 0;
  int maxDepthPVS = 1;
  int maxDepthQS = 4;
  uint16_t lastPV[MAX_DEPTH+3] = {0};
  #ifdef DEV
    clock_t TIME_STARTED;
  #else
    int32_t TIME_STARTED;
  #endif

  int32_t MAX_TIME = 1000;
  bool aborted = false;
};


#ifdef DEV
  #define max(a, b) ((a) > (b) ? (a) : (b))
  clock_t Now() {
    return clock();
  }

  int32_t TimeSince(clock_t tp) {
    return (int32_t)clock()-(int32_t)tp;
  }
#else
  int32_t Now(){
    return millis();
  }
  int32_t TimeSince(int32_t tp){
    return (int32_t)millis() - tp;
  }
#endif

bool maxTime(History *history){
  return TimeSince(history->TIME_STARTED) >= history->MAX_TIME - 50;
}


int32_t perft_(Board &b, int og, int depth){
  
  int32_t nodes = 0;
  
  if (depth == 0)
    return 1ULL;

  StagedMoveHandler stgm = StagedMoveHandler(&b, b.sideToMove);
  uint16_t move = stgm.nextMove();
  while (move != 0){
    b.makeMove(move);
    if (stgm.inCheck()){
      b.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    b.makeNullMove();
    b.unmakeNullMove();
    if (depth == og){
      //printBoard(b);
    }
    int32_t gend = perft_(b, og, depth-1);
    nodes += gend;
    if (depth == og){
      #ifdef DEV
        printf(b.moveToStr(move));
        printf("- ");
        printf("%d\n", gend);
      #else
        Serial.print(b.moveToStr(move));
        Serial.print("- ");
        Serial.println(gend);
      #endif
      
    }
    b.unmakeMove();
    move = stgm.nextMove();
  }
  return nodes;
}

void perft(Board &b, int depth){
  #ifdef DEV
    clock_t rightNow;
  #else
    int32_t rightNow;
  #endif
  rightNow = Now();
  int total = perft_(b, depth, depth);
  #ifdef DEV
    printf("Took %d ms, %d nodes, %f nps\n", TimeSince(rightNow), total, (float)total/TimeSince(rightNow)*1000.0f);
  #else
    Serial.print("Total ");
    Serial.println(total);
  #endif
  
}


int qsearch(Board &board, int alpha, const int beta, History *history, int32_t &node){

  int score = evaluate(board, board.sideToMove);
  int futility = -INFINITE;
  history->aborted = maxTime(history);
  
  if (history->ply >= history->maxDepthQS || history->aborted){
    return score;
  }
  if (score >= beta)
    return score;
  if (score > alpha)
    alpha = score;
  
  int bestScore = score;
  

  StagedMoveHandler stgm = StagedMoveHandler(&board, board.sideToMove);
  stgm.setQuiet(); //Captures only
  uint16_t move = stgm.nextMove();

  if (!stgm.inCheck()){
    futility = score + 165;
  }
  while (move != 0){
    board.makeMove(move);
    if (stgm.inCheck()){
      board.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    node++;
    history->ply++;
    score = -qsearch(board, -beta, -alpha, history, node);
    history->ply--;
    board.unmakeMove();
    if (score > bestScore){
      bestScore = score;
      if (score > alpha){
        alpha = score;
        if (score >= beta){
          break;
        }
      }
    }
    move = stgm.nextMove();
  }
  return bestScore;
}

int alphabeta(Board &board, int alpha, int beta, PV *pv, History *history, int32_t &node){
 

  int bestScore = -INFINITE;
  int score = -INFINITE;

  bool pvNode = alpha != beta - 1;
  bool root = history->ply == 0;
  PV pvFromHere;
  pv->length = 0;

  history->aborted = maxTime(history);

  if (history->ply >= history->maxDepthPVS || history->aborted){
    return qsearch(board, alpha, beta, history, node);
  }
  
  StagedMoveHandler stgm = StagedMoveHandler(&board, board.sideToMove);
  bool inCheck = stgm.inCheck();
  int eval;

  if (inCheck || pvNode){
    goto move_loop;
  }
  // Pruning stuff ig
  eval = evaluate(board, board.sideToMove);
  history->eval[history->ply] = eval;

  /*
  bool improving = !inCheck && history->ply >= 2 && eval > history->eval[history->ply-2];
  RFP
  if (history->ply < 7 &&
      eval - 175 * history->ply / (1+improving) >= beta &&
      abs(beta) < 29001)
      return eval;
  ----
  */

move_loop:
  uint16_t move = history->lastPV[history->ply];
  if (move == 0)
    move = stgm.nextMove();
  
  int moveCount = 0;
  
  while (move != 0){
    board.makeMove(move);
    if (stgm.inCheck() || move == history->lastPV[history->ply]){
      board.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    moveCount++;
    node++;
    history->ply++;
    if (root){
      //printf("%d\n", pv->length);
    }
    if (!pvNode || moveCount > 1)
      score = -alphabeta(board, -alpha-1, -alpha, &pvFromHere, history, node);
    if (pvNode && (score > alpha || moveCount == 1))
      score = -alphabeta(board, -beta, -alpha, &pvFromHere, history, node);
    history->ply--;
    board.unmakeMove();

    if (score > bestScore){
      bestScore = score;
      if ((score > alpha && pvNode) || (root && moveCount == 1)){
        // Update PV
        pv->length = 1 + pvFromHere.length;
        pv->line[0] = move;
        memcpy(pv->line + 1, pvFromHere.line, sizeof(uint16_t) * pvFromHere.length);
      }
      if (score > alpha){
        alpha = score;
        if (score >= beta){
          break;
        }
      }
    }
    
    move = stgm.nextMove();
  }
  if (moveCount == 0){
    //printBoard(board);
    //Serial.println(board.sideToMove);
    return inCheck ? -MATE + history->ply : 0;
  }
  return bestScore;
}

uint16_t iterativeDeep(Board &b, int32_t timeAllowed){
  #ifdef DEV
    clock_t start = Now();
  #else
    int32_t start = Now();
  #endif
  int32_t nodes = 0;
  uint16_t best = 0;
  History hist;
  for (int depth=1;depth<MAX_DEPTH;depth++){
    PV pv;
    hist.TIME_STARTED = start;
    hist.MAX_TIME = timeAllowed;
    hist.maxDepthPVS = depth;
    hist.aborted = false;
    memset(hist.lastPV, 0, sizeof(hist.lastPV));
    
    int score = alphabeta(b, -INFINITE, INFINITE, &pv, &hist, nodes);
    if (best == 0)
        best = pv.line[0]; // Such low time we couldn't even finish a single search, just take a random move
    if (hist.aborted){
      break;
    }
    best = pv.line[0];
    memcpy(hist.lastPV, pv.line, sizeof(pv.line));

    #ifdef DEV
    printf("info pv ");
    for (int i=0;i<pv.length;i++)
      printf("%s ", b.moveToStr(hist.lastPV[i]));
    //printf("%s\n", b.moveToStr(best));
    printf("\ninfo cp %d \ninfo nodes %d\n", score, nodes);
    fflush(stdout);
    #else
    Serial.print("info pv ");
    for (int i=0;i<pv.length;i++){
      Serial.print(b.moveToStr(hist.lastPV[i]));
      Serial.print(" ");
    }
    //printf("%s\n", b.moveToStr(best));
    Serial.print("\ninfo cp ");
    Serial.println(score);
    Serial.print("info nodes ");
    Serial.println(nodes);
    #endif
    
  }
  return best;
}

uint16_t search(Board &b, int32_t timeAllowed){

  //int score = alphabeta(b, -INFINITE, INFINITE, depth, &pv, &hist, nodes);
  uint16_t bestMove = iterativeDeep(b, timeAllowed);
  
  #ifdef DEV
    printf("bestmove ");
    printf(b.moveToStr(bestMove));
    printf("\n");
  #else
    Serial.print("bestmove ");
    Serial.println(b.moveToStr(bestMove));
  #endif
  

  
  //Serial.println();
  //Serial.print("Evaluation: ");
  //Serial.println(score);
  //Serial.println(b.moveToStr(best));
  return bestMove;
}
