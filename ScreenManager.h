#include "raylib.h"

int screenWidth;
int screenHeight;

int fps;
int frame;

char* title;

typedef enum GameScreen GameScreen;

GameScreen currentScreen;

double title_shadow;
Color MyColor;

int RayWhiteComponent;