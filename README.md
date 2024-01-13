# Gitlet - Git-like Version Control System

Gitlet is a simplified version control system inspired by Git, implemented in C++ and using the Boost library for serialization and testing. This project aims to provide a basic understanding of how version control systems like Git work under the hood.

## Features

- Basic version control functionalities: `init`, `add`, `commit`, `log`, etc.
- Simplified handling of branches and merges.
- Serialization of objects using Boost for persistent storage.
- Boost-powered unit tests to ensure reliability.

## Dependencies

- C++ compiler supporting C++17
- [Boost library](https://www.boost.org/) (at least version 1.65)
- CMake (version 3.10 or higher)

## Installation 
1. Clone the repo into your local machine
`git clone https://github.com/fr0m-scratch/gitlet.git`

2. Build the excutable Gitlet
`mkdir build && cd build && cmake .. && make `

3. Put the excutable Gitlet into your desired directory and run `./gitlet [COMMAND]` or
add it to your system PATH.
