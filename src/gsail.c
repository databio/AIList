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
    int cLen = 20, pmode = 0;//print mode: 0 print hitd[i]; 1: print total; 2: print components
    for(int i=3; i<argc; i++){
    	if(strcmp(argv[i], "-L")==0 && i+1<argc)
    		cLen = atoi(argv[i+1]);
    	else if(strcmp(argv[i], "-P")==0 && i+1<argc)
    		pmode = atoi(argv[i+1]);
    }
	if(argc<3)
        return ailist_help(argc, argv, 0);   
        
   	clock_t start, end1, end2, end3;  
    start = clock(); 
    
    //1. Read interval data    	     
    ailist_t *ail =  readBED(argv[1]);
    end1 = clock(); 
    printf("loading time: %f\n", ((double)(end1-start))/CLOCKS_PER_SEC); 
        
    //2. Construct ailist 
    ailist_construct(ail, cLen);
    if(pmode==2){
		for(int i=0;i<ail->nctg;i++){
			ctg_t *p = &ail->ctg[i];
			printf("%s\tnr= %lld, nc=%i\n", p->name, (long long)p->nr, p->nc);
			for(int j=0;j<p->nc;j++)
				printf("	%i\t%i  \n", p->idxC[j], p->lenC[j]);
		}
    }
    end2 = clock();  
    printf("constru time: %f\n", ((double)(end2-end1))/CLOCKS_PER_SEC); 
        
    //3. Search	
	int64_t nol = 0;
	uint32_t nhits=0, mr=1000000;
	uint32_t *hits=malloc(mr*sizeof(uint32_t));
	
	kstream_t *ks;
	kstring_t str = {0,0,0};
	gzFile fp = gzopen(argv[2], "r");
	assert(fp);
	ks = ks_init(fp);
	while (ks_getuntil(ks, KS_SEP_LINE, &str, 0) >= 0) {
		int32_t st1, en1;
		char *ctg;
		ctg = parse_bed(str.s, &st1, &en1);
		if (ctg == 0) continue;	      
		nhits = ailist_query(ail, ctg, st1, en1, &mr, &hits);
		if(pmode==0 && nhits>0)
			printf("%s:\t %i\t %i\t %ld\n", ctg, st1, en1, (long)nhits);
		nol += nhits;
	}	
	end3 = clock();
    printf("Total %lld\n", (long long)nol); 
    printf("query time: %f\n", ((double)(end3-end2))/CLOCKS_PER_SEC);           
  
  	free(str.s);
	free(hits);
	gzclose(fp); 
	ks_destroy(ks);   
	ailist_destroy(ail);  
    return 0;
}

int ailist_help(int argc, char **argv, int exit_code)
{
    fprintf(stderr,"%s, v%s\n" "usage:   %s database-file(.bed) query-file(.bed) [-L coverage-length] \n",
            PROGRAM_NAME, VERSION, PROGRAM_NAME);
    return exit_code;
}

