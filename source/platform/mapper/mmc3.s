.export _bank_switch
.export _set_mirroring
.export bank_switch_nmi, mapper_init
.exportzp CURRENT_BANK

.define MMC3_BANK_REG $8000
.define MMC3_BANK_VAL $8001
.define MMC3_MIRROR_REG $A000
.define MMC3_PRG_RAM_PROTECT $A001
.define MMC3_IRQ_LATCH $C000
.define MMC3_IRQ_RELOAD $C001
.define MMC3_IRQ_DISABLE $E000
.define MMC3_IRQ_ENABLE $E001

.ZEROPAGE

CURRENT_BANK: .res 1

.CODE

_bank_switch:
    sta CURRENT_BANK
    ldx #$06
    stx MMC3_BANK_REG
    sta MMC3_BANK_VAL
bank_switch_nmi:
    rts

_set_mirroring:
    sta MMC3_MIRROR_REG
    rts

mapper_init:
    ;set music bank
    lda #$07
    sta MMC3_BANK_REG
    lda #$0d
    sta MMC3_BANK_VAL

    lda #0
    jsr _bank_switch

    ; set vertical mirroring
    lda #$0
    jmp _set_mirroring
