# Object Library

Here is the object library that separates out all object code from the rest of the game.

## Folder Structure

### Utilities

There is a `util` folder that houses general functions for spawning, drawing, and movement.

### npc.c and enemy.c

These are the two object types in the game, at the moment. You can see their definitions in `assets/data/object_types.csv`.

Notice that the functions have a consistent pattern. For example, all draw routines will look like this:

```C
void draw_enemy(void);
void draw_helicopter(void);
void draw_hotdog(void);
```

Move routines will look like this:

```C
void move_enemy(void);
void move_helicopter(void);
void move_walking(void);
```

You don't need to have a unique function for every object type. Sometimes the functionality will overlap and you can reuse. You can see an example of that with `move_enemy` where `move_walking` is utilized if the object hasn't been hit.

## Object variables

When the functions for objects are called there are special variables that are utilized.

### object_x and object_y

These are the object's location values. `object_x` is a 16-bit value while `object_y` is only and 8-bit value.

### object_type

This is the object's type that is generated from object_types.csv. It generates unique names `OBJ_TYPE_<OBJECT NAME>`. There is also a special object type `OBJ_TYPE_NONE` that lets you destroy and object.

### object_timer

This value is automatically decremented every frame the game is playing. Why?

Imagine you want to check when something is complete after a period of time. You can do something like this:

```C
#define TIME_TO_COMPLETE 120 // 2 seconds in frames
if (object_timer == TIME_TO_COMPLETE) {
    // reset to new state
    obj_set_state(NEW_STATE);
}
```

When the `object_timer` value hits your expected time, it does some logic and then you can set the state to something else. More on this in a bit.

### object_state

Stores any state for the object. This is where you can be creative and use this byte however you see fit. Maybe it stores a state machine value, or a packed coordinate, or some kind of bit flag. Whatever it is, it gets stored for later usage each frame.

## State Functions

There are state setting functions that let you do various things to the `object_state` value. All of these also reset the `object_timer` to zero. Having these both in a function saves quite a few bytes depending on the number of objects you have.

* obj_set_state: `object_state = constant;`
* obj_inc_state: `++object_state;`
* obj_set_state_flag: `object_state |= constant;`
* obj_clear_state_flag: `object_state &= ~constant;`

The last two are useful for setting and clearing bit flag values.

## Note about object_x

Any time you do arithmetic on `object_x` it will be doing 16-bit math. This can get rather expensive so it's best to do this in helper functions and call those. Otherwise, your ROM size can grow quite a bit if you aren't careful.

