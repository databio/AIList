//=============================================================================
//Read .BED datasets, and then find all overlaps 
//by Jianglin Feng  09/05/2018
//Decomposition & simplication: 11/26/2018
//Radix sorting and one-pass loading based on lh3's cgranges: 6/20/2019
//-----------------------------------------------------------------------------
#include "AIList.h"
#define PROGRAM_NAME  "ailist"
#define MAJOR_VERSION "0"
#define MINOR_VERSION "1"
#define REVISION_VERSION "1"
#define BUILD_VERSION "0"
#define VERSION MAJOR_VERSION "." MINOR_VERSION "." REVISION_VERSION

int ailist_help(int argc, char **argv, int exit_code);

int main(int argc, char **argv)
{
    int i, j, cLen = 20, pmode = 0;//print mode: 0 print hitd[i]; 1: print total
    if(argc == 5 && strcmp(argv[3], "-L")==0)
        cLen = atoi(argv[4]);
    else if(argc == 5 && strcmp(argv[3], "-P")==0)
        pmode = atoi(argv[4]);
    else if(argc!=3)
        return ailist_help(argc, argv, 0);   
        
   	clock_t start, end1, end2, end3;  
    start = clock(); 
    //1. Read interval data    	     
    ailist_t *ail =  readBED(argv[2]);
    end1 = clock(); 
    
    //2. Construct ailist 
    ailist_construct(ail, 20);
    end2 = clock();  
    
    //3. Search	
    FILE* fd = fopen(argv[1], "r");
	char *s1, *s2, *s3;
	char buf[1024];
	int64_t nol = 0;
	uint32_t nhits=0, mr=0;
	uint32_t *hits=0; 
    while (fgets(buf, 1024, fd)) {
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");       
		nhits = ailist_query(ail, s1, atol(s2), atol(s3), &mr, &hits);
		nol += nhits;
	}	
	end3 = clock();
	
    printf("Total %lld\n", (long long)nol); 
    printf("loading time: %f\n", ((double)(end1-start))/CLOCKS_PER_SEC);       
    printf("constru time: %f\n", ((double)(end2-end1))/CLOCKS_PER_SEC);       
    printf("query time: %f\n", ((double)(end3-end2))/CLOCKS_PER_SEC);           
    ailist_destroy(ail);    
	free(hits);
	fclose(fd);
    return 0;
}

int ailist_help(int argc, char **argv, int exit_code)
{
    fprintf(stderr,"%s, v%s\n" "usage:   %s query-file(.bed) database-file(.bed) [-L coverage-length] \n",
            PROGRAM_NAME, VERSION, PROGRAM_NAME);
    return exit_code;
}

