.export _bank_switch,bank_switch_nmi
.export mapper_init,mapper_nmi
.export _set_mirroring
.exportzp CURRENT_BANK

.ZEROPAGE

CURRENT_BANK: .res 1

.CODE

banktable:
  ; UNROM support
  .byte $00, $01, $02, $03, $04, $05, $06
  ; UOROM (256KB in 16 banks of 16K)
  .byte $07, $08, $09, $0A, $0B, $0C, $0D, $0E
  ; UNROM-512 (supports up to 32 banks)
  .byte $0F, $11, $12, $13, $14, $15, $16, $17
  .byte $18, $19, $1A, $1B, $1C, $1D, $1E, $1F

_bank_switch:
  sta CURRENT_BANK
bank_switch_nmi:    ; used by nmi so we don't save CURRENT_BANK
  tay
  lda banktable, y
  sta banktable, y
mapper_init:        ; do nothing for init
mapper_nmi:         ; do nothing for nmi reset
_set_mirroring:
  rts
