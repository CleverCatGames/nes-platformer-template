#include "game.h"
#include "level.h"
#include "player.h"
#include "object.h"
#include "sfx.h"

extern u8 pad;

extern u8 player_frame;
extern u8 player_frame_speed;
extern u8 player_animation;
extern u8 player_animation_len;
extern u8 player_health;
extern u8 player_health_max;
extern u8 player_tile;

#define HEALTH_MAXIMUM 16 // absolute upper limit of health
#define HEALTH_INITIAL 3

#define PLAYER_JUMP_STATE (PLAYER_STATE_PLATFORM | PLAYER_STATE_ON_GROUND | PLAYER_STATE_IN_LIQUID)
#define COYOTE_JUMP_TIME (0xFF - 5)

void player_init(void)
{
    // clear out all player data
    time_hours = time_minutes = time_seconds = time_frames = 0;

    player_reset(); // do before setting health the first time

    player_health = HEALTH_INITIAL;
    player_health_max = player_health;
}

// used at initial load and when changing levels
void player_reset(void) {
    player_animation = player_animation_len = player_frame = 0;

    player_state = 0;
    player_velocity_x = player_velocity_y = 0;
    player_x = 64;
    player_y = SCREEN_HEIGHT - PLAYER_HEIGHT - 48;
}

// because this is 16 bit math, it's worth putting in a function
void player_camera_diff(void) {
    camera_diff.word = player_x + camera_x;
}

// Set animation start and length
// Speed [1-8] the higher the value the faster frames update
#define ANIM_LOOP(anim, length, speed) { \
        player_animation = anim; \
        player_animation_len = (length-1); \
        player_frame_speed = ((1 << (8 - speed)) - 1); \
    }
#define ANIM_FRAME(anim) { player_animation = anim; }

#define ANIM_IDLE     ANIM_FRAME(0)
#define ANIM_WALK     ANIM_LOOP(0, 4, 5)
#define ANIM_SLIDE    ANIM_FRAME(0)
#define ANIM_SWIM     ANIM_LOOP(0, 4, 4)
#define ANIM_FALL     ANIM_FRAME(0)
#define ANIM_FLOAT    ANIM_FRAME(1)
#define ANIM_JUMP     ANIM_FRAME(4)
#define ANIM_DOOR     ANIM_FRAME(0)

#define SPRITE(x) (SPRITE_PLAYER + (x * 4))
const u8 player_body[] = {
    SPRITE(0), // idle
    SPRITE(1), SPRITE(0), SPRITE(2), // walk
    SPRITE(3), // fall
};

void draw_slide_dust(void) {
    if (BIT_ON(player_state, PLAYER_STATE_LEFT)) {
        oam_x = player_x+16;
    } else {
        oam_x = player_x-8;
    }
    oam_y = player_y+(PLAYER_HEIGHT-16);
    oam_attr = OAM_PAL1 | (player_state & (OAM_BEHIND | OAM_FLIP_H));
    oam_draw(SPRITE_SLIDE_DUST);
}

void player_draw(void)
{
    player_frame_speed = player_animation_len = 0;
    if (BIT_ON(player_state, PLAYER_STATE_IN_LIQUID))
    {
        if ((level_flags & LEVEL_FLAGS_LIQUID_MASK) == LIQUID_QUICKSAND) {
            ANIM_JUMP;
        } else {
            ANIM_SWIM;
            if (player_velocity_y < 0) {
                player_frame_speed = 0x07;
            }
        }
    } else if (BIT_ON(player_state, PLAYER_STATE_PLATFORM|PLAYER_STATE_ON_GROUND)) {
        tmp1 = ABS(player_velocity_x);
        if (tmp1 < VELOCITY_X_DIV) {
            ANIM_IDLE;
        } else {
            ANIM_WALK;
            // player is sliding if moving in the opposite direction than their input
            if ((player_velocity_x < 0 && (pad & PAD_RIGHT)) ||
                (player_velocity_x > 0 && (pad & PAD_LEFT))) {
                ANIM_SLIDE;
                draw_slide_dust();
            }
        }
    } else {
        if (player_velocity_y < -6) {
            ANIM_JUMP;
        } else if (player_velocity_y > 6) {
            ANIM_FALL;
        } else {
            ANIM_FLOAT; // hang for a second
        }
    }

    // advance frame, if needed
    if ((frame_counter & player_frame_speed) == 0) {
        ++player_frame;
    }

    // check if frame is past the number of animation frames
    if (player_animation_len < player_frame) {
        player_frame = 0; // set to start frame
    }

    // count down invincible timer
    if (player_invincible_timer)
    {
        --player_invincible_timer;
        // flicker when invincible
        if (BIT_ON(player_invincible_timer, 1))
        {
            return;
        }
    }

    tmp4 = player_animation + player_frame;

    oam_x = player_x;
    if (BIT_OFF(player_state, PLAYER_STATE_OFFSCREEN) || player_y > SCREEN_HEIGHT) {
        oam_y = player_y;
        oam_attr = player_state & (OAM_BEHIND | OAM_FLIP_H);

        oam_tile = player_body[tmp4];
        oam_tile_alt = oam_tile + 2;
        oam_draw_pair();
    }
}

// must be long enough to play death music
#define PLAYER_DEATH_TIME 16

void player_kill(void)
{
    player_invincible_timer = player_health = 0;
    player_jump_timer = PLAYER_DEATH_TIME; // reuse jump timer for death frame counter

    // make sure stars show at bottom instead of wrapping
    if (player_y > (SCREEN_HEIGHT-PLAYER_HEIGHT)) {
        player_y = (SCREEN_HEIGHT-PLAYER_HEIGHT);
    }
}

void player_hurt(void)
{
    // exit it player is already dead
    if (player_health == 0) return;

    // or already hurt
    if (player_invincible_timer == 0)
    {
        // did player die?
        if (player_health <= player_damage) {
            player_kill();
        } else {
            player_health -= player_damage;
            camera_shake = 20;
            sound_play_ch0(SFX_HURT);
            player_invincible_timer = INVINCIBLE_HURT_TIME;
        }
    }
}

// do 1 damage to player
void player_hurt1(void) {
    player_damage = 1;
    player_hurt();
}

#define BOUNDS_RIGHT (SCREEN_WIDTH - PLAYER_WIDTH - 1)

void do_move_x(void)
{
    if (player_velocity_x < 0) {
        tmp1 = -player_velocity_x; // convert to unsigned
        player_x -= (tmp1 / VELOCITY_X_DIV); // divide for "sub pixel" velocity
    } else {
        // convert to unsigned and divide for "sub pixel" momentum
        player_x += ((u8)player_velocity_x / VELOCITY_X_DIV);
    }

    // clamp x position to zero if crossed around the screen
    if (player_x > (SCREEN_WIDTH - 8))
    {
        // clamp to left side of screen
        player_x = 0;
        player_velocity_x = 0;
        camera_diff.word = camera_x;
    }
    else if (player_x > BOUNDS_RIGHT)
    {
        // clamp to right side of screen
        player_velocity_x = 0;
        player_x = BOUNDS_RIGHT;
        camera_diff.word = camera_x + BOUNDS_RIGHT;
    }
    else
    {
        player_camera_diff();

        // look for a solid tile at the player position
        if (player_velocity_x < 0)
        {
            tile_get_x = camera_diff.word + COLLIDE_LEFT_OFFSET;
        }
        else
        {
            tile_get_x = camera_diff.word + COLLIDE_RIGHT_OFFSET;
        }
        if (BIT_ON(player_state, PLAYER_STATE_OFFSCREEN)) {
            // check at top of the screen
            tile_get_y = 4;
            get_tile();
        } else if (player_y > (SCREEN_HEIGHT - (PLAYER_HEIGHT*2)) && player_y < SCREEN_HEIGHT) {
            // prevent clipping from bottom of screen (specifically when swimming)
            tile_get_y = player_y;
            goto check_hurt;
        } else {
            tile_get_y = player_y + ((PLAYER_HEIGHT / 2)-2); // midpoint of player
        check_hurt:
            get_tile();
            // if no solid found, look lower
            if (MT_IS_NOT(SOLID)) {
                if (MT_IS(HURT)) {
                    player_hurt1();
                }

                // only check next tile if height is 32
                #if (PLAYER_HEIGHT == 32)
                // do a fast check of the next tile down
                get_tile_next_y();
                if (MT_IS(HURT)) {
                    player_hurt1();
                }
                #endif
            }
        }

        // when the player hits a block, clamp them to the side of it
        if (MT_IS(SOLID)) {
            BIT_SET(player_state, PLAYER_STATE_WALL);
            player_x = (LSB(camera_x) + player_x) & 0xF0;
            if (player_velocity_x < 0) {
                player_x += COLLIDE_RIGHT_OFFSET;
            } else {
                player_x += COLLIDE_LEFT_OFFSET;
            }
            player_x -= LSB(camera_x);
            player_velocity_x = 0;
        }
    }
}

void set_on_ground(void)
{
    player_y &= 0xF0; // y / 16 * 16
    player_velocity_y = VELOCITY_Y_DIV;
    BIT_SET(player_state, PLAYER_STATE_ON_GROUND);
}

void do_move_y(void)
{
    BIT_CLEAR(player_state, PLAYER_STATE_ON_GROUND);

    if (BIT_OFF(player_state, PLAYER_STATE_PLATFORM)) {
        // apply gravity
        player_velocity_y += 3;
    }
    if (player_velocity_y > MAX_Y_VELOCITY) {
        player_velocity_y = MAX_Y_VELOCITY;
    }
    if (player_velocity_y < 0) {
        // convert to unsigned
        tmp1 = -player_velocity_y;
        // divide for "sub pixel" movement
        player_y -= (tmp1 / VELOCITY_Y_DIV);
        if (player_y > SCREEN_HEIGHT) {
            BIT_SET(player_state, PLAYER_STATE_OFFSCREEN);
            // check for upper level exit transition
            if (player_y < 250 && BIT_ON(level_exit, LEVEL_EXIT_U)) {
                BIT_CLEAR(player_state, PLAYER_STATE_OFFSCREEN);
                player_velocity_y = -35;
                level_last = LEVEL_LAST_UP_EXIT;
                ++level_current;
                game_fade_to_black();
            }
            return;
        }
    } else {
        // convert to unsigned and divide for "sub pixel" movement
        player_y += ((u8)player_velocity_y / VELOCITY_Y_DIV);
        if (BIT_ON(player_state, PLAYER_STATE_OFFSCREEN)) {
            // check if onscreen again
            if (player_y < 8) {
                BIT_CLEAR(player_state, PLAYER_STATE_OFFSCREEN);
            }
        } else if (player_y > (SCREEN_HEIGHT-18) && player_y < SCREEN_HEIGHT) {
            BIT_CLEAR(player_state, PLAYER_STATE_OFFSCREEN);
            // check for lower level exit transition
            if (BIT_ON(level_exit, LEVEL_EXIT_D)) {
				level_last = LEVEL_LAST_DOWN_EXIT;
				--level_current;
				game_fade_to_black();
            } else {
                // fell into a pit and died
                player_kill();
            }
            return;
        }
    }

    // check if player is on screen
    if (player_y < (SCREEN_HEIGHT-(PLAYER_HEIGHT*2)))
    {
        tmp1 = player_y + (PLAYER_HEIGHT/2);
        tile_get_x = camera_diff.word + COLLIDE_LEFT_OFFSET;
        if (player_velocity_y < 0) {
            tile_get_y = tmp1 - 6;
        } else {
            tile_get_y = tmp1 + (PLAYER_HEIGHT/2);
        }
        get_tile();
        // check if we should test 2 tiles by making sure the player is offset from the grid edges
        if (MT_IS_NOT(SOLID))
        {
            // save tile_type so we can check if hurt
            tmp1 = tile_type;
            // prevent getting stuck in wall by offsetting by 1
            tile_get_x += (COLLIDE_RIGHT_OFFSET - COLLIDE_LEFT_OFFSET - 1);
            get_tile();
            // only hurt if the tile on the right is not solid
            if (BIT_ON(tmp1, MT_HURT) && TILE_IS_NOT_COLLIDE) {
                player_hurt1();
            }
        }
        // check if other tile hurts the player
        if (MT_IS(HURT)) {
            player_hurt1();
        }

        if (MT_IS(SOLID))
        {
            if (player_velocity_y < 0)
            {
                player_jump_timer = JUMP_MAX_TIME;
                player_velocity_y = 0;
            }
            else
            {
                set_on_ground();
            }
        }
        else
        {
            if (player_velocity_y > 0 && (player_y & 0x08) == 0)
            {
                tile_get_x = camera_diff.word + COLLIDE_LEFT_OFFSET; // center x of player
                tile_get_y = player_y + PLAYER_HEIGHT; // bottom of player
                get_tile();
                if (MT_IS(ONEWAY))
                {
                    set_on_ground();
                }
                else
                {
                    tile_get_x += (COLLIDE_RIGHT_OFFSET - COLLIDE_LEFT_OFFSET - 1);
                    get_tile();
                    if (MT_IS(ONEWAY))
                    {
                        set_on_ground();
                    }
                }
            }
        }
    }
}

void do_quicksand(void)
{
    BIT_SET(player_state, PLAYER_STATE_IN_LIQUID | PLAYER_STATE_ON_GROUND);
    // slow movement on x velocity
    if (player_velocity_x < -MAX_SAND_VELOCITY)
    {
        player_velocity_x = -MAX_SAND_VELOCITY;
    }
    else if (player_velocity_x > MAX_SAND_VELOCITY)
    {
        player_velocity_x = MAX_SAND_VELOCITY;
    }

    // put player behind the tiles so they sink into the quicksand
    BIT_SET(player_state, PLAYER_STATE_BEHIND);

    // reduce upward velocity to restrict jumping
    if (player_velocity_y < -JUMP_SAND_VELOCITY)
    {
        player_velocity_y = -JUMP_SAND_VELOCITY;
    }
    else if (player_velocity_y >= 0)
    {
        player_velocity_y = 0;
        // only move player down every eighth frame
        if ((frame_counter % 4) == 0)
        {
            ++player_y;
        }
        // skip y movement since we're slowly moving down
        return;
    }
}

void do_water(void)
{
    BIT_SET(player_state, PLAYER_STATE_IN_LIQUID);
    if (player_velocity_x < -MAX_WATER_VELOCITY)
    {
        player_velocity_x = -MAX_WATER_VELOCITY;
    }
    else if (player_velocity_x > MAX_WATER_VELOCITY)
    {
        player_velocity_x = MAX_WATER_VELOCITY;
    }

    --player_velocity_y;
    if (player_velocity_y < -JUMP_WATER_MAX_VELOCITY)
    {
        player_velocity_y = -JUMP_WATER_MAX_VELOCITY;
    }
    else if (player_velocity_y > MAX_WATER_DOWN_VELOCITY)
    {
        player_velocity_y = MAX_WATER_DOWN_VELOCITY;
    }
}

// handle liquids (quicksand and water) for player movement
void do_liquid(void)
{
#define liquid_flag tmp5

    if (BIT_ON(player_state, PLAYER_STATE_OFFSCREEN)) goto not_in_liquid;
    liquid_flag = level_flags & LEVEL_FLAGS_LIQUID_MASK;
    // start by checking the tile that is in the upper center of the player's head
    tile_get_x = camera_diff.word + 8;
    tile_get_y = player_y - 2;
    if (tile_get_y > SCREEN_HEIGHT) {
        tile_get_y = 0;
    }
    get_tile();
    // player is fully under quicksand so they should die
    if (tile_liquid)
    {
        if (liquid_flag == LIQUID_QUICKSAND) {
            // if a quicksand level we kill the player immediately after they are "submerged"
            player_kill();
            return;
        }
        goto is_liquid;
    }
    else if (player_y < (SCREEN_HEIGHT-PLAYER_HEIGHT))
    {
        // check if player in liquid tiles
        tile_get_y += 18;
        get_tile();
        if (!tile_liquid && liquid_flag == LIQUID_QUICKSAND)
        {
            get_tile_next_y();
        }
    }

    if (tile_liquid)
    {
is_liquid:
        switch (liquid_flag) {
            case LIQUID_QUICKSAND:
                do_quicksand();
                break;
            case LIQUID_TOXIC:
                player_kill();
                break;
                // fallthrough
            default:
                do_water();
        }
    }
    else
    {
not_in_liquid:
        // not in water so we clear the liquid bit
        BIT_CLEAR(player_state, PLAYER_STATE_IN_LIQUID);
    }
#undef liquid_flag
}

void do_left(void)
{
    BIT_SET(player_state, PLAYER_STATE_LEFT);
    player_velocity_x -= X_SPEED;
    if (player_velocity_x < -MAX_WALK_VELOCITY)
    {
        player_velocity_x = -MAX_WALK_VELOCITY;
    }
}

void do_right(void)
{
    BIT_CLEAR(player_state, PLAYER_STATE_LEFT);
    player_velocity_x += X_SPEED;
    if (player_velocity_x > MAX_WALK_VELOCITY)
    {
        player_velocity_x = MAX_WALK_VELOCITY;
    }
}

const u8 walk_jump_velocity[JUMP_MAX_TIME] = {
    12, 11, 10, 9, 8, 6, 5, 3
};

// if jump timer < JUMP_MAX_TIME, do jump velocity
// if jump timer > JUMP_TIMER_ALLOW_JUMP, allow coyote jumping
// jump timer gets reset to 0xFF when on ground, in liquid, or on platform
void do_jump(void)
{
    if (BIT_ON(pad, PAD_A))
    {
        // check if jump can be started
        if (COYOTE_JUMP_TIME < player_jump_timer)
        {
            player_jump_timer = 0;
            player_velocity_y = 0;
            sound_play_ch0(SFX_JUMP);

            // try to match jump height off of solid tiles
            if (BIT_ON(player_state, PLAYER_STATE_PLATFORM)) {
                ++player_jump_timer;
                player_velocity_y = -5;
            }
        }

        // add velocity to the jump
        if (player_jump_timer < JUMP_MAX_TIME)
        {
            player_velocity_y -= walk_jump_velocity[player_jump_timer];
            ++player_jump_timer;
        }
    }
    else
    {
        if (BIT_ON(player_state, PLAYER_JUMP_STATE))
        {
            // reset jump timer to max value when on ground or in liquid
            player_jump_timer = 0xFF;
        }
        else if (JUMP_MAX_TIME < player_jump_timer)
        {
            // decrease timer from 0xFF to allow jumping for several frames
            --player_jump_timer;
        }
        else
        {
            // stop jumping after player lets go of button
            player_jump_timer = JUMP_MAX_TIME;
        }
    }
}

void do_input(void)
{
    // get actionable tile at player head
    GET_TILE(camera_diff.word + (PLAYER_WIDTH / 2), player_y + (PLAYER_HEIGHT / 4));
    // enter doorway (tile at head)
    if (MT_IS_NOT(DOOR)) {
        // check tile below
        #if (PLAYER_HEIGHT == 32)
            get_tile_next_y();
        #endif
        if (MT_IS(FRONT)) {
            BIT_SET(player_state, PLAYER_STATE_BEHIND);
        }
    }
    if (BIT_ON(player_state, PLAYER_STATE_ON_GROUND|PLAYER_STATE_PLATFORM)) {
        if (BIT_ON(pad, PAD_UP)) {
            // level_next MUST be set to something other than 0xFF to be entered!
            if (MT_IS(DOOR) && level_next != LEVEL_INVALID) {
                sound_play_ch0(SFX_DOOR);
                level_last = level_current | (level_next & LVL_ALTERNATE);
                level_current = level_next & LVL_MASK;
                game_fade_to_black();
            }
        }
        // save tile for draw_arrow
        player_tile = tile_type;
    } else {
        player_tile = 0;
    }

    // handle horizontal movement (no collision checking yet)
    if (pad & PAD_LEFT) {
        do_left();
    } else if (pad & PAD_RIGHT) {
        do_right();
    } else {
        // slow player down to a stop if not pressing direction
        if (player_velocity_x > 0)
        {
            BIT_CLEAR(player_state, PLAYER_STATE_LEFT);
            --player_velocity_x;
        }
        else if (player_velocity_x < 0)
        {
            BIT_SET(player_state, PLAYER_STATE_LEFT);
            ++player_velocity_x;
        }
    }

    do_jump();
}

const u8 dead_offset[] = {32, 26, 28, 30, 22, 12, 18, 14, 10, 8};

void health_draw(void)
{
    oam_x = 24;
    oam_y = 40;
    oam_attr = OAM_PAL1;
    if (player_health == 1) {
        tmp1 = (frame_counter & 0x04) >> 1;
        --tmp1;
        oam_x += tmp1;
    } else if (player_health == 2 && player_health != player_health_max) {
        tmp1 = (frame_counter & 0x08) >> 2;
        --tmp1;
        oam_x += tmp1;
    } else {
        tmp1 = player_health - 3;
        while (tmp1 < HEALTH_MAXIMUM) {
            oam_draw(SPRITE_HEALTH);
            oam_y += 16;
            tmp1 -= 2;
        }
    }
    oam_draw(((player_health & 1) << 1) + SPRITE_HEALTH);
}

void player_update(void)
{
    player_camera_diff();
    if (player_health == 0) {
        sound_play_ch0(SFX_DEATH);
        game_state = GAME_STATE_RESET;
    } else {
        BIT_CLEAR(player_state, PLAYER_STATE_BEHIND|PLAYER_STATE_WALL);

        do_input();

        // no liquid, don't process it!
        if (BIT_ON(level_flags, LEVEL_FLAGS_LIQUID_MASK)) {
            do_liquid();
        }

        do_move_y();
        do_move_x();
        camera_move();
        player_draw();

        health_draw();
        BIT_CLEAR(player_state, PLAYER_STATE_INTERACT|PLAYER_STATE_PLATFORM);
    }
}
