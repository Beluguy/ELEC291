cseg

main_player_1sec:

    push acc
    push psw
    lcall playnum
    pop acc
    pop psw
ret
    
play:
;push pc
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

    setb SPEAKER ; Turn on speaker.
	setb TR1 ; Start playback by enabling Timer 1
    

ret

play_3sec:

    push acc
    push psw
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
	mov w+2, #0x01
	mov w+1, #0x02
	mov w+0, #0x66

    pop acc
    pop psw

ret

play_state:

    ;push acc
    ;push psw
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
	; How many bytes togt5 play? All of them!  Asume 4Mbytes memory: 0x3fffff
	mov w+2, #0x00
	mov w+1, #0xAC
	mov w+0, #0x44

    ;pop acc
    ;pop psw

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
;push pc
    check9:
    cjne a, #9, check8
    lcall nine
    ljmp done

    check8:
    cjne a, #8, check7
    lcall eight
    ljmp done

    check7:
    cjne a, #7, check6
    lcall seven
    ljmp done

    check6:
    cjne a, #6, check5
    lcall six
    ljmp done

    check5:
    cjne a, #5, check4
    lcall five
    ljmp done

    check4:
    cjne a, #4, check3
    lcall four
    ljmp done

    check3:
    cjne a, #3, check2
    lcall three
    ljmp done

    check2:
    cjne a, #2, check1
    lcall two
    ljmp done

    check1:
    cjne a, #1, done
    lcall one
    
    done:
 ret
    
findtens:
    check90:
    cjne a, #9, check80
    lcall ninety
    ljmp done2

    check80:
    cjne a, #8, check70
    lcall eighty
    ljmp done2

    check70:
    cjne a, #7, check60
    lcall seventy
    ljmp done2

    check60:
    cjne a, #6, check50
    lcall sixty
    ljmp done2

    check50:
    cjne a, #5, check40
    lcall fifty
    ljmp done2

    check40:
    cjne a, #4, check30
    lcall fourty
    ljmp done2

    check30:
    cjne a, #3, check20
    lcall thirty
    ljmp done2

    check20:
    cjne a, #2, check10
    lcall twenty
    ljmp done2

    check10:
    cjne a, #1, done
    lcall ten

    done2:
ret

playnum:
;push pc
    checkhundreds:

        mov a, x
        mov b, #100
        div ab
        jz checktens
        lcall findones
        ;mov a, #0
        ;mov dptr,#0
        mov w+2, #0x00
        mov w+1, #0x56
        mov w+0, #0x22
        lcall hundred   
     

    checktens: 
        mov a, b
        mov b, #10
        div ab
        jz checkones
        lcall findtens
        mov dptr, #0
        ;cjne a, #0, checkones

    checkones: 
        mov a, b
        lcall findones
    
    donefinal:

ret

one:
    mov dptr, #sound_index+0
    lcall play
    ret

two:
    ;push pc
    mov dptr, #sound_index+3
    lcall play
    ;ljmp hundred
    ret

three:
    mov dptr, #sound_index+6
    lcall play
    ret
    
four:
    mov dptr, #sound_index+9
    lcall play
    ret

five:
    mov dptr, #sound_index+12
    lcall play
    ret

six:
    mov dptr, #sound_index+15
    lcall play
    ret

seven:
    mov dptr, #sound_index+21
    lcall play
    ret

eight:
    mov dptr, #sound_index+24
    lcall play
    ret

nine:
    mov dptr, #sound_index+27
    lcall play
    ret

ten:
    mov dptr, #sound_index+30
    lcall play
    ret

twenty:
    mov dptr, #sound_index+33
    lcall play
    ret

thirty:
    mov dptr, #sound_index+36
    lcall play
    ret

fourty:
    mov dptr, #sound_index+39
    lcall play
    ret

fifty:
    mov dptr, #sound_index+42
    lcall play
    ret

sixty:
    mov dptr, #sound_index+48
    lcall play
    ret 

seventy:
    mov dptr, #sound_index+54
    lcall play
    ret

eighty:
    mov dptr, #sound_index+57
    lcall play
    ret

ninety:
    mov dptr, #sound_index+60
    lcall play
    ret

hundred:
    ;push pc
    mov dptr, #sound_index+63
    lcall play
    ret

degree_celsius:
    mov dptr, #sound_index+66
    lcall play_state
    ret

current_state:
    mov dptr, #sound_index+72
    lcall play_state
    ret

ramp_to_soak:
    mov dptr, #sound_index+75
    lcall play_state
    ret

soak:
    mov dptr, #sound_index+81
    lcall play_state
    ret

ramp_to_peak:
    mov dptr, #sound_index+87
    lcall play_state
    ret

reflow:
    mov dptr, #sound_index+99
    lcall play_state
    ret

cooling:
    mov dptr, #sound_index+105
    lcall play_state
    ret

safe_temp:
    mov dptr, #sound_index+108
    lcall play_3sec
    ret

pcb_hot:
    mov dptr, #sound_index+120
    lcall play_state
    ret
