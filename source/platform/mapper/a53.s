.export _bank_switch,bank_switch_nmi
.export _set_mirroring
.export mapper_init,mapper_nmi
.exportzp CURRENT_BANK
.import NES_SAVE_RAM

MIRROR_ONE = 0
MIRROR_VERT = 2
MIRROR_HORIZ = 3

A53_REG_SELECT = $5000
A53_REG_VALUE  = $8000
A53_SELECT_CHR = $00
A53_SELECT_PRG = $01

.ZEROPAGE

CURRENT_BANK: .res 1

.CODE

_bank_switch:
  sta CURRENT_BANK

bank_switch_nmi:
  ldx #A53_SELECT_PRG
  stx A53_REG_SELECT

  sta A53_REG_VALUE
  rts

; should only do this when nmi is turned off. otherwise we need guards like bank switching has
_set_mirroring:
  ldx #$80 ; mode
  stx A53_REG_SELECT
  ora #%101100
  sta A53_REG_VALUE
  rts

.PROC mapper_init
  lda #$81 ; outer prg bank
  sta A53_REG_SELECT
  lda #$07
  sta A53_REG_VALUE

  lda #MIRROR_VERT
  jsr _set_mirroring

  lda #$00 ; chr bank
  sta A53_REG_SELECT
  sta A53_REG_VALUE

  jmp _bank_switch
.ENDPROC

; run code to reset during nmi
mapper_nmi:
  rts
