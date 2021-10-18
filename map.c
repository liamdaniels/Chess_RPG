#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PLAYER_SPRITE_NUM 0
#define NUM_SPRITES_CURRENTLY 14

void read_npc_data(Map *m)
{
	FILE *fp = fopen(NPC_FILE, "r");
	if (fp == NULL){
		printf("OH NO! NPC data file not found. Please make sure that it is in the same folder as the game.\n");
		return;
	}

	int i, j, c;
	int current_num;
	const int num_params = 6;
	for (i = 0; i < NUM_NPCS; i++)
		for (j = 0; j < num_params; j++){
			current_num = 0;
			while ((c = getc(fp)) == ' ' || c == '\n');
			if (c == 'n') /* Special case for algo */
				current_num = -1;
			else
				while(c >= '0' && c <= '9'){
					current_num *= 10;
					current_num += c - '0';
					c = getc(fp);
				} 

			switch(j){
				case 0:
					m->NPCs[i].sprite_num = current_num;
					break;
				case 1:
					m->NPCs[i].resident_map = current_num;
					break;
				case 2:
					m->NPCs[i].location = current_num;
					break;
				case 3:
					m->NPCs[i].text_start_position = current_num;
					break;
				case 4:
					m->NPCs[i].text_length = current_num;
					break;
				case 5:
					m->NPCs[i].chess_engine = current_num;
					break;
			}
		}

	fclose(fp);
}

Map *Map_create(enum map_num_t start_map)
{
	Map *map = malloc(sizeof(Map));
	Map_changeto(map, start_map);
	map->text_fp = fopen(TEXT_FILE, "r");
	if (map->text_fp == NULL){
		printf("OH NO! Text file not found. Please make sure that it is in the same folder as the game.\n");
		return NULL;
	}
	map->text_active = 0;
	read_npc_data(map);
	map->player_spritenum = PLAYER_SPRITE_NUM;
	return map;
}

void Map_destroy(Map *m)
{
	fclose(m->text_fp);
	free(m);
}

void Map_changeto(Map *map, enum map_num_t next_map)
{
	FILE *fp;	
	switch(next_map){
		case PARK1:
			fp = fopen(PARK_ONE, "r");
			break;
		case PARK2:
			fp = fopen(PARK_TWO, "r");
			break;
		case PARK3:
			fp = fopen(PARK_THREE, "r");
			break;
		case PARK4:
			fp = fopen(PARK_FOUR, "r");
			break;
		case PARK5:
			fp = fopen(PARK_FIVE, "r");
			break;
	}
	if (fp == NULL){
		printf("OH NO! Map file not found. Please make sure that it is in the same folder as the game.\n");
		return;
	}

	/* Current letter */
	int c;

	int i, j;

	/* Read first line! FEN for tiles */
	char last_added_tile = 'a';/* default val in case of early digit */
	for (i = 0; i < MAP_SIZE; i++)
	{
		c = getc(fp);

		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
			map->tiles[i] = (char)c;
			last_added_tile = (char)c;
		}
		else if (c != '/'){ /* Must be digit */
			int num_times_to_add = c - '0';
			for (j = i; j < i + num_times_to_add && j < MAP_SIZE; j++){
				map->tiles[j] = last_added_tile;
			}
			i = j - 1;
		}
		else
			i--; /* Discount slash */

	}


	while((c = getc(fp)) != '\n');

	/* Read second line! FEN for NPCS */
	/* TODO */
	while((c = getc(fp)) != '\n');

	/* Read entrance/exit data */
	map->player_location = 0;
	while((c = getc(fp)) >= '0' && c <= '9'){
		map->player_location *= 10;
		map->player_location += c - '0';
	} /* While assignment skips space (should only be one) */

	map->exit_tile = 0;
	while((c = getc(fp)) >= '0' && c <= '9'){
		map->exit_tile *= 10;
		map->exit_tile += c - '0';
	} /* While assignment skips space (should only be one) */

	map->next_map = 0;
	while((c = getc(fp)) >= '0' && c <= '9'){
		map->next_map *= 10;
		map->next_map += c - '0';
	} 

	fclose(fp);	

	map->current_map = next_map;
}


/* Puts [current_text] text into the text lines
 * being displayed on screen */
void load_text_into_boxes(Map *m)
{
	int i, j, c;
	for (i = 0; i < TEXTBOX_NUM_ROWS; i++)
		if (m->current_text_position < m->current_text_length){
			for (j = 0; j < TEXTBOX_NUM_COLS - 1; j++){
				c = getc(m->text_fp);
				if (c == '\n' || c == EOF) 
					c = ' ';
				else if (m->current_text_position >= m->current_text_length){
					c = '\0';
				}
				m->writing_lines[i][j] = c;
				m->current_text_position++;
			}
			m->writing_lines[i][TEXTBOX_NUM_COLS - 1] = '\0';
		}
		else{
			m->writing_lines[i][0] = '\0';
		}
}

int can_walk_here(Map *m, int location)
{
	int i;
	for (i = 0; i < NUM_NPCS; i++)
		if (m->NPCs[i].location == location 
		 && m->current_map == m->NPCs[i].resident_map)
			return 0;
	return 1;
}

int Map_update(Map *m, int vert, int horizontal, int confirm, int deny)
{
	if (confirm)
	{
		if (m->text_active){
			if (m->current_text_position >= m->current_text_length){
				m->text_active = 0;
				return m->currently_talking_to.chess_engine;
			}
			else
				load_text_into_boxes(m);		
		}
		else { 
			int adjacent_npc = -1;

			int i, j, mvmt;
			for (j = 0; j < 2; j++)
				for (mvmt = -1; mvmt < 2; mvmt += 2){
					int x_mvmt = j == 0 ? mvmt : 0;
					int y_mvmt = j == 1 ? mvmt : 0;
					int check_x = (m->player_location % NUM_TILES_H) + x_mvmt;
					int check_y = (m->player_location / NUM_TILES_H) + y_mvmt;
					int check = check_x + (NUM_TILES_H * check_y);
					for (i = 0; i < NUM_NPCS; i++)
						if (m->NPCs[i].location == check	
							 && m->current_map == m->NPCs[i].resident_map)
							adjacent_npc = i;
				}

			if (adjacent_npc != -1){
				NPC npc = m->NPCs[adjacent_npc];
				m->currently_talking_to = npc;
				m->current_text_length = npc.text_length;
				fseek(m->text_fp, npc.text_start_position, SEEK_SET);

				m->text_active = 1;
				m->current_text_position = 0;
				load_text_into_boxes(m);
			}
		}
	}
	else {
		int new_player_x = (m->player_location % NUM_TILES_H) + horizontal;
		if (new_player_x < 0)				  new_player_x = 0;
		else if (new_player_x >= NUM_TILES_W) new_player_x = NUM_TILES_W - 1;
		int new_player_y = (m->player_location / NUM_TILES_H) + vert;
		if (new_player_y < 0)				  new_player_y = 0;
		else if (new_player_y >= NUM_TILES_H) new_player_y = NUM_TILES_H - 1;

		int new_location = new_player_x + (NUM_TILES_H * new_player_y);
		if (can_walk_here(m, new_location))
			m->player_location = new_location;

		if (m->player_location == m->exit_tile)
			Map_changeto(m, m->next_map);
	}

	if (deny){
		m->player_spritenum++;
		if (m->player_spritenum >= NUM_SPRITES_CURRENTLY)
			m->player_spritenum = 0;
	}

	return -1;
}
