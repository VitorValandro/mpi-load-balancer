# Use an official Open MPI image as a base
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    openssh-server \
    vim

# Set up SSH
RUN mkdir -p /var/run/sshd && \
    mkdir -p /root/.ssh && \
    ssh-keygen -t rsa -f /root/.ssh/id_rsa -N '' && \
    cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys && \
    chmod 600 /root/.ssh/authorized_keys && \
    echo "Host *\n\tStrictHostKeyChecking no\n" >> /root/.ssh/config

# Copy source code to the container
COPY . /key_db
WORKDIR /key_db

# Copy entrypoint script
COPY entrypoint.sh /key_db/entrypoint.sh
RUN chmod +x /key_db/entrypoint.sh

# Compile the MPI program
RUN mpicc -o key_db main.c load_balancer.c replica.c client.c
