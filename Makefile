pban: src/main.c src/Vec2.c src/Level.c
	cc src/main.c src/Vec2.c src/Level.c `pkg-config --cflags --libs sdl3` -o pban
