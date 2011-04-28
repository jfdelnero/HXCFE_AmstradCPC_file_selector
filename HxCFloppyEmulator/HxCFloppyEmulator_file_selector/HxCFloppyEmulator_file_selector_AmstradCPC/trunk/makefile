# This is an example makefile to show how:
# - to use the SDCC suite to build programs for the Amstrad
# - to build a program from multiple source files
# - to build each file by compiling and assembling
#
# This makefile is designed for GNU make.
#
# The build steps are:
# 1. Compile a C file into a assembler (.asm) file
# 2. Assemble the assembler file into a linkable object (.o) file
# 3. Link all the object files to make the final executable
# The following are Amstrad specific:
# 4. we link against our own crtO.o which defines the Amstrad specific
# startup - in our case to build an executable suitable for Amsdos.
# 5. We then add a Amsdos file header to the output file
# 6. We inject the final file into a CPCEMU style disk image (.dsk)
# using cpcxfs. The final disk image can be transfered to a real Amstrad
# or run on an emulator.

# this is a variable defining the assembler to use
AS = sdasz80

# this is a variable defining the compiler to use
CC = sdcc

# this is a variable defining the parameters to pass to the compiler

# -mz80 defines the CPU architecture to compile for
# -S means to compile but do not assemble or link
# --no-std-crt0 tells SDCC to not use the standard crt0 startup.
CFLAGS = -mz80 -S --no-std-crt0 --vc -I./fat32    --opt-code-size

# this is a variable defining the parameters to pass to the assembler

# -g causes the assembler to ignore undefined lables. The linker
# will try to resolve these.
AFLAGS = -g


# These are the files to build. We specify the path of the file
# with a .o extension instead of a .c extension. The .o extension
# is the final extension we want for each file.
#
# crt0.s must be first because it contains the startup code

OBJS_O = crt0.o cpcbioscall.o fectrl.o gui_utils.o ./fat32/fat_access.o ./fat32/fat_filelib.o ./fat32/fat_misc.o ./fat32/fat_string.o ./fat32/fat_table.o ./fat32/fat_write.o ./fat32/fat_cache.o


# This is the rule to build the executable called 'file'.
#
# The first line will cause all the '.c' files to be built into '.o'
# files using the rule specified later.
#
# The second line links all the object files to make the executable.
#
# Note that 'link-z80' requres a seperate .lnk file to operate correctly
# with SDCC 2.4.1
#
# "make file" or "make"


file: $(OBJS_O)
	sdldz80 -f main.lnk
	./tools/makebin -p -b 256 < file.ihx > hxc.prg
	./tools/exomizer raw hxc.prg -o hxc.prg.tmp.pck
	./tools/exoopt hxc.prg.tmp.pck hxc.prg.pck
	./tools/exomizer raw fasttext.bin -o fasttext.bin.tmp.pck
	./tools/exoopt fasttext.bin.tmp.pck fasttext.bin.pck
	./tools/Pasmo startCore.s startCore.bin
	./tools/Pasmo --amsdos start.s hxc.bin
	./cpcxfs/cpcxfsw -f -b -nd AUTOBOOT.dsk -p hxc.bin
	#mkdir ./Output
	#mv --force ./AUTOBOOT.DSK Output
	./tools/hxcfloppyemulator_convert.exe AUTOBOOT.DSK -HFE >hfeconv.log
	mv --force AUTOBOOT_DSK.hfe AUTOBOOT.HFE

# use this rule to clean up all intermediate build files
#
# e.g. "make clean"
clean:
	rm *.o *.rel *.asm *.sym *.lst *.map *.ihx fat32/*.o fat32/*.rel fat32/*.asm fat32/*.sym fat32/*.lst fat32/*.map fat32/*.bin fat32/*.ihx


# this rule defines how we can produce a '.o' file from a '.c' file
# the first line will instruct the compiler to create a '.asm' file
# from a '.c' file. The second line instructs the compiler to generate
# a '.o' file from a '.asm' file.

%.o: %.c
	$(CC) $(CFLAGS) -o $*.asm $<
	$(AS) $(AFLAGS) -o $*.o $*.asm

# this rules defines how we can produce a '.o' file from a '.s' file.
# We do this by passing the '.s' file through the assembler.
%.o: %.s
	$(AS) $(AFLAGS) -o $*.o $*.s

