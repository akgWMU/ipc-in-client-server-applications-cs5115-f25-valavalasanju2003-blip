# MULTI-CLIENT IPC USING UNIX DOMAIN SOCKETS

## Overview
This C project implements a multi-client server using
UNIX domain sockets (AF_UNIX). Multiple clients can run
in separate terminals and communicate with one server.

## Features
- Supports add, sub, mul, div
- Fork-based concurrency for multiple clients
- Error handling for invalid ops & divide-by-zero
- Fully commented readable code

## How to Compile
Run:

    make

## How to Run
Open 1st terminal:

    ./server

Open another terminal (or several):

    ./client

## Exit
Type 'exit' in client to close it.

## Cleanup
Run:

    make clean
