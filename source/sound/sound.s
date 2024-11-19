.import popa,bank_switch_nmi
.import pushax,_memcpy
.import _frame_counter

.export audio_init,audio_update
.export _music_play,_music_stop
.export _music_pause,_music_resume
.export _music_save,_music_restore
.export _sound_play_ch0,_sound_play_ch1
.exportzp tvSystem

MUSIC_RESUME = $FC
MUSIC_PAUSE = $FE
MUSIC_STOP = $FF

MUSIC_RESTORE = 1 ; requires SFX support

OPTIONS_SFX   = %00000001
OPTIONS_MUSIC = %00000010


FAMISTUDIO_CFG_EXTERNAL = 1
FAMISTUDIO_CFG_NTSC_SUPPORT = 1
FAMISTUDIO_CFG_SFX_SUPPORT = 1
FAMISTUDIO_CFG_SFX_STREAMS = 2

FAMISTUDIO_USE_VOLUME_TRACK = 1

.define FAMISTUDIO_CA65_ZP_SEGMENT ZEROPAGE
.define FAMISTUDIO_CA65_RAM_SEGMENT BSS
.define FAMISTUDIO_CA65_CODE_SEGMENT MUSIC

.include "famistudio_ca65.s"

;.segment "DPCM"
;.incbin "music.dmc"

.segment "MUSIC"

music_data:
.include "music.s"

sfx_data:
.include "sfx.s"

.ZEROPAGE
tvSystem: .res 1
LAST_BANK: .res 1
music_playing: .res 1
music_playing_save: .res 1
music_request: .res 1
sfx_ch0: .res 1
sfx_ch1: .res 1

song_speed: .res 1

.IFDEF MUSIC_RESTORE
.SEGMENT "PRGRAM"
FAMISTUDIO_VAR_START = famistudio_env_value
FAMISTUDIO_VAR_END = famistudio_sfx_addr_lo
FAMISTUDIO_VAR_BYTES = (FAMISTUDIO_VAR_END - FAMISTUDIO_VAR_START)
famistudio_buf: .res FAMISTUDIO_VAR_BYTES
.ENDIF

.CODE

.PROC audio_init
    lda #$FF ; allows first song to play
    sta music_playing
    sta music_request
    sta sfx_ch0
    sta sfx_ch1

    lda #<.BANK(famistudio_init)
    jsr bank_switch_nmi

    ldx #.lobyte(sfx_data)
    ldy #.hibyte(sfx_data)
    lda #1
    jsr famistudio_sfx_init
    ldx #.lobyte(music_data)
    ldy #.hibyte(music_data)
    lda #1 ; NTSC
    jmp famistudio_init
.ENDPROC

.PROC audio_update
    lda #<.BANK(famistudio_update)
    jsr bank_switch_nmi

    lda music_request
    cmp #MUSIC_PAUSE
    bne @resume
    jsr famistudio_music_pause
    bne @sfx ; should always be non zero
@resume:
    cmp #MUSIC_RESUME
    bne @stop
    lda #0 ; set a to zero to resume
    jsr famistudio_music_pause
    bne @sfx ; should always be non zero
@stop:
    cmp #MUSIC_STOP
    bne @playing
    jsr famistudio_music_stop
    bne @sfx ; should always be non zero
@playing:
    cmp music_playing
    beq @sfx
    sta music_playing
    jsr famistudio_music_play

@sfx:
    lda sfx_ch0
    cmp #$FF
    beq @ch1
    ldx #FAMISTUDIO_SFX_CH0
    jsr famistudio_sfx_play
    lda #$FF
    sta sfx_ch0
@ch1:
    lda sfx_ch1
    cmp #$FF
    beq @update
    ldx #FAMISTUDIO_SFX_CH1
    jsr famistudio_sfx_play
    lda #$FF
    sta sfx_ch1

@update:
    jmp famistudio_update
.ENDPROC

.PROC _sound_play_ch0
    tax
    stx sfx_ch0
    rts
.ENDPROC

.PROC _sound_play_ch1
    tax
    stx sfx_ch1
    rts
.ENDPROC

.PROC _music_play
    sta music_request
    rts
.ENDPROC

_music_save:
    cmp music_request
    bne @continue
    rts
@continue:
    pha
    lda music_request
    sta music_playing_save

.IFDEF MUSIC_RESTORE
    ; copy famistudio variables to a backup buffer
    lda #<famistudio_buf       ; dest
    ldx #>famistudio_buf
    jsr pushax
    lda #<FAMISTUDIO_VAR_START ; source
    ldx #>FAMISTUDIO_VAR_START
    jsr pushax
    jsr music_copy ; copy data
.ENDIF

    pla
    jmp _music_play

music_copy:
    lda #<FAMISTUDIO_VAR_BYTES
    ldx #>FAMISTUDIO_VAR_BYTES
    jmp _memcpy

_music_restore:
    lda music_playing_save
    sta music_request

.IFDEF MUSIC_RESTORE
    sta music_playing

    ; copy famistudio backup buffer back to famistudio
    lda #<FAMISTUDIO_VAR_START ; dest
    ldx #>FAMISTUDIO_VAR_START
    jsr pushax
    lda #<famistudio_buf       ; source
    ldx #>famistudio_buf
    jsr pushax
    jsr music_copy

    ; restore APU channels
    lda famistudio_output_buf+0
    sta FAMISTUDIO_APU_PL1_VOL
    lda famistudio_output_buf+1
    sta FAMISTUDIO_APU_PL1_LO
    lda famistudio_output_buf+2
    sta FAMISTUDIO_APU_PL1_HI

    lda famistudio_output_buf+3
    sta FAMISTUDIO_APU_PL2_VOL
    lda famistudio_output_buf+4
    sta FAMISTUDIO_APU_PL2_LO
    lda famistudio_output_buf+5
    sta FAMISTUDIO_APU_PL2_HI

    lda famistudio_output_buf+6
    sta FAMISTUDIO_APU_TRI_LINEAR
    lda famistudio_output_buf+7
    sta FAMISTUDIO_APU_TRI_LO
    lda famistudio_output_buf+8
    sta FAMISTUDIO_APU_TRI_HI

    lda famistudio_output_buf+9
    sta FAMISTUDIO_APU_NOISE_VOL
    lda famistudio_output_buf+10
    sta FAMISTUDIO_APU_NOISE_LO
.ENDIF

    rts

.PROC _music_stop
    lda #MUSIC_STOP
    sta music_request
    rts
.ENDPROC

.PROC _music_pause
    lda #MUSIC_PAUSE
    sta music_request
    rts
.ENDPROC

.PROC _music_resume
    lda #MUSIC_RESUME
    sta music_request
    rts
.ENDPROC

