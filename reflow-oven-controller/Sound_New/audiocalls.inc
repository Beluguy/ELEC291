cseg

main_player_1sec:

    push acc
    push psw

    clr

    mov dptr, #sound_index+6


    lcall playnum

	; multiply R0 by 3 and add it to the dptr
	mov a, R0
	mov b, #3
	mul ab
	add a, dpl
	mov dpl, a
	mov a, b
	addc a, dph
	mov dph, a
	
	; dptr is pointing to the MSB of the 24-bit flash memory address
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	inc dptr
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	inc dptr
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	;frequency: 22050
	;frequency * time to play = bits to play-> convert decimal to hex
	; How many bytes to play? All of them!  Asume 4Mbytes memory: 0x3fffff
	mov w+2, #0x00
	mov w+1, #0x56
	mov w+0, #0x22

    pop acc
    pop psw

ret

main_player_2sec:

    push acc
    push psw

    lcall playnum

	; multiply R0 by 3 and add it to the dptr
	mov a, R0
	mov b, #3
	mul ab
	add a, dpl
	mov dpl, a
	mov a, b
	addc a, dph
	mov dph, a
	
	; dptr is pointing to the MSB of the 24-bit flash memory address
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	inc dptr
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	inc dptr
	clr a
	movc a, @a+dptr
	lcall Send_SPI
	
	;frequency: 22050
	;frequency * time to play = bits to play-> convert decimal to hex
	; How many bytes to play? All of them!  Asume 4Mbytes memory: 0x3fffff
	mov w+2, #0x00
	mov w+1, #0xAC
	mov w+0, #0x44

    pop acc
    pop psw

ret

Load_X MAC
	mov x+0, #low (%0 % 0x10000) 
	mov x+1, #high(%0 % 0x10000) 
	mov x+2, #low (%0 / 0x10000) 
	mov x+3, #high(%0 / 0x10000) 
ENDMAC

Load_Y MAC
	mov y+0, #low (%0 % 0x10000) 
	mov y+1, #high(%0 % 0x10000) 
	mov y+2, #low (%0 / 0x10000) 
	mov y+3, #high(%0 / 0x10000) 
ENDMAC

findones:

    check9:
    cjne a, #9, check8
    lcall nine

    check8:
    cjne a, #8, check7
    lcall eight

    check7:
    cjne a, #7, check6
    lcall seven

    check6:
    cjne a, #6, check5
    lcall six

    check5:
    cjne a, #5, check4
    lcall five

    check4:
    cjne a, #4, check3
    lcall four

    check3:
    cjne a, #3, check2
    lcall three

    check2:
    cjne a, #2, check1
    lcall two

    check1:
    lcall one

 ret
    
findtens:
    check90:
    cjne a, #9, check80
    lcall ninety

    check80:
    cjne a, #8, check70
    lcall eighty

    check70:
    cjne a, #7, check60
    lcall seventy

    check60:
    cjne a, #6, check50
    lcall sixty

    check50:
    cjne a, #5, check40
    lcall fifty

    check40:
    cjne a, #8, check30
    lcall fourty

    check30:
    cjne a, #8, check20
    lcall thirty

    check20:
    cjne a, #2, check10
    lcall twenty

    check10:
    lcall ten

ret

;play x

;main


playnum:
    ;push acc
    ;push psw

    checkhundreds:

        mov a, x
        mov b, #100
        div ab
        jz checktens
        lcall findones
        lcall hundred    

    checktens: 

        mov a, b
        mov b, #10
        div ab
        jz findtens
        lcall findtens

    checkones: 

        mov a, b
        lcall findones

    ;pop psw
    ;pop acc
ret



one:
    mov dptr, #sound_index+0
    ret

two:
    mov dptr, #sound_index+1
    ret

three:
    ret
    
four:
    ret

five:
    ret

six:
    ret

seven:
    ret

eight:
    ret

nine:
    ret

ten:
    ret

twenty:
    ret

thirty:
    ret

fourty:
    ret

fifty:
    ret

sixty:
    ret 

seventy:
    ret

eighty:
    ret

ninety:
    ret

hundred:
    ret


