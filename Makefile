SRCDIR=source
OUTDIR=build
OBJDIR=$(OUTDIR)/source
LIBDIR=$(OUTDIR)/lib

NAME=game
MAPPER=unrom
CONFIG=$(MAPPER)
NES=$(OUTDIR)/$(NAME).nes
EMULATOR=fceux
CMAKE_BUILD=Debug
CMAKE_SYSTEM=NES

PYTHON:=$(shell pyenv which python || python)
LABELFILE=$(NES:.nes=.lbl)

.PHONY: all run run-debug run-release clean assets stats build release debug profile

all: clean stats

run: run-debug

run-debug: EMULATOR+=--inputdisplay 1 --loadlua fceux-debug.lua
run-debug: clean debug
	$(EMULATOR) $(NES)

run-release: EMULATOR+=--inputdisplay 0 --loadlua fceux-debug.lua
run-release: clean release
	$(EMULATOR) $(NES)

release: CMAKE_BUILD=Release
release: build stats

stats: build
	$(PYTHON) tools/nes_stats.py $(NES)

# build assertions
debug: build
	$(PYTHON) tools/labeltonl.py $(LABELFILE)

# compiler profiling code with debug
profile: CMAKE_BUILD=Profile
profile: clean build
	$(EMULATOR) $(NES)

build: clean assets
	cmake -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD) -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM} -B $(OUTDIR) -G Ninja
	cmake --build $(OUTDIR)

clean:
	rm -rf $(OUTDIR)

assets:
	$(MAKE) -j10 -C assets

