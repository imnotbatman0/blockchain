# Blockchain Simulator

Compile code using:

```bash
g++ -std=c++17 -m64 -o simulator simulator.cpp
```

If you get a segmentation fault, it may be because your compiler is restricting memory on your process. This is a heavy simulation and needs lots of memory.

To fix it, allocate high memory by running:

```bash
ulimit -s unlimited
ulimit -v unlimited
```

(works in Linux and Mac)

This allocates unlimited virtual memory and stack to your current compiler.

After execution, you can see a BlockChain Tree for 10 peers in the output.

Here is how it looks for two peers:

```
--- Blockchain Tree for Peer 8 ---
Blk-0 (Genesis)
└──Blk-170 (Depth: 1)
    └──Blk-271 (Depth: 2)
        └──Blk-338 (Depth: 3)
            └──Blk-416 (Depth: 4)
                └──Blk-448 (Depth: 5)
                    └──Blk-543 (Depth: 6)
                        ├──Blk-581 (Depth: 7)
                        └──Blk-630 (Depth: 7)
                            ├──Blk-710 (Depth: 8)
                            └──Blk-723 (Depth: 8)
                                └──Blk-811 (Depth: 9)
                                    └──Blk-893 (Depth: 10)
                                        └──Blk-966 (Depth: 11)
                                            └──Blk-1004 (Depth: 12)
                                                └──Blk-1088 (Depth: 13)
                                                    └──Blk-1137 (Depth: 14)
                                                        └──Blk-1199 (Depth: 15)
                                                            └──Blk-1269 (Depth: 16)
                                                                └──Blk-1350 (Depth: 17)
                                                                    └──Blk-1421 (Depth: 18)
                                                                        └──Blk-1521 (Depth: 19)
                                                                            └──Blk-1573 (Depth: 20)
------------------------------------------

--- Blockchain Tree for Peer 9 ---
Blk-0 (Genesis)
└──Blk-170 (Depth: 1)
    └──Blk-271 (Depth: 2)
        └──Blk-338 (Depth: 3)
            └──Blk-416 (Depth: 4)
                └──Blk-448 (Depth: 5)
                    └──Blk-543 (Depth: 6)
                        └──Blk-630 (Depth: 7)
                            └──Blk-723 (Depth: 8)
                                └──Blk-811 (Depth: 9)
                                    ├──Blk-840 (Depth: 10)
                                    └──Blk-893 (Depth: 10)
                                        └──Blk-966 (Depth: 11)
                                            └──Blk-1004 (Depth: 12)
                                                └──Blk-1088 (Depth: 13)
                                                    └──Blk-1137 (Depth: 14)
                                                        └──Blk-1199 (Depth: 15)
                                                            └──Blk-1269 (Depth: 16)
                                                                └──Blk-1350 (Depth: 17)
                                                                    └──Blk-1421 (Depth: 18)
                                                                        └──Blk-1521 (Depth: 19)
                                                                            └──Blk-1573 (Depth: 20)
------------------------------------------
```
