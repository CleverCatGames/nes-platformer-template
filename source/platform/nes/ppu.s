;NES hardware-dependent functions by Shiru (shiru@mail.ru)
;with improvements by VEG
;Feel free to do anything you want with this code, consider it Public Domain

	.export _ppu_off,_ppu_on_all,_ppu_on_bg,_ppu_on_spr
	.export _ppu_mask,_ppu_system,_ppu_clear_vram
    .export _ppu_wait_frame,_ppu_wait_nmi,_ppu_wait_seconds
	.export _bank_spr,_bank_bg

	.export irq,nmi,detect_ntsc
	.exportzp PPU_MASK_VAR

	.importzp PAL_UPDATE,NAME_UPD_ENABLE
	.importzp SCROLL_X,SCROLL_Y
	.importzp CURRENT_BANK
	.import pal_to_ppu,flush_vram_update_nmi
	.import audio_update
	.import bank_switch_nmi, mapper_nmi


.include "nesdef.inc"

.ZEROPAGE
PPU_CTRL_VAR:       .res 1
PPU_MASK_VAR:       .res 1
NTSC_MODE:          .res 1
FRAME_CNT1:         .res 1
FRAME_CNT2:         .res 1
VRAM_UPDATE:        .res 1
TEMP:               .res 8

.segment "STARTUP"

;NMI handler

nmi:
	pha
	txa
	pha
	tya
	pha

	lda <PPU_MASK_VAR	;if rendering is disabled, do not access the VRAM at all
	and #%00011000
	bne @doUpdate
	jmp	@skipAll

@doUpdate:

	lda #>OAM_BUF		;update OAM
	sta PPU_OAM_DMA

	lda <PAL_UPDATE		;update palette if needed
	beq @updVRAM
	jsr pal_to_ppu

@updVRAM:

	lda <VRAM_UPDATE
	beq @skipUpd
	lda #0
	sta <VRAM_UPDATE

	lda <NAME_UPD_ENABLE
	beq @skipUpd

	jsr flush_vram_update_nmi

@skipUpd:

	lda #0
	sta PPU_ADDR
	sta PPU_ADDR

	lda <SCROLL_X
	sta PPU_SCROLL
	lda <SCROLL_Y
	sta PPU_SCROLL

	lda <PPU_CTRL_VAR
	sta PPU_CTRL

@skipAll:
	jsr mapper_nmi ; sets reset for mmc1 or other mappers

	jsr audio_update

	lda CURRENT_BANK ; restore the bank used before nmi was called
	jsr bank_switch_nmi

	lda <PPU_MASK_VAR
	sta PPU_MASK

	inc <FRAME_CNT1
	inc <FRAME_CNT2
	lda <FRAME_CNT2
	cmp #6
	bne @skipNtsc
	lda #0
	sta <FRAME_CNT2

@skipNtsc:

	pla
	tay
	pla
	tax
	pla

irq:

    rti


;void __fastcall__ ppu_off(void);

_ppu_off:

	lda <PPU_MASK_VAR
	and #%11100111

ppu_onoff:

	sta <PPU_MASK_VAR
	jmp _ppu_wait_nmi



;void __fastcall__ ppu_on_all(void);

_ppu_on_all:

	lda <PPU_MASK_VAR
	ora #%00011000
	bne ppu_onoff	;unconditional



;void __fastcall__ ppu_on_bg(void);

_ppu_on_bg:

	lda <PPU_MASK_VAR
	ora #%00001000
	bne ppu_onoff	;unconditional



;void __fastcall__ ppu_on_spr(void);

_ppu_on_spr:

	lda <PPU_MASK_VAR
	ora #%00010000
	bne ppu_onoff	;unconditional



;void __fastcall__ ppu_mask(byte mask);

_ppu_mask:
	sta <PPU_MASK_VAR
	rts



;byte __fastcall__ ppu_system(void);

_ppu_system:
	lda <NTSC_MODE
	rts



detect_ntsc:

	lda FRAME_CNT1
WaitSync:
    cmp FRAME_CNT1
    beq WaitSync

    ldx #52             ;blargg's code
    ldy #24
DetectNTSC:
    dex
    bne DetectNTSC
    dey
    bne DetectNTSC

    lda PPU_STATUS
    and #$80
    sta <NTSC_MODE
    rts


;void __fastcall__ ppu_wait_frame(void);

.PROC _ppu_wait_frame
	jsr _ppu_wait_nmi

	lda NTSC_MODE
	beq exit        ; if PAL, skip

loop:
	lda FRAME_CNT2
	cmp #5
	beq loop

exit:
	rts
.ENDPROC

.PROC _ppu_wait_seconds
    tax
@sec_loop:
    ldy #60
@loop:
    jsr _ppu_wait_nmi
    dey
    bne @loop
    dex
    bne @sec_loop
    rts
.ENDPROC

.PROC _ppu_clear_vram
	ldx #0
    ldy #$20
    sty PPU_ADDR
    stx PPU_ADDR
    ldy #$10
ClearVRAM:
    sta PPU_DATA
    inx
    bne ClearVRAM
    dey
    bne ClearVRAM
	rts
.ENDPROC

;void __fastcall__ ppu_wait_nmi(void);

.PROC _ppu_wait_nmi
	lda #1
	sta VRAM_UPDATE
	lda FRAME_CNT1   ; store current counter in A

not_equal_last:
	cmp FRAME_CNT1    ; compare A with counter until they don't match
	beq not_equal_last ; this is triggered by "inc FRAME_CNT1" in nmi
	rts
.ENDPROC



;void __fastcall__ bank_spr(byte n);

.PROC _bank_spr
	and #$01
	asl a
	asl a
	asl a
	sta TEMP
	lda PPU_CTRL_VAR
	and #%11110111
	ora TEMP
	sta PPU_CTRL_VAR
	rts
.ENDPROC



;void __fastcall__ bank_bg(byte n);

.PROC _bank_bg
	and #$01
	asl a
	asl a
	asl a
	asl a
	sta TEMP
	lda PPU_CTRL_VAR
	and #%11101111
	ora TEMP
	sta PPU_CTRL_VAR
	rts
.ENDPROC
