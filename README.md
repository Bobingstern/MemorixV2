# MemorixV2
A C++ chess engine designed to run with 2kb of RAM
Very WIP as of right now. Development is exclusively on the Arduino UNO for the time being. You may use the Carriage Return mode on the Arduino IDE Serial Monitor after compiling the sketch and use the following commands
`position (fen | startpos) moves (algebraic notation)`

`go movetime (milliseconds)`

It is very slow (obviously) and not yet fully UCI compliant
I recommend sticking to a longer time for target hardware. Max depth right now for `qsearch` is up to 8. Heres an example

`position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6`

`go movetime 5000`
