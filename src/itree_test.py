#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Oct 23 13:00:12 2018

@author: john
"""
import sys
import numpy as np
from intervaltree import Interval, IntervalTree

if len(sys.argv)<3:
    print("Need two .bed files")
    exit

qfile = sys.argv[1]
dfile = sys.argv[2]


with open(dfile)as f:
    for line in f:
        L = line.strip().split()
        # ... do something with L