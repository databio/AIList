//=============================================================================
//Read .bed datasets, and then find all overlaps 
//by Jianglin Feng  09/05/2018
//-----------------------------------------------------------------------------
#include "AIList.h"
#include <sys/stat.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
uint32_t gstart[25] = {0, 15940, 31520, 44280, 56520, 68190, 79180, 89440, 
        98810, 107670, 116280, 124990, 133570, 140870, 147710, 154220, 160050,
        165420, 170580, 174400, 178550, 181530, 184770, 194650, 198160};

int compare_uint32(const void *a, const void *b) {
    uint32_t pa = *(uint32_t*)a;
    uint32_t pb = *(uint32_t*)b;
    if(pa>pb)
        return 1;
    else if(pa<pb)
        return -1;
    else
        return 0;
}

int compare_rend(const void *a, const void *b)
{
    struct g_data *pa = (struct g_data *) a;
    struct g_data *pb = (struct g_data *) b;
    if(pa->r_end > pb->r_end)
        return 1;
    else if(pa->r_end < pb->r_end)
        return -1;
    else
        return 0;
    //return pa->r_end - pb->r_end;
}

int compare_rend8(const void *a, const void *b)
{
    struct g_data8 *pa = (struct g_data8 *) a;
    struct g_data8 *pb = (struct g_data8 *) b;
    if(pa->r_end > pb->r_end)
        return 1;
    else if(pa->r_end < pb->r_end)
        return -1;
    else
        return 0;
    //return pa->r_end - pb->r_end;
}

int compare_rendx(const void *a, const void *b)
{
    struct g_datax *pa = (struct g_datax *) a;
    struct g_datax *pb = (struct g_datax *) b;
    if(pa->r_end > pb->r_end)
        return 1;
    else if(pa->r_end < pb->r_end)
        return -1;
    else
        return 0;
    //return pa->r_end - pb->r_end;
}

int compare_rend6(const void *a, const void *b)
{
    struct g_data6 *pa = (struct g_data6 *) a;
    struct g_data6 *pb = (struct g_data6 *) b;
    if(pa->r_end > pb->r_end)
        return 1;
    else if(pa->r_end < pb->r_end)
        return -1;
    else
        return 0;
    //return pa->r_end - pb->r_end;
}

int compare_rstart(const void *a, const void *b)
{
    struct g_data *pa = (struct g_data *) a;
    struct g_data *pb = (struct g_data *) b;
    if(pa->r_start > pb->r_start)
        return 1;
    else if(pa->r_start < pb->r_start)
        return -1;
    else
        return 0;    
    //return pa->r_start - pb->r_start;
}

char** str_split_t( char* str, int nItems)
{
    char **splits;
    char *tmp;
    int i;
    if (str == NULL)
        return NULL;
    else {    
        splits = malloc((nItems+1)*sizeof(*splits)); 
        i=0;
        tmp = strtok(str, "\t");
        while(tmp!=NULL && i<nItems){
            splits[i] = tmp;
            tmp = strtok(NULL, "\t");
            i++;
        }
    }
    //printf("%s %s %s \n", splits[0], splits[1], splits[2]);
    return splits;
}

void str_splits( char* str, int *nmax, char **splits)
{   //tsv 
    splits[*nmax] = NULL;
    splits[0] = str;
    char *ch = str;    
    int ns = 1;    
    do {
        if (*ch == '\t'){
            splits[ns++] = &ch[1];
            *ch = '\0';
        }
        ch++;
    } while (*ch != '\0' && ns < *nmax+1);
    *nmax = ns;
}

char** str_split( char* str, char delim, int *nmax)
{   //slightly faster than _t
    char** splits;
    char* ch;    
    int ns;
    if (str == NULL || delim=='\0')
        return NULL;
    else {
        splits = malloc((*nmax+1) * sizeof(*splits));
        splits[*nmax] = NULL;
        ch = str;
        ns = 1;
        splits[0] = str;
        do {
            if (*ch == delim)
            {
                splits[ns++] = &ch[1];
                *ch = '\0';
            }
            ch++;
        } while (*ch != '\0' && ns < *nmax+1);
    }
    *nmax = ns;
    return splits;
}

//-------------------------------------------------------------------------------------
int bSearch1(uint32_t* As, int tc, uint32_t qe)
{   //tc: length of As; find the index of the leftmost item with ts > qe
    //number of intersects=tE-tS
    int tL, tR, tM, tE;    //index  
    tL=0;   tR=tc-1;  
    tE = tc; //not found  
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(As[tM] > qe)
            tR = tM;
        else
            tL = tM + 1;
    }
    if(As[tL] > qe)
        tE = tL;
    else if(As[tR] > qe)
        tE = tR;
    return tE;   //tE-1: index of the last overalpping item; first exclusion item
} 

int bSearch2(uint32_t* Ae, int tc, uint32_t qs)
{   //tc: length of Ae; find the index of the rightmost item with te < qs
    int tL, tR, tM, tS;    //index  
    tL=0;   tR=tc-1;  
    tS = -1;   //not found
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(Ae[tM]<qs)
            tL = tM;
        else
            tR = tM - 1;
    }
    if(Ae[tR]<qs)
        tS = tR;
    else if(Ae[tL]<qs)
        tS = tL;
    return tS+1;   //number of exclusions: index of the starting item
}

int bSearch3(struct g_data* As, int tc, uint32_t qe)
{   //tc: length of As; find the index of the leftmost item with ts > qe
    //number of intersects=tE-tS
    int tL, tR, tM, tE;    //index  
    tL=0;   tR=tc-1;  
    tE = tc; //not found  
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(As[tM].r_start > qe)
            tR = tM;
        else
            tL = tM + 1;
    }
    if(As[tL].r_start > qe)
        tE = tL;
    else if(As[tR].r_start > qe)
        tE = tR;       
    return tE;   //tE-1: index of the last overalpping item; first exclusion item
} 

//assume .end not inclusive
int bSearch3a(struct g_data* As, int tc, uint32_t qe)
{   //tc: length of As; find the index of the first .s<qe start from right
    //number of intersects=tE-tS
    int tL=0, tR=tc-1, tM, tE=-1;    //index  
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(As[tM].r_start >= qe)
            tR = tM-1;
        else
            tL = tM;
    }
    if(As[tR].r_start < qe)
        tE = tR;
    else if(As[tL].r_start < qe)
        tE = tL;       
    return tE;   //tE: index of the first item satisfying .s<qe from right
} 

int bSearch8(struct g_data8* Ae, int tc, uint32_t qs)
{   //tc: length of Ae; find the index of the rightmost item with te < qs
    int tL, tR, tM, tS;    //index  
    tL=0;   tR=tc-1;  
    tS = -1;   //not found
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(Ae[tM].r_end<qs)
            tL = tM;
        else
            tR = tM - 1;
    }
    if(Ae[tR].r_end<qs)
        tS = tR;
    else if(Ae[tL].r_end<qs)
        tS = tL;
    return tS+1;   //number of exclusions: index of the starting item
}

int bSearchx(struct g_datax* Ae, int tc, uint32_t qs)
{   //tc: length of Ae; find the index of the rightmost item with te < qs
    int tL, tR, tM, tS;    //index  
    tL=0;   tR=tc-1;  
    tS = -1;   //not found
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(Ae[tM].r_end<qs)
            tL = tM;
        else
            tR = tM - 1;
    }
    if(Ae[tR].r_end<qs)
        tS = tR;
    else if(Ae[tL].r_end<qs)
        tS = tL;
    return tS+1;   //number of exclusions: index of the starting item
}

int bSearch6(struct g_data6* Ae, int tc, uint32_t qs)
{   //tc: length of Ae; find the index of the rightmost item with te < qs
    int tL, tR, tM, tS;    //index  
    tL=0;   tR=tc-1;  
    tS = -1;   //not found
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(Ae[tM].r_end<qs)
            tL = tM;
        else
            tR = tM - 1;
    }
    if(Ae[tR].r_end<qs)
        tS = tR;
    else if(Ae[tL].r_end<qs)
        tS = tL;
    return tS+1;   //number of exclusions: index of the starting item
}

int bSearch4(struct g_data* Ae, int tc, uint32_t qs)
{   //tc: length of Ae; find the index of the rightmost item with te < qs
    int tL, tR, tM, tS;    //index  
    tL=0;   tR=tc-1;  
    tS = -1;   //not found
    while(tL<tR-1){
        tM = (tL+tR)/2; 
        if(Ae[tM].r_end<qs)
            tL = tM;
        else
            tR = tM - 1;
    }
    if(Ae[tR].r_end<qs)
        tS = tR;
    else if(Ae[tL].r_end<qs)
        tS = tL;
    return tS+1;   //number of exclusions: index of the starting item
}

int iCount(uint32_t* Ts, uint32_t* Te, int tc, uint32_t qs, uint32_t qe)
{
    int tS = bSearch2(Te, tc, qs);
    int tE = bSearch1(Ts, tc, qe);
    //printf("qs qe %u %u, tS tE %i %i\n", qs, qe, tS, tE);
    return tE-tS;
}

int cCount(uint32_t* As, struct g_data* Ge, int tc, uint32_t qs, uint32_t qe)
{
    int tS = bSearch4(Ge, tc, qs);
    int tE = bSearch1(As, tc, qe);
    //printf("qs qe %u %u, tS tE %i %i\n", qs, qe, tS, tE);    
    return tE-tS;
}

int cEnumerate(uint32_t* As, struct g_data* Ge, int tc, uint32_t qs, uint32_t qe, int* idx)
{
    int tS = bSearch4(Ge, tc, qs);
    int tE = bSearch1(As, tc, qe);
    int t = tE-tS;
    //printf("qs qe %u %u, tS tE %i %i\n\n", qs, qe, tS, tE);     
    int hits = t;
    if(t>0)
        idx = malloc(t*sizeof(uint32_t));
    else
        idx = malloc(1*sizeof(uint32_t));
    int j = tS;
    while(t>0){
        if(Ge[j].r_start<=qe){        
            t--;
            idx[t]=j;
        }
        j++;
    } 
    return hits;
}

void iCounter(struct g_data* A, int nA, struct g_data* B, int nB)
{
    printf("get counts only:\n");
    clock_t start1, end1, start2, end2;
    start1 = clock();
    uint32_t* Bs = malloc(nB*sizeof(uint32_t));
    uint32_t* Be = malloc(nB*sizeof(uint32_t));  
    int i, tS, tE;
    for(i=0;i<nB;i++){
        Bs[i] = B[i].r_start;
        Be[i] = B[i].r_end;
    }  
    qsort(Bs, nB, sizeof(uint32_t), compare_uint32);  
    qsort(Be, nB, sizeof(uint32_t), compare_uint32); 
    end1 = clock();
    //-------------------------------------------------------------------------
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0;       
    for(i=0;i<nA;i++){ 
        tS = bSearch2(Be, nB, A[i].r_start);
        tE = bSearch1(Bs, nB, A[i].r_end);      
        Hits[i] = tE-tS;
    }
    for(i=0; i<nA; i++){
        if(Hits[i]>0){
            //printf("%i %u \n", i, Hits[i]);
            Total += Hits[i];
        }
    }
    end2 = clock();
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);     
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);          
    printf("Total:%lld\n", (long long)Total);
    free(Hits);  
    free(Bs);
    free(Be);   
}

void searchAIList(struct g_data* A, int nA, struct g_data* B, int nB)
{   //---pass dynamically allocated array back to caller
    printf("AIList saerch:\n");
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rend);
    uint32_t* aiList = malloc(nB*sizeof(uint32_t));  
    int i, j, t, tS, tE;
    uint32_t tt=B[nB-1].r_start;
    for(i=nB-1; i>=0; i--){
        if(B[i].r_start<tt)
            tt = B[i].r_start;
        aiList[i] = tt;  
    }
    end1 = clock();
    //-------------------------------------------------------------------------
    printf("nA, nB: %i %i\n", nA, nB);
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe;        
    for(i=0;i<nA;i++){
        qs = A[i].r_start;
        qe = A[i].r_end;
        tS = bSearch4(B, nB, qs);                 
        t = tS;
        while(t<nB && aiList[t]<=qe){
            if(B[t].r_start<=qe){
                //Ols[Total] = t;
                //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                Total++;
                Hits[i]++;
            }    
            t++;
        }
    }
    end2 = clock();
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);     
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);      
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
    free(Hits); 
    free(aiList);      
}

void searchAIListDB(struct g_data* A, int nA, struct g_data* B, int nB)
{   //---pass dynamically allocated array back to caller
    printf("AIListDB:\n");
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rend);
    uint32_t* aiList = malloc(nB*sizeof(uint32_t));  
    int i, j, t, tS, tE;
    uint32_t tt=B[nB-1].r_start;
    for(i=nB-1; i>=0; i--){
        if(B[i].r_start<tt)
            tt = B[i].r_start;
        aiList[i] = tt;  
    }
    end1 = clock();
    //-------------------------------------------------------------------------
    printf("nA, nB: %i %i\n", nA, nB);
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe;        
    for(i=0;i<nA;i++){
        qs = A[i].r_start;
        qe = A[i].r_end;
        tS = bSearch4(B, nB, qs);//first item to check   
        tE = bSearch1(aiList, nB, qe); //tE-1: the last item to check                     
        for(t=tS; t<tE; t++){
            if(B[t].r_start<=qe){
                //Ols[Total] = t;
                Total++;
                Hits[i]++;
            }    
        }
    }
    end2 = clock();
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);     
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);     
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
    free(Hits); 
    free(aiList);      
}

// A utility function to create a new Interval Search Tree Node 
struct ITNode * newNode(struct g_data g) 
{ 
    struct ITNode *temp = malloc(1*sizeof(struct ITNode)); 
    temp->i = malloc(1*sizeof(struct g_data));
    temp->i->r_start = g.r_start;
    temp->i->r_end = g.r_end;
    //temp->i = &g;//(struct g_data)g;//Not working!!! 
    temp->max = g.r_end; 
    temp->left = NULL;
    temp->right = NULL;     
    return temp;
}; 

// A utility function to insert a new Interval Search Tree Node 
// This is similar to BST Insert.  Here the low value of interval 
// is used to maintain BST property 
struct ITNode *insert(struct ITNode *root, struct g_data g) 
{ 
    // Base case: Tree is empty, new node becomes root 
    if (root == NULL) 
        return newNode(g); 
    //else
    //    printf("i: %u %u %u\n", root->i->r_start, root->i->r_end, root->max); // pointer !!!          
 
    uint32_t l = root->i->r_start; 
    
    if (g.r_start < l) 
        root->left = insert(root->left, g); 
    else
        root->right = insert(root->right, g); 
  
    // Update the max value of this ancestor if needed 
    if (root->max < g.r_end) 
        root->max = g.r_end;        
    
    return root; 
}

// Standard tree
void overlapSearch(struct ITNode *tmp, struct g_data iv, uint64_t* total) 
{     
    if(tmp==NULL)
    	return;   
    struct g_data g = *(tmp->i);
    if(g.r_start<=iv.r_end && g.r_end>=iv.r_start)
        *total = *total + 1;

    // If left child of root is present and max of left child is
    // greater than or equal to given interval, then i may
    // overlap with an interval is left subtree
    if (tmp->left != NULL && tmp->left->max >= iv.r_start)
       overlapSearch(tmp->left, iv, total); 
    // Else interval can only overlap with right subtree
    if (tmp->right != NULL && g.r_start <= iv.r_end)       
        overlapSearch(tmp->right, iv, total);
}

void inorder(struct ITNode *root) 
{ 
    if (root == NULL) return; 
  
    inorder(root->left); 
    
    printf("[%u, %u, %u]\n", root->i->r_start, root->i->r_end, root->max);
  
    inorder(root->right); 
}

struct ITNode* buildAITree(struct ITNode* root, struct g_data* B, uint32_t start, uint32_t end)
{
    if(end-start > 1)
    {
        uint32_t mid = (start+end)/2; 
        root = insert(root, B[mid]);
        buildAITree(root, B, start, mid);
        buildAITree(root, B, mid+1, end);        
    }
    else if(end-start==1)
    {      
        root = insert(root, B[start]);
    }
    return root;
}

void searchAITree(struct g_data* A, int nA, struct g_data* B, int nB)
{   //---pass dynamically allocated array back to caller
    printf("AITree: nA, nB: %u %u\n", nA, nB);
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rstart);  
    uint32_t i;

    struct ITNode *root = NULL; 
    root = buildAITree(root, B, 0, nB);             
    end1 = clock();
    //inorder(root); 
    uint64_t Total=0;
    printf("Build tree time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);        
    for(i=0; i<nA; i++){
        overlapSearch(root, A[i], &Total);
    }
    end2 = clock();
    printf("Search time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);  
    printf("Total: %lld\n", (long long)Total);      
}

void bEnumerator(struct g_data* A, int nA, struct g_data* B, int nB)
{   //---pass dynamically allocated array back to caller
    printf("Dual binary: \n");
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rend);
    uint32_t* Bs = malloc(nB*sizeof(uint32_t));  
    int i, j, t, tS, tE;
    for(i=0;i<nB;i++)
        Bs[i] = B[i].r_start;  
    qsort(Bs, nB, sizeof(uint32_t), compare_uint32);
    end1 = clock();
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA, nB)*10*sizeof(*Ols));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0; 
    uint32_t qs, qe;        
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        tS = bSearch4(B, nB, qs);
        tE = bSearch1(Bs, nB, qe);
        t = tE-tS;             
        Hits[i] = t;
        if(t>0){
            j = tS;
            while(t>0){
                if(B[j].r_start<=qe){        
                    t--;
                    ///Ols[Total]=j;
                    Total++;                 
                }
                j++;
            }
        }       
    }
    end2 = clock();
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);     
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);    
    printf("Total:%lld\n", (long long)Total);
    //free(Ols);
    free(Hits); 
    free(Bs);      
}

/*
void AIListSearch(struct g_data* A, int nA, struct g_data* B, int nB)
{   //take Flips out--split B into two parts: B1--nearly normal, b2 abnormal
    clock_t start1, end1, start2, end2;
    start1 = clock();     
    int i, i1, j, t, tS, tE;
    qsort(B, nB, sizeof(struct g_data), compare_rstart);  
    struct g_data* Be = malloc(nB*sizeof(struct g_data));
    for(i=0;i<nB;i++){
        Be[i].r_end = B[i].r_end;
        Be[i].r_start = i;// store index!
    }
    qsort(Be, nB, sizeof(struct g_data), compare_rend);       
    
    uint32_t* maxE = malloc(nB*sizeof(uint32_t));  
    uint32_t* nF = calloc(nB, sizeof(uint32_t));    //flips
    uint32_t** Flips = malloc(nB*sizeof(uint32_t*));
    uint32_t* fTmp = malloc(nB*sizeof(uint32_t));
    uint32_t gmax, g, g1, gs;

    //1. Construct Flip list
    //  find all pre ends > current end && pre end > current start
    gmax = B[0].r_end;
    maxE[0] = gmax;
    for(i=1;i<nB;i++){        
        g = B[i].r_end;
        if(g>gmax){ //no Flips
            gmax = g;
            maxE[i] = gmax;
        }
        else{
            maxE[i] = gmax;
            //if(g<=B[i-1].r_end){
                gs = B[i].r_start;
                i1 = i-1;
                while(i1>=0){// && maxE[i1]>=gs){
                    if(B[i1].r_end >= g){
                        fTmp[nF[i]]=i1;
                        nF[i]++;
                    }
                    i1--;
                }
            //}
            //else{
            //    nF[i] = nF[i-1];
            //}
        }
         
        if(nF[i]>0){
            Flips[i] = malloc(nF[i]*sizeof(uint32_t));
            memcpy(Flips[i], fTmp, nF[i]);
        } 
    }
    free(fTmp);  
    free(maxE); 
    end1 = clock();
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe;  
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        tE = bSearch3(B, nB, qe);                    
        Hits[i] = 0;
        t = tE-1;
        
        //printf("\n i, tE-1: %i, %i \n", i, t);
        while(t>=0 && B[t].r_end >= qs){
            //Ols[Total] = t;
            //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
            Total++;
            Hits[i]++;
            t--;
        }
        if(t>=0 && nF[t]>0){
            for(j=0; j<nF[t]; j++){
                if(B[Flips[t][j]].r_end>=qs){
                    Total++;
                    Hits[i]++;
                }
            }
        }
    }
    end2 = clock();
    printf("Time: %f, %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC,((double)(end2-end1))/CLOCKS_PER_SEC);   
    printf("Total, extra:%lld %lld\n", (long long)Total, (long long)nEx);       
    //printf("Total, extra:%lld %lld\n", (long long)Total, (long long)(nEx-Total));
    // free(Ols);
    for(i=0;i<nB;i++){
        if(nF[i]>0)
            free(Flips[i]);
    }   
    free(Flips);
    free(Hits); 
    free(nF); 
    //free(Be);     
}*/
/*
    struct g_data* Be = malloc(nB*sizeof(struct g_data));
    for(i=0;i<nB;i++){
        Be[i].r_end = B[i].r_end;
        Be[i].r_start = i;// store index!
    }
    qsort(Be, nB, sizeof(struct g_data), compare_rend); 
    int nB1=0, nB2=0;
    struct g_data* B1 = malloc(nB*sizeof(struct g_data));       
    struct g_data* B2 = malloc(nB*sizeof(struct g_data));
    for(i=nB-1;i>=0;i--){
        j = nB -1;
        i1 = Be[i].r_start;
        if(i-i1>n0){
            B2[nB2] = B[i1];
            nB2++;
        }
        else{
            B1[nB1] = B[i1];
            nB1++;        
        }
    }
*/

void AIListSearch(struct g_data* A, int nA, struct g_data* B, int nB)
{   //split B into two part: n0=20 is the best
    //nB>n0
    clock_t start1, end1, start2, end2;
    start1 = clock();      
    int n0=20, dist, i, i1, j, t, tS, tE;
    qsort(B, nB, sizeof(struct g_data), compare_rstart);
    int nB1=0, nB2=0;
    struct g_data* B1 = malloc(nB*sizeof(struct g_data));       
    struct g_data* B2 = malloc(nB*sizeof(struct g_data));
    //-------------------------------------------------------------------------
    uint32_t tt;
    if(nB<=n0){
        printf("Data size too small < 20\n");
        return;
    }
    for(i=0;i<nB-n0;i++){
        tt = B[i].r_end;
        j=1;
        while(j<n0 && B[j+i].r_end<tt)
            j++;
        if(j==n0){
            B2[nB2] = B[i];
            nB2++;
        }
        else{
            B1[nB1] = B[i];
            nB1++;
        } 
    }    
    for(i=nB-n0;i<nB;i++){
        B1[nB1] = B[i];
        nB1++;
    }  
    
    printf("nA, nB1, nB2: %i %i %i\n", nA, nB1, nB2);
    //-------------------------------------------------------------------------  
    uint32_t* aiList1 = malloc(nB1*sizeof(uint32_t));   
    uint32_t* aiList2 = malloc((1+nB2)*sizeof(uint32_t));       
    tt=B1[0].r_end;
    aiList1[0] = tt;
    for(i=1; i<nB1; i++){
        if(B1[i].r_end > tt)
            tt = B1[i].r_end;
        aiList1[i] = tt;  
    } 
    if(nB2>16){ 
        tt=B2[0].r_end;
        aiList2[0] = tt;
        for(i=1; i<nB2; i++){
            if(B2[i].r_end > tt)
                tt = B2[i].r_end;
            aiList2[i] = tt;  
        }  
    }    
    end1 = clock();    
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);    
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    //uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    int qhits;
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe, qs0, qe0; 
    if(nB2>16){       
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                    
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            tE = bSearch3(B2, nB2, qe);                 
            t = tE-1;
            while(t>=0 && aiList2[t]>=qs){
                if(B2[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end); 
                    qhits++; 
                    //Total++;
                    //Hits[i]++;
                }    
                t--;
            }  
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);    
            Total += qhits;      
        }
    }
    else{
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);  
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            for(j=0;j<nB2;j++){
                if(qs<=B2[j].r_end && qe>=B2[j].r_start){
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                      
                    //Total++;
                    //Hits[i]++;
                    qhits++;
                }
            } 
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);  
            Total += qhits;                          
        }    
    }
    end2 = clock();
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);      
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
    //    free(Hits); 
    free(B1);
    free(B2);
    free(aiList1);  
    free(aiList2);
}

void SearchSortedESplit(struct g_data* A, int nA, struct g_data* B, int nB)
{   //split B into two part: n0=20 is the best
    clock_t start1, end1, start2, end2;
    start1 = clock();      
    int n0=20, dist, i, i1, j, t, tS, tE;
    qsort(B, nB, sizeof(struct g_data), compare_rstart);
    int nB1=0, nB2=0;
    struct g_data* B1 = malloc(nB*sizeof(struct g_data));       
    struct g_data* B2 = malloc(nB*sizeof(struct g_data));
    //-------------------------------------------------------------------------
    uint32_t tt;
    if(nB<=n0){
        B1=B;
        nB1=nB;
    }
    else{
        for(i=0;i<nB-n0;i++){
            tt = B[i].r_end;
            j=1;
            while(j<n0 && B[j+i].r_end<tt)
                j++;
            if(j==n0){
                B2[nB2] = B[i];
                nB2++;
            }
            else{
                B1[nB1] = B[i];
                nB1++;
            } 
        }    
        for(i=nB-n0;i<nB;i++){
            B1[nB1] = B[i];
            nB1++;
        } 
    }  
    
    printf("nA, nB1, nB2: %i %i %i\n", nA, nB1, nB2);
    //-------------------------------------------------------------------------  
    uint32_t* aiList1 = malloc(nB1*sizeof(uint32_t));   
    uint32_t* aiList2 = malloc((1+nB2)*sizeof(uint32_t));       
    tt=B1[0].r_end;
    aiList1[0] = tt;
    for(i=1; i<nB1; i++){
        if(B1[i].r_end > tt)
            tt = B1[i].r_end;
        aiList1[i] = tt;  
    } 
    if(nB2>16){ 
        tt=B2[0].r_end;
        aiList2[0] = tt;
        for(i=1; i<nB2; i++){
            if(B2[i].r_end > tt)
                tt = B2[i].r_end;
            aiList2[i] = tt;  
        }  
    }    
    end1 = clock();    
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);    
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    //uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    int qhits;
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe, qs0, qe0; 
    if(nB2>16){       
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                    
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            tE = bSearch3(B2, nB2, qe);                 
            t = tE-1;
            while(t>=0 && aiList2[t]>=qs){
                if(B2[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end); 
                    qhits++; 
                    //Total++;
                    //Hits[i]++;
                }    
                t--;
            }  
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);    
            Total += qhits;      
        }
    }
    else{
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);  
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            for(j=0;j<nB2;j++){
                if(qs<=B2[j].r_end && qe>=B2[j].r_start){
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                      
                    //Total++;
                    //Hits[i]++;
                    qhits++;
                }
            } 
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);  
            Total += qhits;                          
        }    
    }
    end2 = clock();
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);      
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
//    free(Hits); 
    free(B1);
    free(B2);
    free(aiList1);  
    free(aiList2);
}

//2. test Optimizing query list A
void AIListSearchQ(struct g_data* A, int nA, struct g_data* B, int nB)
{   //split B into two part: n0=20 is the best
    clock_t start1, end1, start2, end2;
    start1 = clock();      
    int n0=20, dist, i, i1, j, t, tS, tE, tS0, tE0;
    qsort(B, nB, sizeof(struct g_data), compare_rstart);
    qsort(A, nA, sizeof(struct g_data), compare_rstart);    
    int nB1=0, nB2=0;
    struct g_data* B1 = malloc(nB*sizeof(struct g_data));       
    struct g_data* B2 = malloc(nB*sizeof(struct g_data));
    //-------------------------------------------------------------------------
    uint32_t tt;
    if(nB<=n0){
        B1=B;
        nB1=nB;
    }
    else{
        for(i=0;i<nB-n0;i++){
            tt = B[i].r_end;
            j=1;
            while(j<n0 && B[j+i].r_end<tt)
                j++;
            if(j==n0){
                B2[nB2] = B[i];
                nB2++;
            }
            else{
                B1[nB1] = B[i];
                nB1++;
            } 
        }    
        for(i=nB-n0;i<nB;i++){
            B1[nB1] = B[i];
            nB1++;
        } 
    }  
    
    printf("nA, nB1, nB2: %i %i %i\n", nA, nB1, nB2);
    //-------------------------------------------------------------------------  
    uint32_t* aiList1 = malloc(nB1*sizeof(uint32_t));   
    uint32_t* aiList2 = malloc((1+nB2)*sizeof(uint32_t));       
    tt=B1[0].r_end;
    aiList1[0] = tt;
    for(i=1; i<nB1; i++){
        if(B1[i].r_end > tt)
            tt = B1[i].r_end;
        aiList1[i] = tt;  
    } 
    if(nB2>16){ 
        tt=B2[0].r_end;
        aiList2[0] = tt;
        for(i=1; i<nB2; i++){
            if(B2[i].r_end > tt)
                tt = B2[i].r_end;
            aiList2[i] = tt;  
        }  
    }    
    end1 = clock();    
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);    
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    //uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    int qhits;
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe, qs0, qe0; 
    if(nB2>16){       
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                    
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            tE = bSearch3(B2, nB2, qe);                 
            t = tE-1;
            while(t>=0 && aiList2[t]>=qs){
                if(B2[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end); 
                    qhits++; 
                    //Total++;
                    //Hits[i]++;
                }    
                t--;
            }  
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);    
            Total += qhits;      
        }
    }
    else{
        for(i=0;i<nA;i++){
            qhits=0;
            qs = A[i].r_start;
            qe = A[i].r_end;
            tE = bSearch3(B1, nB1, qe);                 
            t = tE-1;
            while(t>=0 && aiList1[t]>=qs){
                if(B1[t].r_end>=qs){
                    //Ols[Total] = t;
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);  
                    //Total++;
                    qhits++;
                    //Hits[i]++;
                }    
                t--;
            }
            for(j=0;j<nB2;j++){
                if(qs<=B2[j].r_end && qe>=B2[j].r_start){
                    //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
                    //printf("%i: %u, %u \n", i, B[t].r_start, B[t].r_end);                      
                    //Total++;
                    //Hits[i]++;
                    qhits++;
                }
            } 
            //printf("%i\t%u\t%u\t%i\n", i, rs, re, qhits);  
            Total += qhits;                          
        }    
    }
    end2 = clock();
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);      
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
//    free(Hits); 
    free(B1);
    free(B2);
    free(aiList1);  
    free(aiList2);
}

void AIListIntersect(char* fQuery, struct g_data** B, int* nB)
{   //split B into two parts: n0=20 is the best
    //bed end not inclusive
    clock_t start1, end1, start2, end2;
    start1 = clock();      
    int n0=20, dist, i, i1, j, t, tS, tE; 
    uint32_t tt;  
    if(nB<=n0){
        printf("Data size too small < 20\n");
        return;
    }
    struct g_data** B1 = malloc(24*sizeof(struct g_data*)); 
    struct g_data** B2 = malloc(24*sizeof(struct g_data*)); 
    uint32_t** aiList1 = malloc(24*sizeof(uint32_t*));   
    uint32_t** aiList2 = malloc(24*sizeof(uint32_t*));       
    int *nB1 = calloc(24, sizeof(int));
    int *nB2 = calloc(24, sizeof(int));   
    for(i=0;i<24;i++){
        qsort(B[i], nB[i], sizeof(struct g_data), compare_rstart);                
        B1[i] = malloc(nB[i]*sizeof(struct g_data));
        B2[i] = malloc(MAX(nB[i]/4, n0)*sizeof(struct g_data));    //nB2<nB/4  
        for(t=0; t<nB[i]-n0; t++){
            tt = B[i][t].r_end;
            j=1;
            while(j<n0 && B[i][j+t].r_end<tt)
                j++;
            if(j==n0){
                B2[i][nB2[i]] = B[i][t];
                nB2[i]++;
            }
            else{
                B1[i][nB1[i]] = B[i][t];
                nB1[i]++;
            } 
        }    
        for(t=nB[i]-n0; t<nB[i]; t++){
            B1[i][nB1[i]] = B[i][t];
            nB1[i]++;
        }  
        //---------------------------------------------------------------------
        aiList1[i] = malloc(nB1[i]*sizeof(uint32_t));   
        aiList2[i] = malloc((1+nB2[i])*sizeof(uint32_t));       
        tt=B1[i][0].r_end;
        aiList1[i][0] = tt;
        for(t=1; t<nB1[i]; t++){
            if(B1[i][t].r_end > tt)
                tt = B1[i][t].r_end;
            aiList1[i][t] = tt;  
        } 
        if(nB2[i]>16){ 
            tt=B2[i][0].r_end;
            aiList2[i][0] = tt;
            for(t=1; t<nB2[i]; t++){
                if(B2[i][t].r_end > tt)
                    tt = B2[i][t].r_end;
                aiList2[i][t] = tt;  
            }  
        }         

        //if(nB[i]>0)
        //    free(B[i]);        
    } 
    printf("nB1, nB2: %i %i\n", nB1[0], nB2[0]);
    //-------------------------------------------------------------------------    
    end1 = clock();    
    printf("Constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);    
    //-------------------------------------------------------------------------
    int bufsize = 1024;   
    int* OlsC = malloc(bufsize*sizeof(int));
    int* OlsI = malloc(bufsize*sizeof(int));
    char buf[1024], s10[128];
    FILE* fd = fopen(fQuery, "r");
    int ichr=-1, lens, ichr0=-1;
    uint32_t qs, qe, rs, re, gs, qhits;
    uint64_t Total=0;
    char *s1, *s2, *s3;   
    strcpy(s10, fQuery);   
    while (fgets(buf, 1024, fd)) {
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");  
        if(strcmp(s1, s10)==0){
            ichr = ichr0;
        } 
        else{
            lens = strlen(s1);   
            if(lens > 5 || lens < 4)
                ichr = -1;  
            else if(strcmp(s1, "chrX")==0)
                ichr = 22;
            else if(strcmp(s1, "chrY")==0)
                ichr = 23;     
            else if (strcmp(s1, "chrM")==0)
                ichr = -1;    
            else{
                ichr = (int)(atoi(&s1[3])-1);
            }
            ichr0 = ichr;
            strcpy(s10, s1); 
        }         
        if(ichr>=0){
            qhits = 0;
            qs = atol(s2);
            qe = atol(s3);   
            //-----------------------------------------------------------------
            tE = bSearch3a(B1[ichr], nB1[ichr], qe);                 
            t = tE;
            while(t>=0 && aiList1[ichr][t]>qs){
                if(B1[ichr][t].r_end>qs){
                    if(qhits<bufsize){
                        OlsC[qhits] = ichr;
                        OlsI[qhits] = t;                
                    }
                    qhits++;
                }    
                t--;
            }
            if(nB2[ichr]>16){
                tE = bSearch3a(B2[ichr], nB2[ichr], qe);                 
                t = tE;
                while(t>=0 && aiList2[ichr][t]>qs){
                    if(B2[ichr][t].r_end>qs){
                        if(qhits<bufsize){
                            OlsC[qhits] = ichr;
                            OlsI[qhits] = t;                
                        }                  
                        qhits++;
                    }    
                    t--;
                }
            }
            else if(nB2[ichr]>0){
                for(j=0;j<nB2[ichr];j++){
                    if(qs<B2[ichr][j].r_end && qe>B2[ichr][j].r_start){
                        if(qhits<bufsize){
                            OlsC[qhits] = ichr;
                            OlsI[qhits] = t;                
                        }
                        qhits++;
                    }
                }
            } 
            if(qhits>0)
                printf("%s\t%u\t%u\t%i\n", s1, qs, qe, qhits);   
            Total += qhits;                   
        }   
    }    
    fclose(fd);       
    end2 = clock();
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);      
    printf("Total:%lld\n", (long long)Total);    
    //printf("Total, extra:%lld %lld \n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
    //for(i=0;i<24;i++){
        //free(B1[i]);
        //if(B2[i]!=NULL)
        //    free(B2[i]);
        //if(aiList1[i]!=NULL)
        //    free(aiList1[i]);
        //if(aiList2[i]!=NULL)
        //    free(aiList2[i]);
    //}
    free(OlsC);
    free(OlsI);
    free(nB1);
    free(nB2);
    free(B1);
    free(B2);
    free(aiList1); 
    free(aiList2);
}

/*
void AIListSearch(struct g_data* A, int nA, struct g_data* B, int nB)
{   //find indeixe of all prev with .end >= current.end
    clock_t start1, end1, start2, end2;
    start1 = clock();    
    
    int i, i1, j, t, tS, tE;
    qsort(B, nB, sizeof(struct g_data), compare_rstart);
    struct g_data* Be = malloc(nB*sizeof(struct g_data));
    for(i=0;i<nB;i++){
        Be[i].r_end = B[i].r_end;
        Be[i].r_start = i;// store index!
    }
    qsort(Be, nB, sizeof(struct g_data), compare_rend);    
    uint32_t* nF = calloc(nB, sizeof(uint32_t));    //flips
    uint32_t** Flips = malloc(nB*sizeof(uint32_t*));
    uint32_t* fTmp = malloc(nB*sizeof(uint32_t));
    uint32_t bE, min_end;

    //1. Construct Flip list
    //  Sort by end
    //  find all pre ends > current end
    for(i=0;i<nB;i++){
        i1 = Be[i].r_start;        
        t=i+1;
        while(t<nB && nF[i1]<=i1){
            j = Be[t].r_start;
            if(j<i1){//index
                fTmp[nF[i1]] = j;
                nF[i1]++;
            }
            t++;
        }
         
        if(nF[i1]>0){
            Flips[i1] = malloc(nF[i1]*sizeof(uint32_t));
            memcpy(Flips[i1], fTmp, nF[i1]);
        } 
    }
    free(fTmp);   
    end1 = clock();
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe;  
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        tE = bSearch3(B, nB, qe);                    
        Hits[i] = 0;
        t = tE-1;
        
        //printf("\n i, tE-1: %i, %i \n", i, t);
        while(t>=0 && B[t].r_end >= qs){
            //Ols[Total] = t;
            //printf("%i: %u, %u ; %u, %u \n", i, qs, qe, B[t].r_start, B[t].r_end);
            Total++;
            Hits[i]++;
            t--;
        }
        if(t>=0 && nF[t]>0){
            j = 0;//sorted
            while(j<nF[t] && B[Flips[t][j]].r_end>=qs){
                Total++;
                Hits[i]++;
                j++;
            }
        }
    }
    end2 = clock();
    printf("Time: %f, %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC,((double)(end2-end1))/CLOCKS_PER_SEC);   
    printf("Total, extra:%lld %lld\n", (long long)Total, (long long)nEx);       
    //printf("Total, extra:%lld %lld\n", (long long)Total, (long long)(nEx-Total));
    // free(Ols);
    for(i=0;i<nB;i++){
        if(nF[i]>0)
            free(Flips[i]);
    }   
    free(Flips);
    free(Hits); 
    free(nF); 
    free(Be);     
}*/

void cEnumerator(struct g_data* A, int nA, struct g_data* B, int nB)
{   //---pass dynamically allocated array back to caller
    //SortedE
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rend);
    uint32_t* Bs = malloc(nB*sizeof(uint32_t));  
    int i, j, t, tS, tE;
    for(i=0;i<nB;i++)
        Bs[i] = B[i].r_start;  
    qsort(Bs, nB, sizeof(uint32_t), compare_uint32);
    end1 = clock();
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*10*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0, nEx=0; 
    uint32_t qs, qe;  
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        tS = bSearch4(B, nB, qs); //using simplified binary search              
        Hits[i] = 0;
        t = tS;
        j = 0;  //number of hits not found
        while(t<nB && Bs[t]<=qe){
            j++;            
            if(B[t].r_start<=qe){
                //Ols[Total] = t;
                Total++;
                Hits[i]++;
                j--;
            }
            else
                nEx++; 
            t++;
        }
        while(j>0){           
            if(B[t].r_start<=qe){
                //Ols[Total] = t;
                Total++;
                Hits[i]++;
                j--;
            }
            else
                nEx++; 
            t++;
        }
        /*while(t<nB && (Bs[t]<=qe || j>0)){
            if(Bs[t]<=qe)
                j++;            
            if(B[t].r_start<=qe){
                Ols[Total] = t;
                Total++;
                Hits[i]++;
                j--;
            } 
            t++;
        }*/
    }
    end2 = clock();
    printf("Time: %f, %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC,((double)(end2-end1))/CLOCKS_PER_SEC);   
    printf("Total, extra:%lld %lld\n", (long long)Total, (long long)nEx);       
    //printf("Total, extra:%lld %lld\n", (long long)Total, (long long)(nEx-Total));
   // free(Ols);
    free(Hits); 
    free(Bs);      
}

/*
void cEnumerator(struct g_data* A, int nA, struct g_data* B, int nB)
{   //A complete solution: generally best????
    printf("cEnumerator: \n");
    clock_t start1, end1, start2, end2;
    start1 = clock();
    qsort(B, nB, sizeof(struct g_data), compare_rend);
    struct g_data* Bs = malloc(nB*sizeof(struct g_data));  
    int i, j, t, tS, tE;
    for(i=0;i<nB;i++){
        Bs[i].r_start = B[i].r_start;
        Bs[i].r_end = i;//index!
    }
    //Bs = B; //memcpy?
    qsort(Bs, nB, sizeof(struct g_data), compare_rstart);
    end1 = clock();
    //-------------------------------------------------------------------------
    //uint32_t* Ols = malloc(MAX(nA,nB)*6*sizeof(uint32_t));  
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0; 
    uint32_t qs, qe;  
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        tS = bSearch4(B, nB, qs); //using simplified binary search              
        Hits[i] = 0;
        t = tS;
        if(Bs[t].r_start<=qe){
            //Ols[Total] = t;
            Total++;
            Hits[i]++;
            t++;        
            while(t<nB && Bs[t].r_start<=qe){       
                //Ols[Total] = Bs[t].r_end;
                Total++;
                Hits[i]++; 
                t++;
            }
        }
    }
    end2 = clock();
    printf("AIList: constructing time: %f\n", ((double)(end1-start1))/CLOCKS_PER_SEC);     
    printf("Searching time: %f\n",((double)(end2-end1))/CLOCKS_PER_SEC);  
    printf("Total:%lld\n", (long long)Total);       
    //printf("Total, extra:%lld %lld\n", (long long)Total, (long long)(nEx-Total));
    //free(Ols);
    free(Hits); 
    free(Bs);      
}*/

void dCounter(struct g_data* A, int nA, struct g_data* B, int nB)
{    //direct searching
    printf("Brute force: \n");
    uint32_t* Hits = calloc(nA, sizeof(uint32_t));
    uint64_t Total=0;
    uint32_t qs, qe;  
    int i, j, t;      
    for(i=0;i<nA;i++){ 
        qs = A[i].r_start;
        qe = A[i].r_end;
        int t=0;
        for(j=0;j<nB;j++){
            if(B[j].r_end>=qs && B[j].r_start<=qe){
                t++;
            }
        }         
        Hits[i] = t;
    }
    //-------------------------------------------------------------------------
    for(i=0; i<nA; i++){
        if(Hits[i]>0){
            //printf("%i %u: ", i, Hits[i]);
            //printf("\n");
            Total += Hits[i];
        }
    }
    printf("Total:%lld\n", (long long)Total);
    free(Hits);     
}

/*void funcM2M(char* ffData)
{   //dual binary; file folder; cross correlation of all data files: 

}*/

void func6(char* fQuery, char* iPath)
{   //for rme_orig: chromatin state
    //each Region is a gemomic object: an actor--interactive element
    //1. Get the files  
    glob_t gResult;
    int rtn = glob(iPath, 0, NULL, &gResult);     
    if(rtn!=0){
        printf("wrong dir path: %s", iPath);
        return;
    }
    char** file_ids = gResult.gl_pathv;
    uint32_t n_files = gResult.gl_pathc; 
    if(n_files<2){   
        printf("Too few files (add to path /*): %u\n", n_files); 
        return;
    }
    
    //2. Read qData
    int i, j, k, nQ, nD;
    struct g_data* pQ = openBed(fQuery, &nQ);
    uint32_t** Ols = malloc(n_files*sizeof(*Ols)); 
    for(i=0;i<n_files;i++)
        Ols[i] = calloc(15, sizeof(uint32_t));  
           
    //3. Read region data
    struct g_data6* pD = NULL;         
    uint32_t* Bs; 
    int t, tS, tE;         
    uint64_t Total=0, nT = 0, extra = 0; 
    uint32_t qs, qe; 
    //Assume data file < memsize
    
    //Timing each part
    clock_t start1, end1, start2, end2, start3, end3, d1=0, d2=0, d3=0;

    //Timing 3: total

    for(i=0;i<n_files;i++){    
        start3 = clock();  
        pD = openBed6(file_ids[i], &nD); 
        end3 = clock();
        d3 += end3-start3;  
        //printf("%u %u %i \n", pD[0].r_start, pD[0].r_end, pD[0].r_item);       
        //printf("%i %s %i %i\n", i, file_ids[i], nD, nQ); 
        //---------------------------------------------------------------------
        start2 = clock();
        qsort(pD, nD, sizeof(struct g_data6), compare_rend6);                
        Bs = malloc(nD*sizeof(uint32_t)); 
        for(j=0;j<nD;j++)
            Bs[j] = pD[j].r_start;  
        qsort(Bs, nD, sizeof(uint32_t), compare_uint32);
        end2 = clock();
        d2 += end2-start2;
        //---------------------------------------------------------------------
        start1 = clock();             
        for(j=0;j<nQ;j++){ 
            qs = pQ[j].r_start;
            qe = pQ[j].r_end;
            tS = bSearch6(pD, nD, qs);
            tE = bSearch1(Bs, nD, qe); 
            t = tE-tS;    
            if(t>0){                
                Total += t;  
                k = tS;
                while(t>0){
                    if(pD[k].r_start<=qe){   
                        t--;     
                        Ols[i][pD[k].r_item]++;
                    }
                    k++;
                }             
                extra += k-tE;   
            }
        }   
        nT += nD;
        end1 = clock();
        d1 += end1-start1;     
        printf("%i: \t", i);
        for(j=0;j<15;j++)
            printf("%u \t", Ols[i][j]);
        printf("\n");
        free(Bs); 
        free(pD);
    }  
    end3 = clock();
    printf("Time for reading: %f \n", ((double)(d3))/CLOCKS_PER_SEC);      
    printf("Time for construction and qsort: %f \n", ((double)(d2))/CLOCKS_PER_SEC); 
    printf("Time for search: %f \n", ((double)(d1))/CLOCKS_PER_SEC);                        
    printf("Total:%lld, %lld, %lld \n", (long long)Total, (long long)nT, (long long)extra);       
    for(i=0; i<n_files; i++){
        free(Ols[i]);
    }
    free(Ols); 
    free(pQ);
    globfree(&gResult);     
}

void createDb(char* iPath, char* oPath)
{   //9/21/18: create a simple dB to store data elements: index of tissue, start, end, chr state  
    //for rme_orig data
    glob_t gResult;
    int rtn = glob(iPath, 0, NULL, &gResult);     
    if(rtn!=0){
        printf("wrong dir path: %s", iPath);
        return;
    }
    char** file_ids = gResult.gl_pathv;
    uint32_t n_files = gResult.gl_pathc; 
    if(n_files<2){   
        printf("Too few files (add to path /*): %u\n", n_files); 
        return;
    }
    
    char buf[1024], ctmp[10];
    int i, nD, nCols = 16, ichr;
    char **splits;
    FILE* fd; 
    uint32_t n, nT = 0;
    for(i=0; i<n_files; i++){
        nD = 0;
        fd = fopen(file_ids[i], "r"); 
        while(fgets(buf, 1024, fd)!=NULL)	
            nD++;
        fclose(fd);          
        nT += nD;
    }
    struct g_datax* pD = malloc(nT*sizeof(struct g_datax));      
    n=0;
    for(i=0; i<n_files; i++){
        fd = fopen(file_ids[i], "r"); 
        while(fgets(buf, 1024, fd)!=NULL){   	        
            splits = str_split(buf,'\t', &nCols); 
            if(strlen(splits[0])>5 || strlen(splits[0])<4 || strcmp(splits[0], "chrM")==0)
                ichr = -1;  
            else if(strcmp(splits[0], "chrX")==0)
                ichr = 22;
            else if(strcmp(splits[0], "chrY")==0)
                ichr = 23;
            else{
                ichr = (int)(atoi(&splits[0][3])-1);
            }           
            if(ichr>=0){
                pD[n].r_ctype = i;
                pD[n].r_start = (uint32_t)atoi(splits[1]) + gstart[ichr];
                pD[n].r_end = (uint32_t)atoi(splits[2]) + gstart[ichr];  
                if(splits[3][1]=='_'){     
                    strncpy(ctmp, splits[3], 1); 
                    ctmp[1]='\0';          
                    pD[n].r_state = atoi(ctmp)-1;
                }
                else{
                    strncpy(ctmp, splits[3], 2);
                    ctmp[2]='\0';
                    pD[n].r_state = atoi(ctmp)-1;
                }
                n++;
            }
            free(splits);       
        }	           
        fclose(fd);   
    }
    
    //sort
    qsort(pD, n, sizeof(struct g_datax), compare_rendx); 
    uint32_t* bS = malloc(n*sizeof(uint32_t));       
    for(i=0;i<n;i++)
        bS[i] = pD[i].r_start;        
    qsort(bS, n, sizeof(uint32_t), compare_uint32);    
    for(i=0;i<n;i++)
        pD[i].r_ss = bS[i];
            
    //save pD, n
    FILE *fp = fopen(oPath, "wb"); 
    if(fp==NULL)
        printf("Can't open file %s", oPath);       
    fwrite(&n_files, sizeof(uint32_t), 1, fp);
    fwrite(&n, sizeof(uint32_t), 1, fp);
    fwrite(pD, sizeof(struct g_datax), n, fp);    
    fclose(fp); 
       
    free(bS);
    free(pD);      
    globfree(&gResult);        
}

void funcx(char* fQuery, char* dbName)
{   //9/21/18: compare timing of two 
    int i, j, k, nQ, nD;
    uint32_t n, n_files;
    struct g_data* pQ = openBed(fQuery, &nQ);
    //Timing each part
    clock_t start1, end1, start2, end2, start3, end3;
    
    //1. Load data
    start3 = clock();     
    FILE* fi = fopen(dbName, "rb");
    if(!fi){
        printf("File not found, %s \n", dbName);
        return;
    }     
    fread(&n_files, sizeof(uint32_t), 1, fi);
    fread(&n, sizeof(uint32_t), 1, fi);
    printf("nFiles, nRegions %u %u\n", n_files, n);
    
    struct g_datax* pD = malloc(n*sizeof(struct g_datax));   
    fread(pD, sizeof(struct g_datax), n, fi);
    uint32_t *bS = malloc(n*sizeof(uint32_t));
    for(i=0;i<n;i++)
        bS[i] = pD[i].r_ss;
    for(i=0;i<100;i++)
        printf("%u %u %u %i %i\n", bS[i], pD[i].r_start, pD[i].r_end, pD[i].r_state, pD[i].r_ctype);
    uint32_t** Ols = malloc(n_files*sizeof(*Ols)); 
    for(i=0;i<n_files;i++)
        Ols[i] = calloc(15, sizeof(uint32_t));  
    end3 = clock();
    fclose(fi);
    printf("Time for reading: %f \n", ((double)(end3-start3))/CLOCKS_PER_SEC);    
                   
    //2. Search without enumeration      
    int t, tS, tE;         
    uint64_t Total=0; 
    uint32_t qs, qe; 
    /*start2 = clock();   
    Total = 0;          
    for(j=0;j<nQ;j++){ 
        qs = pQ[j].r_start;
        qe = pQ[j].r_end;
        tS = bSearchx(pD, n, qs);
        tE = bSearch1(bS, n, qe); 
        t = tE-tS;    
        Total += t;
    }   
    end2 = clock();      
    printf("Time for search without enumerating: %f \n", ((double)(end2-start2))/CLOCKS_PER_SEC);  
    printf("Total %lld\n", (long long)Total);*/ 
     
    //naive: full scan  
    /*start1 = clock();
    Total = 0;          
    for(j=0;j<nQ;j++){ 
        qs = pQ[j].r_start;
        qe = pQ[j].r_end;
        for(i=0;i<n;i++){
            if(pD[i].r_start<=qe && pD[i].r_end >= qs){
                Ols[pD[i].r_ctype][pD[i].r_state]++;
                Total ++;
            }
        }
    }   
    end1 = clock();      
    printf("Time for search with full-scan (worst case): %f \n", ((double)(end1-start1))/CLOCKS_PER_SEC);       
    printf("Total %lld\n", (long long)Total); 
          
    //memset(Ols, 0, 15*n_files*sizeof(uint32_t));
    for(j=0;j<n_files;j++){
        memset(Ols[j], 0, 15*sizeof(uint32_t));
    }  */ 
     
    start1 = clock();
    Total = 0; 
    uint64_t extra = 0;            
    for(j=0;j<nQ;j++){ 
        qs = pQ[j].r_start;
        qe = pQ[j].r_end;
        tS = bSearchx(pD, n, qs);
        tE = bSearch1(bS, n, qe); 
        t = tE-tS;    
        if(t>0){                
            Total += t;  
            k = tS;
            while(t>0){
                if(pD[k].r_start<=qe){   
                    t--;     
                    Ols[pD[k].r_ctype][pD[k].r_state]++;
                }
                k++;
            }             
            extra += k-tE;   
        }
    }   
    end1 = clock();      
    printf("Time for search with enumerating: %f \n", ((double)(end1-start1))/CLOCKS_PER_SEC);   
      
    for(i=0;i<n_files;i++){    
        printf("%i: \t", i);
        for(j=0;j<15;j++)
            printf("%u \t", Ols[i][j]);    
        printf("\n");
    }
    free(bS); 
    free(pD);      
    printf("Total:%lld, %lld, %lld \n", (long long)Total, (long long)n, (long long)extra);       
    for(i=0; i<n_files; i++){
        free(Ols[i]);
    }
    free(Ols); 
    free(pQ);     
}

void func8(char* fQuery, char* iPath)
{   //compare timing of two 
    //Mixed region sets: database; prepare data first?
    //for rme_orig: chromatin state
    //each Region is a gemomic object: an actor--interactive element
    //1. Get the files  
    glob_t gResult;
    int rtn = glob(iPath, 0, NULL, &gResult);     
    if(rtn!=0){
        printf("wrong dir path: %s", iPath);
        return;
    }
    char** file_ids = gResult.gl_pathv;
    uint32_t n_files = gResult.gl_pathc; 
    if(n_files<2){   
        printf("Too few files (add to path /*): %u\n", n_files); 
        return;
    }
    
    //2. Read qData
    int i, j, k, n, nQ, nD;
    struct g_data* pQ = openBed(fQuery, &nQ);
    uint32_t** Ols = malloc(n_files*sizeof(*Ols)); 
    for(i=0;i<n_files;i++)
        Ols[i] = calloc(15, sizeof(uint32_t));  
           
    //3. Read region data        
    int t, tS, tE;         
    uint64_t Total=0; 
    uint32_t qs, qe; 
    //Assume data file < memsize   
    //Timing each part
    clock_t start1, end1, start2, end2, start3, end3, start4, end4;
    
    start3 = clock();
    char buf[1024], ctmp[10];
    int nCols = 16, ichr;
    char **splits;
    FILE* fd; 
    int nT = 0, extra = 0;
    for(i=0; i<n_files; i++){
        nD = 0;
        fd = fopen(file_ids[i], "r"); 
        while(fgets(buf, 1024, fd)!=NULL)	
            nD++;
        fclose(fd);          
        nT += nD;
    }
    struct g_data8* pD = malloc(nT*sizeof(struct g_data8));  
    n=0;
    for(i=0; i<n_files; i++){
        fd = fopen(file_ids[i], "r"); 
        while(fgets(buf, 1024, fd)!=NULL){   	        
            splits = str_split(buf,'\t', &nCols); 
            if(strlen(splits[0])>5 || strlen(splits[0])<4)
                ichr = -1;  
            else if(strcmp(splits[0], "chrX")==0)
                ichr = 22;
            else if(strcmp(splits[0], "chrY")==0)
                ichr = 23;
            else{
                ichr = (int)(atoi(&splits[0][3])-1);
            }           
            if(ichr>=0){
                pD[n].r_ctype = i;
                pD[n].r_start = (uint32_t)atoi(splits[1]) + gstart[ichr];
                pD[n].r_end = (uint32_t)atoi(splits[2]) + gstart[ichr];  
                if(splits[3][1]=='_'){     
                    strncpy(ctmp, splits[3], 1); 
                    ctmp[1]='\0';          
                    pD[n].r_state = atoi(ctmp)-1;
                }
                else{
                    strncpy(ctmp, splits[3], 2);
                    ctmp[2]='\0';
                    pD[n].r_state = atoi(ctmp)-1;
                }
                n++;
            }
            free(splits);       
        }	           
        fclose(fd);   
    }
    end3 = clock();
    printf("Time for reading: %f \n", ((double)(end3-start3))/CLOCKS_PER_SEC);      
        
    //----------------------------
    start2 = clock();
    qsort(pD, n, sizeof(struct g_data8), compare_rend8);                
    uint32_t* Bs = malloc(n*sizeof(uint32_t)); 
    for(j=0;j<n;j++)
        Bs[j] = pD[j].r_start;  
    qsort(Bs, n, sizeof(uint32_t), compare_uint32);
    end2 = clock();        
    printf("Time for construction and qsort: %f \n", ((double)(end2-start2))/CLOCKS_PER_SEC);  
       
    //---------------------------- 
    start4 = clock();   
    Total = 0;          
    for(j=0;j<nQ;j++){ 
        qs = pQ[j].r_start;
        qe = pQ[j].r_end;
        tS = bSearch8(pD, n, qs);
        tE = bSearch1(Bs, n, qe); 
        t = tE-tS;    
        Total += t;
    }   
    end4 = clock();      
    printf("Time for search without enumerating: %f \n", ((double)(end4-start4))/CLOCKS_PER_SEC);  
    printf("Total %lld\n", (long long)Total);  
    start1 = clock();
    Total = 0; 
    extra = 0;            
    for(j=0;j<nQ;j++){ 
        qs = pQ[j].r_start;
        qe = pQ[j].r_end;
        tS = bSearch8(pD, n, qs);
        tE = bSearch1(Bs, n, qe); 
        t = tE-tS;    
        if(t>0){                
            Total += t;  
            k = tS;
            while(t>0){
                if(pD[k].r_start<=qe){   
                    t--;     
                    Ols[pD[k].r_ctype][pD[k].r_state]++;
                }
                k++;
            }             
            extra += k-tE;   
        }
    }   
    end1 = clock();      
    printf("Time for search with enumerating: %f \n", ((double)(end1-start1))/CLOCKS_PER_SEC);   
      
    for(i=0;i<n_files;i++){    
        printf("%i: \t", i);
        for(j=0;j<15;j++)
            printf("%u \t", Ols[i][j]);    
        printf("\n");
    }
    free(Bs); 
    free(pD);      
    printf("Total:%lld, %lld, %lld \n", (long long)Total, (long long)nT, (long long)extra);       
    for(i=0; i<n_files; i++){
        free(Ols[i]);
    }
    free(Ols); 
    free(pQ);
    globfree(&gResult);     
}

void func12M(char* fQuery, char* iPath)
{   //each Region is a gemomic object: an actor--interactive element
    //1. Get the files  
    glob_t gResult;
    //strcat(iPath, "*");
    int rtn = glob(iPath, 0, NULL, &gResult);     
    if(rtn!=0){
        printf("wrong dir path: %s", iPath);
        return;
    }
    char** file_ids = gResult.gl_pathv;
    uint32_t n_files = gResult.gl_pathc; 
    if(n_files<2){   
        printf("Too few files (add to path /*): %u\n", n_files); 
        return;
    }
    
    //2. Read qData
    int i, nQ, nD;
    struct g_data* pQ = openBed(fQuery, &nQ);
    
    //3. Read region data
    struct g_data* pD = NULL;  
    //assume data file < memsize
    
    
    //timing
    clock_t start1, end1, start2, end2, start3, end3;
    start1 = clock();     
    i=0;
    while(i<n_files){  
        pD = openBed(file_ids[i], &nD);          
        free(pD);
        i++;
    } 
    end1 = clock();    
    
    start2 = clock();
    i=0;
    while(i<n_files){  
        pD = openBed(file_ids[i], &nD);          
        printf("%i %s %i\n", i, file_ids[i], nD); 
        cEnumerator(pQ, nQ, pD, nD);
        free(pD);
        i++;
    } 
    end2 = clock();
    printf("Time for reading: %f \n", ((double)(end1-start1))/CLOCKS_PER_SEC);      
    printf("Time for data reading and qsort: %f \n", ((double)(end2-start2))/CLOCKS_PER_SEC); 
    
    free(pQ);
    globfree(&gResult);     
}

void func12Mi(char* fQuery, char* iPath)
{   //each Region is a gemomic object: an actor--interactive element
    //1. Get the files  
    glob_t gResult;
    //strcat(iPath, "*");
    int rtn = glob(iPath, 0, NULL, &gResult);     
    if(rtn!=0){
        printf("wrong dir path: %s", iPath);
        return;
    }
    char** file_ids = gResult.gl_pathv;
    uint32_t n_files = gResult.gl_pathc; 
    if(n_files<2){   
        printf("Too few files (add to path /*): %u\n", n_files); 
        return;
    }
    
    //2. Read qData
    int i, nQ, nD;
    struct g_data* pQ = openBed(fQuery, &nQ);
    
    //3. Read region data
    struct g_data* pD = NULL;
    //assume data file < memsize
    i=0;
    while(i<n_files){   
        pD = openBed(file_ids[i], &nD); 
        printf("%i %s %i\n", i, file_ids[i], nD);           
        iCounter(pQ, nQ, pD, nD);
        free(pD);
        i++;
    } 
    free(pQ);
    globfree(&gResult);     
}

/*
    clock_t start, end;
    start = clock();  
    end = clock();    
    //printf("time: %f \n", ((double)(end-start))/CLOCKS_PER_SEC);
*/

struct g_data6* openBed6(char* bFile, int* nR)
{   //open a .bed file and construct g_data3  
    char buf[1024], ctmp[10];
    int nCols = 16, ichr;
    char **splits;
    FILE* fd = fopen(bFile, "r"); 
    int nD=0;
    while(fgets(buf, 1024, fd)!=NULL)	
        nD++;
    fseek(fd, 0, SEEK_SET);
    struct g_data6* pD = malloc(nD*sizeof(struct g_data6));
    nD=0;
    while(fgets(buf, 1024, fd)!=NULL){	        
        splits = str_split(buf,'\t', &nCols); 
        if(strlen(splits[0])>5 || strlen(splits[0])<4)
            ichr = -1;  
        else if(strcmp(splits[0], "chrX")==0)
            ichr = 22;
        else if(strcmp(splits[0], "chrY")==0)
            ichr = 23;
        else{
            ichr = (int)(atoi(&splits[0][3])-1);
        }           
        if(ichr>=0){
            pD[nD].r_start  = (uint32_t)atoi(splits[1]) + gstart[ichr];
            pD[nD].r_end  = (uint32_t)atoi(splits[2]) + gstart[ichr];  
            if(splits[3][1]=='_'){     
                strncpy(ctmp, splits[3], 1); 
                ctmp[1]='\0';          
                pD[nD].r_item = atoi(ctmp)-1;
            }
            else{
                strncpy(ctmp, splits[3], 2);
                ctmp[2]='\0';
                pD[nD].r_item = atoi(ctmp)-1;
            }
            nD++;
        }
        free(splits);
    }
    fclose(fd);
    *nR = nD;
    return pD;
}
 
struct g_data** openBed24(char* bFile, int* nD)
{   //open a .bed file and construct g_data  
    char buf[1024], s10[128];
    int i, k, ichr, ichr0, lens;    
    char *s1, *s2, *s3;   
    FILE* fd = fopen(bFile, "r"); 
    strcpy(s10, bFile);      
    while(fgets(buf, 1024, fd)!=NULL){
        s1 = strtok(buf, "\t");
        if(strcmp(s1, s10)==0){
            ichr = ichr0;
        }
        else{          
            lens = strlen(s1);   
            if(lens > 5 || lens < 4)
                ichr = -1;  
            else if(strcmp(s1, "chrX")==0)
                ichr = 22;
            else if(strcmp(s1, "chrY")==0)
                ichr = 23;     
            else if (strcmp(s1, "chrM")==0)
                ichr = -1;    
            else{
                ichr = atoi(&s1[3])-1;
            }
            strcpy(s10, s1);
            ichr0 = ichr;             
        }    
        if(ichr>=0)
            nD[ichr]++;
    }	
    fseek(fd, 0, SEEK_SET);
    //-------------------------------------------------------------------------   
    struct g_data** gD = malloc(24*sizeof(struct g_data*));
    for(i=0;i<24;i++){
        gD[i] = NULL;
        if(nD[i]>0)
            gD[i] = malloc(nD[i]*sizeof(struct g_data));
        nD[i]=0;
    }
    while (fgets(buf, 1024, fd)) {
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t"); 
        if(strcmp(s1, s10)==0){
            ichr = ichr0;
        }
        else{           
            lens = strlen(s1);   
            if(lens > 5 || lens < 4)
                ichr = -1;  
            else if(strcmp(s1, "chrX")==0)
                ichr = 22;
            else if(strcmp(s1, "chrY")==0)
                ichr = 23;     
            else if (strcmp(s1, "chrM")==0)
                ichr = -1;    
            else{
                ichr = atoi(&s1[3])-1;
            } 
            strcpy(s10, s1);
            ichr0 = ichr;
        }         
        if(ichr>=0){
            k = nD[ichr];
            gD[ichr][k].r_start  = atol(s2);
            gD[ichr][k].r_end  = atol(s3);
            nD[ichr]++;
        }     
    } 
    fclose(fd);
    return gD;  
} 
 
struct g_data* openBed(char* bFile, int*nR)
{ 
    char buf[1024], s10[128];
    FILE* fd = fopen(bFile, "r");
    int nD=0, ichr, ichr0=-1, lens;
    while(fgets(buf, 1024, fd)!=NULL)	
        nD++;
    fseek(fd, 0, SEEK_SET);
    struct g_data* pD = malloc(nD*sizeof(struct g_data));
    char *s1, *s2, *s3;    
    nD=0; 
    strcpy(s10, bFile);     
    while (fgets(buf, 1024, fd)) {
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");  
        if(strcmp(s1, s10)==0){
            ichr = ichr0;
        }
        else{ 
            lens = strlen(s1);   
            if(lens > 5 || lens < 4)
                ichr = -1;  
            else if(strcmp(s1, "chrX")==0)
                ichr = 22;
            else if(strcmp(s1, "chrY")==0)
                ichr = 23;     
            else if (strcmp(s1, "chrM")==0)
                ichr = -1;    
            else{
                ichr = atoi(&s1[3])-1;
            } 
            strcpy(s10, s1);
            ichr0 = ichr;
        }         
        if(ichr>=0){
            pD[nD].r_start  = (uint32_t)atol(s2) + gstart[ichr];
            pD[nD].r_end  = (uint32_t)atol(s3) + gstart[ichr]-1;  //bed file definition !!
            nD++;
        }     
    } 
    fclose(fd);
    *nR = nD;
    return pD;    
}
 
/*struct g_data* openBed(char* bFile, int* nR)
{   //open a .bed file and construct g_data  
    char buf[1024], ch;
    int nCols = 16, ichr;
    char **splits;
    FILE* fd = fopen(bFile, "r"); 
    int nD=0, lens;
    while(fgets(buf, 1024, fd)!=NULL)	
        nD++;
    fseek(fd, 0, SEEK_SET);
    struct g_data* pD = malloc(nD*sizeof(struct g_data));
    nD=0;
    while(fgets(buf, 1024, fd)!=NULL){	        
        splits = str_split(buf,'\t', &nCols); 
        lens = strlen(splits[0]);
        if(lens > 5 || lens < 4)
            ichr = -1;  
        else if(strcmp(splits[0], "chrX")==0)
            ichr = 22;
        else if(strcmp(splits[0], "chrY")==0)
            ichr = 23;
        else if(strcmp(splits[0], "chrM")==0)
            ichr = -1;            
        else{
            ichr = (int)(atoi(&splits[0][3])-1);
        }           
        if(ichr>=0){
            pD[nD].r_start  = (uint32_t)atoi(splits[1]) + gstart[ichr];
            pD[nD].r_end  = (uint32_t)atoi(splits[2]) + gstart[ichr];  
            nD++;
        }
        free(splits);
    }
    fclose(fd);
    *nR = nD;
    return pD;
}*/

int main(int argc, char **argv)
{
    int choice;
    if(argc == 3)
        choice = 8;//default
    else if(argc == 4)
        choice = atoi(argv[3]);        
    else{
        printf("input error: data file, query file, option [0-12] \n");
        return 0;
    }
    char *qfName = argv[1];
    char *dfName = argv[2];      
    struct stat st = {0}; 
    char ftmp[128];    

    FILE* fd;
    if(choice!=10){    
        fd = fopen(qfName, "r");       
        if(fd==NULL){
            printf("File %s not found!\n", qfName);
            return 0;
        } 
        fclose(fd);
    }
    //-------------------------------------------------------------------------      
    int i, nD, nQ;
    for(i=0;i<25;i++){
        gstart[i]*=16384;
    }
    //------------------------------------------------------------------------- 
    if(choice<=8){
        fd = fopen(dfName, "r");
        if(fd==NULL){
            printf("File %s not found!\n", dfName);
            return 0;
        } 
        fclose(fd);   
    }
    else if (choice!=10){
        if(dfName[strlen(dfName)-1]=='/')
            strcat(dfName, "*");
        else if(dfName[strlen(dfName)-1]!='*')
            strcat(dfName, "/*");
        strncpy(ftmp, dfName, strlen(dfName)-2);
        if(stat(ftmp, &st) != 0){
            printf("Dir %s not exist!\n", ftmp);
        }  
    }
    if(choice==8){
        clock_t start, end;
        start = clock();     
        uint32_t* nD24 = calloc(24, sizeof(uint32_t));
        struct g_data** pD24 = openBed24(dfName, nD24);
        end = clock();    
        printf("loading time: %f \n", ((double)(end-start))/CLOCKS_PER_SEC);           
        AIListIntersect(qfName, pD24, nD24);
        free(pD24);        
        free(nD24);
    }
    else if(choice<=7){  
        clock_t start, end;
        start = clock();  
        struct g_data* pQ = openBed(qfName, &nQ);
        struct g_data* pD = openBed(dfName, &nD);        
        end = clock();    
        printf("loading time: %f \n", ((double)(end-start))/CLOCKS_PER_SEC);     
        if(choice==0)               //BITS: two separated bs, count only
            iCounter(pQ, nQ, pD, nD);  
        else if(choice==1){          //MaxE augment interval list        
            searchAIList(pQ, nQ, pD, nD); 
        }
        else if(choice==2){          //SortedE augmented interval list      
            cEnumerator(pQ, nQ, pD, nD);   
        }        
        else if(choice==3)          //dual-binary                                   
            bEnumerator(pQ, nQ, pD, nD);              
        else if(choice==4)          //Augmented dual-binary
            searchAIListDB(pQ, nQ, pD, nD);  
        else if(choice==5)          //augmented interval-tree
            searchAITree(pQ, nQ, pD, nD);                    
        else if(choice==6)  
            AIListSearch(pQ, nQ, pD, nD);   
        else                 
            dCounter(pQ, nQ, pD, nD);  //brute force            
        free(pQ);
        free(pD);
    }
    else if(choice==9){ //dfName: a file folder
        func12Mi(qfName, dfName);
    }    
    else if(choice==10){ //dfName: a file folder
        func12M(qfName, dfName);
    }
    else if(choice==11){
        func6(qfName, dfName);
    }
    else if(choice==13){
        func8(qfName, dfName);
    } 
    else if(choice==12){
        createDb(dfName, qfName);//iPath, oPath
    }
    else if(choice==14){
        funcx(qfName, dfName);
    }   
    else{
        printf("Not supported option %s\n", argv[3]);
    }
    return 0;
}
