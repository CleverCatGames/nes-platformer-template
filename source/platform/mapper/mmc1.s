.export _bank_switch,bank_switch_nmi
.export _set_mirroring
.export mapper_init,mapper_nmi
.exportzp CURRENT_BANK
.import NES_SAVE_RAM

MIRROR_ONE = 0
MIRROR_VERT = 2
MIRROR_HORIZ = 3

MMC1_CTRL	= $8000
MMC1_CHR0	= $a000
MMC1_CHR1	= $c000
MMC1_PRG	= $e000

; macro to write to an mmc1 register, which goes one bit at a time, 5 bits wide.
.macro mmc1_register_write addr
	.repeat 4
		sta addr
		lsr
	.endrepeat
	sta addr
.endmacro

.macro mmc1_register_reset addr
  lda #$80
  sta addr
.endmacro

.ZEROPAGE

CURRENT_BANK: .res 1
mmc1_interrupted: .res 1

.CODE

.PROC _bank_switch
  sta CURRENT_BANK
start_switch:
  ldy #0
  sty mmc1_interrupted
  mmc1_register_write MMC1_PRG
  lda mmc1_interrupted
  beq switch_ok

  ; bank switch was interrupted by nmi or irq, need to reload it
  mmc1_register_reset MMC1_PRG
  lda CURRENT_BANK
  jmp start_switch

switch_ok:
  rts
.ENDPROC

; special bank switching routine that doesn't save to CURRENT_BANK
bank_switch_nmi:
  mmc1_register_write MMC1_PRG
  rts

; should only do this when nmi is turned off. otherwise we need guards like bank switching has
_set_mirroring:
  and #%00000011 ; only keep mirroing bits
  ora #%00001100 ; fix $c000, bank $8000, 4k chr bank
write_ctrl:
  mmc1_register_write MMC1_CTRL
  rts

.PROC mapper_init
  lda #$0e ; mirror vertical, fixed $c000
  jsr write_ctrl
  lda #1
  mmc1_register_write MMC1_CHR1
  lda #0
  mmc1_register_write MMC1_CHR0
  lda #0
  jmp _bank_switch
.ENDPROC

; run code to reset during nmi
mapper_nmi:
  lda #1
  sta mmc1_interrupted
  mmc1_register_reset MMC1_PRG ; might have been in the middle of something so we reset bank pointer
  rts
