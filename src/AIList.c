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

char *parse_bed(char *s, int32_t *st_, int32_t *en_)
{
	char *p, *q, *ctg = 0;
	int32_t i, st = -1, en = -1;
	for (i = 0, p = q = s;; ++q) {
		if (*q == '\t' || *q == '\0') {
			int c = *q;
			*q = 0;
			if (i == 0) ctg = p;
			else if (i == 1) st = atol(p);
			else if (i == 2) en = atol(p);
			++i, p = q + 1;
			if (c == '\0') break;
		}
	}
	*st_ = st, *en_ = en;
	return i >= 3? ctg : 0;
}

uint32_t bSearch(gdata_t* As, uint32_t idxS, uint32_t idxE, uint32_t qe)
{   //find tE: index of the first item satisfying .s<qe from right
    int tL=idxS, tR=idxE-1, tM, tE=-1;
    if(As[tR].start < qe)
        return tR;
    else if(As[tL].start >= qe)
        return -1;
    while(tL<tR-1){
        tM = (tL+tR)/2; 
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

//-------------------------------------------------------------------------------
ailist_t* readBED(const char* fn)
{   //faster than strtok()
	gzFile fp;
	ailist_t *ail;
	kstream_t *ks;
	kstring_t str = {0,0,0};
	int32_t k = 0;
	if ((fp = gzopen(fn, "r")) == 0)
		return 0;
	ks = ks_init(fp);
	ail = ailist_init();
	while (ks_getuntil(ks, KS_SEP_LINE, &str, 0) >= 0) {
		char *ctg;
		int32_t st, en;
		ctg = parse_bed(str.s, &st, &en);
		if (ctg) ailist_add(ail, ctg, st, en, k++);
	}
	free(str.s);
	ks_destroy(ks);
	gzclose(fp);
	return ail;
} 

void ailist_construct(ailist_t *ail, int cLen)
{   
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
            //----------------------------------------
        	gdata_t *D0 = malloc(nr*sizeof(gdata_t)); 	//D0:            
			int32_t *di = malloc(nr*sizeof(int32_t));	//int64_t?			  
            //----------------------------------------
            memcpy(L0, L1, nr*sizeof(gdata_t));			
            iter = 0;	k = 0;	k0 = 0;
            lenT = nr;
            while(iter<MAXC && lenT>minL){  
            	//setup di---------------------------			
		        for(j=0;j<lenT;j++){				//L0:{.start= end, .end=idx, .value=idx1}
					D0[j].start = L0[j].end;
					D0[j].end = j;
				}
				radix_sort_intv(D0, D0+lenT);
				for(j=0;j<lenT;j++){				//assign i=29 to L0[i].end=2
					t = D0[j].end;
					di[t] = j-t;					//>0 indicate containment
				}  
				//----------------------------------- 
                len = 0;
		        for(t=0;t<lenT-cLen;t++){
					if(di[t]>cLen)
				        memcpy(&L2[len++], &L0[t], sizeof(gdata_t));    			
					else
						memcpy(&L1[k++], &L0[t], sizeof(gdata_t)); 
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
                        lenT = 0;						//exit!
                    }
                   	p->nc = iter;                   
                }
                else memcpy(L0, L2, lenT*sizeof(gdata_t));
            }
            free(L2),free(L0), free(D0), free(di);   
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

void ailist_construct0(ailist_t *ail, int cLen)
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
                        lenT = 0;	//exit!
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

uint32_t ailist_query(ailist_t *ail, char *chr, uint32_t qs, uint32_t qe, uint32_t *mr, uint32_t **ir)
{   
    uint32_t nr = 0, m = *mr, *r = *ir;
    int32_t gid = get_ctg(ail, chr);
    if(gid>=ail->nctg || gid<0)return 0;
    ctg_t *p = &ail->ctg[gid];	
    for(int k=0; k<p->nc; k++){					//search each component
        int32_t cs = p->idxC[k];
        int32_t ce = cs + p->lenC[k];			
        int32_t t;
        if(p->lenC[k]>15){
            t = bSearch(p->glist, cs, ce, qe); 	//rs<qe: inline not better 
            if(t>=cs){
		        if(nr+t-cs>=m){
		        	m = nr+t-cs + 1024;
		        	r = realloc(r, m*sizeof(uint32_t));
		        }
		        while(t>=cs && p->maxE[t]>qs){
		            if(p->glist[t].end>qs)               	
		                r[nr++] = t;
		            t--;
		        }
            }
        }
        else{
        	if(nr+ce-cs>=m){
        		m = nr+ce-cs + 1024;
        		r = realloc(r, m*sizeof(uint32_t));
        	}
            for(t=cs; t<ce; t++)
                if(p->glist[t].start<qe && p->glist[t].end>qs)
                    r[nr++] = t;                           
        }
    }    
    *ir = r, *mr = m;                  
    return nr;                              
}

