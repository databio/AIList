//=============================================================================
//Read .BED datasets, and then find all overlaps 
//by Jianglin Feng  09/05/2018
//Decomposition & simplication: 11/26/2018
//Radix sorting and one-pass loading based on lh3's cgranges: 6/20/2019
//-----------------------------------------------------------------------------
#include "AIList.h"
#define gdata_t_key(r) ((r).start)
KRADIX_SORT_INIT(intv, gdata_t, gdata_t_key, 4)
KHASH_MAP_INIT_STR(str, int32_t)
typedef khash_t(str) strhash_t;

//-------------------------------------------------------------------------------
void readBED(ailist_t *ail, const char* fn)
{   //faster than strtok()
    char buf[1024];  
    char *s1, *s2, *s3;
	int32_t k = 0;    
    FILE* fd = fopen(fn, "r");
    while(fgets(buf, 1024, fd)){
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t");  
		if (s1) ailist_add(ail, s1, atol(s2), atol(s3), k++);
	}
	fclose(fd);
	return;
} 

uint32_t bSearch(gdata_t* As, uint32_t idxS, uint32_t idxE, uint32_t qe)
{   //find tE: index of the first item satisfying .s<qe from right
    int tL=idxS, tR=idxE-1, tM, tE=-1;
    if(As[tR].start < qe)
        return tR;
    else if(As[tL].start >= qe)
        return -1;
    while(tL<tR-1){
        tM = tL + (tR - tL)/2; 
        if(As[tM].start >= qe)
            tR = tM-1;
        else
            tL = tM;
    }
    if(As[tR].start < qe)
        tE = tR;
    else if(As[tL].start < qe)
        tE = tL;       
    return tE; 
}

ailist_t *ailist_init(void)
{
	ailist_t *ail = malloc(1*sizeof(ailist_t));
	ail->hc = kh_init(str);
	ail->nctg = 0;
	ail->mctg = 32;
	ail->ctg = malloc(ail->mctg*sizeof(ctg_t));
	return ail;
}

void ailist_destroy(ailist_t *ail)
{
	int32_t i;
	if (ail == 0) return;
	for (i = 0; i < ail->nctg; ++i){
		free(ail->ctg[i].name);
		free(ail->ctg[i].glist);
		free(ail->ctg[i].maxE);
	}
	free(ail->ctg);
	kh_destroy(str, (strhash_t*)ail->hc);
	free(ail);
}

void ailist_add(ailist_t *ail, const char *chr, uint32_t s, uint32_t e, int32_t v)
{
	if(s > e)return;
	int absent;
	khint_t k;
	strhash_t *h = (strhash_t*)ail->hc;
	k = kh_put(str, h, chr, &absent);
	if (absent) {
		if (ail->nctg == ail->mctg)
			EXPAND(ail->ctg, ail->mctg);							
		kh_val(h, k) = ail->nctg;		
		ctg_t *p = &ail->ctg[ail->nctg++];
		p->name = strdup(chr);
		p->nr=0;	p->mr=64;
		p->glist = malloc(p->mr*sizeof(gdata_t));
		kh_key(h, k) = p->name;
	}
	int32_t kk = kh_val(h, k);
	ctg_t *q = &ail->ctg[kk];
	if (q->nr == q->mr)
		EXPAND(q->glist, q->mr);	
	gdata_t *p = &q->glist[q->nr++];
	p->start = s;
	p->end   = e;
	return;
}


void ailist_construct(ailist_t *ail, int cLen)
{   //New continueous memory?   
    int cLen1=cLen/2, j1, nr, minL = MAX(64, cLen);     
    cLen += cLen1;      
    int lenT, len, iter, i, j, k, k0, t;            	
	for(i=0; i<ail->nctg; i++){
		//1. Decomposition
		ctg_t *p    = &ail->ctg[i];
		gdata_t *L1 = p->glist;							//L1: to be rebuilt
		nr 			= p->nr;
		radix_sort_intv(L1, L1+nr);                 		               
        if(nr<=minL){        
            p->nc = 1, p->lenC[0] = nr, p->idxC[0] = 0;                
        }
        else{         
        	gdata_t *L0 = malloc(nr*sizeof(gdata_t)); 	//L0: serve as input list
            gdata_t *L2 = malloc(nr*sizeof(gdata_t));   //L2: extracted list 
            memcpy(L0, L1, nr*sizeof(gdata_t));			
            iter = 0;	k = 0;	k0 = 0;
            lenT = nr;
            while(iter<MAXC && lenT>minL){   
                len = 0;            
                for(t=0; t<lenT-cLen; t++){
                    uint32_t tt = L0[t].end;
                    j=1;    j1=1;
                    while(j<cLen && j1<cLen1){
                        if(L0[j+t].end>=tt) j1++;
                        j++;
                    }
                    if(j1<cLen1) memcpy(&L2[len++], &L0[t], sizeof(gdata_t));
                    else memcpy(&L1[k++], &L0[t], sizeof(gdata_t));                 
                } 
                memcpy(&L1[k], &L0[lenT-cLen], cLen*sizeof(gdata_t));   
                k += cLen, lenT = len;                
                p->idxC[iter] = k0;
                p->lenC[iter] = k-k0;
                k0 = k, iter++;
                if(lenT<=minL || iter==MAXC-2){			//exit: add L2 to the end
                    if(lenT>0){
                        memcpy(&L1[k], L2, lenT*sizeof(gdata_t));
                        p->idxC[iter] = k;
                        p->lenC[iter] = lenT;
                        iter++;
                    }
                   	p->nc = iter;                   
                }
                else memcpy(L0, L2, lenT*sizeof(gdata_t));
            }
            free(L2),free(L0);     
        }
        //2. Augmentation
        p->maxE = malloc(nr*sizeof(uint32_t)); 
        for(j=0; j<p->nc; j++){ 
            k0 = p->idxC[j];
            k = k0 + p->lenC[j];
            uint32_t tt = L1[k0].end;
            p->maxE[k0]=tt;
            for(t=k0+1; t<k; t++){
                if(L1[t].end > tt) tt = L1[t].end;
                p->maxE[t] = tt;  
            }             
        } 
	}
}

int32_t get_ctg(const ailist_t *ail, const char *chr)
{
	khint_t k;
	strhash_t *h = (strhash_t*)ail->hc;
	k = kh_get(str, h, chr);
	return k == kh_end(h)? -1 : kh_val(h, k);
}

uint32_t ailist_query_c(ailist_t *ail, char *chr, uint32_t qs, uint32_t qe)
{   
    uint32_t nr = 0;
    int32_t id = get_ctg(ail, chr);
    if(id>=ail->nctg || id<0)return 0;
    ctg_t *p = &ail->ctg[id];	
    int k;
    for(k=0; k<p->nc; k++){					//search each component
        int32_t cs = p->idxC[k];
        int32_t ce = cs + p->lenC[k];			
        int32_t t;
        if(p->lenC[k]>15){
            t = bSearch(p->glist, cs, ce, qe); 	//rs<qe: inline not better 
            while(t>=cs && p->maxE[t]>qs){
                if(p->glist[t].end>qs)
                	nr++;
                t--;
            }
        }
        else{
            for(t=cs; t<ce; t++){
                if(p->glist[t].start<qe && p->glist[t].end>qs)
                	nr++; 
			}                           
        }
    }               
    return nr;                                  
}

//caller allocate
int64_t queryBED(ailist_t *ail, const char* fn, int64_t nq, uint32_t *hits)
{	//find the number of overlaps for each query interval
    char buf[1024];  
    char *s1, *s2, *s3;
	int64_t nTotal = 0, i = 0, n;
    FILE* fq = fopen(fn, "r");
    while(i<nq && fgets(buf, 1024, fq)){
        s1 = strtok(buf, "\t");
        s2 = strtok(NULL, "\t");
        s3 = strtok(NULL, "\t"); 	
		if (s1){
			n = ailist_query_c(ail, s1, atol(s2), atol(s3));
			nTotal += n;
			hits[i] = n;
		}
		i++;
	}
	fclose(fq);	
	return nTotal;
}

uint32_t ailist_query(ailist_t *ail, char *chr, uint32_t qs, uint32_t qe, int32_t *gid, uint32_t *mr, uint32_t **ir)
{   
    uint32_t nr = 0, m = *mr, *r = *ir;
    int32_t id = get_ctg(ail, chr);
    if(id>=ail->nctg || id<0)return 0;
    ctg_t *p = &ail->ctg[id];
    int k;	
    for(k=0; k<p->nc; k++){					//search each component
        int32_t cs = p->idxC[k];
        int32_t ce = cs + p->lenC[k];			
        int32_t t;
        if(p->lenC[k]>15){
            t = bSearch(p->glist, cs, ce, qe); 	//rs<qe: inline not better 
            while(t>=cs && p->maxE[t]>qs){
                if(p->glist[t].end>qs){
                	if(nr==m) EXPAND(r, m);
                    r[nr++] = t;
                }
                t--;
            }
        }
        else{
            for(t=cs; t<ce; t++){
                if(p->glist[t].start<qe && p->glist[t].end>qs){
                	if(nr==m) EXPAND(r, m);
                    r[nr++] = t; 
                } 
			}                           
        }
    }    
    *gid = id, *ir = r, *mr = m;             
    return nr;                                  
}

