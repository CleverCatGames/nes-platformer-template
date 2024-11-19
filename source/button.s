.export _button_pressed,_button_released
.import _last_pad,_pad
.importzp tmp1

.PROC _button_pressed
    sta tmp1
    lda _last_pad
    eor _pad
    and _pad
    and tmp1
    rts
.ENDPROC

.PROC _button_released
    sta tmp1
    lda _last_pad
    eor _pad
    and _last_pad
    and tmp1
    rts
.ENDPROC

