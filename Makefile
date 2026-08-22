.PHONY: all pc vita clean

VITA_CC := arm-vita-eabi-gcc

VITA_TITLE := Portalban
VITA_APP_VER := 01.00
VITA_TITLEID := IVOA00007

LIBS := sdl3

EXE := portalban
CFLAGS += $(shell pkg-config --cflags $(LIBS))
LDFLAGS += $(shell pkg-config --libs $(LIBS))
VITA_CFLAGS += -Wl,-q -std=gnu17 $(shell arm-vita-eabi-pkg-config --cflags $(LIBS))
VITA_LDFLAGS += -Wl,-q -std=gnu17 -Wl,-z,nocopyreloc $(shell arm-vita-eabi-pkg-config --libs $(LIBS))

SOURCES := $(wildcard src/*.c)

OBJS := $(addprefix pc_build/, $(SOURCES:src/%.c=%.o))
VITA_OBJS := $(addprefix vita_build/, $(SOURCES:src/%.c=%.o)) $(VITASDK)/arm-vita-eabi/lib/libSDL3.a

all: pc vita
pc: $(EXE)
vita: $(EXE).vpk

$(EXE): $(OBJS)
	$(CC) -g $^ $(LDFLAGS) -o $@

pc_build/%.o: src/%.c | pc_build
	$(CC) -c $(CFLAGS) -o $@ $<

$(EXE).vpk: vita_build/$(EXE).self vita_build/param.sfo sce_sys levels sprites
	vita-pack-vpk -s vita_build/param.sfo -b vita_build/$(EXE).self \
		--add sce_sys/icon0.png=sce_sys/icon0.png \
		--add sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
		--add levels=levels \
		--add sprites=sprites \
		$(EXE).vpk

vita_build/$(EXE).self: vita_build/$(EXE).velf
	vita-make-fself -c -s $< $@

vita_build/param.sfo:
	vita-mksfoex \
		-s APP_VER="$(VITA_APP_VER)" \
		-s TITLE_ID="$(VITA_TITLEID)" \
		"$(VITA_TITLE)" vita_build/param.sfo

vita_build/$(EXE).velf: vita_build/$(EXE).elf
	vita-elf-create $< $@

vita_build/$(EXE).elf: $(VITA_OBJS)
	$(VITA_CC) -g $^ $(VITA_LDFLAGS) -o $@

%_build:
	mkdir -p $@

vita_build/%.o : src/%.c | vita_build
	$(VITA_CC) -c $(VITA_CFLAGS) -o $@ $<

clean:
	rm -rf $(EXE) $(EXE).vpk *_build
