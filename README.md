# MemorixV2
A C++ chess engine designed to run with 2kb of RAM
Very WIP as of right now. Testing is exclusively on the Arduino UNO for the time being. You may use the Carriage Return mode on the Arduino IDE Serial Monitor after compiling the sketch. Remember to comment out the `#define DEV` in `main.cpp` and use the following commands
`position (fen | startpos) moves (algebraic notation)`

`go movetime (milliseconds)`

`go wtime (ms) btime (ms)`

`go depth (depth)`

It is very slow (obviously) on target hardware and supports very basic UCI features. Currently it uses up 95% of the program space on the arduino uno, I am still working on a way to track RAM usage. You can use `#define SEARCHINFO` to log what is being searched as a sort of loading bar (defined by default).
I recommend sticking to a longer ms time for target hardware (around 20s maybe). Max depth right now for `qsearch` is up to 4 for the microcontroller. Heres an example

`position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6`

`go movetime 25000`

How strong is it? So far it's been tested against Stash-V10 (1620 ccrl elo?) at 8+0.08 time control on standard hardware
```
Score of MemorixV2 vs stash-10.0: 2077 - 1521 - 1402  [0.556] 5000
...      MemorixV2 playing White: 1077 - 696 - 727  [0.576] 2500
...      MemorixV2 playing Black: 1000 - 825 - 675  [0.535] 2500
...      White vs Black: 1902 - 1696 - 1402  [0.521] 5000
Elo difference: 38.8 +/- 8.2, LOS: 100.0 %, DrawRatio: 28.0 %
```
