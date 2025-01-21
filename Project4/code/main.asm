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

	; Print "level 1" or "level 2" based on W
    MOVLW   1   ; Load 1 into W for level 1
    CALL    print_level
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

    ; Convert number in W to ASCII and display
    MOVWF   TEMP         ; Store the level number in TEMP
    CLRF    DIGIT1       ; Clear tens digit counter

div_loop
    MOVLW   10
    SUBWF   TEMP, W      ; Subtract 10 from TEMP and store in W
    BTFSS   STATUS, C    ; Skip next instruction if W >= TEMP (no borrow)
    GOTO    done_div     ; If borrow occurred, exit loop
    INCF    DIGIT1, F    ; Increment tens digit counter
    MOVF    W, F         ; Move result back to TEMP
    GOTO    div_loop     ; Repeat subtraction

done_div
    ; TEMP now contains the ones digit, DIGIT1 contains the tens digit

    ; Display tens digit
    MOVF    DIGIT1, W    ; Move tens digit to W
    ADDLW   '0'          ; Convert to ASCII
    CALL    display_char ; Display the tens digit

    ; Display ones digit
    MOVF    TEMP, W      ; Move ones digit to W
    ADDLW   '0'          ; Convert to ASCII
    CALL    display_char ; Display the ones digit

    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                Variables
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TEMP        RES     1    ; Temporary storage for level number
DIGIT1      RES     1    ; Tens digit


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