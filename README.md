Unlike most other compression methods, the Burrows-Wheeler method works in block mode. The input stream is read block by block
and encoded separately as one string. It is also known as the block-sorting compression. BW method usually compresses as good as
PPM method without being as slow. The speed of BW method is usually on par with the LZ family of compressors.

This implementation uses an array data structure to hold the blocks and their rotations. A more efficient method is to use suffix trees
for sorting.

Compression Efficiency:
I tested the algorithm on several files with very good compression ratios, usually (4-5) : 1, and in one case i got a compression ratio of about
100 (the file content was very skewed).
The compression efficiency was slightly improved after implenting exclusion.

average Compression speed: testFile -> caglary corpus
(the speed usually depends on the size of the block and how much comparisons need to be done to sort each row in the block)
Encoding => 1 mb/sec.
Decoding => 2.5 mb/sec

system specs: core i5, 8GB RAM, 1.9GHZ processor speed.

The table below shows the compression efficiency of this implementation when compressing the calgary corpus. I also include the 
results of Cleary and Witten implementation for comparisons.

|file    |   size(bytes)    |     bpc(BW94)     | BW(mine)(750kblock)  |
|--------|------------------|-------------------|----------------------|
|bib     |  111261			|      2.07		    |		2.38		   |
|book1   |  768771			|      2.49		    |		2.69		   |
|book2   |  610856			|      2.13		    |		2.40		   |
|geo     |  102400			|      4.45		    |		3.23		   |
|news    |  377109			|      2.59		    |		2.81		   |
|obj1    |  21504			|      3.98         |       3.61           |
|obj2    |  246814			|      2.64			|		1.99		   |
|paper1	 |  53161			|	   2.55			|		2.74		   |
|paper2	 |  82199			|	   2.51			|		2.74		   |
|pic     |  513216			|	   0.83			|		1.21		   |
|progc	 |  39611			|	   2.58			|		2.76		   |
|progl	 |  71646			|	   1.80			|		2.09		   |
|progp	 |  49379			|	   1.79			|		2.07		   |
|trans	 |  93695			|      1.57			|		1.91		   |
|average |  224402			|	   2.48			|		2.60		   |
