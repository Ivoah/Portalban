pban: src/main.c src/Vec2.h src/Vec2.c src/Level.h src/Level.c
	clang -g -fsanitize=address src/main.c src/Vec2.c src/Level.c `pkg-config --cflags --libs sdl3` -o pban

PHONY := all package clean
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC := arm-vita-eabi-gcc
CXX := arm-vita-eabi-g++
STRIP := arm-vita-eabi-strip

PROJECT_TITLE := Portalban
PROJECT_TITLEID := IVOA00007

PROJECT := portalban
CFLAGS += -Wl,-q -g $(shell arm-vita-eabi-pkg-config --cflags sdl3)

SRC_C :=$(call rwildcard, src/, *.c)

OBJ_DIR := out/
OBJS := $(addprefix out/, $(SRC_C:src/%.c=%.o))

LIBS += $(shell arm-vita-eabi-pkg-config --libs sdl3)

all: package

package: $(PROJECT).vpk

$(PROJECT).vpk: eboot.bin param.sfo sce_sys/icon0.png sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/template.xml levels sprites
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add sce_sys/icon0.png=sce_sys/icon0.png \
		--add sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
		--add levels=levels \
		--add sprites=sprites \
	$(PROJECT).vpk

eboot.bin: $(PROJECT).velf
	vita-make-fself $(PROJECT).velf eboot.bin

param.sfo:
	vita-mksfoex -s TITLE_ID="$(PROJECT_TITLEID)" "$(PROJECT_TITLE)" param.sfo

$(PROJECT).velf: $(PROJECT).elf
	$(STRIP) -g $<
	vita-elf-create $< $@

$(PROJECT).elf: $(OBJS)
	$(CC) -g $^ $(LIBS) -o $@

$(OBJ_DIR):
	mkdir -p $@

out/%.o : src/%.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROJECT).velf $(PROJECT).elf $(PROJECT).vpk param.sfo eboot.bin
	rm -rf $(abspath $(OBJ_DIR))
