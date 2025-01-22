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

#define BUTTONS PORTB
#define LEDS    PORTC

COUNTER    EQU    30
OVERFLOW_CNT    EQU    31
game_state    EQU 32            ; Reserve one byte for game state
current_level 	EQU 33            ; Reserve one byte for current level

random_reg  EQU 34            ; Reserve space for the random value
temp    EQU 35            ; Temporary variable to store the value
tens    EQU 36            ; Reserve one byte for tens digit
ones    EQU 37            ; Reserve one byte for ones digit


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;             Program begins
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ORG 0       ; Default start address 
    NOP         ; Required for ICD mode
    GOTO    start       ; Jump to main program


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        Interrupt Service Routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	ORG	004
	
	BANKSEL  PIR1
    BTFSC 	PIR1,0
    goto 	TIMER1_ISR
    BANKSEL INTCON
	BTFSC   INTCON, 2  
	goto 	TIMER0_ISR

	BTFSS INTCON, 0	; Check if the source of interrupt is change on RB4...7.
	goto other_int

	BANKSEL game_state        ; Select the bank for game_state
    MOVLW 1                   ; Load value 1 into W
    XORWF game_state,0 	  ; Compare W with game_state
    BTFSS STATUS, Z           ; If game_state != 1
    GOTO set_game_state       ; Set game_state to 1 and exit
	BANKSEL  PORTB 
      

	BTFSC PORTB, 4	; Check if PortB.4 (column 4)
	goto check_col_2
    BANKSEL BUTTONS         ; Select bank for BUTTONS
    MOVF    BUTTONS, W      ; Read PORTB states
    BANKSEL LEDS            ; Select bank for LEDS
	XORWF   LEDS, F
	BCF INTCON, 0	; Reset the interrupt flag
	goto other_int

check_col_2
	BTFSC PORTB, 5	; Check if PortB.5 (column 5)
	goto check_col_3
	BANKSEL BUTTONS         ; Select bank for BUTTONS
    MOVF    BUTTONS, W      ; Read PORTB states
    BANKSEL LEDS            ; Select bank for LEDS
	XORWF   LEDS, F
	BCF INTCON, 0	; Reset the interrupt flag
	goto other_int

check_col_3
	BTFSC PORTB, 6	; Check if PortB.6 (column 6)
	goto other_int
	BANKSEL BUTTONS         ; Select bank for BUTTONS
    MOVF    BUTTONS, W      ; Read PORTB states
    BANKSEL LEDS            ; Select bank for LEDS
	XORWF   LEDS, F
	BCF INTCON, 0	; Reset the interrupt flag
	goto other_int

check_col_4
	BTFSC PORTB, 7	; Check if PortB.7 (column 7)
	goto other_int
    BANKSEL BUTTONS         ; Select bank for BUTTONS
    MOVF    BUTTONS, W      ; Read PORTB states
    BANKSEL LEDS            ; Select bank for LEDS
	XORWF   LEDS, F
	BCF INTCON, 0	; Reset the interrupt flag
	goto other_int


set_game_state
    BANKSEL game_state        ; Select bank for game_state
    MOVLW 1                   ; Load value 1
    MOVWF game_state          ; Set game_state = 1
    BCF INTCON, 0             ; Reset the interrupt flag
    GOTO start_after_press

other_int
	movf PORTB,1 	; Read PortB (to itself) to end mismatch condition
	RETFIE			; Return from interrupt



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
    MOVLW    B'11101000'    ; enable GIE (bit 7), RBIE (bit 3)
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

    CALL    inid    			; Initialize the display


wait GOTO    wait

 
start_after_press

	CALL    random_value
    CALL    print_level

 	BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0xC0            ; Load the command for setting the cursor to the second row
    CALL    send            ; Send the command using the send subroutine

	CALL    random_value
    CALL    print_level

	CALL    one_second_delay
	CALL    print_try_again
	CALL    one_second_delay
	CALL    print_congratulations
	CALL    one_second_delay
	CALL 	print_press_any_button
	CALL    one_second_delay
	CALL 	clear_display

	RETFIE

loop 
	GOTO    loop       ; Stay in infinite loop


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;           Print Level Subroutine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


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
    MOVLW   10          ; Update temp with the new value
    SUBWF   temp,1            ; Subtract 10 from W
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
    MOVLW   D'2'        ; Wait for 2ms for the display to process
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
    BANKSEL PORTC       ; Select bank 0
    BSF PORTC,5      ; Clear LED outputs


EXIT_ISR:
    RETFIE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 		 	Timer two second
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


TIMER0_ISR:
    
    BCF     INTCON, 2      ; Clear Timer0 interrupt flag
    INCF    COUNTER, F     ; Increment the overflow counter
    MOVLW   45             ; Compare counter with 31 overflows (2 seconds)
    SUBWF   COUNTER,0      ; Subtract 10 from W
    BTFSS   STATUS, Z     
    GOTO    Exit_ISR       ; If not, exit ISR
    CLRF    COUNTER        ; Reset counter after 31 overflows (2 seconds)
	BSF PORTC,5      ; Clear LED outputs
    
	BANKSEL PORTC       ; Select bank 0
    BSF PORTC,5      ; Clear LED outputs

Exit_ISR:
   RETFIE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 		Variables declaration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


generated_sequence   RES     22      ; Allocate an array of 22 bytes
user_sequence   RES     22      ; Allocate an array of 22 bytes



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       Include LCD driver routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    INCLUDE "LCDIS.INC"

    END