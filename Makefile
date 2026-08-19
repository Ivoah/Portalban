pban: src/main.c src/Vec2.h src/Vec2.c src/Level.h src/Level.c
	clang -g -fsanitize=address src/main.c src/Vec2.c src/Level.c `pkg-config --cflags --libs sdl3 sdl3-ttf` -o pban
