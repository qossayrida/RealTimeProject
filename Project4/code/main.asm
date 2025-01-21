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

    ; Display "HELLO" on the LCD
    MOVLW   'H'
    CALL    display_char
    MOVLW   'E'
    CALL    display_char
    MOVLW   'L'
    CALL    display_char
    MOVLW   'L'
    CALL    display_char
    MOVLW   'O'
    CALL    display_char

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
;              Subroutine for LCD
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

display_char    
    BSF  Select,  RS   ; Select data mode
    CALL    send        ; Send the character to LCD
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       Include LCD driver routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    INCLUDE "LCDIS.INC"

    END