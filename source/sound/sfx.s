; This file is for the FamiStudio Sound Engine and was generated by FamiStudio


.if FAMISTUDIO_CFG_C_BINDINGS
.export _sounds=sounds
.endif

sounds:
	.word @ntsc
	.word @ntsc
@ntsc:
	.word @sfx_ntsc_death
	.word @sfx_ntsc_door
	.word @sfx_ntsc_hit
	.word @sfx_ntsc_jump
	.word @sfx_ntsc_pause

@sfx_ntsc_death:
	.byte $81,$26,$82,$03,$80,$3f,$89,$f0,$06,$81,$f8,$05,$00
@sfx_ntsc_door:
	.byte $81,$70,$82,$00,$80,$3f,$89,$f0,$02,$81,$54,$01,$80,$30,$01,$81
	.byte $4f,$80,$3f,$02,$81,$70,$02,$81,$c9,$01,$80,$30,$01,$81,$fd,$80
	.byte $3f,$01,$00
@sfx_ntsc_hit:
	.byte $81,$64,$82,$00,$80,$3f,$89,$f0,$02,$81,$86,$01,$81,$e1,$01,$81
	.byte $3f,$82,$01,$02,$81,$1a,$82,$02,$02,$81,$89,$82,$03,$01,$81,$b8
	.byte $82,$04,$01,$81,$f1,$82,$07,$01,$00
@sfx_ntsc_jump:
	.byte $81,$1a,$82,$02,$80,$3f,$89,$f0,$01,$81,$df,$82,$01,$01,$81,$ab
	.byte $01,$80,$30,$01,$81,$52,$80,$3f,$01,$81,$3f,$01,$81,$0c,$01,$00
@sfx_ntsc_pause:
	.byte $81,$9f,$82,$00,$80,$3f,$89,$f0,$03,$80,$30,$03,$80,$3f,$03,$00

.export sounds