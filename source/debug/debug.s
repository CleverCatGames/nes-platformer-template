.importzp tmp1,tmp2

.IFDEF DEBUG

.export _print_int
.export _print_dump
.export _print_hex

.ZEROPAGE

decOnes: .res 1
decHundreds: .res 1
decTenThousands: .res 1

.CODE

PPU_ADDR = $2006
PPU_DATA = $2007
ASCII_OFFSET = $10

hexLow = tmp1
hexHigh = tmp2

;temp register and decHundreds are doubled up to save ram...
temp = decHundreds


Times256_Low:
    .byte $00,$38,$0C,$44,$18,$50,$24,$5C
    .byte $30,$04,$3C,$10,$48,$1C,$54,$28
Times256_Med:
    .byte $00,$02,$05,$07,$0A,$0C,$0F,$11
    .byte $14,$17,$19,$1C,$1E,$21,$23,$26


Times16_Low:
    .byte $00
Times4096_Low:
    .byte $00
Times4096_Med:
    .byte $00
Times4096_High:
    .byte $00 + ASCII_OFFSET

    .byte $10,$60,$28,$00 + ASCII_OFFSET   ; interlaced tables, this allows less shifts to be made...
    .byte $20,$5C,$51,$00 + ASCII_OFFSET
    .byte $30,$58,$16,$01 + ASCII_OFFSET
    .byte $40,$54,$3F,$01 + ASCII_OFFSET
    .byte $50,$50,$04,$02 + ASCII_OFFSET
    .byte $60,$4C,$2D,$02 + ASCII_OFFSET
    .byte $0C,$48,$56,$02 + ASCII_OFFSET
    .byte $1C,$44,$1B,$03 + ASCII_OFFSET
    .byte $2C,$40,$44,$03 + ASCII_OFFSET
    .byte $3C,$3C,$09,$04 + ASCII_OFFSET
    .byte $4C,$38,$32,$04 + ASCII_OFFSET
    .byte $5C,$34,$5B,$04 + ASCII_OFFSET
    .byte $08,$30,$20,$05 + ASCII_OFFSET
    .byte $18,$2C,$49,$05 + ASCII_OFFSET
    .byte $28,$28,$0E,$06 + ASCII_OFFSET

ShiftedBcdTab:
    .byte $00,$01,$02,$03,$04,$08,$09,$0A,$0B,$0C
    .byte $10,$11,$12,$13,$14,$18,$19,$1A,$1B,$1C
    .byte $20,$21,$22,$23,$24,$28,$29,$2A,$2B,$2C
    .byte $30,$31,$32,$33,$34,$38,$39,$3A,$3B,$3C
    .byte $40,$41,$42,$43,$44,$48,$49,$4A,$4B,$4C


_print_int:
    sta hexLow
    stx hexHigh

StartHexToDec:
    lda    hexHigh               ;3  @3
    and    #$0F                  ;2  @5
    tax                          ;2  @7
    eor    hexHigh               ;3  @10
    lsr                          ;2  @12   carry is clear, shifting just 2 times instead of 4,
    lsr                          ;2  @14   since interlaced tables are used.
    tay                          ;2  @16
    lda    Times4096_High,Y      ;4  @20
    sta    decTenThousands       ;3  @23
    lda    Times4096_Low,Y       ;4  @27
    adc    Times256_Low,X        ;4  @31
    sta    temp                  ;3  @34
    lda    Times4096_Med,Y       ;4  @38
    adc    Times256_Med,X        ;4  @42
    tay                          ;2  @44

    lda    hexLow                ;3  @47
    and    #$F0                  ;2  @49
    lsr                          ;2  @51
    lsr                          ;2  @53
    tax                          ;2  @55
    tya                          ;2  @57
    cpx    #13*4                 ;2  @59   times 4 due to interlaced table
    adc    #0                    ;2  @61
    cpx    #7*4                  ;2  @63
    adc    #0                    ;2  @65
    tay                          ;2  @67

    lda    hexLow                ;3  @70
    and    #$0F                  ;2  @72
    adc    Times16_Low,X         ;4  @76
    adc    temp                  ;3  @79
    bcs    @sub100               ;2³ @81/82
    cmp    #100                  ;2  @83
    bcc    @skip1                ;2³ @85/86

@sub100:
    sbc    #100                  ;2  @87
    iny                          ;2  @89
@skip1:
    cmp    #100                  ;2  @91
    bcc    @skip2                ;2³ @93/94
    sbc    #100                  ;2  @95
    iny                          ;2  @97
@skip2:
    lsr                          ;2  @99
    tax                          ;2  @101
    lda    ShiftedBcdTab,X       ;4  @105
    tax                          ;2  @107
    rol                          ;2  @109
    and    #$0F                  ;2  @111
  .IF ASCII_OFFSET
    ora    #ASCII_OFFSET         ;2
  .ENDIF
    sta    decOnes               ;3  @114

    txa                          ;2  @116
    lsr                          ;2  @118
    lsr                          ;2  @120
    lsr                          ;2  @122
  .IF ASCII_OFFSET
    ora    #ASCII_OFFSET         ;2
  .ENDIF
    tax                          ;2  @124   or STA decTens
    tya                          ;2  @126
    cmp    #100                  ;2  @128
    bcc    @skip3                ;2³ @130/131
    sbc    #100                  ;2  @132
    inc    decTenThousands       ;5  @137
@skip3:
    lsr                          ;2  @139
    tay                          ;2  @141
    lda    ShiftedBcdTab,Y       ;4  @145
    tay                          ;2  @147
    rol                          ;2  @149
    and    #$0F                  ;2  @151

  .IF ASCII_OFFSET
    ora    #ASCII_OFFSET         ;2
  .ENDIF
    sta    decHundreds           ;3  @154

    tya                          ;2  @156
    lsr                          ;2  @158
    lsr                          ;2  @160
    lsr                          ;2  @162
  .IF ASCII_OFFSET
    ora    #ASCII_OFFSET         ;2
  .ENDIF
                                 ;   A = decThousands

    tay
    lda decTenThousands
    cmp #ASCII_OFFSET
    beq @skipTenThousand
    sta PPU_DATA
@skipTenThousand:
    tya
    sta PPU_DATA ; thousands
    lda decHundreds
    sta PPU_DATA
    stx PPU_DATA ; tens
    lda decOnes
    sta PPU_DATA
    rts

hex_to_ascii:
  ;      0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
  .byte $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$21,$22,$23,$24,$25,$26

_print_hex:
  pha               ; save byte
  lsr               ; shift upper nybble to lower
  lsr
  lsr
  lsr
  ; print nybble
  and #$0F
  tay
  lda hex_to_ascii,y
  sta PPU_DATA

  pla               ; restore byte
  ; print nybble
  and #$0F
  tay
  lda hex_to_ascii,y
  sta PPU_DATA
  ; print space
  rts

print_line:
@loop:
  inc $02
  ldy $02
  lda $00,y
  jsr _print_hex
  dex
  bne @loop
  rts

_print_dump:
  ; print first 2 values
  lda $00
  jsr _print_hex
  lda $01
  jsr _print_hex
  lda $02
  jsr _print_hex

  lda #0
  sta $00
  sta $01
  lda #2
  sta $02

  ldx #(128-3)
  jsr print_line

  lda #$23
  sta PPU_ADDR
  lda #$d0
  sta PPU_ADDR
  lda #$18
.REPEAT 16
  sta PPU_DATA
.ENDREPEAT

  ;ldx #8
  ;jsr print_line

  rts

.ENDIF ; DEBUG
