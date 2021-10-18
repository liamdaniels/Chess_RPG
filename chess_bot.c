#include "chess_bot.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int position_eval_twodeep(ChessBot *bot, int (*eval_game)(ChessGame *g));

/* For some reason, rand() does... the same thing every time? >:( 
 * So here's some strange mix with rand and the unix time */
int rando(){
	/* "seed" rand */
	srand(time(NULL));
	return rand();
}

ChessBot *ChessBot_create(ChessGame *game, BotAlgo algo, char color)
{
	ChessBot *cb_local = (ChessBot *) malloc(sizeof(ChessBot));

	cb_local->game = game;
	cb_local->algo_type = algo;
	cb_local->color = color;

	return cb_local;
}

void ChessBot_destroy(ChessBot *bot)
{
	free(bot);
}

Move ChessBot_find_next_move(ChessBot *bot)
{
	/* Will be finding index, and then indexing in the legal
	 * moves. By default, will be zero. */
	int move_index = 0;
	
	switch(bot->algo_type){
		case RANDOM_MOVE:
			move_index = rando() % (bot->game->num_possible_moves);
			break;
		case MIN_OPPT_MOVES:
			move_index = ChessBot_position_eval(bot, &min_oppt_moves_eval);
			break;
		case MAX_SELF_MOVES:
			move_index = position_eval_twodeep(bot, &max_moves_eval);
			break;
		case HOG_MATERIAL:
			move_index = position_eval_twodeep(bot, &material_hog_eval);
			break;
		case MATERIAL_BOTH:
			move_index = position_eval_twodeep(bot, &material_both_eval);
			break;
	}

	return bot->game->current_possible_moves[move_index];
}

int position_eval_twodeep(ChessBot *bot, int (*eval_game)(ChessGame *g))
{
	ChessGame *game_copy = Game_create();
	ChessGame *double_copy = Game_create();

	long max_score = LONG_MIN;
	long min_score;
	int max_index = 0;
	int current_score;
	

	int i, j;
	for (i = 0; i < bot->game->num_possible_moves; i++){
		Game_copy(bot->game, game_copy);
		Move_set_shorttitle(&game_copy->current_possible_moves[i], game_copy);
		/*printf("Eval for %s\n", game_copy->current_possible_moves[i].title);*/
		Game_advanceturn_index(game_copy, i);
		min_score = 99999;
		for (j = 0; j < game_copy->num_possible_moves; j++){
			Game_copy(game_copy, double_copy);
		Move_set_shorttitle(&double_copy->current_possible_moves[j], double_copy);
		/*printf(" %s : ", double_copy->current_possible_moves[j].title);*/
			Game_advanceturn_index(double_copy, j);

			current_score = (*eval_game)(double_copy);
			/*printf("EVAL is %d \n", current_score);*/

			if (current_score < min_score)
				min_score = current_score;
		}
		current_score = min_score;
		/*printf("MIN SCORE %ld\n", min_score);*/

		if (current_score > max_score){
			max_score = current_score;
			max_index = i;
		}
		else if (current_score == max_score){
			/* Randomize if they are swapped or not.
			 * Spices things up. */
			const int swapped = rando() % 2;
			if (swapped == 1){
				max_score = current_score;
				max_index = i;
			}
		}
	}
	/*printf("MAX SCORE %ld\n", max_score);*/

	Game_destroy(game_copy);
	return max_index;
}

int ChessBot_position_eval(ChessBot *bot, int (*eval_game)(ChessGame *g))
{
	ChessGame *game_copy = Game_create();

	long max_score = LONG_MIN;
	int max_index = 0;
	int current_score;

	int i;
	for (i = 0; i < bot->game->num_possible_moves; i++){
		Game_copy(bot->game, game_copy);
		Game_advanceturn_index(game_copy, i);

		current_score = (*eval_game)(game_copy);

		if (current_score > max_score){
			max_score = current_score;
			max_index = i;
		}
		else if (current_score == max_score){
			/* Randomize if they are swapped or not.
			 * Spices things up. */
			const int swapped = rando() % 2;
			if (swapped == 1){
				max_score = current_score;
				max_index = i;
			}
		}
	}

	Game_destroy(game_copy);
	return max_index;
}


/* SPECIFIC BOTS */

int min_oppt_moves_eval(ChessGame *g)
{
	return -1 * g->num_possible_moves;
}

int max_moves_eval(ChessGame *g)
{
	return g->num_possible_moves;
}

int material_hog_eval(ChessGame *g)
{
	char mover = g->current_pos->to_move;
	int r, c;
	int sum = 0;
	ChessPiece current_p;
	for (r = 0; r < 8; r++)
		for (c = 0; c < 8; c++){
			current_p = g->current_pos->piece_locations[c + (8 * r)];
			if (same_color(current_p, mover)){
				switch(current_p % 6){
					case W_Q:
						sum += 9;
						break;
					case W_N:
					case W_B:
						sum += 3;
						break;
					case W_R:
						sum += 5;
						break;
					case W_P:
						sum += 1;
						break;
					default:
						break;
				}
			}
		}
	return sum;
}

int material_both_eval(ChessGame *g)
{
	char mover = g->current_pos->to_move;
	int r, c;
	int sum = 0;
	ChessPiece current_p;
	for (r = 0; r < 8; r++)
		for (c = 0; c < 8; c++){
			current_p = g->current_pos->piece_locations[c + (8 * r)];
			if (same_color(current_p, mover)){
				switch(current_p % 6){
					case W_Q:
						sum += 9;
						break;
					case W_N:
					case W_B:
						sum += 3;
						break;
					case W_R:
						sum += 5;
						break;
					case W_P:
						sum += 1;
						break;
					default:
						break;
				}
			}
			else {
				switch(current_p % 6){
					case W_Q:
						sum -= 9;
						break;
					case W_N:
					case W_B:
						sum -= 3;
						break;
					case W_R:
						sum -= 5;
						break;
					case W_P:
						sum -= 1;
						break;
					default:
						break;
				}
			}
		}
	return 5*sum + max_moves_eval(g);
}
