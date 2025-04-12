#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "staged.hpp"
#include "eval.hpp"

#define MAX_DEPTH 5

typedef struct PV {
    int length;
    uint16_t line[MAX_DEPTH+5] = {0};
} PV;

struct History {
  int eval[MAX_DEPTH+5] = {0};
  int ply = 0;
};


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
      Serial.print(b.moveToStr(move));
      Serial.print("- ");
      Serial.println(gend);
    }
    b.unmakeMove();
    move = stgm.nextMove();
  }
  return nodes;
}

void perft(Board &b, int depth){
  int total = perft_(b, depth, depth);
  Serial.print("Total ");
  Serial.println(total);
}

int qsearch(Board &board, int alpha, const int beta, int32_t &node){

  int score = evaluate(board, board.sideToMove);
  int futility = -INFINITE;
  
  if (board.ply >= MAX_DEPTH){
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
    int ptv = EgScore((int32_t)pgm_read_dword(pieceTypeValues + board.sqType(getTo(move))));
    
    if (futility + ptv <= alpha && getFlag(move) < 8){
          // Serial.println(board.sqType(getTo(move)));
          board.unmakeMove();
          move = stgm.nextMove();
          bestScore = max(bestScore, futility + ptv);
          continue;
      }
    
    score = -qsearch(board, -beta, -alpha, node);
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

int alphabeta(Board &board, int alpha, int beta, int depth, uint16_t &bm, PV *pv, History *history, int32_t &node){
 

  int bestScore = -INFINITE;
  int score = -INFINITE;

  bool pvNode = alpha != beta - 1;
  PV pvFromHere;
  pv->length = 0;

  if (depth <= 0){
    return qsearch(board, alpha, beta, node);
  }
  
  StagedMoveHandler stgm = StagedMoveHandler(&board, board.sideToMove);
  bool inCheck = stgm.inCheck();
  

  if (inCheck || pvNode){
    goto move_loop;
  }
  // Pruning stuff ig
  int eval = evaluate(board, board.sideToMove);
  history->eval[history->ply] = eval;

  bool improving = !inCheck && history->ply >= 2 && eval > history->eval[history->ply-2];\
  // RFP
  if (history->ply < 7 &&
      eval - 175 * history->ply / (1+improving) >= beta &&
      abs(beta) < 29001)
      return eval;
  // ----

move_loop:
  uint16_t move = stgm.nextMove();
  uint16_t bestMove = move;
  
  int moveCount = 0;
  bool root = history->ply == 0;
  while (move != 0){
    // if (root)
    //   Serial.print("Searching ");
    board.makeMove(move);
    if (stgm.inCheck()){
      board.unmakeMove();
      move = stgm.nextMove();
      continue;
    }
    moveCount++;
    node++;
    history->ply++;
    if (!pvNode || moveCount > 1)
      score = -alphabeta(board, -alpha-1, -alpha, depth-1, bm, &pvFromHere, history, node);

    if (pvNode && (score > alpha || moveCount == 1))
      score = -alphabeta(board, -beta, -alpha, depth-1, bm, &pvFromHere, history, node);
    history->ply--;
    board.unmakeMove();

    if (root){
      //Serial.print(board.moveToStr(move));
      Serial.print("Nodes Searched: ");
      Serial.println(node);
    }
    if (score > bestScore){
      bestScore = score;
      bestMove = move;

      if ((score > alpha && pvNode) || (root && moveCount == 1)){
        // Update PV
        pv->length = 1 + pvFromHere.length;
        pv->line[0] = move;
        memcpy(pv->line + 1, pvFromHere.line, sizeof(uint16_t) * pvFromHere.length);
      }

      if (root)
        bm = move;
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

uint16_t search(Board &b, int depth){
  uint16_t best = 0;
  PV pv;
  History hist;
  int32_t nodes = 0;
  int score = alphabeta(b, -INFINITE, INFINITE, depth, best, &pv, &hist, nodes);
  Serial.print("PV Line: ");
  for (int i=0;i<pv.length;i++){
    Serial.print(b.moveToStr(pv.line[i]));
  }
  Serial.println();
  Serial.print("Evaluation: ");
  Serial.println(score);
  //Serial.println(b.moveToStr(best));
  return best;
}
