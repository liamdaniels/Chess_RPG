CC = clang

CFLAGS = -Wall -Werror 
CFLAGS2 = -ansi -c

SDLOBJ = ../../my_API/sdl/sdl_util.o

TOTALOBJ = game_main.o map.o chess_display.o chess.o chess_bot.o

all: GAME

GAME: $(TOTALOBJ) $(SDLOBJ)
	$(CC) $(CFLAGS) $(TOTALOBJ) $(SDLOBJ) -o GAME -lSDL2 -lSDL2_image

game_main.o: game_main.c
	 $(CC) $(CFLAGS) $(CFLAGS2) game_main.c 

map.o: map.c
	 $(CC) $(CFLAGS) $(CFLAGS2) map.c 

chess_display.o: chess_display.c
	 $(CC) $(CFLAGS) $(CFLAGS2) chess_display.c 

chess_bot.o: chess_bot.c
	 $(CC) $(CFLAGS) $(CFLAGS2) chess_bot.c 

chess.o: chess.c
	 $(CC) $(CFLAGS) $(CFLAGS2) chess.c 

clean:
	rm *.o GAME
