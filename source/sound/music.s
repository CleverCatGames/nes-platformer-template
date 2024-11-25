; This file is for the FamiStudio Sound Engine and was generated by FamiStudio

.if FAMISTUDIO_CFG_C_BINDINGS
.export _music_data_untitled=music_data_untitled
.endif

music_data_untitled:
	.byte 1
	.word @instruments
	.word @samples-4
; 00 : Song 1
	.word @song0ch0
	.word @song0ch1
	.word @song0ch2
	.word @song0ch3
	.word @song0ch4
	.byte .lobyte(@tempo_env_1_mid), .hibyte(@tempo_env_1_mid), 0, 0

.export music_data_untitled
.global FAMISTUDIO_DPCM_PTR

@instruments:
	.word @env1,@env2,@env4,@env0 ; 00 : Instrument 1

@env0:
	.byte $00,$c0,$7f,$00,$02
@env1:
	.byte $00,$cf,$7f,$00,$02
@env2:
	.byte $c0,$7f,$00,$01
@env3:
	.byte $7f,$00,$00
@env4:
	.byte $c1,$7f,$00,$00

@samples:

@tempo_env_1_mid:
	.byte $03,$05,$80

@song0ch0:
@song0ch0loop:
	.byte $47, .lobyte(@tempo_env_1_mid), .hibyte(@tempo_env_1_mid), $7b, $80
@song0ref7:
	.byte $0f, $8b, $00, $8d, $12, $8b, $00, $8d, $14, $8b, $00, $ab, $17, $8b, $00, $ab, $16, $8b, $00, $8d, $17, $8b, $00, $8d
	.byte $41, $0c
	.word @song0ref7
	.byte $0d, $8b, $00, $c9, $0d, $8b, $00, $8d, $48
	.byte $41, $18
	.word @song0ref7
	.byte $1b, $8b, $00, $8d, $19, $8b, $00, $8d, $17, $8b, $00, $ab, $12, $8b, $00, $e7, $42
	.word @song0ch0loop
@song0ch1:
@song0ch1loop:
@song0ref66:
	.byte $ff, $ff, $ff, $df, $ff, $ff, $ff, $df, $42
	.word @song0ch1loop
@song0ch2:
@song0ch2loop:
	.byte $80, $20
@song0ref80:
	.byte $9b, $00, $9b, $20, $9b, $00, $9b, $20, $9b, $00, $9b, $1b
	.byte $41, $0c
	.word @song0ref80
	.byte $9b, $00, $9b, $1e
	.byte $41, $0b
	.word @song0ref80
	.byte $23, $9b, $00, $9b, $22, $9b, $00, $9b, $20, $9b, $00, $9b, $1e, $9b, $00, $9b, $20, $9b, $00, $d7, $42
	.word @song0ch2loop
@song0ch3:
@song0ch3loop:
	.byte $41, $08
	.word @song0ref66
	.byte $42
	.word @song0ch3loop
@song0ch4:
@song0ch4loop:
	.byte $41, $08
	.word @song0ref66
	.byte $42
	.word @song0ch4loop
