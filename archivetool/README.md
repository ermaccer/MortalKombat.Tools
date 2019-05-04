## archiveTool

Create and extract SSF archives and archives inside SFF

`Usage: ssfx <mode> <input> <pad/table> <pad>`
<mode> - create/extract
<input> - input file, for creation - input *folder*
<pad/table> - for extraction - pad(small/big) / for creation - table file 
<pad> - small/big

### To extract

`ssfx extract file.ssf padvalue`

Files are extracted to filename - extension folder in following pattern:
`0.dat,1.dat...`
Table filename is filename + .txt.
Order filename is filename + _ order +.txt.

**Pad values**
Use "big" for full archives, eg. frost.ssf will require big mode;
Use "small" for archives inside archives, eg. 1.dat inside frost.sff will require small mode;

### To build

`ssfx create folder folder.txt padvalue`

Resulting file: foldername + .ssf.
Same rules apply for padvalue as in extraction.
Keep order file near .txt!
To add more files to new archive, edit order file (keep order though).


