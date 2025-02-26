# T2ME

This is a workshop for the course "Stochastic Models" at Universidad Nacional de Colombia.

## Description

1. Program designed in C++ programming language for simulating M/M/N where N is the number of servers (we use only one queue). This program can be used to perform various experiments and can be easily extended to other types of models.

2. Variable names were changed from English to Spanish.

3. The input file "mm1.in" was changed to "param.txt" (for clarity and easy modification of input parameters).

    - The first line of the file contains the mean interarrival time (in minutes).
    - The second line of the file contains the mean service time (in minutes).
    - The third line of the file contains the maximum time (in minutes) to run the simulation.
    - The fourth line of the file contains the maximum number of customers to generate.
    - The fifth line of the file contains the number of servers.

4. The output file "mm2.out" was changed to "result.txt" (for easier reading of statistics). 

5. The file "lcgrand.cpp" was taken from http://www.sju.edu/~sforman/courses/2000F_CSC_5835/, which is a simulations website. Look for the Simlib.c link (a simulation library).

## Installation

To execute the simulation, it is necessary to install the C++ programming language and use an IDE for proper execution. Additionally, all files stored in the repository must be downloaded and ensured that they are in the same folder.

## Usage

To execute the simulation, the main.cpp file must be compiled and executed. The result of the simulation will be stored in the "result.txt" file. By default, you can execute the simulation with the following command:

```bash
./ejecutable.exe
```

## License

[MIT](https://choosealicense.com/licenses/mit/)

## Authors

Jhonatan Steven Rodriguez Ibañez \
Edgar Daniel Gonzalez Diaz \
Bryan Smith Colorado Lopez \
Miguel Angel Puentes Cespedes \
Juan Camilo Zambrano Lopez
