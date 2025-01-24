;----------------------------------------------------
;----------------------------------------------------
;         MAIN.ASM        21-01-2025
;----------------------------------------------------
;----------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;             Configuration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    PROCESSOR 16F877
    __CONFIG 0x3731 
    INCLUDE "P16F877A.INC"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                Data Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

COUNTER         EQU    30
OVERFLOW_CNT    EQU    31
game_state      EQU    32            ; Reserve one byte for game state
current_level   EQU    33            ; Reserve one byte for current level

random_reg      EQU    34            ; Reserve space for the random value
temp            EQU    35            ; Temporary variable to store the value
tens            EQU    36            ; Reserve one byte for tens digit
ones            EQU    37            ; Reserve one byte for ones digit
loop_size       EQU    38            ; Reserve one byte for loop_size

index           	 EQU 0xA0  ; Reserve one byte for loop_size
save_random			 EQU 0xA1
generated_sequence   EQU 0xA2  ; Starting address of generated_sequence
user_sequence        EQU 0xB8  ; Starting address of user_sequence (A0H + 22 bytes)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;             Program begins
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ORG 0x00       ; Default start address 
    NOP         ; Required for ICD mode
    GOTO    start       ; Jump to main program


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        Interrupt Service Routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ORG     0x04

ISR:
    ; Check if the interrupt is from Port B change
    BTFSS   INTCON, RBIF       ; Check if the source of interrupt is change on RB4...7.
    GOTO    other_int          ; If not, handle other interrupts

              
    ; Check which button was pressed and flash the corresponding LED
    BANKSEL PORTB              ; Select the bank for PORTB

    ; Check if RB4 was pressed
    BTFSS   PORTB, 4           ; Check if RB4 is low (button pressed)
    GOTO    check_RB5          ; If not, check RB5

	; Handle Port B change interrupt
    BANKSEL game_state         ; Select the bank for game_state
    MOVF    game_state, W      ; Load game_state into W
    BTFSC   STATUS, Z          ; Check if game_state == 0 (first time)
    GOTO    set_game_state ; If not first time, handle button press

    ; Flash RC4 for one second
    BANKSEL PORTC              ; Select the bank for PORTC
    BSF     PORTC, 4           ; Set RC4 high
    CALL    one_second_delay   ; Delay for one second
    BCF     PORTC, 4           ; Set RC4 low

    ; Store 1 in user_sequence
    BANKSEL user_sequence      ; Select the bank for user_sequence
    MOVLW   1                  ; Load value 1 into W
    CALL    store_user_input   ; Store the value in user_sequence
    GOTO    end_isr            ; Skip to the end of the ISR

check_RB5:
    ; Check if RB5 was pressed
    BTFSS   PORTB, 5           ; Check if RB5 is low (button pressed)
    GOTO    check_RB6          ; If not, check RB6

	; Handle Port B change interrupt
    BANKSEL game_state         ; Select the bank for game_state
    MOVF    game_state, W      ; Load game_state into W
    BTFSC   STATUS, Z          ; Check if game_state == 0 (first time)
    GOTO    set_game_state ; If not first time, handle button press

    ; Flash RC5 for one second
    BANKSEL PORTC              ; Select the bank for PORTC
    BSF     PORTC, 5           ; Set RC5 high
    CALL    one_second_delay   ; Delay for one second
    BCF     PORTC, 5           ; Set RC5 low

    ; Store 2 in user_sequence
    BANKSEL user_sequence      ; Select the bank for user_sequence
    MOVLW   2                  ; Load value 2 into W
    CALL    store_user_input   ; Store the value in user_sequence
    GOTO    end_isr            ; Skip to the end of the ISR

check_RB6:
    ; Check if RB6 was pressed
    BTFSS   PORTB, 6           ; Check if RB6 is low (button pressed)
    GOTO    check_RB7          ; If not, check RB7

	; Handle Port B change interrupt
    BANKSEL game_state         ; Select the bank for game_state
    MOVF    game_state, W      ; Load game_state into W
    BTFSC   STATUS, Z          ; Check if game_state == 0 (first time)
    GOTO    set_game_state ; If not first time, handle button press

    ; Flash RC6 for one second
    BANKSEL PORTC              ; Select the bank for PORTC
    BSF     PORTC, 6           ; Set RC6 high
    CALL    one_second_delay   ; Delay for one second
    BCF     PORTC, 6           ; Set RC6 low

    ; Store 3 in user_sequence
    BANKSEL user_sequence      ; Select the bank for user_sequence
    MOVLW   3                  ; Load value 3 into W
    CALL    store_user_input   ; Store the value in user_sequence
    GOTO    end_isr            ; Skip to the end of the ISR

check_RB7:
    ; Check if RB7 was pressed
    BTFSS   PORTB, 7           ; Check if RB7 is low (button pressed)
    GOTO    end_isr            ; If not, skip to the end of the ISR

	; Handle Port B change interrupt
    BANKSEL game_state         ; Select the bank for game_state
    MOVF    game_state, W      ; Load game_state into W
    BTFSC   STATUS, Z          ; Check if game_state == 0 (first time)
    GOTO    set_game_state ; If not first time, handle button press

    ; Flash RC7 for one second
    BANKSEL PORTC              ; Select the bank for PORTC
    BSF     PORTC, 7           ; Set RC7 high
    CALL    one_second_delay   ; Delay for one second
    BCF     PORTC, 7           ; Set RC7 low

    ; Store 4 in user_sequence
    BANKSEL user_sequence      ; Select the bank for user_sequence
    MOVLW   4                  ; Load value 4 into W
    CALL    store_user_input   ; Store the value in user_sequence

set_game_state:
    ; First time: Set game_state to 1 and do nothing else
    MOVLW   1                  ; Load value 1 into W
    MOVWF   game_state         ; Set game_state = 1

end_isr:
    ; Clear the Port B change interrupt flag
    BCF     INTCON, RBIF       ; Clear the Port B change interrupt flag
    RETFIE                     ; Return from interrupt

other_int:
    ; Handle other interrupts (Timer 0, Timer 1, etc.)
    BANKSEL INTCON
	BTFSC   INTCON, T0IF   
	goto 	TIMER0_ISR

	BANKSEL  PIR1
    BTFSC 	PIR1,TMR1IF 
    goto 	TIMER1_ISR

    RETFIE                     ; Return from interrupt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       Store User Input in user_sequence
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

store_user_input:
	BANKSEL save_random
	MOVWF	save_random

	MOVLW   user_sequence   	 ; Load the base address of generated_sequence
    ADDWF   index, W             ; Add the index to the base address
    MOVWF   FSR                  ; Store the result in FSR
	MOVF    save_random,W
    MOVWF   INDF                 ; Store W at the address pointed to by FSR

    ; Increment index for the next value
    INCF    index, F             ; Increment index for the next value
    RETURN



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;               MAIN LOOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

start   

	BANKSEL TRISC       ; Select bank 1
    CLRF    TRISC       ; Set PORTC as output
    MOVLW   0xF0        ; Set RB4-RB7 as input (1) and others as output (0)
    MOVWF   TRISB       ; Configure TRISB register
    CLRF    TRISD       ; Display port is output

    BANKSEL PORTC       ; Select bank 0
    CLRF    PORTC       ; Clear LED outputs
    CLRF    PORTD       ; Clear display outputs
    MOVLW    B'10001000'    ; enable GIE (bit 7), RBIE (bit 3)
    MOVWF    INTCON        ; PortB pins

	;**************** TIMER2 CONFIG ****************
	MOVLW   0x00            ; Clear the Timer 2 control register
    MOVWF   T2CON           ; Initialize Timer 2 control register
    MOVLW   0x01            ; Load prescaler value (prescaler 1:4)
    MOVWF   T2CON           ; Apply the prescaler value
    BSF     T2CON, TMR2ON   ; Enable Timer 2

	;**************** TIMER1 CONFIG ****************
	BANKSEL OVERFLOW_CNT
    CLRF    OVERFLOW_CNT        ; Clear overflow counter
    MOVLW   B'00110001'          ; Prescaler 1:8, Internal clock, Timer1 ON
    BANKSEL T1CON
    MOVWF   T1CON          ; Load into T1CON register

    ; Load TIMER1 initial value (TMR1H:TMR1L = 0x17B8 for 59,464 ticks)
    MOVLW   0x17           ; High byte of initial value
    MOVWF   TMR1H          ; Load TMR1H
    MOVLW   0xB8           ; Low byte of initial value
    MOVWF   TMR1L          ; Load TMR1L

	BANKSEL  PIE1
   	BSF     PIE1, 0   ; Enable Timer1 overflow interrupt
   	BANKSEL INTCON

	;**************** TIMER0 CONFIG ****************
	CLRF    COUNTER        ; Clear overflow counter
    MOVLW   B'10000111'           ; Set prescaler to 1:256
    BANKSEL OPTION_REG
    MOVWF   OPTION_REG     ; Configure prescaler for Timer0
    MOVLW   0x85          ; Preload Timer0 with 133 (0x85 in hex, 133 in decimal)
    MOVWF   TMR0           ; Load Timer0 with preload value

	
	BANKSEL game_state        ; Select bank for game_state
    MOVLW 0                   ; Load value 0
    MOVWF game_state          ; Set game_state = 0

	BANKSEL current_level     ; Select bank for current_level
    MOVLW 0                   ; Load value 0
    MOVWF current_level       ; Set current_level = 0

    CALL    inid    			; Initialize the display
	


	CALL 	print_press_any_button
	CALL    one_second_delay
	CALL 	clear_display
	
	MOVLW   0
    XORWF   game_state, 0     ; Compare W with game_state
    BTFSS   STATUS, Z         ; If game_state != 1, skip to set_game_state
	GOTO    wait_until_press
	CALL 	print_press_any_button
	CALL    one_second_delay
	CALL 	clear_display

    MOVLW   0         
    XORWF   game_state, 0     ; Compare W with game_state
    BTFSS   STATUS, Z         ; If game_state != 1, skip to set_game_state
	GOTO    wait_until_press
	CALL 	print_press_any_button
	CALL    one_second_delay
	CALL 	clear_display

	MOVLW   0
    XORWF   game_state, 0     ; Compare W with game_state
    BTFSS   STATUS, Z         ; If game_state != 1, skip to set_game_state
	GOTO    wait_until_press
	CALL 	print_press_any_button


wait_until_press:
    BANKSEL game_state        ; Select the bank for game_state
    MOVLW   1                 ; Load value 1 into W
    XORWF   game_state, 0     ; Compare W with game_state
    BTFSS   STATUS, Z         ; If game_state != 1, skip to set_game_state
    GOTO    wait_until_press  ; If game_state == 0, stay in the loop
    GOTO    continue_program  ; If game_state == 1, continue with the program



continue_program:
    ; Disable interrupts
    BANKSEL INTCON          ; Select the bank containing INTCON
    BCF     INTCON, GIE     ; Clear the Global Interrupt Enable (GIE) bit to disable interrupts
	
    CALL    clear_display

    ; Increment current_level and print it
    BANKSEL current_level
    INCF    current_level, F ; current_level = current_level + 1
    MOVF    current_level, W
    CALL    print_level

    ; Declare loop_size and set it to current_level + 2
    BANKSEL current_level
    MOVF    current_level, W ; Load current_level into W
    ADDLW   D'2'             ; Add 2 to W (current_level + 2)
    BANKSEL loop_size        ; Select the bank for loop_size
    MOVWF   loop_size        ; Store the result in loop_size

    ; Initialize index for generated_sequence
    BANKSEL generated_sequence
    CLRF    index            ; Clear index (start from 0)

main_loop:
    ; Generate a random value (1-4)
    CALL    random_value     ; random_value returns a value (1-4) in W
	BANKSEL save_random
	MOVWF	save_random	 		

    ; Save the random value in generated_sequence[index]
    MOVLW   generated_sequence   ; Load the base address of generated_sequence
    ADDWF   index, W             ; Add the index to the base address
    MOVWF   FSR                  ; Store the result in FSR
	MOVF    save_random,W
    MOVWF   INDF                 ; Store W at the address pointed to by FSR

    ; Increment index for the next value
    INCF    index, F             ; Increment index for the next value

    ; Check the random value and turn on the corresponding RC pin
    BANKSEL PORTC                ; Select the bank for PORTC
    MOVWF   temp                 ; Store the random value in temp

    ; Check if random value is 1
    MOVLW   D'1'
    SUBWF   temp, W              ; Subtract 1 from the random value
    BTFSS   STATUS, Z            ; Skip if random value == 1
    GOTO    check_2              ; If not 1, check for 2

    ; Turn on RC4 for one second
    BSF     PORTC, 4             ; Set RC4 high
    CALL    one_second_delay     ; Delay for one second
    BCF     PORTC, 4             ; Set RC4 low
    GOTO    end_loop             ; Skip to the end of the loop

check_2:
    ; Check if random value is 2
    MOVLW   D'2'
    SUBWF   temp, W              ; Subtract 2 from the random value
    BTFSS   STATUS, Z            ; Skip if random value == 2
    GOTO    check_3              ; If not 2, check for 3

    ; Turn on RC5 for one second
    BSF     PORTC, 5             ; Set RC5 high
    CALL    one_second_delay     ; Delay for one second
    BCF     PORTC, 5             ; Set RC5 low
    GOTO    end_loop             ; Skip to the end of the loop

check_3:
    ; Check if random value is 3
    MOVLW   D'3'
    SUBWF   temp, W              ; Subtract 3 from the random value
    BTFSS   STATUS, Z            ; Skip if random value == 3
    GOTO    check_4              ; If not 3, check for 4

    ; Turn on RC6 for one second
    BSF     PORTC, 6             ; Set RC6 high
    CALL    one_second_delay     ; Delay for one second
    BCF     PORTC, 6             ; Set RC6 low
    GOTO    end_loop             ; Skip to the end of the loop

check_4:
    ; Check if random value is 4
    MOVLW   D'4'
    SUBWF   temp, W              ; Subtract 4 from the random value
    BTFSS   STATUS, Z            ; Skip if random value == 4
    GOTO    end_loop             ; If not 4, skip to the end of the loop

    ; Turn on RC7 for one second
    BSF     PORTC, 7             ; Set RC7 high
    CALL    one_second_delay     ; Delay for one second
    BCF     PORTC, 7             ; Set RC7 low

end_loop:
    ; Decrement loop_size and check if it is zero
    BANKSEL loop_size
    DECFSZ  loop_size, F         ; Decrement loop_size and skip if zero
    GOTO    main_loop            ; If loop_size is not zero, repeat the loop

    ; Enable interrupts
    BANKSEL INTCON          ; Select the bank containing INTCON
    BSF     INTCON, GIE     ; Set the Global Interrupt Enable (GIE) bit to enable interrupts
	
	CLRF    index            ; Clear index (start from 0)

loop:
    GOTO    loop            ; Stay in infinite loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;           Print Level Subroutine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_level
    MOVWF   temp          ; Store W into temp

    ; Display "LEVEL "
    MOVLW   'L'
    CALL    display_char
    MOVLW   'E'
    CALL    display_char
    MOVLW   'V'
    CALL    display_char
    MOVLW   'E'
    CALL    display_char
    MOVLW   'L'
    CALL    display_char
    MOVLW   ' '          ; Space after "LEVEL"
    CALL    display_char

    ; Initialize tens and ones to zero
    CLRF    tens          ; Clear tens register
    CLRF    ones          ; Clear ones register

tens_loop
    MOVLW   10            ; Update temp with the new value
    SUBWF   temp,1        ; Subtract 10 from W
    BTFSS   STATUS, C     ; If result is negative (W < 10), stop loop
    GOTO    ones_calc     ; Go to calculate ones digit
    INCF    tens, F       ; Increment tens counter
    GOTO    tens_loop     ; Repeat the loop

ones_calc
    ADDWF   temp,1
    MOVF    temp, W       ; Remaining value in temp is the ones digit
    MOVWF   ones          ; Store it in ones register

    ; Convert tens digit to ASCII and display
    MOVF    tens, W       ; Load tens value into W
    ADDLW   '0'           ; Convert to ASCII
    CALL    display_char  ; Display tens digit

    ; Convert ones digit to ASCII and display
    MOVF    ones, W       ; Load ones value into W
    ADDLW   '0'           ; Convert to ASCII
    CALL    display_char  ; Display ones digit

    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;           Print "Press any button"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_press_any_button

	; Move cursor to the beginning of the first row
	BCF     Select, RS      ; Ensure command mode (RS = 0)
	MOVLW 0x80      ; Command to set cursor to the first row, first column
	CALL send       ; Send the command

    MOVLW   'P'
    CALL    display_char
    MOVLW   'r'
    CALL    display_char
    MOVLW   'e'
    CALL    display_char
    MOVLW   's'
    CALL    display_char
    MOVLW   's'
    CALL    display_char
    MOVLW   ' '
	CALL    display_char
	MOVLW   'a'
    CALL    display_char
    MOVLW   'n'
    CALL    display_char
    MOVLW   'y'
    CALL    display_char          
	MOVLW   ' '          
    CALL    display_char
	MOVLW   'b'
    CALL    display_char
    MOVLW   'u'
    CALL    display_char
    MOVLW   't'
    CALL    display_char
    MOVLW   't'
    CALL    display_char
    MOVLW   'o'
    CALL    display_char
    MOVLW   'n'          
    CALL    display_char

	BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0xC0            ; Load the command for setting the cursor to the second row
    CALL    send            ; Send the command using the send subroutine

	MOVLW   't'
    CALL    display_char
    MOVLW   'o'
    CALL    display_char
    MOVLW   ' '
    CALL    display_char
    MOVLW   's'
    CALL    display_char
    MOVLW   't'
    CALL    display_char
    MOVLW   'a'          
    CALL    display_char
	MOVLW   'r'
    CALL    display_char
    MOVLW   't'
    CALL    display_char

	RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         Print "Congratulations"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


print_congratulations

    ; Move cursor to the beginning of the first row
    BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0x80            ; Command to set cursor to the first row, first column
    CALL    send            ; Send the command

    ; Print "Congratulations"
    MOVLW   'C'
    CALL    display_char
    MOVLW   'o'
    CALL    display_char
    MOVLW   'n'
    CALL    display_char
    MOVLW   'g'
    CALL    display_char
    MOVLW   'r'
    CALL    display_char
    MOVLW   'a'
    CALL    display_char
    MOVLW   't'
    CALL    display_char
    MOVLW   'u'
    CALL    display_char
    MOVLW   'l'
    CALL    display_char
    MOVLW   'a'
    CALL    display_char
    MOVLW   't'
    CALL    display_char
    MOVLW   'i'
    CALL    display_char
    MOVLW   'o'
    CALL    display_char
    MOVLW   'n'
    CALL    display_char
    MOVLW   's'
    CALL    display_char

    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         Print "Try again"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_try_again

    ; Move cursor to the beginning of the first row
    BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0x80            ; Command to set cursor to the first row, first column
    CALL    send            ; Send the command

    ; Print "Try again"
    MOVLW   'T'
    CALL    display_char
    MOVLW   'r'
    CALL    display_char
    MOVLW   'y'
    CALL    display_char
    MOVLW   ' '
    CALL    display_char
    MOVLW   'a'
    CALL    display_char
    MOVLW   'g'
    CALL    display_char
    MOVLW   'a'
    CALL    display_char
    MOVLW   'i'
    CALL    display_char
    MOVLW   'n'
    CALL    display_char

    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;           one second delay
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

one_second_delay

    MOVLW    D'250'
    CALL    xms
	MOVLW    D'250'
    CALL    xms
	MOVLW    D'250'
    CALL    xms
	MOVLW    D'250'
    CALL    xms

    RETURN    


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;           Clear the LCD display
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

clear_display

	BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0x01        ; Command to clear the display
    CALL    send        ; Send the command to the LCD
    MOVLW   D'5'        ; Wait for 2ms for the display to process
    CALL    xms         ; Delay for 2ms
    RETURN              ; Done
           

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;              Subroutine for LCD
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

display_char    
    BSF  Select,  RS    ; Select data mode
    CALL    send        ; Send the character to LCD
    RETURN


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 		 Generate a random value
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

random_value

    BSF     T2CON, TMR2ON    ; Turn on Timer 2
    BSF     T2CON, T2CKPS0   ; Prescaler setting (e.g., 1:4, modify as needed)
    BSF     T2CON, T2CKPS1   ; Prescaler setting (e.g., 1:4, modify as needed)
    
    ; Wait for Timer 2 to overflow (set Timer 2 to generate a pseudo-random value)
    MOVF    TMR2, W          ; Move Timer 2 value into WREG
    XORLW   0xA3            ; XOR WREG with some constant (e.g., 0xA3) to introduce randomness
    MOVWF   random_reg      ; Store result in random_reg
    
    ; Mask the result to get a value between 0 and 3 (mod 4)
    MOVF    random_reg, W   ; Get the value from random_reg
    ANDLW   0x03            ; Mask the lower 2 bits (values from 0 to 3)
    
    ; Add 1 to make the range from 1 to 4
    ADDLW   0x01            ; Add 1 to the result (range now 1 to 4)
    
    ; Return with WREG containing a random value between 1 and 4
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 		 	Timer one minute
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TIMER1_ISR:
    BANKSEL  PIR1

    BCF     PIR1, 0   ; Clear Timer1 interrupt flag
    INCF    OVERFLOW_CNT, F ; Increment overflow counter

    MOVLW   0x75          ; Check if 2 overflows occurred

    SUBWF  OVERFLOW_CNT,0            ; Subtract 10 from W
    BTFSS   STATUS, Z
    GOTO    EXIT_ISR       ; If not 2, exit ISR
    CLRF    OVERFLOW_CNT   ; Reset overflow counter
    

	MOVLW   0x20
    BANKSEL PORTC            ; Select bank for LEDS
	XORWF   PORTC, F


EXIT_ISR:
    RETFIE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 		 	Timer two second
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


TIMER0_ISR:
    
    BCF     INTCON, 2      ; Clear Timer0 interrupt flag
    INCF    COUNTER, F     ; Increment the overflow counter
    MOVLW   99             ; Compare counter with 45 overflows (2 seconds)
    SUBWF   COUNTER,0      ; Subtract 10 from W
    BTFSS   STATUS, Z     
    GOTO    Exit_ISR       ; If not, exit ISR
    CLRF    COUNTER        ; Reset counter after 45 overflows (2 seconds)
    
	
    MOVLW   0x40
    BANKSEL PORTC            ; Select bank for LEDS
	XORWF   PORTC, F

Exit_ISR:
   RETFIE


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       Include LCD driver routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    INCLUDE "LCDIS.INC"

    END