.exportzp _obj_id
.exportzp _obj_active_id
.exportzp _object_type
.exportzp _object_timer
.exportzp _object_state
.exportzp _object_y
.exportzp _collide_result
.exportzp _vram_buffer_end
.exportzp _vram_offset
.exportzp _time_hours,_time_minutes,_time_seconds,_time_frames
.exportzp _pTemp
.exportzp _pLevelBuffer
.exportzp _pMetatileAttr,_pMetatileData
.exportzp _pTilePtr
.exportzp _pMapData
.exportzp _tmp1, _tmp2, _tmp3, _tmp4, _tmp5

; Use this file to define zero page variables
; To use in C they must start with an underscore and be defined as zpsym in var_defines.h

.ZEROPAGE

_obj_id: .res 1
_obj_active_id: .res 1
_object_type: .res 1
_object_state: .res 1
_object_y: .res 1
_object_timer: .res 1
_collide_result: .res 1
_vram_buffer_end: .res 1
_vram_offset: .res 1
_pTemp: .res 2
_pTilePtr: .res 2
_pLevelBuffer: .res 2
_pMapData: .res 2
_pMetatileAttr: .res 2
_pMetatileData: .res 2

; time values
_time_hours: .res 1
_time_minutes: .res 1
_time_seconds: .res 1
_time_frames: .res 1

; C temp variables
_tmp1:              .res 1
_tmp2:              .res 1
_tmp3:              .res 1
_tmp4:              .res 1
_tmp5:              .res 1

