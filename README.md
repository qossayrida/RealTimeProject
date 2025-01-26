# Real-Time Applications & Embedded Systems Projects

This repository contains projects for the **Real-Time Applications & Embedded Systems** course (ENCS4330) at Birzeit University. Each project focuses on different aspects of real-time systems, including multi-processing, multi-threading, inter-process communication (IPC), and synchronization techniques under Linux.


## Projects Overview

### 1. **Project 1: [Project Name]**
   - **Description**: [Brief description of Project 1]
   - **Technologies**: [Technologies used, e.g., POSIX threads, OpenGL]
   - **Directory**: [Link to Project 1 directory]
   - **README**: [Link to Project 1 README]

### 2. **Project 2: File Management Simulation**
   - **Description**: A multi-processing application that simulates the creation, handling, and management of CSV files using IPC techniques. The simulation involves file generators, calculators, movers, and inspectors.
   - **Technologies**: Multi-processing, shared memory, semaphores, OpenGL.
   - **Directory**: [Project 2 Directory](https://github.com/qossayrida/RealTimeProject/tree/master/Project2)
   - **README**: [Project 2 README](https://github.com/qossayrida/RealTimeProject/tree/master/Project2/README.md)

### 3. **Project 3: Counter Espionage Agency Simulation**
   - **Description**: A multi-processing and multi-threading simulation of a counter espionage agency. The simulation involves resistance groups, civilians, and counter espionage agency members, with the goal of protecting resistance members from enemy spies.
   - **Technologies**: POSIX threads, multi-processing, IPC, OpenGL.
   - **Directory**: [Project 3 Directory](https://github.com/qossayrida/RealTimeProject/tree/master/Project3)
   - **README**: [Project 3 README](https://github.com/qossayrida/RealTimeProject/tree/master/Project3/README.md)



## How to Use

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/qossayrida/RealTimeProject.git
   ```

2. **Navigate to a Project**:
   - For example, to access Project 2:
     ```bash
     cd RealTimeProject/Project2
     ```

3. **Build and Run**:
   - Use the `Makefile` in each project directory to build and run the simulation.
     ```bash
     make
     make run
     ```

4. **Clean Up**:
   - To remove object files and the executable:
     ```bash
     make clean
     ```


## Technologies Used

- **C Programming**: Core implementation language for all projects.
- **POSIX Threads (Pthreads)**: Used for multi-threading in Project 3.
- **Inter-Process Communication (IPC)**: Shared memory, pipes, and semaphores are used for communication and synchronization.
- **OpenGL**: Used for graphical visualization in Projects 2 and 3.
- **Makefile**: For building and managing the projects.



## Notes

- **Configuration Files**: Each project uses a `config.txt` file to load user-defined settings, avoiding hard-coded values.
- **Debugging**: Use `gdb` for debugging. Compile with the `-g` flag to include debugging symbols.
- **Realism**: The simulations are designed to be realistic, with user-defined thresholds and random events.


## Contributors

- **Qossay Rida**
- **Mohammed Fareed**
- **Ahmad Hamdan**


## 🔗 Links

[![facebook](https://img.shields.io/badge/facebook-0077B5?style=for-the-badge&logo=facebook&logoColor=white)](https://www.facebook.com/qossay.rida?mibextid=2JQ9oc)

[![Whatsapp](https://img.shields.io/badge/Whatsapp-25D366?style=for-the-badge&logo=Whatsapp&logoColor=white)](https://wa.me/+972598592423)

[![linkedin](https://img.shields.io/badge/linkedin-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/qossay-rida-3aa3b81a1?utm_source=share&utm_campaign=share_via&utm_content=profile&utm_medium=android_app )

[![twitter](https://img.shields.io/badge/twitter-1DA1F2?style=for-the-badge&logo=twitter&logoColor=white)](https://twitter.com/qossayrida)
