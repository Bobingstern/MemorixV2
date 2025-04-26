# MemorixV2
A C++ chess engine designed to run with 2kb of RAM
Very WIP as of right now. Testing is exclusively on the Arduino UNO for the time being. You may use the Carriage Return mode on the Arduino IDE Serial Monitor after compiling the sketch. Remember to comment out the `#define DEV` in `main.cpp` and use the following commands
`position (fen | startpos) moves (algebraic notation)`

`go movetime (milliseconds)`

`go wtime (ms) btime (ms)`

It is very slow (obviously) on target hardware and supports very basic UCI features. Currently it uses up 95% of the program space on the arduino uno, I am still working on a way to track RAM usage.
I recommend sticking to a longer ms time for target hardware. Max depth right now for `qsearch` is up to 4. Heres an example

`position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6`

`go movetime 5000`

How strong is it? So far it's been tested against Stash-V9 at 5+0.05 time control on standard hardware
```
Score of MemorixV2 vs stash-9.0: 965 - 101 - 257  [0.827] 1323
...      MemorixV2 playing White: 492 - 48 - 122  [0.835] 662
...      MemorixV2 playing Black: 473 - 53 - 135  [0.818] 661
...      White vs Black: 545 - 521 - 257  [0.509] 1323
Elo difference: 271.2 +/- 20.1, LOS: 100.0 %, DrawRatio: 19.4 %
```
