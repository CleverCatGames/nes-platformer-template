.export _obj_set_state, _obj_inc_state, _obj_set_state_flag
.export _obj_and_state_flag
.importzp _object_state, _object_timer

; MUST be in CODE since not all objects are in OBJECTS segment

_obj_set_state:
    sta _object_state
    jmp reset_timer

_obj_inc_state:
    inc _object_state
    jmp reset_timer

_obj_set_state_flag:
    ora _object_state
    sta _object_state
reset_timer:
    lda #0
    sta _object_timer
    rts

_obj_and_state_flag:
    and _object_state
    sta _object_state
    jmp reset_timer

