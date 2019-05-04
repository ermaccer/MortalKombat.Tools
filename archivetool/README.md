## archiveTool

Pseudo-create and extract IMP archives from Mortal Kombat Unchained.

`Usage: archivetool <mode> <input>`

### To extract
Place ARCHIVE.BIN and ARCHIVE.IMP in the program location, then simply extract by executing:

`archivetool extract archive.bin`

Files are extracted to whatever is in .BIN, another file is created - mku_files.txt.

### To rebuild

`archivetool create archive.bin`

Name of the IMP archive will be created from whatever is in .bin (usually ARCHIVE.IMP).

### About mku_files.txt
This file contains .bin offsets and filenames, do not edit by hand nor add more files, the method 
to rebuild archive bases on offsets only!
