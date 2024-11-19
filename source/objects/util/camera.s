.export _move_camera_to_object_x
.import _camera_setup, _object_x, _player_x, _camera_x

.PROC _move_camera_to_object_x
    ; move player to object x value
    clc
    adc _object_x
    sta _player_x
    ; load page
    lda _object_x+1
    sta _camera_x+1
    lda #0
    sta _camera_x
    ; adjust the camera after the player is in place
    jmp _camera_setup
.ENDPROC
