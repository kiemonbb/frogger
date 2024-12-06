## Table of contents
* [General Info](#general-info)
* [Setup](#setup)

## General Info
This Project is a simple terminal game, based on the arcade game "Frogger". It was written in C using ncurses library.

## Setup
To run this game you'll need ncurses library installed on your system.
* Ubuntu
```
$ sudo apt install libncurses5-dev libncursesw5-dev
```
* Fedora
```
$ sudo dnf ncurses-devel
```
* Arch
```
$ pacman -S ncurses
```

## Running the game
To play the game you have to compile the project by running the following command:
```
$ make
```
Then to run the program, do:
```
$ ./frogger
```

## Controls

### In the main menu
* Use <kbd>n</kbd> to play the game while recording it and then press <kbd>m</kbd> to replay it
* Use <kbd>1</kbd>/<kbd>2</kbd>/<kbd>3</kbd> to play one of three maze-like levels
* Use <kbd>4</kbd>/<kbd>5</kbd>/<kbd>6</kbd> to play one of three progressively harder levels
* Use any other key to start  the game in normal mode
  
### During the game
* Use <kbd>w</kbd>/<kbd>s</kbd>/<kbd>a</kbd>/<kbd>d</kbd> to go **up**/**down**/**left**/**right** respectively
* Use <kbd>r</kbd> to **mount**/**unmount** friendly (green) cars
* Use <kbd>x</kbd> to exit the game anytime

