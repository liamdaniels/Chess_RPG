#include <stdio.h>
#include "../../my_API/sdl/sdl_util.h"
#include "map.h"
#include "chess_display.h"

#define FRAME_DELAY 10

#define SPRITE_PIC "pics/sprites.png"
#define SPRITE_COLS 16
#define SPRITE_SIDELEN 64


#define VOID_COLOR 0x000000FF
#define GRASS_COLOR 0x0DD951FF
#define WATER_COLOR 0x6794E0FF
#define GRAY_COLOR 0xCFB5B4FF
/* Just have textbox fill up first two(?) rows of
 * screen, w/ text. */
#define TEXTBOX_COLOR 0xCFE815FF

#define FONT_PIC "pics/tom_vii_font.png"
#define FONT_HEIGHT 17
#define FONT_WIDTH 11
#define FONTCHARS " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`-=[]\\;',./~!@#$%^&*()_+{}|:\"<>?"
/* Width, height of each letter in pixels */
#define FONTSIZE_H 32
#define FONTSIZE_W 22
#define FSPACE 18

#define TILE_SIDELEN 64
#define WIN_W TILE_SIDELEN * NUM_TILES_W
#define WIN_H TILE_SIDELEN * NUM_TILES_H

#define CONFIRM_KEY SDLK_c
#define DENY_KEY	SDLK_x
#define UP_KEY		SDLK_UP
#define DOWN_KEY	SDLK_DOWN
#define LEFT_KEY	SDLK_LEFT
#define RIGHT_KEY	SDLK_RIGHT
#define NUM_AVAIL_BUTTONS 6 
#define CONFIRM_BUTNUM 0
#define DENY_BUTNUM    1
#define UP_BUTNUM      2
#define DOWN_BUTNUM    3
#define LEFT_BUTNUM    4
#define RIGHT_BUTNUM   5


typedef enum mode_t { MAP_MODE, CHESS_MODE } GameMode;

typedef struct {
	int happening_now;
	GameMode next_mode;
	BotAlgo next_algo;
	GameCondition game_result;
} Transition;

/* Contains game "globals" except not actually global,
 * just passed thru functions */
struct game_container_t {
	WinRend winrend;	
	GameMode current_mode;

	/* ~ Mappings ~
	 * Shown in define statements
	 *
	 * Each is a bool (0 or 1) for if button
	 * is pressed that frame or not. 
	 */
	int inputs[NUM_AVAIL_BUTTONS];

	SDL_MouseButtonEvent *mouse_ev;

	/* 1: game running, 0: should quit and done */
	int running;

	/* Game things */
	Map *map;
	
	Sprite *human_drawer;
	Font *text_writer;
	
	Chess_UI chess_ui;

	Transition trans;
};

/* Returns 1 if input was made, else 0.
 * Updates game's input fields. */
int read_inputs(struct game_container_t *game)
{
	int i;
	for (i = 0; i < NUM_AVAIL_BUTTONS; i++)
		game->inputs[i] = 0;
	game->mouse_ev = NULL;

	SDL_Event event;

	if (SDL_WaitEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				game->running = 0;
				return 0;	
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
					case CONFIRM_KEY:
						game->inputs[CONFIRM_BUTNUM] = 1;
						return 1;
					case DENY_KEY:
						game->inputs[DENY_BUTNUM] = 1;
						return 1;
					case UP_KEY:
						game->inputs[UP_BUTNUM] = 1;
						return 1;
					case DOWN_KEY:
						game->inputs[DOWN_BUTNUM] = 1;
						return 1;
					case LEFT_KEY:
						game->inputs[LEFT_BUTNUM] = 1;
						return 1;
					case RIGHT_KEY:
						game->inputs[RIGHT_BUTNUM] = 1;
						return 1;
				}
			case SDL_MOUSEBUTTONDOWN:
				game->mouse_ev = (SDL_MouseButtonEvent*)&event;
				return 1;
		}
	}
	return 0;
}

void mapmode_update(struct game_container_t *game)
{
	int vert = game->inputs[DOWN_BUTNUM] - game->inputs[UP_BUTNUM];
	int hori = game->inputs[RIGHT_BUTNUM] - game->inputs[LEFT_BUTNUM];
	int mapup = Map_update(game->map, vert, hori, game->inputs[CONFIRM_BUTNUM],
						   game->inputs[DENY_BUTNUM]);
	if (mapup != -1){
		game->trans.happening_now = 1;
		game->trans.next_mode = CHESS_MODE;
		game->trans.next_algo = mapup;
	}
}

void mapmode_draw(struct game_container_t *game)
{
	SDLUTIL_clearscreen(&(game->winrend), VOID_COLOR);

	int i, j;
	unsigned long tile_color;
	for (i = 0; i < NUM_TILES_W; i++){
		for (j = 0; j < NUM_TILES_H; j++){
			switch(game->map->tiles[i + (j * NUM_TILES_H)]){
				case GRASS_CHAR:
					tile_color = GRASS_COLOR;
					break;
				case WATER_CHAR:
					tile_color = WATER_COLOR;
					break;
				case VOID_CHAR:
					tile_color = VOID_COLOR;
					break;
				case GRAY_CHAR:
					tile_color = GRAY_CHAR;
					break;
				default:
					tile_color = VOID_COLOR;
			}

			if (game->map->text_active && j >= FIRST_TEXTBOX_ROW){
				tile_color = TEXTBOX_COLOR;
			}

			SDLUTIL_fillrect(&(game->winrend), 
							 i * TILE_SIDELEN, j * TILE_SIDELEN,
							 TILE_SIDELEN, TILE_SIDELEN, tile_color);
		}
	}

	if (game->map->text_active)
		for (i = 0; i < TEXTBOX_NUM_ROWS; i++)
			Font_draw_string(game->text_writer, game->map->writing_lines[i], 15,
							(FIRST_TEXTBOX_ROW + i) * TILE_SIDELEN + 10, FSPACE);

	/* Draw player */
	Sprite_drawat(game->human_drawer, 
				  TILE_SIDELEN * (game->map->player_location % NUM_TILES_H),
				  TILE_SIDELEN * (game->map->player_location / NUM_TILES_H),
				  game->map->player_spritenum);
	/* Draw sprites */
	for (i = 0; i < NUM_NPCS; i++){
		NPC npc = game->map->NPCs[i];
		if (npc.resident_map == game->map->current_map)
			Sprite_drawat(game->human_drawer, 
						  TILE_SIDELEN * (npc.location % NUM_TILES_H),
						  TILE_SIDELEN * (npc.location / NUM_TILES_H),
						  npc.sprite_num);
	}

	SDL_RenderPresent(game->winrend.rend);
}

void loop(struct game_container_t *game)
{
	for (;;)
	{
		if (game->trans.happening_now){
			switch(game->trans.next_mode){
				case MAP_MODE:
					game->current_mode = MAP_MODE;
					break;
				case CHESS_MODE:
					game->chess_ui.bot->algo_type = game->trans.next_algo;
					/* Reset game FEN to start (or custom?) */
					reset_chessgame(&game->chess_ui);	
					game->current_mode = CHESS_MODE;
					break;
			}
			game->trans.happening_now = 0;
		}

		SDL_Delay(FRAME_DELAY);

		if (!game->chess_ui.anim.active || game->current_mode != CHESS_MODE)
			read_inputs(game);

		switch(game->current_mode)
		{
			case MAP_MODE:
				mapmode_update(game);
				mapmode_draw(game);
				break;
			case CHESS_MODE:
				chessmode_update(&game->chess_ui, game->mouse_ev);
				chessmode_draw(&game->chess_ui);

				/* TODO: if game ended, and no animation, then set up
				 * the transition */
				if (game->chess_ui.game_status != PLAYING
					&& !game->chess_ui.anim.active){
					game->trans.happening_now = 1;
					game->trans.next_mode = MAP_MODE;
					game->trans.game_result = game->chess_ui.game_status;
				}
				break;
		}

		if (!game->running)	return;
	}
}

int main(int argc, char **argv)
{
	struct game_container_t game;
	SDLUTIL_begin(SDL_INIT_VIDEO, &(game.winrend), WIN_W, WIN_H, "Game");

	/* Init game */
	game.current_mode = MAP_MODE;
	game.running = 1;
	game.map = Map_create(0);

	game.text_writer = Font_create(
							FONT_PIC, strlen(FONTCHARS), FONT_HEIGHT, FONT_WIDTH,
							game.winrend.rend, FONTCHARS);
	Font_resize(game.text_writer, FONTSIZE_W, FONTSIZE_H);

	game.human_drawer = Sprite_create_from_picture(
						SPRITE_PIC, 0, 0, SPRITE_COLS, 
						SPRITE_SIDELEN, 
						SPRITE_SIDELEN, game.winrend.rend);
	Sprite_resize(game.human_drawer, TILE_SIDELEN, TILE_SIDELEN);

	game.trans.happening_now = 0;
	
	/* Chess part */
	game.chess_ui.winrend = game.winrend;
	game.chess_ui.game = Game_create();
	game.chess_ui.bot = ChessBot_create(game.chess_ui.game, 
			RANDOM_MOVE, BLACK_MOVE);
	game.chess_ui.bot_playing = 1;
	UI_assign_defaults(&game.chess_ui);


	loop(&game);

	/* Destroy */
	ChessBot_destroy(game.chess_ui.bot);
	Game_destroy(game.chess_ui.game);
	Sprite_destroy(game.chess_ui.pieces_spr);
	Map_destroy(game.map);
	Font_destroy(game.text_writer);
	Sprite_destroy(game.human_drawer);
	SDLUTIL_end(&(game.winrend));
	return 0;
}
