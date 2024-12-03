## Table of contents
* [General Info](#general-info)
* [Setup](#setup)

## General Info
This Project is a simple terminal game, based on the arcade game "Frogger". It was written in C using ncurses library.

## Setup
To run this game you'll need ncurses library installed on your system.
* Ubuntu
$ sudo apt install libncurses5-dev libncursesw5-dev
* Fedora
$ sudo dnf ncurses-devel
* Arch
$ pacman -S ncurses

## Running the game
To play the game you have to compile the project by running the following command:
$ make
Then to run the program, do:
$ ./frogger
