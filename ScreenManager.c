#include "ScreenManager.h"
#include "raylib.h"

int screenWidth = 1100; //800
int screenHeight = 650; //450

int fps = 60;
int frame = 0;

char* title = "VOID ASSEMBLER";

typedef enum GameScreen {LOGO = 0, TITLE, MENU, LEVEL_SELECT, GAMEPLAY, ENDING} GameScreen;

GameScreen currentScreen = TITLE;

double title_shadow = 0;
Color MyColor;

int RayWhiteComponent = 245;