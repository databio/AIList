
# Augmented Interval Lists

### Motivation

Genomic data is frequently stored as segments or intervals. Because this data type is so common, interval-based comparisons are fundamental to genomic analysis. As the volume of available genomic data grows, developing efficient and scalable methods for searching interval data is necessary.

### Results

We present a new data structure, the augmented interval list (AIList), to enumerate intersections between a query interval `q` and an interval set `R`. An AIList is constructed by first sorting `R` as a list by the interval `start` coordinate, then decomposing it into a few approximately flattened components (sublists), and then augmenting each sublist with the running maximum interval `end`. The query time for AIList is `O(n+logN)`, where `n` is the number of overlaps between `R` and `q`, and `N` is the number of intervals in the set `R`. Tested with a large number of real genomic interval datasets, AIList code runs 5 - 18 times faster than standard high-performance code based on augmented interval-trees (AITree), nested containment lists (NCList), or R-trees (BEDTools). For large datasets, the memory-usage for AIList is 4% - 60% of other methods. The AIList data structure, therefore, provides a significantly improved arithmetic foundation for highly scalable genomic data analysis. 
