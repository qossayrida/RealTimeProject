# Counter Espionage Agency Simulation

This project is a multi-processing and multi-threading simulation of a counter espionage agency using POSIX threads under Unix/Linux. The simulation involves resistance groups, civilians, and counter espionage agency members, with the goal of protecting resistance members from enemy spies. The program uses inter-process communication and OpenGL for visualization.

## Project Structure

The project has the following structure:

```plaintext
counter_espionage_simulation/
├── bin/                        # Contains the compiled executable
├── config.txt                  # Configuration file with simulation settings
├── include/                    # Header files for each module
│   ├── config.h                # Config module header for loading settings
│   ├── linked_list.h           # Linked list utility for data structures
│   ├── sem.h                   # Header file for enemy-related logic
├── src/                        # Source code files
│   ├── main.c                  # Main file that starts the simulation
│   ├── citizen.c               # Civilian module implementation
│   ├── config.c                # Config module implementation to load settings
│   ├── counter_espionage_agency.c  # Counter Espionage Agency module implementation
│   ├── enemy.c                 # Enemy module implementation
│   ├── gui.c                   # Graphical User Interface (GUI) implementation using OpenGL
│   ├── resistance_group.c      # Resistance Group module implementation
├── Makefile                    # Makefile for building the project
└── README.md                   # This readme file
```

## Files Description

- **`config.txt`**: A configuration file containing simulation settings such as the number of resistance groups, civilians, agency members, and thresholds for suspicion, injury, and death.

- **`include/`**: Contains header files for each module.
  - `config.h`: Defines functions and data structures for loading simulation settings from `config.txt`.
  - `linked_list.h`: Provides utility functions for managing linked lists used in the simulation.
  - `sem.h`: Contains definitions and functions related to enemy activities and interactions.

- **`src/`**: Contains the main code for the simulation.
  - `main.c`: Initializes the simulation, loads settings, and starts the main process.
  - `citizen.c`: Implements civilian behavior and interactions with resistance groups.
  - `config.c`: Implements functions to load settings from `config.txt`.
  - `counter_espionage_agency.c`: Handles counter-espionage agency member activities, including data analysis and spy detection.
  - `enemy.c`: Manages enemy activities, including targeting resistance groups and agency members.
  - `gui.c`: Implements the graphical user interface using OpenGL for visualizing the simulation.
  - `resistance_group.c`: Contains logic for resistance group activities, including member management and enemy targeting.

## Processes and Threads

- **Processes**:
  - **Resistance Groups**: Each resistance group is a process that manages its members and activities.
  - **Civilians**: Civilians are processes that interact with resistance groups and may spy on them.
  - **Counter Espionage Agency**: The agency is a process that manages its members and analyzes data from resistance groups.
  - **Enemy**: The enemy is a process that targets resistance groups and agency members.

- **Threads**:
  - **Resistance Group Members**: Each member of a resistance group is a thread within the resistance group process.
  - **Counter Espionage Agency Members**: Each agency member is a thread within the agency process.

## Inter-Process Communication (IPC)

- Use **pipes** and **shared memory** to communicate data between resistance groups, civilians, the counter espionage agency, and the enemy.
- Use **signals** to synchronize actions such as enemy targeting, spy detection, and member replacement.

## Build and Run

1. **Build the Project**: Run the following command to compile the project and generate the executable:

    ```bash
    make
    ```

2. **Run the Simulation**: Use the following command to run the simulation:

    ```bash
    make run
    ```

3. **Clean Up**: Use the `make clean` command to remove object files and the executable:

    ```bash
    make clean
    ```

## Notes

### Reasons for Multi-Processing and Multi-Threading
- **Multi-Processing**: Each resistance group, civilian, counter espionage agency, and enemy operates independently, making processes ideal for managing their activities.
- **Multi-Threading**: Members within resistance groups and the agency need to perform concurrent tasks, making threads suitable for managing individual members.

### Counter Espionage Agency Tasks:
- Analyze data from resistance groups to detect potential spies.
- Arrest and question potential spies based on suspicion thresholds.
- Replace killed or caught agency members with new members.

### Resistance Group Tasks:
- Manage group members and their activities.
- Share information with the counter espionage agency.
- Handle enemy targeting, including member injuries, deaths, and replacements.

### Civilian Tasks:
- Interact with resistance groups and potentially spy on them.
- Share information with the counter espionage agency if suspicious activities are detected.

### Enemy Tasks:
- Target resistance groups and counter espionage agency members.
- Infiltrate resistance groups with spies.
