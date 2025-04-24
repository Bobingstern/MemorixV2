# MemorixV2
A C++ chess engine designed to run with 2kb of RAM
Very WIP as of right now. Testing is exclusively on the Arduino UNO for the time being. You may use the Carriage Return mode on the Arduino IDE Serial Monitor after compiling the sketch. Remember to comment out the `#define DEV` in `main.cpp` and use the following commands
`position (fen | startpos) moves (algebraic notation)`

`go movetime (milliseconds)`

`go wtime (ms) wtime (ms)`

It is very slow (obviously) on target hardware and supports very basic UCI features. Currently it uses up 95% of the program space on the arduino uno, I am still working on a way to track RAM usage.
I recommend sticking to a longer ms time for target hardware. Max depth right now for `qsearch` is up to 4. Heres an example

`position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6`

`go movetime 5000`
