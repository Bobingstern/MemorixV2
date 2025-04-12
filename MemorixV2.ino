#include "src/engine.hpp"

#define MAX_MESSAGE 128

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

void go(Board &board, char *str){
  int dep = 1;
  setLimit(str, "depth", &dep);
  search(board, dep);
}

void printUint16(uint16_t b){
  for (int i=15;i>=0;i--){
    Serial.print((int)( (b >> i) & 0x01  ));
  }
  Serial.println("");
}



Board board = Board();
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing MemorixV2");

  //board.parseFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
  //printBoard(board);
  // int startMicro = micros();
  // search(board, 2);
  // Serial.print("Time taken: ");
  // Serial.println( (micros() - startMicro) / 1000000.0f);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  static char buffer[MAX_MESSAGE];
  static unsigned char index = 0;
  char inch;
  
  while (Serial.available() > 0) {
    inch = Serial.read();
    if (inch == '\r') {
      // ----
      switch (hashInput(buffer)) {
            case GO         : go(board, buffer);  break;
            case UCI        :     break;
            case ISREADY    :      break;
            case POSITION   : board.uciPosition(buffer); printBoard(board); break;
      }

      //---
      buffer[0] = 0;
      index = 0;
    } else {        
      if (index < MAX_MESSAGE-1) {
        buffer[index++] = inch;
        buffer[index] = 0;
      }
    }
  }

}
