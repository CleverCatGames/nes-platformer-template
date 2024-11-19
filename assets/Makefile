OUTDIR=../build/assets
GFX_ASSETS=$(shell find gfx/ -type f -name "*.png")
TS_ASSETS=$(filter-out tilesets/objects.tsx, $(wildcard tilesets/*.tsx))
CSV_ASSETS=$(shell find data/ -type f -name "*.csv")
DLG_ASSETS=$(shell find data/ -type f -name "*.dlg")
LVL_ASSETS=$(shell find levels/ -type f -name "*.lvl")

CHR=$(addprefix $(OUTDIR)/, $(GFX_ASSETS:.png=.h))
METATILES=$(addprefix $(OUTDIR)/, $(TS_ASSETS:.tsx=.h))
CSV_DATA=$(addprefix $(OUTDIR)/, $(CSV_ASSETS:.csv=.h))
LEVELS=$(addprefix $(OUTDIR)/, $(LVL_ASSETS:.lvl=.h))

PYTHON:=$(shell pyenv which python || python)
MKDIR=@mkdir -p $(dir $@)

IMG2CHR=$(PYTHON) ../tools/img2chr.py
TSX2MT=$(PYTHON) ../tools/tsx2metatile.py
CSV2H=$(PYTHON) ../tools/csv2h.py
LVL2H=$(PYTHON) ../tools/lvl2h.py

.PHONY: all
.SUFFIXES: .tsx .tmx .png .csv .lvl .csv .txt

all: $(CHR) $(METATILES) $(CSV_DATA) $(LEVELS)

$(OUTDIR)/tilesets/%.h: tilesets/%.tsx
	$(MKDIR)
	$(TSX2MT) -r $< $@

$(OUTDIR)/gfx/%.h: gfx/%.png
	$(MKDIR)
	$(IMG2CHR) -r $< $@

$(OUTDIR)/levels/%.h: levels/%.lvl
	$(MKDIR)
	$(LVL2H) $< $@

$(OUTDIR)/data/%.h: data/%.csv
	$(MKDIR)
	$(CSV2H) $< $@

$(OUTDIR)/title: title/title.c
	gcc $< -o $@

