# Augmented Interval List: a novel data structure for efficient genomic interval search

## Updates

Augmented interval list (AIList) is a new $data structure$ for enumerating intersections between a query interval $q$ and an interval set $R$. It is constructed by first sorting $R$ as a list by the interval $start$ coordinate, then decomposing it into a few flattened components (sublists), and then augmenting each sublist with the running maximum interval $end$. It nearly minimizes the $search space$ with ignorable additional cost, and thus optimizes the interval search. A preliminary implementation of AIList showed that AIList is several times faster than [interval tree] [ITree], [NCList][NCList] and BEDTools, see [ailist_paper][ailist_paper]. The complete documentation can be found at [ailist_doc][ailist_doc]. 

Since the first release of our [ailist_paper][ailist_paper], a new representative implementation of the ITree is developed by Dr Heng Li (Li's ITree or [LITree][LITree]), which showed significant improvement over other implementations of the ITree and it achieves similar efficiency of the original implementation of AIList on a few datasets. Here we re-implemented AIList by following the superior treatments of LITree on sorting and data loading (for which we are grateful to Dr Li) and updated the results for many more datasets. We also analyzed the results with emphasis on the understanding of the underline $data structure$ differences among AIList, NCList, and ITrees.

### Test results

Nine datasets in the following table are used for the test. The first 7 datasets are used in our original paper and the last two are used by LITree. They are all available 
[here](http://big.databio.org/example_data/AIList/aIListTestData.tgz). A flat interval list (null interval list) can be treated as a simple list, so only a sinlge binary search is needed for a query. Dataset 1 is flat (1 total list with no sublist) and datasets 2 and 7 are near flat (only one sublist).

|Dataset#  |Name(.bed)        |size(x1000) |non-flatness |
|:---------|:-----------------|:-----------|:------------|
|0         |chainRn4          |2,351       |6            |
|1         |fBrain            |199         |1            |
|2         |exons             |439         |2            |
|3         |chainOrnAna1      |1,957       |6            |
|4         |chainVicPac2      |7,684       |8            |
|5         |chainXenTro3Link  |50,981      |7            |
|6         |chainMonDom5Link  |128,187     |7            |
|7         |ex-anno           |1,194       |2            |
|8         |ex-rna            |9,945       |6            |

The re-implemented AIList has the same size of core data members as LITree and the time for the construction and result output of AIList is almost the same as that of the LITree, so the difference is on the query speed, which is the purpose of this comparision.

The command for AIList:
```
time ailist dataset1.bed dataset2.bed > /dev/null
```
For cgranges (LITree):
```
time bedcov-cr dataset1.bed dataset2.bed -c >/dev/null
```
The first BED file is loaded into RAM and constructed as a 'database', intervals in the second file are used as queries against the 'database' as shown here for columns 2 and 3; columns 4 and 5 reverses the order of the two datasets. 

|Datasets  |AIList    |LITree    | |AIList,r  |LITree,r  |
|:---------|:---------|:---------|:|:---------|:---------|
|0, 1      |0.441s    |0.440s    | |0.774s    |0.882s    |
|0, 2      |0.538s    |0.595s    | |0.805s    |0.940s    |
|0, 3      |2.124s    |3.352s    | |2.598s    |5.777s    |
|0, 4      |7.512s    |14.102s   | |6.573s    |11.430    |
|0, 5      |26.499s   |43.498s   | |39.562s   |1m10.207s |
|0, 6      |50.551s   |1m14.538s | |1m20.744s |2m24.714s |
|0, 7      |0.955s    |1.089s    | |0.939s    |1.052s    |
|0, 8      |3.382s    |4.006s    | |9.430s    |9.867s    |
|7, 8      |6.868s    |6.967s    | |3.198s    |3.848s    |
|3, 6      |2m20.725s |5m36.132s | |1m17.778s |2m7.386s  |

### Understanding the underline data structures of AIList and LITree

* As an implementation of ITree, LITree (cgranges) gets the best out of the ITree $data structure$: 1) it gets rid of the pointers to child nodes, so it minimizes the data storage (memory); 2) it is natually balanced, so it gets rid of the separated and costy tree balancing processing; 3) cgranges sorts the whole list (array) before indexing, so it breaks down long coverage (see ailist_paper) and reduces the search space, and it also maintains a unique tree representation (in conventional ITree for the same set of intervals, the final tree structure is dependent on the order of the intervals being added and on the balancing processing). So cgranges is much better than other implementations of the $data structure$.

* The above test results show that ailist is generally faster than LITree, which is expected from their underline data structures. For flat or nearly flat datasets (#1, #2 and #7), they are close; for non-flat datasets (#0, #3, etc), AIList is significantly faster than LITree. For datasets 3 and 6, the actual query time is 105s for AIList and 296s for LITree, which accounts for a $2.8x$ difference in query efficiency. As a data structure aimed to minimize the $search space$, i.e., to reduce the search space to the level of a flat list, NCList does it completely but with heavily additional cost (extra storage, extra binary search on sub-lists, see [ailist_paper][ailist_paper]), LITree achieves that partially with ignorable additional cost, and AIList does it nearly completely with ignorable cost. 

### Use ailist in python

A Python wrapper of the AIList c code is included in folder src_py and it is very fast (~3 times slower than AIList c code but faster than most other c codes).
To install after cloning this repo: 
```
cd AIList/src_py
python setup.py install
```
A small python program $ailist_test.py$ is included, which can be used this way:
```
python ailist_test.py test1.bed test2.bed
```

[ailist_doc]: http://ailist.databio.org 
[ailist_paper]: https://www.biorxiv.org/content/10.1101/593657v1
[LITree]: https://github.com/lh3/cgranges
[NCList]: https://github.com/hunt-genes/ncls
[ITree]: https://en.wikipedia.org/wiki/Interval_tree
