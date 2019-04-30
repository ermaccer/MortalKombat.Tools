## FSYSCreate

Tool to create File System archives of MK4 PC.

`Usage: fsysextract <folder> <file>`

Reads files from folder named in following pattern:
`0.dat 1.dat 2.dat...`

It is **required** to keep file order as MK4 PC doesn't really check for content, it reads sequentially.
No more than 799 files gets read.
