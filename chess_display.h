#include "chess_bot.h"

#define MOVES_STRLEN 2048

/*  Info for moving pieces around the board,
 *  i.e. "animating" them. */
typedef struct {
	int active;
	int src;
	int dest;
	int frame;
	int drawx;
	int drawy;
} PieceAnimator;

typedef struct {
	WinRend winrend;	

	int draw_board;
	int draw_moves;

	Sprite *pieces_spr;
	Font *font;

	ChessGame *game;

	/* Should both be -1 if nothing on board
	 * is selected */
	int row_selected;
	int col_selected;

	/* Should have -1 as attributes while move is
	 * not being selected. Once both src and dest are full, 
	 * either the move is valid and it goes through or its 
	 * attributes reset if it's not valid. */
	Move move;

	PieceAnimator anim;

	/* For writing the moves played on the screen */
	char moves_str[MOVES_STRLEN];
	int str_position;

	GameCondition game_status;
	/* True if game is over and all moves have been
	 * written, else false. */
	int end_status_written;

	/* Plays against human player, if applicable. */
	ChessBot *bot;
	/* True if bot is playing, false if 2 humans are playing or
	 * if we are watching a movie. */
	int bot_playing;
} Chess_UI;



void UI_assign_defaults(Chess_UI *ui);

void chessmode_update(Chess_UI *ui, SDL_MouseButtonEvent *ev);
void chessmode_draw(Chess_UI *ui);

void reset_chessgame(Chess_UI *ui);
