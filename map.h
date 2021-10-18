#include <stdio.h>

#define NUM_TILES_W 12
#define NUM_TILES_H 12
#define MAP_SIZE NUM_TILES_W * NUM_TILES_H


/* All rects draws starting w/this row will be textbox-colored
 * when text is being displayed. */
#define FIRST_TEXTBOX_ROW 9
#define TEXTBOX_NUM_COLS 40
#define TEXTBOX_NUM_ROWS 3

#define GRASS_CHAR 'G'
#define WATER_CHAR 'B'
#define VOID_CHAR  'v'
#define GRAY_CHAR  'g'

#define TEXT_FILE "all_game_text.txt"

/* Each NPC is a line 
 * - sprite : Map : location : startptr : length : engine (n for none, else num)-
 */
#define NPC_FILE "npc_data.txt"
#define NUM_NPCS 20


/* The maps!! 
 * line 0: TILE DATA IN FEN
 * line 1: NPCs TODO
 * line 2: entrance tile, exit tile, exit num
 */
enum map_num_t { PARK1, PARK2, PARK3, PARK4, PARK5 };
#define PARK_ONE "map_files/park1.txt"
#define PARK_TWO "map_files/park2.txt"
#define PARK_THREE "map_files/park3.txt"
#define PARK_FOUR "map_files/park4.txt"
#define PARK_FIVE "map_files/park5.txt"


/* TODO: NPC struct! */
typedef struct _npc_t
{
	unsigned int sprite_num;
	enum map_num_t resident_map;
	unsigned int location;
	unsigned int text_start_position;
	unsigned int text_length;
	/* Will be converted to enum BotAlgo. 
	 * If -1, then does not play chess with
	 * player. */
	int chess_engine;
} NPC;


typedef struct _game_map_t 
{
	/* Dynamic map data */
	unsigned int player_location; /* Derive row, col with mod/divide */
	/* Text stuff */	
	unsigned int text_active;
	char writing_lines[TEXTBOX_NUM_ROWS][TEXTBOX_NUM_COLS];
	unsigned int current_text_length;
	unsigned int current_text_position; 


	/* Static map data */
	char tiles[MAP_SIZE];
	unsigned int exit_tile; /* Derive row, col with mod/divide */
	enum map_num_t current_map;
	enum map_num_t next_map;

	/* Same data independent of current map */
	FILE *text_fp;	
	NPC NPCs[NUM_NPCS];
	NPC currently_talking_to;

	int player_spritenum;
} 
Map;

Map *Map_create(enum map_num_t start_map);
void Map_destroy(Map *m);
void Map_changeto(Map *m, enum map_num_t next_map);

int Map_update(Map *m, int vert, int horizontal, int confirm, int deny);
