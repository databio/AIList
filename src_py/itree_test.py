#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Oct 23 13:00:12 2018

@author: john
"""
import sys
import os
import time
from intervaltree import IntervalTree

gstart = [0, 15940, 31520, 44280, 56520, 68190, 79180, 89440, 
        98810, 107670, 116280, 124990, 133570, 140870, 147710, 154220, 160050,
        165420, 170580, 174400, 178550, 181530, 184770, 194650, 198160]
for i in range(0, 25):
    gstart[i] = 16384*gstart[i]

#print(gstart)
    
if len(sys.argv)<3:
    print("Need two .bed files")
    sys.exit(os.EX_OK)

qfile = sys.argv[1]
dfile = sys.argv[2]
t1 = time.time()
tr = IntervalTree()
with open(dfile) as f:
    for line in f:
        L = line.strip().split()
        if len(L[0]) < 6 and L[0][3] != 'M':
            if L[0][3] == 'X':
                ichr = 22
            elif L[0][3] == 'Y':
                ichr = 23
            else:
                ichr = int(L[0][3:])-1
            #print(ichr)
            start = int(L[1])+gstart[ichr]
            end = int(L[2])+gstart[ichr]
            tr.addi(start, end, 100)
f.close()  
t2 = time.time()
print("Tree build time: ", t2-t1)      
#print(tr.items())
Total = 0
with open(qfile) as f:
    for line in f:
        L = line.strip().split()  
        if len(L[0]) < 6 and L[0][3] != 'M':
            if L[0][3] == 'X':
                ichr = 22
            elif L[0][3] == 'Y':
                ichr = 23
            else:
                ichr = int(L[0][3:])-1
            start = int(L[1])+gstart[ichr]
            end = int(L[2])+gstart[ichr]
            ols = tr.search(start, end)
            if len(ols)>0:
                Total += len(ols)
                #print(start, ", ", end-1, ":")
                #print(ols, "\n")  
f.close()      
print("Total: ", Total)
t3 = time.time()
print("Tree search time: ", t3-t2)

        
