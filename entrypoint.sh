#!/bin/bash
# Start the SSH service
service ssh start

# Wait for all SSH services to start
sleep 10

# Allow running mpirun as root
export OMPI_ALLOW_RUN_AS_ROOT=1
export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

# Run the MPI program
mpirun --hostfile /key_db/hostfile -np 6 ./key_db