# Dual binary search for genomic region data analysis 

An interval (region) data set cannot be ordered completely since each interval has two data elements, so binary search cannot be used directly. Interval data are usually pretreated with B+ tree algorithm to store the result as a database for later query. Although it has been shown previously that by using two binary searches one can efficiently infer the number of overlaps, enumerating the actual overlaps is concluded as inefficient. Here we show that by using two coupled binary searches (dual-binary search), not only counting but also enumerating the overlaps can be carried out efficiently. 

