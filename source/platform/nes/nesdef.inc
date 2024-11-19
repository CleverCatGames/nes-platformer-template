.globalzp TEMP
.globalzp PPU_CTRL_VAR

PPU_CTRL     = $2000
PPU_MASK     = $2001
PPU_STATUS   = $2002
PPU_OAM_ADDR = $2003
PPU_OAM_DATA = $2004
PPU_SCROLL   = $2005
PPU_ADDR     = $2006
PPU_DATA     = $2007
PPU_OAM_DMA  = $4014
DMC_FREQ     = $4010
CTRL_PORT1   = $4016
CTRL_PORT2   = $4017

PAL_BUF      = $01c0
OAM_BUF      = $0200

PTR         =TEMP   ;word
LEN         =TEMP+2 ;word
SRC         =TEMP+4 ;word
DST         =TEMP+6 ;word
