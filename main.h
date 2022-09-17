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

void free_game_mem();

void clearRestOfLine();
void clearOtherLines();

void draw();
void drawUI();
void clearUI();

void keyPressed();
void clearScreen();

#define MIN(a, b) ((a) < (b) ? (a) : (b))