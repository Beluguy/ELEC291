$NOLIST
;----------------------------------------------------
; marcos for project 1 
;----------------------------------------------------

CSEG

Send_BCD mac
	push ar0
	mov r0, %0
	lcall ?Send_BCD
	pop ar0
endmac

?Send_BCD:
	push acc
	mov a, r0
	swap a
	anl a, #0fh
	orl a, #30h
	lcall putchar
	mov a, r0
	anl a, #0fh
	orl a, #30h
	lcall putchar
	pop acc
	ret

Left_blank mac
	mov a, %0
	anl a, #0xf0
	swap a
	jz Left_blank_%M_a
	ljmp %1
Left_blank_%M_a:
	Display_char(#' ')
	mov a, %0
	anl a, #0x0f
	jz Left_blank_%M_b
	ljmp %1
Left_blank_%M_b:
	Display_char(#' ')
endmac

inc_setting MAC
    mov a, %0
    add a, #1
    mov %0, a
ENDMAC

dec_setting MAC
    mov a, %0
    cjne a, #0, dec_setting_next_%M
    sjmp dec_setting_end_%M
dec_setting_next_%M:
    subb a, #1
    mov %0, a
dec_setting_end_%M:
ENDMAC

; macro for Load_Configuration
getbyte mac
    clr a
    movc a, @a+dptr
    mov %0, a
    inc dptr
ENDMAC

; marco for save_config
loadbyte mac
	mov a, %0
	movx @dptr, a
	inc dptr
ENDMAC
