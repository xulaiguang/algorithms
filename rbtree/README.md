# Conventional Red-Black tree implemented in C

`generate_random_sequence.py` generates a sequence of random numbers
and writes them to `random_sequence.txt`.

Example
```
$ ./generate_random_sequence.py 6
generating 6 numbers
$ ./generate_random_sequence.py ^C
$ gcc -g main.c util.c rbtree.c
$ ./a.out
inserting 4
 4
inserting 1
 ┌ 4
 1*
inserting 5
 ┌ 4 ┐
 1*  5*
inserting 2
   ┌── 4 ──┐
   1 ┐     5
     2*
inserting 3
   ┌── 4 ──┐
 ┌ 2 ┐     5
 1*  3*
inserting 0
       ┌────── 4 ──────┐
   ┌── 2*──┐           5
 ┌ 1       3
 0*
0 1 2 3 4 5
       ┌────── 4 ──────┐
   ┌── 2*──┐           5
 ┌ 1       3
 0*
erasing 4
   ┌── 3 ──┐
 ┌ 1*┐     5
 0   2
erasing 1
   ┌── 3 ──┐
   0 ┐     5
     2*
erasing 5
 ┌ 2 ┐
 0   3
erasing 2
   0 ┐
     3*
erasing 3
 0
erasing 0
$
```
