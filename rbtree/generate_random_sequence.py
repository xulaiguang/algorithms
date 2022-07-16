#!/usr/bin/env python3
#coding=utf-8

import random, sys

num = 20
if len(sys.argv) == 2:
    num = int(sys.argv[1])

print("generating %d numbers" % num)

f = open("random_sequence.txt", 'w')
numbers = random.sample(range(0, num), num)
for n in numbers:
    f.write(str(n) + '\n')

f.close()

