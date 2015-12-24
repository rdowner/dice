
loadabs/loadabs 					      loadabs/loadabs

			      LOADABS.DOC

		      GENERATING ROMABLE FIRMWARE
		     LOADING AN EXECUTABLE ABSOLUTE

    LoadAbs exefile -o outfile	-A addr

	exefile 	- executable to do the absolute relocation on
	outfile 	- resulting image file
	addr		- 0xHEX absolute relocation address

    LoadAbs takes a standard Amiga executable and generates an image file
    relocated to the absolute location specified.  The image file is layed
    out in the same order as the hunks appear in the amiga executable.	BSS
    hunks will generate 0's in the image file.

    This program will do 32 bit relocations only.  Generally you only use
    LoadAbs with -mD -mC compiled programs.

