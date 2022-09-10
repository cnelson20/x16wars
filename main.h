void main();
void setup();
void loadGraphics();
void loadPalette();

void game_start();

void change_directory(char *s);
void setup_menu();
void load_dir_menu();
void menu();

void print_ascii_str(char *string, unsigned char break_on_period);

void draw();
void drawUI();
void clearUI();

void keyPressed();
void clearScreen();

#define OPTION_NULL 0
#define OPTION_END 1
#define OPTION_CONCEDE 2
#define OPTION_QUIT 3 
#define OPTION_DROP 4
#define OPTION_WAIT 5 
#define OPTION_LOAD 6
#define OPTION_CAPTURE 7
#define OPTION_ATTACK 8
#define OPTION_MOUSETOGGLE 9
#define OPTION_JOIN 10
#define OPTION_MENU 11