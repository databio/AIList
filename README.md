## AIList: an effective algorithm for searching genomic interval data 

### Motivation: Genomic data are signals recorded as intervals on the genome coordinates. Interval-based comparing operation is fundamental to nearly all genomic analysis. As the available genomic data rapidly grows, developing effective and dedicated algorithms for searching this data becomes necessary. 

### Results: An augmented interval-list (AIList) algorithm is presented. The interval list is first sorted by interval start, then partitioned into two approximately normalized sub-lists, and then an augmenting list of the maximum interval end  is added to each sub-list. The query time is O(n+logN), where n is the number of overlaps and N is the size of the dataset. Tested with genomic interval datasets,  code based on this algorithm runs significantly faster than code based on existing algorithms such as augmented interval-tree (AITree), nested containment list (NCList) and R-tree.  

### How to build AIList

```
git clone https://github.com/databio/AIList.git
cd AIList
make
```

### How to run AIList

```
AIList "path/to/query_file" "path/to/data_file"

where:

- path/to/query_file is the path to the query file

- path/to/data_file is the path to the igd data

```

### Test datasets

Downloads of some .bed datasets are available at [databio.org/AIList](http://big.databio.org/AIList).

