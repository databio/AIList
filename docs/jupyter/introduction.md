
# GSAIL installation and hello world

In this vignette we will show you how to compile and install `gsail`, and then show you how to run it on a simple example dataset. First, clone the repository and use `make` to compile the binary:


```bash
cd
git clone git@github.com:/databio/AIList.git
cd AIList
make
```

```output
Cloning into 'AIList'...
remote: Enumerating objects: 111, done.[K
remote: Counting objects: 100% (111/111), done.[K
remote: Compressing objects: 100% (79/79), done.[K
remote: Total 111 (delta 44), reused 99 (delta 32), pack-reused 0[K
Receiving objects: 100% (111/111), 821.73 KiB | 0 bytes/s, done.
Resolving deltas: 100% (44/44), done.
Checking connectivity... done.
mkdir -p obj
mkdir -p bin
cc -c  src/AIList.c -o obj/AIList.o 
cc -o bin/AIList obj/AIList.o

```

This creates a binary in `bin/AIList`. If you want to add it to your path, you can copy it over into a general bin by running `sudo cp bin/AIList /usr/local/bin`. You can see the usage of the command line tool by running it without any arguments:


```bash
./bin/AIList
```

```output
input error: data file (.bed), query file (.bed), option (-L coverage length) 

```

To run it, we need to provide 2 `.bed` files: a query file and a database file. The `-L` parameter is optional and you 
should be fine using the default in most cases. We've included 2 very small bed files in the `example_data` folder to use as an example. Let's first see what's in those files. The database is a set of genomic intervals, each 5 base pairs long, that range from position 1 to 100 on chr1:


```bash
cat example_data/database.bed
```

```output
chr1	1	5	
chr1	6	10	
chr1	11	15	
chr1	16	20	
chr1	21	25	
chr1	26	30	
chr1	31	35	
chr1	36	40	
chr1	41	45	
chr1	46	50	
chr1	51	55	
chr1	56	60	
chr1	61	65	
chr1	66	70	
chr1	71	75	
chr1	76	80	
chr1	81	85	
chr1	86	90	
chr1	91	95	
chr1	96	100

```

The query is a set of 5 intervals that cover one or more of the database regions:


```bash
cat example_data/query.bed
```

```output
chr1	5	17
chr1	32	49
chr1	60	62
chr1	71	90
chr1	79	95

```

If we run `gsail` with these files, we will get the query back with an additional column, counting how many database regions overlap it:


```bash
./bin/AIList example_data/query.bed example_data/database.bed
```

```output
chr1	5	17	3
chr1	32	49	4
chr1	60	62	1
chr1	71	90	4
chr1	79	95	4

```
