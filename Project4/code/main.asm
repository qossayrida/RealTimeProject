;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   DISPLAY_HELLO.ASM    MPB     Ver 1.0     21-01-2025
;
;   Display "HELLO" on 16x2 LCD
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    PROCESSOR 16F877
;   Clock = XT 4MHz, standard fuse settings
    __CONFIG 0x3731 ; 00011011100110001

; LABEL EQUATES ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    INCLUDE "P16F877A.INC"    

; Program begins ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ORG 0       ; Default start address 
    NOP         ; Required for ICD mode

    BANKSEL TRISC       ; Select bank 1
    MOVLW   B'11110000' ; Keypad direction code
    MOVWF   TRISC       ; Set PORTC directions
    CLRF    TRISD       ; Display port is output

    BANKSEL PORTC       ; Select bank 0
    MOVLW   0FF         ; Set keypad outputs high
    MOVWF   PORTC       ; 
    CLRF    PORTD       ; Clear display outputs
    GOTO    start       ; Jump to main program

; MAIN LOOP ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

start   CALL    inid    ; Initialize the display

; Display "HELLO" on the LCD

    MOVLW   'H'         ; Load ASCII code for 'H'
    CALL    display_char
    MOVLW   'E'         ; Load ASCII code for 'E'
    CALL    display_char
    MOVLW   'L'         ; Load ASCII code for 'L'
    CALL    display_char
    MOVLW   'L'         ; Load ASCII code for 'L'
    CALL    display_char
    MOVLW   'O'         ; Load ASCII code for 'O'
    CALL    display_char

loop    GOTO    loop    ; Stay in infinite loop

; Subroutine to display a character on LCD
;--------------------------------------------

; Receives character in W and uses the LCD driver routines

display_char    BSF  Select,  RS       ; Select data mode
    CALL    send     ; Send the character to LCD
    RETURN

; Include LCD driver routine

    INCLUDE "LCDIS.INC"

    END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
