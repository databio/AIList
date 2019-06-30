#!/usr/bin/env python

import ailist as aiL
from timeit import default_timer as timer
import sys

def main(argv):
	if len(argv) < 3:
		print("Usage: bedcov.py <loaded.bed> <streamed.bed>")
		sys.exit(1)

	pmode = 0
	if len(argv)>3 and (argv[3]=="-P" or argv[3]=="-p"):
		pmode = 1
	ail = aiL.ailist()	
	start = timer()
	with open(argv[1]) as fp:
		for line in fp:
			t = line[:-1].split("\t")
			ail.add(t[0], int(t[1]), int(t[2]))
	sys.stderr.write("Read in {} sec\n".format(timer() - start))
	start = timer()
	ail.construct()
	sys.stderr.write("Construct in {} sec\n".format(timer() - start))
	start = timer()
	total = 0
	with open(argv[2]) as fp:
		for line in fp:
			t = line[:-1].split("\t")		
			st1, en1, cnt = int(t[1]), int(t[2]), 0
			n = ail.query(t[0], st1, en1)
			total += n
			if pmode==0:
				print("{}\t{}\t{}\t{}".format(t[0], st1, en1, n))
	sys.stderr.write("Query in {} sec\n".format(timer() - start))
	print("Total: ", total, "\n")
	
if __name__ == "__main__":
	main(sys.argv)
        
