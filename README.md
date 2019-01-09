# GSAIL: An improved algorithm for genome search with augmented interval lists

## Motivation

Genomic data are signals recorded as intervals on the genome coordinates. Interval-based comparing operation is fundamental to nearly all genomic analysis. As the available genomic data rapidly grows, developing effective and dedicated algorithms for searching this data becomes necessary. 

## Results

An augmented interval-list (AIList) algorithm is presented. The interval list is first sorted by interval start, then partitioned into two approximately normalized sub-lists, and then an augmenting list of the maximum interval end  is added to each sub-list. The query time is O(n+logN), where n is the number of overlaps and N is the size of the dataset. Tested with genomic interval datasets,  code based on this algorithm runs significantly faster than code based on existing algorithms such as augmented interval-tree (AITree), nested containment list (NCList) and R-tree.  

## How to build sailer

```
git clone https://github.com/databio/AIList.git
cd AIList
make
sudo cp bin/AIList /usr/local/bin
```

## How to run AIList (for evaluation)

```
time AIList "path/to/query_file" "path/to/data_file" > /dev/null

where:

- path/to/query_file is the path to the query file

- path/to/data_file is the path to the dataset file

```

## Build and run NCList and AITree from 3rd parties; run BEDTools for performance comparison

```
cd AIList/src_AITree
make
sudo cp AITree /usr/local/bin
```
```
cd AIList/src_NCList
gcc -o NCList intervaldb.c
sudo cp NCList /usr/local/bin
```
```
time AITree "path/to/query_file" "path/to/data_file" > /dev/null

time NCList "path/to/query_file" "path/to/data_file" > /dev/null
```

```
time bedtools intersect -a "path/to/query_file" -b "path/to/data_file" -c > /dev/null
```

git clone git@github.com:/databio/AIList.git
cd AIList
make

wget http://big.databio.org/example_data/sailer/AIListTestData.tgz
tar -xf AIListTestData.tgz


## Test datasets

Downloads of some .bed datasets are available at [big.databio.org/example_data/sailer](http://big.databio.org/example_data/sailer).


## Compare results

```
./AIList/bin/AIList AIListTestData/chainOrnAna1.bed AIListTestData/exons.bed >sailer.temp
bedtools intersect -a AIListTestData/chainOrnAna1.bed -b AIListTestData/exons.bed -c > bedtools.temp
diff sailer.temp bedtools.temp
```



