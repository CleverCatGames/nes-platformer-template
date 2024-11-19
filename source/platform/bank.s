.export _bank_switch_save,_bank_restore,_bank_call
.import _bank_switch
.import popax,callax,tmp1
.importzp CURRENT_BANK

.ZEROPAGE

BANK_SAVE:          .res 1

.CODE

;void __fastcall__ bank_switch_save(u8 bank);

_bank_switch_save:
	ldx CURRENT_BANK
	stx BANK_SAVE
	jmp _bank_switch


;void __fastcall__ bank_restore(void);

_bank_restore:
	lda BANK_SAVE
	jmp _bank_switch ; call mapper specific bank function


;void __fastcall__ bank_restore(callbackDef func, u8 bank);

_bank_call:
    tay
    lda CURRENT_BANK
    pha               ; push current bank on stack
    tya
    jsr _bank_switch  ; change to requested bank

    jsr popax
    jsr callax        ; call requested func

    pla               ; pull current bank
    jmp _bank_switch  ; switch back to original bank

