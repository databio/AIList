//=====================================================================================
//Common structs, parameters, functions
//by Jianglin Feng  09/5/2018
//-------------------------------------------------------------------------------------
#ifndef __AILIST_H__
#define __AILIST_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <glob.h>
#include <errno.h>
#include <sysexits.h>
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

extern uint64_t maxCount;
extern uint32_t *g2ichr;

//-------------------------------------------------------------------------------------
struct g_data
{
    uint32_t r_start;      			//region start: 0-based
    uint32_t r_end;        			//region end: not included
};

// Structure to represent a node in Interval Search Tree 
struct ITNode 
{ 
    struct g_data *i;  // 'i' could also be a normal variable 
    uint32_t max; 
    struct ITNode *left;
    struct ITNode *right;    
}; 
  
// A utility function to create a new Interval Search Tree Node 
struct ITNode* newNode(struct g_data g);
struct ITNode* buildAITree(struct ITNode* root, struct g_data* B, uint32_t start, uint32_t end);
// A utility function to insert a new Interval Search Tree Node 
// This is similar to BST Insert.  Here the low value of interval 
// is used tomaintain BST property 
struct ITNode *insert(struct ITNode *root, struct g_data g);

// The main function that searches a given interval i in a given 
// Interval Tree. 
void overlapSearch(struct ITNode *tmp, struct g_data iv, uint64_t *total);

void inorder(struct ITNode *root); 

struct g_data6
{
    uint32_t r_start;      			//region start
    uint32_t r_end;        			//region end
    int r_item;                     //index of dataset, chromatin states, cell type, signal value
};

struct g_datax
{
    uint32_t r_start;               //region start
    uint32_t r_end;                 //region end
    int r_state;                    //index of chromatin state
    int r_ctype;                    //index of tissue/cell type
    uint32_t r_ss;                  //sorted start
};

struct g_data8
{
    uint32_t r_start;               //region start
    uint32_t r_end;                 //region end
    int r_state;                    //index of chromatin state
    int r_ctype;                    //index of tissue/cell type
};

char** str_split( char* str, char delim, int *nmax);
char** str_split_t( char* str, int nItems);
void str_splits( char* str, int *nmax, char **splits);

//-------------------------------------------------------------------------------------
int compare_uint32(const void *a, const void *b);
int compare_rend(const void *a, const void *b);
int compare_rstart(const void *a, const void *b);
int compare_rend6(const void *a, const void *b);
int compare_rend8(const void *a, const void *b);
int compare_rendx(const void *a, const void *b);
//-------------------------------------------------------------------------------------
int bSearch1(uint32_t* As, int tc, uint32_t qe);
int bSearch2(uint32_t* Ae, int tc, uint32_t qs);
int bSearch3(struct g_data* As, int tc, uint32_t qe);       
int bSearch4(struct g_data* Ae, int tc, uint32_t qs);
int bSearch6(struct g_data6* Ae, int tc, uint32_t qs);
int bSearch8(struct g_data8* Ae, int tc, uint32_t qs);
int bSearchx(struct g_datax* Ae, int tc, uint32_t qs);

int iCount(uint32_t* Ts, uint32_t* Te, int tc, uint32_t qs, uint32_t qe);
int cCount(uint32_t* As, struct g_data* Ge, int tc, uint32_t qs, uint32_t qe);
int cEnumerate(uint32_t* As, struct g_data* Ge, int tc, uint32_t qs, uint32_t qe, int* idx);

void iCounter(struct g_data* A, int nA, struct g_data* B, int nB);
void cCounter(struct g_data* A, int nA, struct g_data* B, int nB);
void cEnumerator(struct g_data* A, int nA, struct g_data* B, int nB);
void bEnumerator(struct g_data* A, int nA, struct g_data* B, int nB);   //2 binary
void searchAIList(struct g_data* A, int nA, struct g_data* B, int nB);
void searchAIListDB(struct g_data* A, int nA, struct g_data* B, int nB);
void searchAITree(struct g_data* A, int nA, struct g_data* B, int nB);
void AIListSearch(struct g_data* A, int nA, struct g_data* B, int nB);
void AIListSearchS(struct g_data* A, int nA, struct g_data* B, int nB);
void AIListIntersect(char* fQuery, struct g_data** B, int* nB);
void dCounter(struct g_data* A, int nA, struct g_data* B, int nB);

void func12Mi(char* fQuery, char* iPath);
void func12M(char* fQuery, char* iPath);
void func6(char* fQuery, char* iPath);
void func8(char* fQuery, char* iPath);
void funcx(char* fQuery, char* iPath);

struct g_data** openBed24(char* bFile, int* nD);//10/09/18: 24 chr
struct g_data* openBed(char* bFile, int* nR);//10/09/18: 24 chr
struct g_data6* openBed6(char* bFile, int* nR);
#endif
