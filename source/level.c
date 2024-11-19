#include "ppu.h"
#include "assert.h"
#include "player.h"
#include "object.h"
#include "trigger.h"
#include "data/zones.h"

#include "levels/levels.h"

// convert 0-7 to the equivalent bit
const u8 one_shift[8] = {
    0b00000001, // 0 bit
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000  // 7 bit
};

// buf = pMapData + screen_to_tile_offset[MSB(x16)];
#define SCREEN_TO_TILE(x16, buf) { \
	__asm__("lda %v+1", x16); \
	__asm__("asl"); \
	__asm__("tay"); \
	__asm__("lda %v,y", screen_to_tile_offset); \
	__asm__("clc"); \
	__asm__("adc %v", pMapData); \
	__asm__("sta %v", buf); \
	__asm__("lda %v+1,y", screen_to_tile_offset); \
	__asm__("adc %v+1", pMapData); \
	__asm__("sta %v+1", buf); \
}

// buf += column_to_tile_offset[LSB(x16) / 32];
#define COLUMN_TO_TILE(x16, buf) { \
	__asm__("lda %v", x16); \
	__asm__("lsr"); \
	__asm__("lsr"); \
	__asm__("lsr"); \
	__asm__("lsr"); \
	__asm__("lsr"); \
	__asm__("tay"); \
	__asm__("lda %v,y", column_to_tile_offset); \
	__asm__("clc"); \
	__asm__("adc %v", buf); \
	__asm__("sta %v", buf); \
	__asm__("lda #0"); \
	__asm__("adc %v+1", buf); \
	__asm__("sta %v+1", buf); \
}

#define WORLD_TO_TILE(x16, buf) { \
	SCREEN_TO_TILE(x16, buf); \
	COLUMN_TO_TILE(x16, buf); \
}

#define LEVEL_LOAD_INITIAL_OFFSET 56 // how much of the level to load initial
#define RLE_UPPER_LIMIT 0xE0

extern u8 screen_buffer[];
extern u8 tile_buffer[];
extern u8 attr_buffer[];
// This holds the vertical column index currently being rendered (0-63)
extern u8 level_col;

// temporary variables (be careful calling other functions with these variables!)
#define i tmp1
#define index tmp2
#define offset tmp3
#define metatile_val tmp4

extern u8 level_state;
extern u8 level_pages;
extern u8 level_zone;

extern u8 next_obj_page;
extern u8 next_obj_pos;

void trigger_save_vars(void);

#define LEVEL_PTR(lookup, pointer) \
	__asm__("lda %v", level_current); \
	__asm__("asl"); \
	__asm__("tay"); \
	__asm__("lda %v,y", lookup); \
	__asm__("sta %v", pointer); \
	__asm__("lda %v+1,y", lookup); \
	__asm__("sta %v+1", pointer);

const u16 screen_to_tile_offset[] = {
	0,
	1 * SCREEN_COLS * SCREEN_ROWS,
	2 * SCREEN_COLS * SCREEN_ROWS,
	3 * SCREEN_COLS * SCREEN_ROWS,
	4 * SCREEN_COLS * SCREEN_ROWS,
	5 * SCREEN_COLS * SCREEN_ROWS,
	6 * SCREEN_COLS * SCREEN_ROWS,
	7 * SCREEN_COLS * SCREEN_ROWS,
	8 * SCREEN_COLS * SCREEN_ROWS,
	9 * SCREEN_COLS * SCREEN_ROWS,
	10 * SCREEN_COLS * SCREEN_ROWS,
	11 * SCREEN_COLS * SCREEN_ROWS,
	12 * SCREEN_COLS * SCREEN_ROWS,
	13 * SCREEN_COLS * SCREEN_ROWS,
	14 * SCREEN_COLS * SCREEN_ROWS,
	15 * SCREEN_COLS * SCREEN_ROWS,
	16 * SCREEN_COLS * SCREEN_ROWS,
	17 * SCREEN_COLS * SCREEN_ROWS,
	18 * SCREEN_COLS * SCREEN_ROWS,
	19 * SCREEN_COLS * SCREEN_ROWS,
	20 * SCREEN_COLS * SCREEN_ROWS,
	21 * SCREEN_COLS * SCREEN_ROWS,
	22 * SCREEN_COLS * SCREEN_ROWS,
	23 * SCREEN_COLS * SCREEN_ROWS,
};

const u8 column_to_tile_offset[] = {
	0,
	1 * SCREEN_ROWS,
	2 * SCREEN_ROWS,
	3 * SCREEN_ROWS,
	4 * SCREEN_ROWS,
	5 * SCREEN_ROWS,
	6 * SCREEN_ROWS,
	7 * SCREEN_ROWS,
};


#define COLUMN_WIDTH 32

#define MAX_SCROLL_SPEED 3

void camera_clamp_speed(void) {
    // only warp camera when loading a level
    if (game_state == GAME_STATE_PLAY) {
        if (tmp1 <= MAX_SCROLL_SPEED) {
            // do nothing
        } else {
            tmp1 = MAX_SCROLL_SPEED;
        }
    }
}

void camera_move(void)
{
	if (player_x < PLAYER_SCREEN_LEFT)
	{
		tmp1 = PLAYER_SCREEN_LEFT - player_x;
		tmp2 = (MSB(camera_x) > camera_page_min) ? 1 : 0;

        if (tmp2 || LSB(camera_x) >= tmp1) {
            camera_clamp_speed();
			camera_x = camera_x - tmp1;
			level_x = level_x - tmp1;
			player_x += tmp1;
		} else if (MSB(camera_x) == 0) { // far left page
			// check if player leaves left exit
			if (player_x <= 4 && BIT_ON(level_exit, LEVEL_EXIT_L)) {
				level_last = LEVEL_LAST_LEFT_EXIT;
				--level_current;
				game_fade_to_black();
			}
		}
	}
	else if (player_x > PLAYER_SCREEN_RIGHT)
	{
		tmp1 = player_x - PLAYER_SCREEN_RIGHT;
		tmp2 = (MSB(camera_x) < camera_page_max) ? 1 : 0;

        if (tmp2 || LSB(camera_x) >= tmp1) {
            camera_clamp_speed();
			camera_x += tmp1;
			level_x += tmp1;
            player_x = player_x - tmp1;
		} else if (MSB(camera_x) == level_pages) { // far right page
			// check if players leaves right exit
			if (player_x > (256-PLAYER_WIDTH-4) && BIT_ON(level_exit, LEVEL_EXIT_R)) {
				level_last = LEVEL_LAST_RIGHT_EXIT;
				++level_current;
				game_fade_to_black();
			}
		}
	}

	// check if boundary condition is met and should clamp to page boundary
	if (tmp2 == 0 && LSB(camera_x) < 3) {
		camera_x &= 0xFF00;
		level_x = 0;
	}
}

void camera_setup(void) {
    if (BIT_ON(level_flags, LEVEL_FLAGS_PAGE_LOCK)) {
        tmp1 = player_x;
		player_x += LSB(camera_x);
        camera_x += tmp1; // offset to player first then truncate
		__asm__("lda #0");
		__asm__("sta %v", camera_x);
		camera_page_min = MSB(camera_x);
        camera_page_max = camera_page_min;
    }

	camera_move();
}

void finalize_get_tile(void) {
	__asm__("ldy %v", tmp4);
	// tile_type = pMetatileAttr[tmp4] & MT_TYPE_MASK;
	__asm__("lda (%v),y", pMetatileAttr);
	__asm__("tay");
	__asm__("and #%b", MT_TYPE_MASK);
	__asm__("sta %v", tile_type);
	// tile_liquid = pMetatileAttr[tmp4] & MT_TYPE_LIQUID;
	__asm__("tya");
	__asm__("and #%b", MT_TYPE_LIQUID);
	__asm__("sta %v", tile_liquid);

    tile_type = one_shift[tile_type];
	bank_restore();
}

void world_to_tile(void) {
	WORLD_TO_TILE(tile_get_x, pTilePtr);
}

void get_tile(void)
{
    // only check for tile if it's within the level data
	if (MSB(tile_get_x) <= level_pages) {

        // clamp to the bottom tile if below the screen
        if (tile_get_y > 204) {
            tile_get_y = 204;
        }

		world_to_tile();
		pTilePtr += tile_get_y / 32; // add 0-7

		// get offset based on even/odd 16 pixel column
		offset = (LSB(tile_get_x) & 16) >> 3;
		// check if y in the bottom half of the 32x32 megatile
		if (tile_get_y & 16) {
			++offset; // add 1 to offset for bottom tile
		}
		bank_switch_save(level_bank);
		tmp4 = load_megatile(offset);
		finalize_get_tile();
	} else {
		// tiles outside the level data should be considered solid
		tile_type = MT_SOLID;
	}
}

// This function gets the next tile below the last checked
// MUST not modify tmp3 or tmp4 for this to work
// ALSO this bypasses offscreen checks
void get_tile_next_y(void) {
	bank_switch_save(level_bank);
	// if offset is odd, move to the next megatile down
	if (offset & 1) {
		++pTilePtr;
		tmp4 = load_megatile(offset & 2); // top left or right side
	} else {
		__asm__("ldy %v", offset);
		__asm__("iny"); // ++offset
		__asm__("lda (ptr4),y"); // get tile data
		__asm__("sta %v", tmp4);
	}
	finalize_get_tile();
}

// MUST match order of LEVEL_FLAGS_EXIT_* values
const u8 level_exits[8] = {
	LEVEL_EXIT_U,
	LEVEL_EXIT_D,
	LEVEL_EXIT_L,
	LEVEL_EXIT_R,
	LEVEL_EXIT_U|LEVEL_EXIT_D,
	LEVEL_EXIT_L|LEVEL_EXIT_R,
	LEVEL_EXIT_U|LEVEL_EXIT_L,
	LEVEL_EXIT_D|LEVEL_EXIT_R,
};

// Gets pointer to data from lookup and stores in pMapData
// assumes no more than 128 levels because of left shift
void init_map_pointers(void)
{
	assert(level_current != LEVEL_INVALID);
	level_bank = level_banks[level_current] & 7;

	bank_switch(level_bank);

    // grab the info pointer and put in ptr1 to read
    __asm__("lda %v", level_current);
    __asm__("asl");
    __asm__("tay");

    __asm__("lda %v,y", level_map_info);
    __asm__("sta ptr1");
    __asm__("lda %v+1,y", level_map_info);
    __asm__("sta ptr1+1");

    // read header bytes
    __asm__("ldy #0");
    __asm__("lda (ptr1),y");
    __asm__("tax");
    __asm__("inx");
    __asm__("stx %v", tmp1); // used to check number of pages
    __asm__("sta %v", level_pages);
    __asm__("iny");

    __asm__("lda (ptr1),y");
    __asm__("sta %v", level_zone);
    __asm__("iny");

    __asm__("lda (ptr1),y");
    __asm__("sta %v", level_flags);

    __asm__("ldx #0");
load_map_screens: // offsets for map in pause screen
    __asm__("iny");
    __asm__("lda (ptr1),y");
    __asm__("sta %v,x", map_screens);

    __asm__("inx");
    __asm__("cpx %v", tmp1);
    __asm__("bne %g", load_map_screens);

    // add bytes read to ptr and store in pMapData
    __asm__("iny"); // increment final time to get to block data
    __asm__("tya");
    __asm__("clc");
    __asm__("adc ptr1");
    __asm__("sta %v", pMapData);
    __asm__("lda #0");
    __asm__("adc ptr1+1");
    __asm__("sta %v+1", pMapData);

	if (BIT_ON(level_flags, LEVEL_FLAGS_EXIT)) {
		tmp1 = level_flags & LEVEL_FLAGS_EXIT_MASK;
		level_exit = level_exits[tmp1];
	} else {
		level_exit = 0;
	}

    megatile_unrle();

    // get ready to load graphics
	bank_switch(BANK_GFX);
}

void load_tile_column(void)
{
	bank_switch(level_bank);
	for (i = 0; i < SCREEN_ROWS*2; i += 2)
	{
		load_megatile_buffer(i);
	}

	// set attribute buffer
	for (i = 0; i < SCREEN_ROWS; ++i) {
    #if 0
		index = i*2;
		offset = tile_buffer[index];
		metatile_val = pMetatileAttr[offset] >> 6;

		++index;
		offset = tile_buffer[index];
		metatile_val |= (pMetatileAttr[offset] & MT_PAL_MASK) >> 2;

		index += ((SCREEN_ROWS * 2) - 1);
		offset = tile_buffer[index];
		metatile_val |= (pMetatileAttr[offset] & MT_PAL_MASK) >> 4;

		++index;
		offset = tile_buffer[index];
		offset = pMetatileAttr[offset];
		metatile_val |= offset & MT_PAL_MASK;
		attr_buffer[i] = metatile_val;
    #else
        __asm__("lda %v", i);
        __asm__("asl");
        __asm__("tax");
        __asm__("ldy %v,x", tile_buffer);
        __asm__("lda (%v),y", pMetatileAttr);
        __asm__("lsr");
        __asm__("lsr");
        __asm__("lsr");
        __asm__("lsr");
        __asm__("sta %v", metatile_val);

        __asm__("inx");
        __asm__("ldy %v,x", tile_buffer);
        __asm__("lda (%v),y", pMetatileAttr);
        __asm__("and #%b", (u8)MT_PAL_MASK);
        __asm__("ora %v", metatile_val);
        __asm__("sta %v", metatile_val);

        __asm__("txa");
        __asm__("clc");
        __asm__("adc #%b", (u8)((SCREEN_ROWS*2)-1));
        __asm__("tax");
        __asm__("ldy %v,x", tile_buffer);
        __asm__("lda (%v),y", pMetatileAttr);
        __asm__("and #%b", (u8)MT_PAL_MASK);
        __asm__("lsr");
        __asm__("lsr");
        __asm__("ora %v", metatile_val);
        __asm__("lsr");
        __asm__("lsr");
        __asm__("sta %v", metatile_val);

        __asm__("inx");
        __asm__("ldy %v,x", tile_buffer);
        __asm__("lda (%v),y", pMetatileAttr);
        __asm__("and #%b", (u8)MT_PAL_MASK);
        __asm__("ora %v", metatile_val);
        __asm__("ldy %v", i);
        __asm__("sta %v,y", attr_buffer);
    #endif

	}
}

#define VRAM_TILES_PER_COLUMN (SCREEN_ROWS * 4)

/**
 * @modifies i, offset(tmp2), tmp4
 */
void fill_vram_buffer_with_tiles(void)
{
	bank_switch(level_bank);
	vram_buffer[0] = ((level_col & 32) >> 3) | (0x20 | NT_UPD_VERT);
	vram_buffer[1] = level_col % 32;
	vram_buffer[2] = VRAM_TILES_PER_COLUMN+2;
	for (i = 0; i < VRAM_TILES_PER_COLUMN; ++i)
	{
		tmp2 = i / 2;
		if ((level_col & 0x02) != 0) {
			tmp2 += SCREEN_ROWS*2;
		}
		// get the tile
		tmp2 = tile_buffer[tmp2];

		tmp4 = 0;
		// rotate upper two bits of tmp2 into tmp4
		__asm__("lda %v", tmp2);
		__asm__("asl");
		__asm__("rol %v", tmp4);
		__asm__("asl");
		__asm__("rol %v", tmp4);
		__asm__("sta %v", tmp2);

		tmp2 += (level_col % 2) + ((i % 2) * 2);

		__asm__("lda %v", pMetatileData);
		__asm__("sta ptr1+0");
		// add tmp4 to pMetatileData high byte
		__asm__("lda %v", tmp4);
		__asm__("clc");
		__asm__("adc %v+1", pMetatileData);
		__asm__("sta ptr1+1");
		// load the tile from pMetatileData[tmp2]
		__asm__("ldy %v", tmp2);
		__asm__("lda (ptr1),y");
		__asm__("sta %v", tmp2);

		tmp4 = i + 3;
		vram_buffer[tmp4] = tmp2;
	}
	// extend the bottom
	vram_buffer[VRAM_TILES_PER_COLUMN+3] = vram_buffer[VRAM_TILES_PER_COLUMN+1];
	vram_buffer[VRAM_TILES_PER_COLUMN+4] = vram_buffer[VRAM_TILES_PER_COLUMN+2];
	vram_buffer_end = VRAM_TILES_PER_COLUMN + 2 + 3;
}

/**
 * @modifies offset
 */
void fill_vram_attributes(void)
{
	offset = ((level_col / 4) % 8) | 0xC0;
    __asm__("ldy #0");
    __asm__("ldx #0");

attr_loop:
    __asm__("iny");
    __asm__("lda %v", offset);
    __asm__("sta %v,y", vram_buffer); // vram_buffer[n+1] = offset
    __asm__("clc");
    __asm__("adc #8");
    __asm__("sta %v", offset);

    __asm__("iny");
    __asm__("lda %v,x", attr_buffer);
    __asm__("sta %v,y", vram_buffer); // vram_buffer[n+2] = attr_buffer[x]

    __asm__("iny");
    __asm__("inx");
    __asm__("cpx #7");
    __asm__("bne %g", attr_loop);

	// set bottom attribute
	vram_buffer[22] = offset;
	tmp4 = attr_buffer[6];
	vram_buffer[23] = tmp4 >> 4;

	vram_buffer[0] = vram_buffer[3] = vram_buffer[6] = vram_buffer[9] =
		vram_buffer[12] = vram_buffer[15] = vram_buffer[18] = vram_buffer[21] =
		((level_col & 32) >> 3) | 0x23;
	vram_buffer_end = 8 * 3;
}

void vram_column(void)
{
	fill_vram_buffer_with_tiles();
	set_vram_update(vram_buffer);
    ++level_col;
	level_col = level_col % 64;
}

void vram_attributes(void)
{
	fill_vram_attributes();
	set_vram_update(vram_buffer);
}

/**
 * @modifies i
 */
void calc_level_byte_offset(void) {
	WORLD_TO_TILE(camera_x, pLevelBuffer);
}

// run gfx routines for zone
void zone_gfx(void) {
	bank_switch(gfx_bank[level_zone]);
    LUT_CALL(gfx_routines, level_zone);
    bank_switch(BANK_GFX); // finally switch back to GFX bank
}

/**
 * @modifies i, level_x, pLevelBuffer, level_col
 */
void hold_state(void)
{
	if (level_x > 200) {
		// scroll left
		level_x += COLUMN_WIDTH;
		calc_level_byte_offset();
		pLevelBuffer -= SCREEN_ROWS*2;
		level_col = ((LSB(camera_x) / 8) - 11) % 64;
		if (MSB(camera_x) & 1) {
			level_col = (level_col + 32) % 64;
		}
	} else if (level_x >= COLUMN_WIDTH) {
		// scroll right
		level_x -= COLUMN_WIDTH;
		calc_level_byte_offset();
		pLevelBuffer += SCREEN_ROWS * (SCREEN_COLS + 1);
		level_col = ((LSB(camera_x) / 8) + 36) % 64;
		if (MSB(camera_x) & 1) {
			level_col = (level_col + 32) % 64;
		}
	} else {
		--level_state;
	}
    zone_gfx();
}

void level_load_initial()
{
	level_x = LSB(camera_x) % COLUMN_WIDTH;
	calc_level_byte_offset();
	pLevelBuffer -= SCREEN_ROWS*2;

	// calculate level column (0-63)
	tmp1 = (LSB(camera_x) % 32) / 8;
	level_col = ((LSB(camera_x) / 8) - tmp1 - 8);
	if (MSB(camera_x) & 1) { // even or odd page
		level_col = (level_col + 32);
	}
	level_col %= 64;

	tmp5 = LEVEL_LOAD_INITIAL_OFFSET / 4;
	while (--tmp5) {
		load_tile_column();
		fill_vram_attributes();
		flush_vram();

		fill_vram_buffer_with_tiles();
		flush_vram();
		++level_col;

		fill_vram_buffer_with_tiles();
		flush_vram();
		++level_col;

		fill_vram_buffer_with_tiles();
		flush_vram();
		++level_col;

		fill_vram_buffer_with_tiles();
		flush_vram();
		++level_col;
	}

	// run initial gfx call for zone
    level_state = 3; // MUST be graphics update routine
    level_update();
	flush_vram(); // MUST flush vram after level_update
}

void level_load_objects(void)
{
	object_init();

	bank_switch(level_bank);
	LEVEL_PTR(level_objs, pTemp);
	next_obj_page = 0;
	obj_id = 0;
	trigger_id = 0;

	while (TRUE)
	{
        bank_switch(level_bank);
		tmp1 = 2; // num bytes processed
		DEREF_PTR_INTO(pTemp, 0, next_obj_pos);
		if (next_obj_pos == OBJ_TYPE_NONE) break;

		DEREF_PTR_INTO(pTemp, 1, object_type);

		// check if object is on the next page
		if (BIT_ON(object_type, OBJ_NEW_PAGE)) {
			if (next_obj_page < level_pages)
			{
				next_obj_page += 1;
			}
		}

		// check if the object has extra data
		if (BIT_ON(object_type, OBJ_EXTRA_DATA)) {
			__asm__("ldy %v", tmp1);
			__asm__("lda (%v),y", pTemp);
			__asm__("sta %v", object_state);
			++tmp1;
		} else {
			object_state = 0;
		}

		pTemp += tmp1; // advance by so many bytes

		// x value is store in two parts (high/low)
		__asm__("lda %v", next_obj_page);
		__asm__("sta %v+1", object_x);
		__asm__("lda %v", next_obj_pos);
		__asm__("and #$F0");
		__asm__("sta %v", object_x);

		// y value is in 16 pixel increments
		object_y = (next_obj_pos & 0xF) << 4;

		// clear the upper bits of object_type
		BIT_CLEAR(object_type, OBJ_NEW_PAGE|OBJ_EXTRA_DATA);

		object_timer = 0;
        bank_switch(BANK_OBJECTS);
		// if the object type is empty, keep the same obj_id
        if (object_type < (MAX_OBJECT_ID - TRIGGER_NUM)) {
            // don't load more than max objects
            if (obj_id == MAX_OBJECTS) {
                #ifdef DEBUG
                __asm__("brk");
                #else
                continue;
                #endif
            }
            OBJ_CALL(spawn_routines);
            if (object_type == OBJ_TYPE_NONE) continue;
            object_save_vars();
            ++obj_id; // increment the object id
        } else {
            // don't load more than max triggers
            if (trigger_id == MAX_TRIGGERS) {
                #ifdef DEBUG
                __asm__("brk");
                #else
                continue;
                #endif
            }
            // get correct type by calculating from top of object ids
            __asm__("lda %v", object_type);
            __asm__("sec");
            __asm__("sbc #%b", MAX_OBJECT_ID);
            __asm__("eor #$FF");
            __asm__("sta %v", object_type);
            __asm__("inc %v", object_type);
            OBJ_CALL(trigger_spawn_routines);
            trigger_save_vars();
            ++trigger_id;
		}
	}
}

const callbackDef level_updaters[] = {
	hold_state,       // check if we should draw the next column
	load_tile_column, // first metatile column
	vram_attributes,  // 32x32 attribute column
	zone_gfx,         // update graphics for zone
	vram_column,      // first 8 pixel column
	vram_column,      // second 8 pixel column
	vram_column,      // third 8 pixel column
	vram_column,      // last 8 pixel column
};

void level_update(void)
{
	level_state = level_state % 8;
	LUT_CALL(level_updaters, level_state);
    ++level_state;
	tmp2 = tmp1 = 0;
	if (camera_shake) {
		--camera_shake;
		tmp2 = (frame_counter & 8) >> 2;
		tmp1 = (frame_counter & 4) >> 1;
	}
	scroll(camera_x + tmp1, tmp2);
}

void level_load_start(void) {
    // clear sprites to avoid single frame flashes after transition
    oam_clear();
	ppu_off();

	init_map_pointers(); // load map data

	level_next = LEVEL_INVALID; // set to invalid next level to prevent instant respawns

	// load tileset
	load_level_gfx();

	level_load_objects();
}

void level_load_camera(void) {
	camera_page_min = 0;
	camera_page_max = level_pages;

	if (MSB(camera_x) >= camera_page_max) {
		player_x += LSB(camera_x);
		camera_x = camera_page_max*256;
	}
	switch (level_last) {
		case LEVEL_LAST_UP_EXIT:
			player_y = (SCREEN_HEIGHT-(PLAYER_HEIGHT*2));
			break;
		case LEVEL_LAST_DOWN_EXIT:
			player_y = 0;
			break;
		case LEVEL_LAST_RIGHT_EXIT:
			player_x = 16;
            // fallthrough
        case LEVEL_INVALID:
			camera_x = 0;
			break;
		case LEVEL_LAST_LEFT_EXIT:
			camera_x = level_pages << 8;
			player_x = 256-(PLAYER_WIDTH*2);
			break;
	}

	camera_setup();
}

void level_load_finish(void) {
	level_load_initial();

	level_state = 0;
	frame_counter = 0;
    camera_shake = 0;

    music_play(MUSIC_BG);

	ppu_on_all();
}

void level_init(void) {
    level_load_start();
    level_load_camera();
    level_load_finish();
}
