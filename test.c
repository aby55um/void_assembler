// todo: increase memory, fix resolution and full screen, create different files for different functionalities
// todo: refactoring (create config variables, save them into a file), multiple cursor actions when holding a button
// todo: add all the menus and the transition between them, add instructions, whitespace handling in code
// todo: complete refactoring and cleaning of code
// todo: implement an upper limit to register values
// todo: fix ; - not to be almost useless
// todo: add level progression, save it into a file
// todo: add level descriptions
// todo: solve file operations using raylib, not the C library
// todo: make it portable (using only raylib)
// todo: add error handling
// todo: more sophisticated level solution testing
// todo: add cursor at the program counter
// todo? maybe add an instruction that adds the registers
// todo: write a better instruction set
// todo: when starting a new game, ask if you are sure
// todo: the # character is redundant(?), it is not necessary to indicate a constant

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "ScreenManager.h"


typedef enum GameScreen {LOGO = 0, TITLE, MENU, LEVEL_SELECT, GAMEPLAY, CREDITS, SETTINGS, ENDING} GameScreen;

int main(void){

	int fd;

	int titleHeight = (int)((double)screenHeight * 0.25);
	int titleWidth = (int)((double)screenWidth * 0.3);
	int titleSize = (int)((double)screenHeight * 0.08);

	int upperMenuHeight = (int)((double)screenHeight * 0.45);
	int menuWidth = (int)((double)screenWidth * 0.42);
	int menuSize = (int)((double)screenHeight * 0.05);
	int menuLineSpacing = (int)((double)screenHeight / 15);

	char menu[5][15] = { "Continue Game", "New game", "Options", "Credits", "Quit" };

	int activeMenu = 0;
	int rectHeight = upperMenuHeight;
	int rectWidth = menuWidth - (int)((double)menuWidth * 0.05);
	int rectSideX = menuSize / 2;
	int rectSideY = menuSize * 0.9;
	int rectPositionModifier = menuSize * 1.35;
	int menuFrameCounter = 0;
	Color cursorColor = (Color){ 80, 80, 80, 255};

	int levelSelect = 0;
	int numberOfLevels = 12;
	int levelVar;
	int unlockedLevel=1;
	char levelText[12][20] = {"Add", "Multiply", "End of\nDemo"};
	char levelObjectives[12][100] = {"Add r1 and r2\n\nStore the result\nat memory 0x80", "Multiply r1 and r2\n\nStore the result\nat memory 0x80"};

	int currentLevel;

	int programCounter = 0;
	int r1 = 0;
	int r2 = 0;
	int r3 = 0;
	int flag = 0;

	char *programCounterString = malloc(snprintf(NULL,0,"%d",programCounter)+1);
	char *r1String = malloc(snprintf(NULL,0,"%d",programCounter)+1);
	char *r2String = malloc(snprintf(NULL,0,"%d",programCounter)+1);
	char *jumpString = malloc(snprintf(NULL,0,"%d",programCounter)+1);
	char *flagString = malloc(snprintf(NULL,0,"%d",programCounter)+1);


	//todo: mem length as variable
	int memory[168];
	char *memoryVar = malloc(sizeof(int)+1);

	struct levelLayout{
		int memory[168];
		int r1;
		int r2;
		int programCounter;
		int flag;
	};

	struct levelLayout levelLayout[12];

	for(int i=0;i<12;i++){
		for(int j=0;j<168;j++){
			levelLayout[i].memory[j]=0;
		}
		levelLayout[i].r1=0;
		levelLayout[i].r2=0;
		levelLayout[i].programCounter=0;
		levelLayout[i].flag=0;
	}

	levelLayout[0].r1=20;
	levelLayout[0].r2=20;

	levelLayout[1].r1=15;
	levelLayout[1].r2=13;

	char *memoryPlaceHolder[21] = {"0:","8:","10:","18:","20:","28:","30:","38:","40:","48:","50:","58:","60:","68:","70:","78:","80:","88:","90:","98:","A0:"};

	for(int i=0;i<168;i++){
		memory[i]=0;
	}

	char program[12][24];
	char programString[2] = "\0";
	int programTextEnd[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int gameCursorPosX = 0;
	int gameCursorPosY = 0;

	int pressedChar;


	void initProgressFile(){
		if (!FileExists("progress")){
			unlockedLevel=1;
			FILE *fptr = fopen("progress", "w");
			fprintf(fptr,"01\n");
			fclose(fptr);	
		}
		else {
			FILE *fptr = fopen("progress", "r");
			char level[4];
			fgets(level,4,fptr);
			unlockedLevel = (level[0]-'0')*10+level[1]-'0';
			fclose(fptr);
		}
	}

	void writeToLevelFile(){
		
		char levelFileName[8] = "level01";
		levelFileName[5] = ((currentLevel+1)/10)+'0';
		levelFileName[6] = ((currentLevel+1)%10)+'0';
		levelFileName[7] = 0;

		char linProg[12*24];

		int i=0;
		int j=0;
		while(j<24){
			for(int k=0;k<programTextEnd[j];k++){
				linProg[i]=program[k][j];
				i++;
			}
			linProg[i]='\n';
			i++;
			j++;
		}
		linProg[i]=0;


		FILE *fptr = fopen(levelFileName,"w");
		fprintf(fptr,"%s",linProg);		
		fclose(fptr);
		
	}

	void DrawGameCursor(int posX, int posY){
		DrawRectangle(0.11 * screenWidth + posX * 0.02 * screenWidth, 0.16 * screenHeight + posY * 0.03 * screenHeight, 0.003 * screenWidth, 0.038 * screenHeight, MyColor);
	}	

	void moveCursorRight(bool typing){
		if(gameCursorPosX < programTextEnd[gameCursorPosY] || (gameCursorPosX == programTextEnd[gameCursorPosY] && typing)){
			gameCursorPosX += 1;
		} else if(gameCursorPosX == programTextEnd[gameCursorPosY] && gameCursorPosY < 23){
			gameCursorPosX = 0;
			gameCursorPosY += 1;
		}
	}

	void moveCursorLeft(){
		if(gameCursorPosX == 0 && gameCursorPosY > 0){
			gameCursorPosY -= 1;
			gameCursorPosX = programTextEnd[gameCursorPosY];
		} else if(gameCursorPosX > 0){
			gameCursorPosX -= 1;
		}

	}

	void moveCursorUp(){
		if(gameCursorPosY > 0){
			gameCursorPosY -= 1;
			if(gameCursorPosX > programTextEnd[gameCursorPosY]){
				gameCursorPosX = programTextEnd[gameCursorPosY];
			}
		}
	}

	void moveCursorDown(){
		if(gameCursorPosY < 23){
			gameCursorPosY += 1;
			if(gameCursorPosX > programTextEnd[gameCursorPosY]){
				gameCursorPosX = programTextEnd[gameCursorPosY];
			}
		}
	}

	void typeChar(char character){
		if(programTextEnd[gameCursorPosY] < 11){
			programTextEnd[gameCursorPosY]+=1;
			for(int i = programTextEnd[gameCursorPosY]-1;i>=gameCursorPosX;i--){
				program[i+1][gameCursorPosY] = program[i][gameCursorPosY];
			}
			program[gameCursorPosX][gameCursorPosY] = character;
			moveCursorRight(true);
		}
	}

	void deleteChar(){
		if(gameCursorPosX==0 && gameCursorPosY>0 && programTextEnd[gameCursorPosY-1]>0){
			moveCursorLeft();
			deleteChar();
		} else if(gameCursorPosX==0 && gameCursorPosY>0 && programTextEnd[gameCursorPosY-1]==0){
			for(int i=gameCursorPosY;i<24;i++){
				if(programTextEnd[i]==0){
					programTextEnd[i-1]=0;
				}
				for(int j=0;j<programTextEnd[i];j++){
					program[j][i-1]=program[j][i];
					programTextEnd[i-1]=programTextEnd[i];
				}
				for(int j=programTextEnd[i];j<12;j++){
					program[j][i-1]=0;
				}
			}
			for(int i=0;i<programTextEnd[23];i++){
				program[i][23]=0;
			}
			programTextEnd[23]=0;
			moveCursorUp();
		} else if(gameCursorPosX==0){
		} else {
			for(int i=gameCursorPosX;i<programTextEnd[gameCursorPosY];i++){
				program[i-1][gameCursorPosY]=program[i][gameCursorPosY];
			}
			program[programTextEnd[gameCursorPosY]-1][gameCursorPosY]=0;
			programTextEnd[gameCursorPosY]-=1;
			moveCursorLeft();
		}	
	}

	void newLine(){
		if(programTextEnd[23]==0){
			for(int i = 22;i > gameCursorPosY; i--){
				for(int j=0; j<12; j++){
					program[j][i+1] = program[j][i];
				}
				programTextEnd[i+1]=programTextEnd[i];
			}
			for(int i=gameCursorPosX;i<12;i++){
				program[i-gameCursorPosX][gameCursorPosY+1]=program[i][gameCursorPosY];
				program[i][gameCursorPosY]=0;
			}
			programTextEnd[gameCursorPosY+1]=programTextEnd[gameCursorPosY] - gameCursorPosX;
			programTextEnd[gameCursorPosY]=gameCursorPosX;
			gameCursorPosX = 0;
			gameCursorPosY += 1;
		}
	}

	void initProgram(){
		for(int i=0; i<12; i++){
			for (int j=0;j<24;j++){
				program[i][j] = 0;
			}
		}
	}

	void initLevel(){
		char levelFileName[8] = "level01";
		levelFileName[5] = ((currentLevel+1)/10)+'0';
		levelFileName[6] = ((currentLevel+1)%10)+'0';
		levelFileName[7] = 0;

		if(FileExists(levelFileName)){

			for(int i=0; i<24; i++){
				programTextEnd[i]=0;
			}

			FILE *fptr = fopen(levelFileName, "r");

			int progLine=0;
			int charPos=0;

			for(char c=getc(fptr);c!=EOF;c=getc(fptr)){
				if(c!='\n'){
					//typeChar(c);
					program[charPos][progLine]=c;
					charPos++;
					programTextEnd[progLine]++;
				}
				else {
					for(int i=charPos;i<12;i++){
						program[charPos+i][progLine]=0;
					}
					progLine++;
					charPos=0;
				}
			}
			fclose(fptr);
		}

		for(int i=0;i<186;i++){
			memory[i]=levelLayout[currentLevel].memory[i];
		}
		r1=levelLayout[currentLevel].r1;
		r2=levelLayout[currentLevel].r2;
		programCounter=levelLayout[currentLevel].programCounter;
		flag=levelLayout[currentLevel].flag;
	}

	void drawProgram(){
		for(int i=0;i<12;i++){
			for(int j=0;j<24;j++){
				programString[0] = program[i][j];
				DrawText(programString,0.11 * screenWidth + i * 0.02 * screenWidth, 0.16 * screenHeight + j * 0.03 * screenHeight, 0.02 * screenWidth, MyColor);
			}
		}
	}

	void assemble(){
		int currMem = 0;
		for(int i=0;i<12*24;i++){
			if((program[i%12][i/12]>='0' && program[i%12][i/12]<='9') || program[i%12][i/12] == 'm' || program[i%12][i/12]=='#' || program[i%12][i/12]=='@' || program[i%12][i/12]=='r' || program[i%12][i/12]=='i' || program[i%12][i/12]==';' || program[i%12][i/12]=='t' || program[i%12][i/12]=='j' || program[i%12][i/12]=='d' || program[i%12][i/12]=='x' || program[i%12][i/12]=='y'){
				if(program[i%12][i/12]>='0' && program[i%12][i/12]<='9'){
					memory[currMem] = program[i%12][i/12]-'0';
				} else {
					switch(program[i%12][i/12]){
						case 'm':
						{
							memory[currMem]=10;
						} break;
						case '#':
						{
							memory[currMem]=11;
						} break;
						case '@':
						{
							memory[currMem]=12;
						} break;
						case 'i':
						{
							memory[currMem]=13;
						} break;
						case 'r':
						{
							memory[currMem]=14;							
						} break;
						case ';':
						{
							memory[currMem]=15;
						} break;
						case 't':
						{
							memory[currMem]=16;
						} break;
						case 'j':
						{
							memory[currMem]=17;
						} break;
						case 'd':
						{
							memory[currMem]=18;
						} break;
						case 'x':
						{
							memory[currMem]=19;
						} break;
						case 'y':
						{
							memory[currMem]=20;
						} break;
					}
				}
				
				currMem++;
			}
		}
	}

	void doCommand(){
		int from = memory[programCounter+1];
		int fromIndex = 0;
		int fromSize = 0;
		char *valueToCopy;
		int toIndex = 0;
		int toRevIndex = 0;
		int toSize = 0;
		int toMem = 0;
		int toMemLength = 0;
		int r=0;

		switch(memory[programCounter]){
			case 10:
			{	
				switch(from){
					case 11:
					{
						fromIndex = programCounter + 2;
						int i=0;
						while(memory[programCounter+2+i]>=0 && memory[programCounter+2+i]<=9){
							fromSize++;
							toRevIndex++;
							toIndex++;
							i++;
						}
						toIndex += 2;
						toRevIndex += 2;

						int j=0;
						while(memory[programCounter+1+toIndex+j]>=0 && memory[programCounter+1+toIndex+j]<=9){
							toSize++;
							j++;
						}

						for(int k=0;k<toSize;k++){
							toMem+= (int)pow(10,toSize-1-k)*memory[programCounter+1+toIndex+k];
						}

					} break;
					case 12:
					{
						int fromRevIndex = 0;
						int memLength = 0;
						int i=0;
						do{
							i++;
							memLength++;
							fromRevIndex++;
							toRevIndex++;
						} while(memory[programCounter+1+i]>=0 && memory[programCounter+1+i]<=9);

						memLength--;

						fromIndex = 0;
						for(int k=0;k<memLength;k++){
							fromIndex += (int)pow(10,memLength-1-k)*memory[programCounter+2+k];
						}
						
						fromRevIndex++;
						toRevIndex++;
						int fromRevSize = 0;

						while(memory[programCounter+2+i]>=0 && memory[programCounter+2+i]<=9){
							fromRevSize++;
							toRevIndex++;
							i++;
						}
						toRevIndex += 1;

						fromSize=0;
						for(int k=0;k<fromRevSize;k++){
							fromSize += (int)pow(10,fromRevSize-1-k)*memory[programCounter+1+fromRevIndex+k];
						}
			
						toIndex = i;
						while(memory[programCounter+3+i]>=0 && memory[programCounter+3+i]<=9){
							toMemLength++;
							i++;
						}
			
						for(int k=0;k<toMemLength;k++){
							toMem+= (int)pow(10,toMemLength-1-k)*memory[programCounter+3+toIndex+k];
						}
					} break;
					
					case 14:
					{
						if(memory[programCounter+2]==1){
							r=r1;
						}
						else if(memory[programCounter+2]==2){
							r=r2;
						}
						else if(memory[programCounter+2]==3){
							r=r3;
						}
						if(r==0){
							fromSize = 1;
						} else {
							fromSize = floor(log10(abs(r))) + 1;
						}
						fromIndex = -1;
						toIndex = 2;
						int k=0;
						toMem = 0;
						while(memory[programCounter+4+k]>=0 && memory[programCounter+4+k]<=9){
							k++;	
						}
						toMemLength = k;
						for(int i=0;i<toMemLength;i++){
							toMem+=memory[programCounter+4+i]*(int)pow(10,toMemLength-i-1);							
						}
					} break;
				}

				if(memory[programCounter+1]==11 || memory[programCounter+1]==12){
					switch(memory[programCounter+toRevIndex]){
						case 12:
						{
							for(int i=0;i<fromSize;i++){
								if(!(memory[fromIndex+i]>=0 && memory[fromIndex+i]<=9)){
									fromSize = i;
									break;
								}
								memory[toMem + i] = memory[fromIndex+i];
							}
							memory[toMem + fromSize]=19;
						} break;
						case 14:
						{	
							r=0;
							for(int i=0;i<fromSize;i++){
								if(!(memory[fromIndex+i]>=0 && memory[fromIndex+i]<=9)){
									fromSize=i;
									break;
								}
							}
							for(int i=0;i<fromSize;i++){
								r += memory[fromIndex+i] * (int)pow(10,fromSize-1-i);
							}
							if(memory[programCounter+toRevIndex+1]==1){
								r1=r;
							}
							else if(memory[programCounter+toRevIndex+1]==2){
								r2=r;
							}
							else if(memory[programCounter+toRevIndex+1]==3){
								r3=r;
							}
						} break;
					}
				} else if(memory[programCounter+1]==14){
					switch(memory[programCounter+3]){
						case 12:
						{
							for(int i=0;i<fromSize;i++){
								memory[toMem + i] = (r/(int)pow(10,fromSize-1-i))%10;
							}
							memory[toMem+fromSize]=19;
						} break;
						case 14:
						{	
							if(memory[programCounter+4]==1){
								r1=r;
							}
							else if(memory[programCounter+4]==2){
								r2=r;
							}
							else if(memory[programCounter+4]==3){
								r3=r;
							}
						} break;	
					}
				}
			} break;
			case 13:
			{
				switch(memory[programCounter+1]){
					case 14:
					{
						if(memory[programCounter+2]==1){
							r1++;
						}
						else if(memory[programCounter+2]==2){
							r2++;
						}
					} break;
					case 12:
					{
						int k=0;
						while(memory[programCounter+2+k]>=0 && memory[programCounter+2+k]<=9){
							fromSize++;
							k++;
						}
						for(int i=0;i<fromSize;i++){
							r+=memory[programCounter+2+i]*(int)pow(10,fromSize-1-i);
						}						
						if(memory[r]>=0 && memory[r]<=8){
							memory[r]++;
						}
						else if(memory[r]==9){
							memory[r]=0;
						}
					}
				}
			} break;
			case 18:
			{
				switch(memory[programCounter+1]){
					case 14:
					{
						if(memory[programCounter+2]==1){
							r1--;
						}
						else if(memory[programCounter+2]==2){
							r2--;
						}
					} break;
					case 12:
					{
						int k=0;
						while(memory[programCounter+2+k]>=0 && memory[programCounter+2+k]<=9){
							fromSize++;
							k++;
						}
						for(int i=0;i<fromSize;i++){
							r+=memory[programCounter+2+i]*(int)pow(10,fromSize-1-i);
						}						
						if(memory[r]>=1 && memory[r]<=9){
							memory[r]++;
						}
						else if(memory[r]==0){
							memory[r]=9;
						}
					} break;
				}
			} break;
			case 16:
			{
				if(r1>=r2){
					flag=0;
				}
				else{
					flag=1;
				}
			} break;
			case 17:
			{
				if(flag==0){
					programCounter=r3-1;
				}
			} break;		

		} 
		programCounter++;

	}

	void run(){
		programCounter = 0;
		while(programCounter<12*24 && memory[programCounter]!=15){
			doCommand();
		}	
	}

	void step(){
		doCommand();
		while(memory[programCounter]!=10 && memory[programCounter]!=13 && memory[programCounter]!=16 && memory[programCounter]!=17 && memory[programCounter]!=18 && memory[programCounter]!=15 && memory[programCounter]<167){
			programCounter++;
		}
	}

	void levelPassed(){
		if(currentLevel+1==unlockedLevel){
			unlockedLevel++;
			currentScreen = LEVEL_SELECT;
			if(levelSelect<12){levelSelect++;}
			char tempLevel[2];
			if(unlockedLevel<10){
				tempLevel[0]='0';
			} else if(unlockedLevel>=10){
				tempLevel[0]='1';
			}
			tempLevel[1]=unlockedLevel%10+'0';
			tempLevel[2]=0;
			FILE *fptr = fopen("progress", "w");
			fprintf(fptr,"%s",tempLevel);
			fclose(fptr);
			for(int i=0;i<12;i++){
				for(int j=0;j<24;j++){
					program[i][j]=0;
				}
			}
			gameCursorPosX=0;
			gameCursorPosY=0;
		}
	}

	void testCurrentLevel(){
		switch(currentLevel){
			case 0:
			{
				if(memory[128]==4 && memory[129]==0){
					levelPassed();
				}
			} 
			case 1:
			{
				if(memory[128]==1 && memory[129]==9 && memory[130]==5){
					levelPassed();
				}
			}
		}
	}


	void DrawMemory(){
		for(int i=0;i<168;i++){
			switch(memory[i])
			{
				case 0:
				{
					memoryVar = "0";
				} break;
				case 1:
				{
					memoryVar = "1";
				} break;
				case 2:
				{
					memoryVar = "2";
				} break;
				case 3:
				{
					memoryVar = "3";
				} break;
				case 4:
				{
					memoryVar = "4";
				} break;
				case 5:
				{
					memoryVar = "5";
				} break;
				case 6:
				{
					memoryVar = "6";
				} break;
				case 7:
				{
					memoryVar = "7";
				} break;
				case 8:
				{
					memoryVar = "8";
				} break;
				case 9:
				{
					memoryVar = "9";
				} break;
				case 10:
				{
					memoryVar = "A";
				} break;
				case 11:
				{
					memoryVar = "B";
				} break;
				case 12:
				{
					memoryVar = "C";
				} break;
				case 13:
				{
					memoryVar = "D";
				} break;
				case 14:
				{
					memoryVar = "E";
				} break;
				case 15:
				{
					memoryVar = "F";
				} break;
				case 16:
				{
					memoryVar = "G";
				} break;
				case 17:
				{
					memoryVar = "H";
				} break;
				case 18:
				{
					memoryVar = "J";
				} break;
				case 19:
				{
					memoryVar = "K";
				} break;
				case 20:
				{
					memoryVar = "L";
				}
				default:
				{
					memoryVar = "?";
				}
			}

			DrawText(memoryVar, (0.47 + i % 8 * 0.02) * screenWidth, (0.15 + i / 8 * 0.035) * screenHeight, 0.025 * screenWidth, MyColor);
		}

		for(int i=0;i<21;i++){
			DrawText(memoryPlaceHolder[i], 0.42 * screenWidth, (0.15 + i * 0.035) * screenHeight, 0.025 * screenWidth, MyColor);
		}
	}

	void DrawFrame(){
		DrawRectangle(screenWidth * 0.03, screenHeight * 0.03, screenWidth * 0.94, screenWidth * 0.009, MyColor);
		DrawRectangle(screenWidth * 0.03, screenHeight * 0.96, screenWidth * 0.94, screenWidth * 0.009, MyColor);
	}

	void DrawCustomFrame(double coordX, double coordY, double width, double height, double thickness, Color color){
		DrawRectangle(coordX * screenWidth, coordY * screenHeight, width * screenWidth, thickness * screenWidth, color);
		DrawRectangle(coordX * screenWidth, (coordY + height) * screenHeight, width * screenWidth, thickness * screenWidth, color);
		DrawRectangle(coordX * screenWidth, coordY * screenHeight + thickness * screenWidth, thickness * screenWidth, height * screenHeight - thickness * screenWidth, color);
		DrawRectangle((coordX + width - 0.9 * thickness) * screenWidth, coordY * screenHeight + thickness * screenWidth, thickness * screenWidth, height * screenHeight - thickness * screenWidth, color);
	}

	void DrawLevelSelector(int coordX, int coordY){
		DrawRectangle(coordX * screenWidth * 0.2 + screenWidth * 0.12, coordY * screenHeight * 0.3 + screenHeight * 0.13, screenWidth * 0.007, screenWidth * 0.095, MyColor);
		DrawRectangle(coordX * screenWidth * 0.2 + screenWidth * 0.27, coordY * screenHeight * 0.3 + screenHeight * 0.13, screenWidth * 0.007, screenWidth * 0.095, MyColor);
		DrawRectangle(coordX * screenWidth * 0.2 + screenWidth * 0.127, coordY * screenHeight * 0.3 + screenHeight * 0.13, screenWidth * 0.144, screenWidth * 0.007, MyColor);
		DrawRectangle(coordX * screenWidth * 0.2 + screenWidth * 0.127, coordY * screenHeight * 0.3 + screenHeight * 0.279, screenWidth * 0.144, screenWidth * 0.007, MyColor);
		DrawText(levelText[coordY*4+coordX], coordX * screenWidth * 0.2 + screenWidth * 0.15, coordY * screenHeight * 0.3 + screenHeight * 0.15, 0.025 * screenWidth, MyColor);
	}


	InitWindow(screenWidth, screenHeight, title);
	//ToggleFullscreen(); // Messes up the OS resolution 
	SetExitKey(0);

	InitAudioDevice();
	Sound menuSound = LoadSound("menuSound.mp3");
	Sound menuButtonSound = LoadSound("menuButton.mp3");

	SetTargetFPS(fps);

	initProgram();

	initProgressFile();
	

	while(!WindowShouldClose()){

		ClearBackground(BLACK);

		if(frame==0){
			PlaySound(menuSound);
		}

		if(!IsSoundPlaying(menuSound) /*&& (currentScreen == TITLE || currentScreen == MENU || currentScreen == LEVEL_SELECT || currentScreen == GAMEPLAY)*/){
			PlaySound(menuSound);
		}

		frame += 1;

		switch(currentScreen)
		{
			case TITLE:
			{
				if(title_shadow < 2){
					title_shadow += GetFrameTime();
				} else { currentScreen = MENU;}

				MyColor = (Color){RayWhiteComponent, RayWhiteComponent, RayWhiteComponent, 0 + title_shadow * 50};
			} break;
		}

		BeginDrawing();

		DrawFrame();
		
		switch(currentScreen)
		{
			case TITLE:
			{
				DrawText(title, titleWidth, titleHeight, titleSize, MyColor);
				DrawText("Demo",titleWidth+screenWidth*0.4,titleHeight+screenWidth*0.06,titleSize/3,MyColor);
			} break;

			case MENU:
			{
				menuFrameCounter += 1;
				if(menuFrameCounter > fps){
					menuFrameCounter = 0;
				}

				if(menuFrameCounter < fps/1.5){
					cursorColor = (Color){80, 80, 80, 255};
				}
				else {
					cursorColor = (Color){80, 80, 80, 0};
				}

				if(IsKeyPressed(KEY_UP)){
					activeMenu -= 4;
					activeMenu %= 5;
					PlaySound(menuButtonSound);
				}

				if(IsKeyPressed(KEY_DOWN)){
					activeMenu -= 1;
					activeMenu %= 5;
					PlaySound(menuButtonSound);
				}

				if(IsKeyPressed(KEY_ENTER)){
					switch(activeMenu){
						case -4:
						{
							CloseWindow();
							activeMenu = 0;
						} break;
						case 0:
						{
							currentScreen	 = LEVEL_SELECT;
							activeMenu = 0;
						} break;
						case -1:
						{	
							char levelName[7]="level";
							levelName[7]=0;
							remove("progress");
							for(int i=1;i<=12;i++){
								levelName[5]=i/10+'0';
								levelName[6]=i%10+'0';
								printf("%s",levelName);
								remove(levelName);
							}
							currentScreen = LEVEL_SELECT;
						} break;
						case -2:
							{
								currentScreen = SETTINGS;
							} break;
						case -3:
						{
							currentScreen = CREDITS;
						} break;
					}
				}

				ClearBackground(BLACK);
				DrawText(title, titleWidth, titleHeight, titleSize, MyColor);
				DrawText("Demo",titleWidth+screenWidth*0.4,titleHeight+screenWidth*0.06,titleSize/3,MyColor);

				for(int i=0; i<5; i++){
					DrawText(menu[i],menuWidth, upperMenuHeight + i * menuLineSpacing, menuSize, MyColor);
				}

				DrawRectangle(rectWidth, rectHeight - activeMenu * rectPositionModifier, rectSideX, rectSideY, cursorColor);
			} break;

			case LEVEL_SELECT:
			{
				for(int i=0; i<3; i++){
					for (int j=0; j<4; j++){
						if(4*i+j<unlockedLevel){
							DrawLevelSelector(j,i);
						}
					}
				}

				if(IsKeyPressed(KEY_UP)){
					levelVar = levelSelect - 4;
					levelSelect = levelVar >= 0 ? levelVar : levelSelect;
					PlaySound(menuButtonSound);
				}
				if(IsKeyPressed(KEY_DOWN)){
					if(levelSelect+4<unlockedLevel){
						levelVar = levelSelect + 4;
						levelSelect = levelVar <= 11 ? levelVar : levelSelect;
					}
					PlaySound(menuButtonSound);
				}
				if(IsKeyPressed(KEY_LEFT)){
					levelVar = levelSelect / 4 - (levelSelect - 1) / 4;
					levelSelect = levelVar > 0 || levelSelect == 0 ? levelSelect : levelSelect - 1;
					PlaySound(menuButtonSound);
				}
				if(IsKeyPressed(KEY_RIGHT)){
					if(levelSelect+1<unlockedLevel){
						levelVar = (levelSelect + 1) / 4 - levelSelect / 4;
						levelSelect = levelVar > 0 ? levelSelect : levelSelect + 1;
					}
					PlaySound(menuButtonSound);
				}
				if(IsKeyPressed(KEY_ESCAPE)){
					currentScreen = MENU;
					levelSelect = 0;
				}
				if(IsKeyPressed(KEY_ENTER)){
					currentScreen = GAMEPLAY;
					currentLevel = levelSelect;
					//levelSelect = 0;
					initLevel();
				}

				//todo: rows and columns instead of hardcoded numbers
				DrawLevelSelector(levelSelect%4, levelSelect/4);
				DrawLevelSelector(levelSelect%4, levelSelect/4);
			} break;
			//todo: frame and text position and size parameters should not be hardcoded
			case GAMEPLAY:
			{
				DrawCustomFrame(0.1,0.14,0.25,0.75,0.005,MyColor);
				DrawText("PROGRAM", 0.17 * screenWidth, 0.1 * screenHeight, 0.025 * screenWidth, MyColor);
				
				DrawCustomFrame(0.4,0.14,0.25,0.75,0.005,MyColor);
				DrawText("MEMORY", 0.48 * screenWidth, 0.1 * screenHeight, 0.025 * screenWidth, MyColor);
				
				DrawCustomFrame(0.7,0.14,0.25,0.29,0.005,MyColor);
				DrawText("REGISTERS",0.76 * screenWidth, 0.1 * screenHeight, 0.025 * screenWidth, MyColor);
				
				DrawText("Instruction", 0.72 * screenWidth, 0.17 * screenHeight, 0.019 * screenWidth, MyColor);
				DrawText("R1", 0.72 * screenWidth, 0.22 * screenHeight, 0.019 * screenWidth, MyColor);
				DrawText("R2", 0.72 * screenWidth, 0.27 * screenHeight, 0.019 * screenWidth, MyColor);
				DrawText("R3 (Jump)", 0.72 * screenWidth, 0.32 * screenHeight, 0.019 * screenWidth, MyColor);
				DrawText("Flags", 0.72 * screenWidth, 0.37 * screenHeight, 0.019 * screenWidth, MyColor);

				DrawText(levelObjectives[currentLevel],0.72*screenWidth, 0.55*screenHeight, 0.019*screenWidth, MyColor);

				programCounterString = malloc(snprintf(NULL,0,"%d",programCounter)+1);
				r1String = malloc(snprintf(NULL,0,"%d",programCounter)+1);
				r2String = malloc(snprintf(NULL,0,"%d",programCounter)+1);
				jumpString = malloc(snprintf(NULL,0,"%d",programCounter)+1);
				flagString = malloc(snprintf(NULL,0,"%d",programCounter)+1);
				
				snprintf(programCounterString, snprintf(NULL,0,"%d",programCounter) + 1,"%d", programCounter);
				DrawText(programCounterString, 0.87 * screenWidth, 0.17 * screenHeight, 0.019 * screenWidth, MyColor);

				snprintf(r1String, snprintf(NULL,0,"%d",r1) + 1,"%d", r1);
				DrawText(r1String, 0.87 * screenWidth, 0.22 * screenHeight, 0.019 * screenWidth, MyColor);

				snprintf(r2String, snprintf(NULL,0,"%d",r2) + 1,"%d", r2);
				DrawText(r2String, 0.87 * screenWidth, 0.27 * screenHeight, 0.019 * screenWidth, MyColor);

				snprintf(jumpString, snprintf(NULL,0,"%d",r3) + 1,"%d", r3);
				DrawText(jumpString, 0.87 * screenWidth, 0.32 * screenHeight, 0.019 * screenWidth, MyColor);

				snprintf(flagString, snprintf(NULL,0,"%d",flag) + 1,"%d", flag);
				DrawText(flagString, 0.87 * screenWidth, 0.37 * screenHeight, 0.019 * screenWidth, MyColor);

				DrawMemory();
				DrawGameCursor(gameCursorPosX, gameCursorPosY);


				if(IsKeyPressed(KEY_UP)){
					moveCursorUp();	
				}
				if(IsKeyPressed(KEY_DOWN)){
					moveCursorDown();
				}
				if(IsKeyPressed(KEY_LEFT)){
					moveCursorLeft();
				}
				if(IsKeyPressed(KEY_RIGHT)){
					moveCursorRight(false);
				}

				if(true){

					if(IsKeyPressed(KEY_ENTER)){
						newLine();
					}

					pressedChar = GetCharPressed();
					if(pressedChar){
						typeChar(pressedChar);
					}

					if(IsKeyPressed(KEY_BACKSPACE)){
						deleteChar();
					}

					writeToLevelFile();
				}

				drawProgram();


				if(IsKeyPressed(KEY_LEFT_CONTROL)){
					assemble();
				}
				if(IsKeyPressed(KEY_RIGHT_CONTROL)){
					run();
					testCurrentLevel();
				}
				if(IsKeyPressed(KEY_RIGHT_ALT)){
					step();
				}

				if(IsKeyPressed(KEY_Q)){
					initLevel();
				}
				if(IsKeyPressed(KEY_ESCAPE)){
					currentScreen=LEVEL_SELECT;
					for(int i=0;i<12;i++){
						for(int j=0;j<12;j++){
							program[i][j]=0;
						}
					}
					gameCursorPosX=0;
					gameCursorPosY=0;
				}

			} break;

			case CREDITS:
			{
				DrawText("Developer: Potato Joe",menuWidth - 0.06*screenWidth, upperMenuHeight, menuSize, MyColor);
				if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)){
					currentScreen=TITLE;
				}
			} break;

			case SETTINGS:
			{
				DrawText("No settings in debug test version 0.1\n\nControls",menuWidth - 0.15*screenWidth, upperMenuHeight - 0.2*screenHeight, menuSize, MyColor);
				DrawText("\nLeft ctrl: assemble\nRight ctrl: run\nm#123@80: Move constant 123 to memory 0x80\nir1: Increment r1\ndr2: decrement r2\nt: if r1>=r2, set the flag register to 0, else to 1\nj: if the flag register is 0, jump to the value of r3\nmov@80#3r3: mov 3 digits starting from memory 0x80 to r3",menuWidth-0.15*screenWidth,upperMenuHeight-0.05*screenHeight, menuSize*0.8,MyColor);
				if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)){
					currentScreen=TITLE;
				}
			} break;
		}
		EndDrawing();
	}

	CloseAudioDevice();
	CloseWindow();

	return 0;
}