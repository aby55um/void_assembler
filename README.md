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

how to play (at the current state of the program):

Continue game -> Choose any of the boxes (levels) to get to the gameplay screen

commands:

refer to register: r1, r2

refer to memory: @[memory]#[size], eg. @80#3 will refer to a 3 digit long int starting at memory 80, or @[memory] without giving size

refer to constant: #[constant], eg. #5 refers to the number 5

m: move

example commands:

m#123@80;	<- move 123 to memory address 80

m@80#3r1;	<- move the three digit number starting at memory 80 to register 1

i: increment

ir2;		<- increment register 2

i@80;		<- increment the digit at memory 80

t: test

if r1 >= r2, then the flag register is set to 0

else the flag register is set to 1

j: jump

if the flag is 0, then the program counter is set to the value of r1

assemble a command: left control

run the assembled command: right control	
