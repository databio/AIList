
# GSAIL benchmarks

Here we will show you how to benchmark the code. We assume you have already finished the introduction and have compiled and put the `gsail` executable in your path.

First, download some test data:


```bash
cd
wget http://big.databio.org/example_data/sailer/AIListTestData.tgz
tar -xf AIListTestData.tgz
```

```output
--2019-01-08 17:25:00--  http://big.databio.org/example_data/sailer/AIListTestData.tgz
Resolving big.databio.org (big.databio.org)... 128.143.8.170
Connecting to big.databio.org (big.databio.org)|128.143.8.170|:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 413053197 (394M) [application/octet-stream]
Saving to: ‘AIListTestData.tgz’

AIListTestData.tgz  100%[===================>] 393.92M   104MB/s    in 3.9s    

2019-01-08 17:25:04 (102 MB/s) - ‘AIListTestData.tgz’ saved [413053197/413053197]


```

Next, we will make sure we can run `gsail` on this data:


```bash
./AIList/bin/AIList AIListTestData/chainOrnAna1.bed AIListTestData/exons.bed | head
```

```output
chr1	11871	25924	13
chr1	14786	15089	2
chr1	16586	17305	3
chr1	17962	18067	1
chr1	18118	18426	1
chr1	19159	24916	1
chr1	24680	24904	1
chr1	29183	29815	1
chr1	49736	63898	0
chr1	52067	70851	1

```

Now, here is how to reproduce the benchmark figures from the paper...

