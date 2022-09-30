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
void print_converted_str(unsigned char *string);

void free_game_mem();

void clearRestOfLine();
void clearOtherLines();

void calculate_unit_path();

void clear_layer1();

void load_co_music();

void draw();
void drawUI();
void clearUI();

void keyPressed();
void clearScreen();

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAP_HIRAM_BANK 1

#define UNIT_EXPLODING_BANK 2
#define MAP_CURSOR_MOVE_BANK 7
#define MENU_CURSOR_MOVE_BANK 9
#define SELECT_NO_UNIT_BANK 11
#define SELECT_UNIT_BANK 13
#define UNSELECT_BANK 15
#define INVALID_ACTION_BANK 17

#define MUSIC_START_BANK 19
#define MISSION_SUCCESS_MUSIC_BANK 19

#define FILENAMES_BANK 32
#define CO_MUSIC_BANK 33

#define CO2_MUSIC_BANK 41

#define HIRAM_START 0xA000