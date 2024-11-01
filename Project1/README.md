# Bungee Jumping Simulation

This project is a multi-processing simulation of a bungee jumping game using C. The simulation involves three teams of players performing bungee jumps, coordinated by a referee process. The program uses signals and pipes for inter-process communication and visualizes the game using OpenGL.

## Project Structure

The project has the following structure:

```plaintext
bungee_simulation/
├── bin/                   # Contains the compiled executable
├── config.txt             # Configuration file with game settings
├── include/               # Header files for each module
│   ├── referee.h          # Referee module header
│   ├── team.h             # Team module header
│   ├── player.h           # Player module header
│   └── config.h           # Config module header for loading settings
├── obj/                   # Compiled object files for each source file
├── src/                   # Source code files
│   ├── main.c             # Main file that starts the game
│   ├── referee.c          # Referee module implementation
│   ├── team.c             # Team module implementation
│   ├── player.c           # Player module implementation
│   └── config.c           # Config module implementation to load settings
├── Makefile               # Makefile for building the project
└── README.md              # This readme file
```

## Files Description

- **`config.txt`**: A configuration file containing game settings such as initial energy levels, target score, and maximum game time.
  
- **`include/`**: Contains header files for each module, including `config.h` which defines functions for loading game settings.
  - `referee.h`: Manages game control, scorekeeping, and inter-process communication.
  - `team.h`: Defines the team structure and handles team-based actions like jump coordination.
  - `player.h`: Manages player energy and actions like jumping and pulling.
  - `config.h`: Defines the functions and data structures needed to load game settings from `config.txt`.
  
- **`src/`**: Contains the main code for the simulation, including the new `config.c` file.
  - `main.c`: Initializes the game, loads settings, and starts the main simulation.
  - `referee.c`: Contains logic for the referee's role, managing signals, and game control.
  - `team.c`: Manages team actions, coordinating between players during jumps and pulls.
  - `player.c`: Handles individual player actions, including energy management.
  - `config.c`: Implements functions to load settings from `config.txt`.

## Processes and Inter-Process Communication (IPC)

   - Use **pipes** to communicate scores and status between the referee and players, ensuring data is passed along without interfering with process timing.
   - Use **signals** to initiate and synchronize actions between the referee and team players (e.g., start jump, stabilize, and pull up actions).

## Build and Run

1. **Build the Project**: Run the following command to compile the project and generate the executable:

    ```bash
    make
    ```

2. **Run the Simulation**: Use the following command to run the simulation:

    ```bash
    ./bin/bungee_simulation config.txt
    ```

3. **Clean Up**: Use the `make clean` command to remove object files and the executable:

    ```bash
    make clean
    ```

