; Startup code for cc65 and Shiru's NES library
; based on code by Groepaz/Hitmen <groepaz@gmx.net>, Ullrich von Bassewitz <uz@cc65.org>

    .export _exit,__STARTUP__:absolute=1
    .import irq,nmi,_main,detect_ntsc
    .import _ppu_off,_ppu_clear_vram
    .import _oam_clear,_pal_clear,_pal_bright,_set_rand
    .import mapper_init
    .import audio_init

; Linker generated symbols
    .import __RAM_START__   ,__RAM_SIZE__
    .import __ROM0_START__  ,__ROM0_SIZE__
    .import __DATA_SIZE__
    .import __STARTUP_LOAD__,__STARTUP_RUN__,__STARTUP_SIZE__
    .import __CODE_LOAD__   ,__CODE_RUN__   ,__CODE_SIZE__
    .import __RODATA_LOAD__ ,__RODATA_RUN__ ,__RODATA_SIZE__
    .import NES_MAPPER, NES_PRG_BANKS, NES_CHR_BANKS, NES_MIRRORING, NES_SAVE_RAM

    .importzp PPU_MASK_VAR
    .include "zeropage.inc"
    .include "nesdef.inc"

.segment "HEADER"
.byte "NES", $1a
.byte <NES_PRG_BANKS
.byte <NES_CHR_BANKS
.byte <NES_MIRRORING | ((<NES_MAPPER << 4) & $f0) | ((<NES_SAVE_RAM & $1) << 1)
.byte <NES_MAPPER & $f0
.res 8,0


.segment "STARTUP"

.macro wait_vblank
  : bit PPU_STATUS
    bpl :-
.endmacro

_exit:
reset:

    sei

    ldx #$ff            ;reset stack pointer
    txs

    inx                 ;x=0
    stx PPU_MASK
    stx DMC_FREQ
    stx PPU_CTRL        ;no NMI

    bit PPU_STATUS      ;init ppu
    wait_vblank

    txa
ClearWRAM:
    sta $000,x
    sta $100,x
    sta $200,x
    sta $300,x
    sta $400,x
    sta $500,x
    sta $600,x
    sta $700,x
    inx
    bne ClearWRAM

    wait_vblank         ;wait for next vblank

    lda #$3f            ;clear palette
    sta PPU_ADDR
    ldx #$00
    stx PPU_ADDR
    lda #$0f
    ldx #$20
ClearPalette:
    sta PPU_DATA
    dex
    bne ClearPalette

    lda #0
    jsr _ppu_clear_vram

    lda #4
    jsr _pal_bright
    jsr _pal_clear
    jsr _oam_clear
    jsr mapper_init

    lda #<(__RAM_START__+__RAM_SIZE__)
    sta sp
    lda #>(__RAM_START__+__RAM_SIZE__)
    sta sp+1            ; Set argument stack ptr

    lda #%10000000
    sta <PPU_CTRL_VAR
    sta PPU_CTRL        ;enable NMI
    lda #%00000110
    sta <PPU_MASK_VAR

    jsr detect_ntsc
    jsr _ppu_off

    lda #0
    sta PPU_SCROLL
    sta PPU_SCROLL
    sta PPU_OAM_ADDR

    jsr audio_init

    jmp _main           ;no parameters

.segment "VECTORS"
    .word nmi, reset, irq
