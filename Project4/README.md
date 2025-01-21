# PIC 16F877A PIC Microcontroller-based Game

This project is an interactive memory-based game implemented using a 16F877A PIC microcontroller. The game involves players replicating LED light sequences using push buttons, with instructions displayed on a 16x2 character LCD.

## Project Overview

- **Microcontroller**: 16F877A PIC
- **Development Environment**: MPLAB IDE
- **Hardware Components**:
  - 4 colored LEDs
  - 4 push buttons
  - 16x2 character LCD

## Game Description

1. **Startup**:
   - The LCD displays "Press any button to start" with a blinking effect before transitioning to the game.

2. **Gameplay**:
   - **Level 1**: Displays a random 3-LED sequence with each LED staying ON for 1 second.
   - **Subsequent Levels**: Increase the sequence length by one LED each level, up to a maximum of 22 LEDs at Level 20.
   - Players must replicate the LED sequence using push buttons. The system checks for a match and either congratulates the player or prompts to try again. Players have up to 3 attempts per level.

3. **End of Game**:
   - The game resets to Level 1 if no buttons are pressed for over a minute.

## Hardware Setup

- **LEDs** and **push buttons** connected to 16F877A pins.
- LCD operates in 4-bit mode. A 4.7KΩ resistor pulls up the RS pin.
- Push buttons use 10KΩ pull-up resistors or the internal weak pull-up resistors of PortB.
- A 4MHz oscillator with two 15pF capacitors is used along with a 10KΩ pull-up resistor for the MCLR pin.
- Optional: Enable interrupts on the push buttons for enhanced performance.

## Software Development

- **Programming Language**: Assembly
- **IDE**: MPLAB IDE
- **Simulation**: Proteus