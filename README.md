# crle

RLE Data Compressor

Usage: 
```
Usage: crle.exe *.csv [/hi] [/arr] [/h] [/b] [/p] [/n] [/sp]

/hi hide information.
/arr show array information.
/h hex format.
/b binary format.
/p cpp header format.
/n cpp header no rle compression.
/sp  [w] [h] split file.
```

Example output:
```
[ INFORMATION ]
File name: test.csv
Write to file: test.txt
Original data size: 100 b.
RLE data size: 35 b.

RLE tag: 0x01
Compress: 65.00%
```

Example data file:

```
const unsigned char t2[35] = {
0x01,0x00,0x01,0x17,0x70,0x00,0x01,0x07,0xA0,0x00,0x01,0x03,0xB0,0x00,0x01,0x07,
0xB0,0x00,0x01,0x1F,0x70,0x00,0x01,0x07,0xA0,0x00,0x01,0x03,0xB0,0x00,0x01,0x03,
0x00,0x01,0x00
};
```
