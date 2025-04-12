# MemorixV2
A C++ chess engine designed to run with 2kb of RAM
Very WIP as of right now. Development is exclusively on the Arduino UNO for the time being. You may use the Carriage Return mode on the Arduino IDE Serial Monitor after compiling the sketch and use the following commands
`position (fen | startpos) moves (algebraic notation)`

`go depth (depth)`

It is very slow (obviously) and not yet UCI compliant
I recommend sticking to a depth of 2 or 3. Max depth right now for `qsearch` is up to 5. Heres an example

`position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6`

`go depth 2`

Output:
```
r . . . k . . r 
p . p p q p b . 
B n . . p n p . 
. . . P N . . . 
. p . . P . . . 
. . N . . Q . p 
P P P B . P P P 
R . . . K . . R 

Nodes Searched: 461
Nodes Searched: 478
Nodes Searched: 495
Nodes Searched: 515
Nodes Searched: 532
Nodes Searched: 552
Nodes Searched: 774
Nodes Searched: 793
Nodes Searched: 1117
Nodes Searched: 1408
Nodes Searched: 1436
Nodes Searched: 1456
Nodes Searched: 1473
Nodes Searched: 1496
Nodes Searched: 1512
Nodes Searched: 1529
Nodes Searched: 1549
Nodes Searched: 1564
Nodes Searched: 1580
Nodes Searched: 1596
Nodes Searched: 1612
Nodes Searched: 1629
Nodes Searched: 1646
Nodes Searched: 1663
Nodes Searched: 1685
Nodes Searched: 1707
Nodes Searched: 1723
Nodes Searched: 1740
Nodes Searched: 1850
Nodes Searched: 1871
Nodes Searched: 2801
Nodes Searched: 3037
Nodes Searched: 3057
Nodes Searched: 3108
Nodes Searched: 3122
Nodes Searched: 3134
PV Line: b4c3 d2c3 
Evaluation: -245
```
