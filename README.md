# Self Organizing Map for iris_data

This project was coded from scratch using only standard C libraries (and Unix C for minimalist interface)
It is a self organising map for iris data classification.

To use this implementation of SOM, start by cloning this repository and enter the created folder:

```shell script
git clone https://github.com/sachahu1/Self_Organizing_Map_C-.git
```

## How to run the script ?

Running the algorithm is very simple and self-explanatory:
```shell script
make # Compiles the project
./Iris # Launches the script
```
Then, simply use the arrow keys and the enter key to select among the available options.

## What is in this project ?
- The Iris dataset : https://archive.ics.uci.edu/ml/datasets/iris (iris_data.txt)
- A configuration file which allows for easy transfer of this algorithm onto another dataset (config.txt)
- Makefile
- Iris.c (standard C + Unix C libraries)
