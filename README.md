# void_assembler
simple low-level programming game

license: CC BY-NC-ND 4.0

tested on a Remnux distribution

how to compile:
1. install raylib ( https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux )
2. enter the directory of test.c and the two sound files
3. compile the program 
	cc test.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o void_assembler
3. run the program 
	./void_assembler
