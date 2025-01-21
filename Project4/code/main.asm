;----------------------------------------------------
;----------------------------------------------------
;                  MAIN.ASM        21-01-2025
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




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;             Program begins
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ORG 0       ; Default start address 
    NOP         ; Required for ICD mode

    BANKSEL TRISC       ; Select bank 1
    CLRF    TRISC       ; Set PORTC as output
    MOVLW   0xF0        ; Set RB4-RB7 as input (1) and others as output (0)
    MOVWF   TRISB       ; Configure TRISB register
	CLRF    TRISD       ; Display port is output

    BANKSEL PORTC       ; Select bank 0
    CLRF    PORTC       ; Clear LED outputs
	CLRF    PORTD       ; Clear display outputs

    GOTO    start       ; Jump to main program


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        Interrupt Service Routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	ORG	004
	BTFSS INTCON, 0	; Check if the source of interrupt is change on RB4...7.
	goto other_int

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

other_int
	movf PORTB,1 	; Read PortB (to itself) to end mismatch condition
	RETFIE			; Return from interrupt



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;               MAIN LOOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

start   
    CALL    inid    ; Initialize the display


    MOVLW   1   ; Load 1 into W for level 1
    CALL    print_level

 	BCF     Select, RS      ; Ensure command mode (RS = 0)
    MOVLW   0xC0            ; Load the command for setting the cursor to the second row
    CALL    send            ; Send the command using the send subroutine

    MOVLW   22   ; Load 22 into W for level 22
    CALL    print_level

loop 
	GOTO    loop       ; Stay in infinite loop


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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Variables declaration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

temp    RES 1            ; Temporary variable to store the value
tens    RES 1            ; Reserve one byte for tens digit
ones    RES 1            ; Reserve one byte for ones digit


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;              Subroutine for LCD
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

display_char    
    BSF  Select,  RS    ; Select data mode
    CALL    send        ; Send the character to LCD
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       Include LCD driver routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    INCLUDE "LCDIS.INC"

    END