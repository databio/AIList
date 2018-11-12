#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "interval_tree.h"
#include "interval_tree_generic.h"

unsigned long gstart[25] = {0, 15940, 31520, 44280, 56520, 68190, 79180, 89440, 
        98810, 107670, 116280, 124990, 133570, 140870, 147710, 154220, 160050,
        165420, 170580, 174400, 178550, 181530, 184770, 194650, 198160};

static struct rb_root root = RB_ROOT;

static inline unsigned long
search(unsigned long start, unsigned long end, struct rb_root *root)
{
	struct interval_tree_node *node;
	unsigned long results = 0;

	for (node = interval_tree_iter_first(root, start, end); node;
	     node = interval_tree_iter_next(node, start, end))
		results++;
	return results;
}

static void print_nodes(unsigned long start, unsigned long end)
{
	struct interval_tree_node *n = interval_tree_iter_first(&root,
								start, end);

	printf("Tree nodes:");
	while (n) {
		printf(" (%lu, %lu)", n->start, n->last);
		n = interval_tree_iter_next(n, start, end);
	}
	printf("\n");
}

#define LINE_LEN	1024
int main(int argc, char **argv)
{
    if(argc!=3){
        printf("input: data file, query file \n");
        return 0;		    
    }
    clock_t start1, end1, end2;
    start1 = clock();     
	int i, ichr;
	char *qfile = argv[1];
	char *dfile = argv[2];
	char *s1, *s2, *s3;
	FILE *fp;
	char line[LINE_LEN];
    unsigned long start, end;
    for(i=0;i<25;i++){
        gstart[i]*=16384;
    }

	fp = fopen(dfile, "r");
	if (fp == NULL) {
        printf("File %s not found!\n", dfile);
        return 0;
	}

    while (fgets(line, LINE_LEN, fp)) {
        struct interval_tree_node *n;
        n = calloc(1, sizeof(*n));
        s1 = strtok(line, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");
        ichr = -1;       
        if(strlen(s1)>5 || strlen(s1)<4)
            ichr = -1;  
        else if(strcmp(s1, "chrX")==0)
            ichr = 22;
        else if(strcmp(s1, "chrY")==0)
            ichr = 23;
        else if(strcmp(s1, "chrM")==0)
            ichr = -1;            
        else{
            ichr = (int)(atoi(&s1[3])-1);
        }          
        if(ichr >= 0){         
            n->start  = atol(s2) + gstart[ichr];
            n->last   = atol(s3) + gstart[ichr];         
            interval_tree_insert(n, &root);
        }       
    }  
    fclose(fp);
    end1 = clock();      
    //print_nodes(0, ULONG_MAX);
    printf("Tree-building time: %f \n", ((double)(end1-start1))/CLOCKS_PER_SEC);
      
    fp = fopen(qfile, "r");
    if (fp == NULL) {
        printf("File %s not found!\n", dfile);
        return 0;
    }
    unsigned long Total = 0;
    while (fgets(line, LINE_LEN, fp)) {
        s1 = strtok(line, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");
        if(strlen(s1)>5 || strlen(s1)<4)
            ichr = -1;  
        else if(strcmp(s1, "chrX")==0)
            ichr = 22;
        else if(strcmp(s1, "chrY")==0)
            ichr = 23;
        else if(strcmp(s1, "chrM")==0)
            ichr = -1;            
        else{
            ichr = (int)(atoi(&s1[3])-1);
        }           
        if(ichr>=0){
            start = atol(s2) + gstart[ichr];			
            end   = atol(s3) + gstart[ichr];				
            Total += search(start, end, &root);
        }
    }
    fclose(fp);
    end2 = clock();
    printf("Total: %lld\n", (long long)Total);
    printf("Searching time: %f \n", ((double)(end2-end1))/CLOCKS_PER_SEC);      
    return 0;
}
