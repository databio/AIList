#!/usr/bin/env python
import ailist as aiL
from timeit import default_timer as timer
import sys
import numpy as np
import pandas as pd

def main(argv):
    if len(argv) < 3:
        print("Usage: ailist.py <loaded.bed> <streamed.bed>")
        sys.exit(1)
    pmode = 0
    if len(argv)>3 and (argv[3]=="-P" or argv[3]=="-p"):
        pmode = 1
        
    #------------------------------------------------------------------
    #1. Load interval set 1
    ail = aiL.ailist()	
    start = timer()
    ail.addAll(argv[1])
    sys.stderr.write("Read in {} sec\n".format(timer() - start))
 
    #------------------------------------------------------------------
    #2. Construct
    start = timer()
    ail.construct()
    sys.stderr.write("Construct in {} sec\n".format(timer() - start))
    
    #------------------------------------------------------------------
    #3.1 Search all: load csv
    start = timer()
    r = pd.read_csv(argv[2], header=None, sep="\t")
    nq = len(r.index)  
    sys.stderr.write("Load qs in {} sec\n".format(timer() - start))      
    start = timer()
    hits = np.zeros(nq, dtype='uint32')
    total = ail.queryAll(argv[2], nq, hits)
    if pmode == 1:
        for i in range(nq):
            print("{}\t{}\t{}\t{}".format(r[0][i], r[1][i], r[2][i], hits[i]))   
    print("Nq, Total: ", nq, ", ", total, "\n")               
    sys.stderr.write("Search 1 in {} sec\n".format(timer() - start))
    
    #------------------------------------------------------------------
    #3.2 Direct search
    '''start = timer()
    total = 0	
    with open(argv[2]) as fp:
        for line in fp:
            t = line[:-1].split("\t")		
            st1, en1 = int(t[1]), int(t[2])
            n = ail.query(t[0], st1, en1)
            total += n
            if pmode==1:
                print("{}\t{}\t{}\t{}".format(t[0], st1, en1, n))
    sys.stderr.write("Query in {} sec\n".format(timer() - start))
    print("Total: ", total, "\n")'''
	
if __name__ == "__main__":
	main(sys.argv)
        
