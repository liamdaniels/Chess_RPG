#include "../../my_API/sdl/sdl_util.h"
#include "chess_display.h"
#include <string.h>

#define BOARD_X 50
#define BOARD_Y 50
#define SQUARE_SIDELEN 80

#define DARK_SQ_COLOR 0x592A0AFF
#define LIGHT_SQ_COLOR 0xFFC6A1FF
#define SELECT_SQ_COLOR 0x4A9CB0FF

#define PIECES_PIC "pics/pieces.png"
#define PIECES_COLS 6
#define PIECES_FRAME_SIDELEN 128

#define MAX_MOVES_PER_LINE 3

#define MOVES_Y BOARD_Y - 30
#define MOVES_X BOARD_X + (8 * SQUARE_SIDELEN) + 10

#define FRAME_DELAY 16
#define MAX_ANIMATION_FRAMES 20




void anim_update(Chess_UI *ui)
{
	const int src_x  = ((ui->anim.src % 8 ) * SQUARE_SIDELEN) + BOARD_X;
	const int src_y  = ((ui->anim.src / 8 ) * SQUARE_SIDELEN) + BOARD_Y;
	const int dest_x = ((ui->anim.dest % 8) * SQUARE_SIDELEN) + BOARD_X;
	const int dest_y = ((ui->anim.dest / 8) * SQUARE_SIDELEN) + BOARD_Y;

	const int x_inc = (dest_x - src_x) / MAX_ANIMATION_FRAMES;
	const int y_inc = (dest_y - src_y) / MAX_ANIMATION_FRAMES;
	
	ui->anim.drawx = src_x + (x_inc * ui->anim.frame);
	ui->anim.drawy = src_y + (y_inc * ui->anim.frame);

	if (ui->anim.frame >= MAX_ANIMATION_FRAMES)
		ui->anim.active = 0;
	else
		ui->anim.frame++;
}

void anim_begin(Chess_UI *ui, int src, int dest)
{
	ui->anim.active = 1;
	ui->anim.src = src;
	ui->anim.dest = dest;
	ui->anim.frame = 0;
	anim_update(ui);
}

int is_anim(Chess_UI *ui, int row, int col)
{
	const int dest_row = ui->anim.dest / 8;
	const int dest_col = ui->anim.dest % 8;
	return row == dest_row && col == dest_col;
}

void chessmode_draw(Chess_UI *ui)
{
	SDLUTIL_clearscreen(&(ui->winrend), 0xFFFFFFFF);


	if (ui->draw_board)
	{
		unsigned long color;
		int is_light;
		int r, c;
		for (c = 0; c < 8; c++){
			for (r = 0; r < 8; r++){
				const int square_x = c * SQUARE_SIDELEN + BOARD_X;
				const int square_y = r * SQUARE_SIDELEN + BOARD_Y;
				/* Draw board square */
				is_light = (c % 2) == (r % 2);
				color = is_light ? LIGHT_SQ_COLOR : DARK_SQ_COLOR;

				if (ui->row_selected == r && ui->col_selected == c)	
					color = SELECT_SQ_COLOR;

				SDLUTIL_fillrect(&(ui->winrend), 
								 square_x, square_y,
								 SQUARE_SIDELEN, SQUARE_SIDELEN, color);
				/* Draw piece, as long as it's not being animated */ 
				const ChessPiece piece = Game_pieceat(ui->game, r, c);
				if (piece != EMT && !(ui->anim.active && is_anim(ui, r, c)))
						Sprite_drawat(ui->pieces_spr, square_x, square_y, piece);
			}
		}
		/* Draw animated piece */
		if (ui->anim.active){
			const ChessPiece piece = Game_pieceat(ui->game,
												  ui->anim.dest / 8,
												  ui->anim.dest % 8);
			Sprite_drawat(ui->pieces_spr, ui->anim.drawx, ui->anim.drawy, piece);
		}
	}

	SDL_RenderPresent(ui->winrend.rend);
}




/* Write move string to UI's move-containing big string */
void UI_write_move(Chess_UI *ui, Move move)
{
	/* Set move title */
	Move_set_shorttitle(&move, ui->game);
	
	int pos = ui->str_position;

	/* Add move num if necessary */
	if (ui->game->current_pos->to_move == WHITE_MOVE){
		int move_num = ui->game->current_pos->fullmove_clock;

		if ((move_num % MAX_MOVES_PER_LINE) == 1)
			ui->moves_str[pos++] = '\n';

		/* Assume game has no more than 1000 moves */
		if (move_num >= 100){
			ui->moves_str[pos++] = (move_num / 100) + '0';
			move_num = move_num % 100;
		}
		if (move_num >= 10){
			ui->moves_str[pos++] = (move_num / 10) + '0';
			move_num = move_num % 10;
		}
		ui->moves_str[pos++] = move_num + '0';
		ui->moves_str[pos++] = '.';
	}

	/* Copy m's title into string. String should always
	 * end with a null terminator per Move_set_shorttitle. */
	int i;
	for (i = 0; move.title[i] != '\0'; i++)
		ui->moves_str[pos++] = move.title[i];

	ui->moves_str[pos++] = ' ';
	ui->moves_str[pos] = '\0';
	ui->str_position = pos;
}

/* Append score of game to the end */
void UI_write_endscore(Chess_UI *ui)
{
	const int MAX_SCORE_LEN = 8;
	char score[MAX_SCORE_LEN];

	if (ui->game_status == WHITE)
		strcpy(score, "1-0");
	else if (ui->game_status == BLACK)
		strcpy(score, "0-1");
	else
		strcpy(score, "1/2-1/2");

	int pos = ui->str_position;
	int i;
	for (i = 0; score[i] != '\0'; i++)
		ui->moves_str[pos++] = score[i];
	ui->moves_str[pos] = '\0';
}


void chessmode_update(Chess_UI *ui, SDL_MouseButtonEvent *mouse_ev)
{
	int move_selection = 0;
	
	if (ui->anim.active){
		anim_update(ui);
		return;
	}

	if (mouse_ev != NULL){
		/* Check that click is in board bounds */
		if (mouse_ev->x >= BOARD_X 
		 && mouse_ev->x < BOARD_X + SQUARE_SIDELEN * 8
		 && mouse_ev->y >= BOARD_Y 
		 && mouse_ev->y < BOARD_Y + SQUARE_SIDELEN * 8){
			ui->row_selected = (mouse_ev->y - BOARD_Y) 
											/ SQUARE_SIDELEN;
			ui->col_selected = (mouse_ev->x - BOARD_X) 
											/ SQUARE_SIDELEN;
			move_selection = 1;
		}
		else{
			ui->row_selected = -1;
			ui->col_selected = -1;
		}
	}

	/* Manage move */
	if (move_selection && 
		(!ui->bot_playing ||
		 ui->game->current_pos->to_move != ui->bot->color)){

		if (ui->move.src == -1)
			ui->move.src = ui->col_selected + (8 * ui->row_selected);
		else { /* Editing destination! Attempt move. */
			ui->move.dest = ui->col_selected + (8 * ui->row_selected);
			
			/* TODO: make promotions to non-queen pieces available */
			
			int legal_ind = Game_get_legal(ui->game, ui->move);
			if (legal_ind != -1){
				UI_write_move(
						ui,ui->game->current_possible_moves[legal_ind]);

				ui->game_status = 
					Game_advanceturn_index(ui->game, legal_ind);

				/* Animation */
				anim_begin(ui, ui->move.src, ui->move.dest);

				ui->move.src     = -1;
				ui->move.dest    = -1;
				ui->row_selected = -1;
				ui->col_selected = -1;
			}
			else { /* Assume changing source */
				ui->move.src = ui->move.dest;
				ui->move.dest = -1;
			}
		}
	}

	if (ui->game_status != PLAYING){
		if (!ui->end_status_written){
			UI_write_endscore(ui);			
			ui->end_status_written = 1;
		}
	}
	else if (ui->anim.active){
		/* Do nothing. Necessary if player makes move
		 * we want animation to play out, so nothing 
		 * should be done here. */
	}
	else if (ui->bot_playing && 
			 ui->game->current_pos->to_move == ui->bot->color
			 ){
		/* Bot move! */
		Move bot_move = ChessBot_find_next_move(ui->bot);
		UI_write_move(ui, bot_move);
		ui->game_status = Game_advanceturn(ui->game, bot_move);

		/* Animation */
		anim_begin(ui, bot_move.src, bot_move.dest);

	}

}

void UI_reset_defaults(Chess_UI *ui)
{
	ui->draw_board = 1;
	ui->draw_moves = 1;
	
	ui->row_selected = -1;
	ui->col_selected = -1;

	ui->move.src  = -1;
	ui->move.dest = -1;
	ui->move.promoting_to = EMT;

	ui->str_position= 0;
	ui->moves_str[0] = '\0';

	ui->game_status = PLAYING;

	ui->anim.active = 0;
	ui->anim.frame = 0;
	ui->anim.src = ui->anim.dest = 0;
	ui->anim.drawx = ui->anim.drawy = 0;

	ui->end_status_written = 0;
}

void UI_assign_defaults(Chess_UI *ui)
{
	ui->pieces_spr = Sprite_create_from_picture(
						PIECES_PIC, 0, 0, PIECES_COLS, 
						PIECES_FRAME_SIDELEN, 
						PIECES_FRAME_SIDELEN, ui->winrend.rend);
	Sprite_resize(ui->pieces_spr, SQUARE_SIDELEN, SQUARE_SIDELEN);

	UI_reset_defaults(ui);
}


void reset_chessgame(Chess_UI *ui)
{
	UI_reset_defaults(ui);
	/* TODO: MAKE FEN INSTEAD?? This seems kinda
	 * hacky lol */
	Game_destroy(ui->game);
	ui->game = Game_create();
	ui->bot->game = ui->game;
}

#if 0
int main(int argc, char **argv)
{
	Chess_UI ui;

	/* TODO: let user set things with argv? */

	ui.game = Game_create();
	
	/* BOT */
	ui.bot = ChessBot_create(ui.game, BOT_ALGO, BLACK_MOVE);
	ui.bot_playing = 1;

	/* FEN, if wanted */
	if (STARTING_FROM_FEN)
		Game_read_FEN(ui.game, FENFILE);

	/* Movie */
	ui.is_file_movie = LOAD_MOVIE;	
	if (LOAD_MOVIE){
		ui.movie = Movie_create_from_PGN(MOVIEFILE);
		if (ui.movie == NULL){
			printf("Something went wrong. Not initializing movie.\n");
			ui.is_file_movie = 0;
			ui.movie = Movie_create();
		}
	}
	else
		ui.movie = Movie_create();


	SDLUTIL_begin(SDL_INIT_VIDEO, &(ui.winrend), WIN_W, WIN_H, "chess");
	UI_assign_defaults(&ui);

	loop(&ui);

	/* Destroy */
	ChessBot_destroy(ui.bot);
	Game_destroy(ui.game);
	Sprite_destroy(ui.pieces_spr);
	Movie_destroy(ui.movie);
	Font_destroy(ui.font);
	SDLUTIL_end(&(ui.winrend));
	return 0;
}
#endif
