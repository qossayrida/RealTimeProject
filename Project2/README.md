# File Management Simulation

This project is a multi-processing application that simulates the creation, handling, and management of CSV files using inter-process communication (IPC) techniques under Linux. The simulation involves file generators, calculators, movers, and inspectors, each performing specific tasks to manage CSV files. The program uses IPC mechanisms and OpenGL for visualization.

## Project Structure

The project has the following structure:

```plaintext
file_management_simulation/
├── bin/                        # Contains the compiled executable
├── config.txt                  # Configuration file with simulation settings
├── include/                    # Header files for each module
│   ├── config.h                # Config module header for loading settings
│   ├── gui.h                   # Graphical User Interface (GUI) module header
│   ├── sem.h                   # Semaphore utility for synchronization
├── src/                        # Source code files
│   ├── main.c                  # Main file that starts the simulation
│   ├── config.c                # Config module implementation to load settings
│   ├── file_calculator.c       # CSV file calculator module implementation
│   ├── file_generator.c        # CSV file generator module implementation
│   ├── file_mover.c            # CSV file mover module implementation
│   ├── file_type1_inspector.c  # Type 1 inspector module implementation
│   ├── file_type2_inspector.c  # Type 2 inspector module implementation
│   ├── file_type3_inspector.c  # Type 3 inspector module implementation
│   ├── gui.c                   # Graphical User Interface (GUI) implementation using OpenGL
├── Makefile                    # Makefile for building the project
└── README.md                   # This readme file
```

## Files Description

- **`config.txt`**: A configuration file containing simulation settings such as the number of file generators, calculators, movers, inspectors, and thresholds for file processing, backup, and deletion.

- **`include/`**: Contains header files for each module.
  - `config.h`: Defines functions and data structures for loading simulation settings from `config.txt`.
  - `gui.h`: Contains definitions and functions related to the graphical user interface using OpenGL.
  - `sem.h`: Provides utility functions for semaphore-based synchronization.

- **`src/`**: Contains the main code for the simulation.
  - `main.c`: Initializes the simulation, loads settings, and starts the main process.
  - `config.c`: Implements functions to load settings from `config.txt`.
  - `file_calculator.c`: Handles the calculation of column averages for CSV files.
  - `file_generator.c`: Generates CSV files with random data and stores them in the home directory.
  - `file_mover.c`: Moves processed CSV files to the `Processed` directory.
  - `file_type1_inspector.c`: Inspects the home directory and moves unprocessed files to the `UnProcessed` directory.
  - `file_type2_inspector.c`: Inspects the `Processed` directory and moves old files to the `Backup` directory.
  - `file_type3_inspector.c`: Inspects the `Backup` directory and deletes old files.
  - `gui.c`: Implements the graphical user interface using OpenGL for visualizing the simulation.

## Processes and IPC

- **Processes**:
  - **File Generators**: Generate CSV files with random data and store them in the home directory.
  - **File Calculators**: Calculate the average per column for CSV files and store the results in a shared location.
  - **File Movers**: Move processed CSV files to the `Processed` directory.
  - **Type 1 Inspectors**: Move unprocessed CSV files to the `UnProcessed` directory.
  - **Type 2 Inspectors**: Move old processed CSV files to the `Backup` directory.
  - **Type 3 Inspectors**: Delete old CSV files from the `Backup` directory.

- **Inter-Process Communication (IPC)**:
  - Use **shared memory** to store and share data such as file counts, averages, and file statuses.
  - Use **semaphores** for synchronization between processes to avoid race conditions.

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

### Simulation Workflow

1. **File Generation**:
   - File generators create CSV files with random data and store them in the home directory.
   - The total number of generated files is reported in a shared location.

2. **File Calculation**:
   - File calculators compute the average per column for each CSV file.
   - The results are stored in a shared location, and the total number of processed files is reported.

3. **File Moving**:
   - File movers move processed files to the `Processed` directory.

4. **File Inspection**:
   - **Type 1 Inspectors**: Move unprocessed files to the `UnProcessed` directory.
   - **Type 2 Inspectors**: Move old processed files to the `Backup` directory.
   - **Type 3 Inspectors**: Delete old files from the `Backup` directory.

5. **Simulation End Conditions**:
   - The simulation ends if:
     - The number of processed files exceeds a user-defined threshold.
     - The number of unprocessed files exceeds a user-defined threshold.
     - The number of files moved to the `Backup` directory exceeds a user-defined threshold.
     - The number of deleted files exceeds a user-defined threshold.
     - The simulation runs for more than a user-defined amount of time.
