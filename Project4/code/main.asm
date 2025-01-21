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
    BANKSEL BUTTONS    ; Select bank for BUTTONS
    MOVF    BUTTONS, W ; Move BUTTONS state to W
    BANKSEL LEDS       ; Select bank for LEDS
    
    ; Check each button and toggle corresponding LED
    BTFSS   W, 4       ; Check if RB4 is pressed
    GOTO    toggle_RC4 ; If pressed, toggle RC4
    BTFSS   W, 5       ; Check if RB5 is pressed
    GOTO    toggle_RC5 ; If pressed, toggle RC5
    BTFSS   W, 6       ; Check if RB6 is pressed
    GOTO    toggle_RC6 ; If pressed, toggle RC6
    BTFSS   W, 7       ; Check if RB7 is pressed
    GOTO    toggle_RC7 ; If pressed, toggle RC7

    GOTO    loop       ; Stay in infinite loop

toggle_RC4
    XORWF   LEDS, F    ; Toggle RC4
    GOTO    loop

toggle_RC5
    XORWF   LEDS, F    ; Toggle RC5
    GOTO    loop

toggle_RC6
    XORWF   LEDS, F    ; Toggle RC6
    GOTO    loop

toggle_RC7
    XORWF   LEDS, F    ; Toggle RC7
    GOTO    loop


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

 	; Store W (level) into temp variable
    CLRF    tens          ; Clear tens register
    CLRF    ones          ; Clear ones register

tens_loop
    MOVF    temp, W       ; Load current value of temp into W
    SUBLW   10            ; Subtract 10 from W
    BTFSS   STATUS, C     ; If result is negative (W < 10), stop loop
    GOTO    ones_calc     ; Go to calculate ones digit
    MOVWF   temp          ; Update temp with the new value
    INCF    tens, F       ; Increment tens counter
    GOTO    tens_loop     ; Repeat the loop

ones_calc
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