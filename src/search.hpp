#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "staged.hpp"
#include "eval.hpp"

#ifdef DEV
  #define MAX_DEPTH 8
  #define MAX_QS_DEPTH 8
#else
 #define MAX_DEPTH 4
  #define MAX_QS_DEPTH 4
#endif
struct PV {
    int length = 0;
    uint16_t line[MAX_DEPTH+3] = {0};
};

struct History {
  int eval[MAX_DEPTH+3] = {0}; // For extensions if I add them
  int ply = 0;
  int qply = 0;
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
  #define min(a, b) ((a) < (b) ? (a) : (b))
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

  //printf("%d\n", b.castleHistory[0]);
  while (move != 0){
    if (depth == og){
      //printBoard(b);
      //printf("DEBUG %s\n", b.moveToStr(move));
    }
    b.makeMove(move);
    if (stgm.inCheck()){
      b.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    b.makeNullMove();
    b.unmakeNullMove();
    
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
    Serial.print("Took ");
    Serial.print(TimeSince(rightNow));
    Serial.println(" ms");
  #endif
  
}

bool checkRepetition(Board &board){
  int8_t reps = 0;
  for (int i=board.keyIndex-1;i>=0;i--){
    if (board.keys[i] == board.keys[board.keyIndex])
      reps++;
    if (reps >= 2){
      //printf("REPEAT\n");
      return true;
    }
  }
  return false;
}

int qsearch(Board &board, int alpha, const int beta, int depth, History *history, int32_t &node){

  int score = evaluate(board, board.sideToMove);
  history->aborted = maxTime(history);
  
  if (checkRepetition(board))
    return 0;

  if (depth <= 0 || history->aborted){
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

  while (move != 0){
    board.makeMove(move);
    if (stgm.inCheck()){
      board.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    // Futility Pruning
    // if (score + 165 + EgScore(getValue(pieceTypeValues, board.captureHistory[board.ply])) <= alpha 
    //     && getFlag(move) < KNIGHT_PROMO){
    //   bestScore = max(bestScore, score + 165 + EgScore(getValue(pieceTypeValues, board.captureHistory[board.ply])));
    //   board.unmakeMove();
    //   move = stgm.nextMove();
    //   continue;
    // }

    node++;
    history->ply++;
    history->qply++;
    score = -qsearch(board, -beta, -alpha, depth-1, history, node);
    history->ply--;
    history->qply--;
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

int alphabeta(Board &board, int alpha, int beta, int depth, PV *pv, History *history, int32_t &node){
 

  int bestScore = -INFINITE;
  int score = -INFINITE;

  bool pvNode = alpha != beta - 1;
  bool root = history->ply == 0;
  PV pvFromHere;
  pv->length = 0;

  history->aborted = maxTime(history);

  // Check if this is a 3fold
  if (!root){
    if (checkRepetition(board))
      return 0;
  }

  if (depth <= 0 || history->aborted){
    history->qply = 0;
    return qsearch(board, alpha, beta, (int)MAX_QS_DEPTH, history, node);
  }
  
  StagedMoveHandler stgm = StagedMoveHandler(&board, board.sideToMove);
  bool inCheck = stgm.inCheck();
  int eval;
  //board.moveHistory[board.ply] == 0
  if (inCheck || pvNode){
    goto move_loop;
  }

  // Reverse Futility Pruning
  eval = evaluate(board, board.sideToMove);
  if (depth < 6 && !root && !inCheck && !pvNode && eval - 75 * depth >= beta)
    return eval;

  // Null Move Pruning 18.7 +/- 10.3
  // eval >= beta Results 21.8 +/- 11.2
  if ( depth >= 3 && eval >= beta){
    board.makeNullMove();
    PV nmpPV;
    int nmpv = -alphabeta(board, -beta, -beta+1, depth-3, &nmpPV, history, node);
    board.unmakeNullMove();
    if (nmpv >= beta){
      return nmpv;
    }
  }
  

move_loop:
  
  uint16_t move = 0;
  if (root){
    move = history->lastPV[0];
  }
  if (move == 0)
    move = stgm.nextMove();
  
  int moveCount = 0;
  
  while (move != 0){
    board.makeMove(move);
    bool isQuiet = getQuietStage(stgm.stage);
    if (stgm.inCheck() || (moveCount > 0 && root && move == history->lastPV[0])){
      board.unmakeMove();
      move = stgm.nextMove();
      
      continue;
    }
    #ifdef SEARCHINFO
    if (root){
      #ifdef DEV
      printf("Searching move ");
      printf(board.moveToStr(move));
      printf(" ");
      #else
      Serial.print("Searching move ");
      Serial.print(board.moveToStr(move));
      Serial.print(" ");
      #endif
    }
    #endif

    moveCount++;
    node++;
    history->ply++;

    bool doLMR = depth > 3 && moveCount > 2 && isQuiet;
    // if (doLMR){
    //   score = -alphabeta(board, -alpha-1, -alpha, depth-2, &pvFromHere, history, node);
    //   if (score > alpha)
    //     score = -alphabeta(board, -alpha-1, -alpha, depth-1, &pvFromHere, history, node);
    // }
    if (!pvNode || moveCount > 1)
      score = -alphabeta(board, -alpha-1, -alpha, depth-1, &pvFromHere, history, node);

    if (pvNode && (score > alpha || moveCount == 1))
      score = -alphabeta(board, -beta, -alpha, depth-1, &pvFromHere, history, node);

    #ifdef SEARCHINFO
    if (root){
      #ifdef DEV
        printf("%d\n", score);
      #else
        Serial.println(score);
      #endif
    }
    #endif

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
uint16_t iterativeDeep(Board &b, int32_t timeAllowed, int searchDepth){
  #ifdef DEV
    clock_t start = Now();
  #else
    int32_t start = Now();
  #endif
  int32_t nodes = 0;
  uint16_t best = 0;
  History hist;
  for (int depth=1;depth<=(searchDepth >= 1 ? searchDepth : MAX_DEPTH);depth++){
    PV pv;
    hist.TIME_STARTED = start;
    hist.MAX_TIME = (searchDepth >= 1 ? 32000 : timeAllowed);
    hist.aborted = false;
    
    int score = alphabeta(b, -INFINITE, INFINITE, depth, &pv, &hist, nodes);
    if (best == 0)
        best = pv.line[0]; // Such low time we couldn't even finish a single search, just take a random move
    if (hist.aborted){
      break;
    }
    best = pv.line[0];
    for (int i=0;i<pv.length;i++)
      hist.lastPV[i] = pv.line[i];
    //memcpy(hist.lastPV, pv.line, sizeof(pv.line));

    #ifdef DEV
    printf("info depth %d score cp %d nodes %d ", depth, score, nodes);
    printf("pv ");
    for (int i=0;i<pv.length;i++)
      printf("%s ", b.moveToStr(hist.lastPV[i]));
    printf("\n");
    //printf("%s\n", b.moveToStr(best));
    fflush(stdout);
    #else
    Serial.print("info depth ");
    Serial.print(depth);
    Serial.print(" score cp ");
    Serial.print(score);
    Serial.print(" nodes ");
    Serial.print(nodes);
    Serial.print(" pv ");
    for (int i=0;i<pv.length;i++)
      Serial.print(b.moveToStr(hist.lastPV[i]));
    Serial.print("\n");
    #endif
    
  }
  return best;
}

uint16_t search(Board &b, int32_t timeAllowed, int searchDepth){

  //int score = alphabeta(b, -INFINITE, INFINITE, depth, &pv, &hist, nodes);
  uint16_t bestMove = iterativeDeep(b, timeAllowed, searchDepth);
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
