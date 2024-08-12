#### Distributed Key-Value Storage Service with Load Balancing and Replication

### How to compile and run

#### Requirements

- GCC;
- MPI;

##### Compiling

To compile the program, just run the following command:

```
mpicc -o key_db main.c load_balancer.c replica.c client.c key_value_db.c utils.c
```

##### Running

To run the program, just run the following command:

```
mpirun -np 6 ./key_db
```

6 is the minimum number of processes to run the program. This value can be replaced by any other value greater than it.

#### Extra: running with distributed nodes in a Docker network

In order to run the program on a network of distributed nodes, I tried to create a Docker network with 6 nodes, each with a container running the program. Unfortunately, I haven't had enough time to make it work 100%. It currently seems that not all the containers are communicating correctly because when I check the logs I can see that some replicas are ignoring messages sent by clients, which doesn't happen when running the program on a single node. Despite this, the program still works correctly on the docker network, only some operations are lost.

You can try running the program on a network of distributed nodes by following the steps below.

##### Requirements

To run the program on a network of distributed nodes, you need to have Docker installed on your machine, along with Docker Compose.

##### Docker image build

The `Dockerfile` contains the instructions for creating a Docker image with the program. There it sets up SSH for the machines, copies all the files into the image and compiles the code. To create the image, simply run the following command in the root directory of the project:

```
docker-compose build
```

or

```
docker build .
```

##### Creating networks and nodes

To create the containers and the network there is the file `docker-compose.yml` which contains the individual settings for each container and the network. To create the network and containers, simply run the following command in the root directory of the project (after creating the Docker image):

```
docker-compose up -d
```

##### Adding more containers

If you want to add more containers, you'll have to register them individually in the `docker-compose.yml` file, following the pattern of the other containers. It is important to assign a unique IP to each container and then update the `hostfile` file including the new containers so that MPI can recognize them.

### Technical Details

The aim of this section is to present the general operation of the system's components and clarify the implementation decisions. Details on the implementation of each function and data structure are in the code by means of comments.

#### Distributed System Components

The system consists of 1 process that acts as a load balancer, N processes that are the clients that make the write and read requests and M processes that act as replicas of the database.

The system accepts any number of processes equal to or greater than 6, but the proportion of each component is fixed, with N and M having a 3:2 ratio. So, if a process has 3 clients, it will have 2 replicas, if it has 6 clients, it will have 4 replicas and so on. There will always be only 1 load balancer.

This ratio has been chosen to ensure that the system has a minimum number of replicas and clients and that this is sufficient to test the scalability of the system and its distributed nature.

#### The Key-Value Database

To implement this work, an in-memory database was created which stores key-value pairs. The data structure used was a chained hash table. The hash table is a vector of chained lists with 100 positions, where each position stores a list of key-value pairs that have the same key.

The hash function used is a shift-and-add that uses the ASCII value of each character in the key to calculate the index of the hash table where the key-value pair will be stored.

All replicas start empty and maintain their own hash table and copy of the data.

#### The Load Balancer

A load balancer has been implemented for client requests. Regardless of the number of processes, there is only 1 load balancer which is responsible for distributing client requests among the database replicas.

Before it starts listening to requests, the load balancer calculates the number and ranks of replicas and clients, storing them in vectors that will be used during execution.

The load balancer enters an infinite loop and uses `MPI_Recv` (blocking) to wait for requests from clients. Here it listens for requests from any source and with any tag. When it receives a message, it analyzes the TAG to check whether it is a read or write request.

In the case of a read, it chooses a replica using round-robin and sends the request to it.
In the case of a write, a repeat loop is used to broadcast the request to all replicas.
If the message is neither read nor write, the program checks to see if it is a completion message. If it is, it increments the counter of terminated clients. When the counter of terminated clients equals the number of clients, the load balancer broadcasts a message to all replicas indicating that the program can be terminated.

#### The Replicas

The replicas are responsible for maintaining a copy of the database and for responding to write and read requests received from the load balancer. There are M replicas, where M is an integer greater than or equal to 2 (considering a minimum of 6 processes). The number of replicas is calculated according to a 2:3 ratio in relation to the number of clients.

Each replica initializes its own key-value database and then enters an infinite loop that listens for messages from the load balancer, which can have any TAG. MPI_Recv is blocking, so the replica waits until a message is received. When a message is received by the replica, the program checks the type of the message within the content received and checks whether it is a write or read operation.

In the case of read operations, the system receives a message containing the key to be searched for in the database. The replica tries to find the key in the hash table and the value associated with it. If the value for that key is not found, the replica uses `NULL` as a response. With the value in hand, the replica creates a new message using the `new_reply_message` constructor and sends the reply directly to the client that made the request, whose rank is also contained in the content of the message.

In the case of write operations, all replicas receive the same message containing the key and value to be written. Each replica tries to insert the key-value pair into its hash table. If the key already exists, the value is updated. After inserting or editing, the replica listens again for new messages, without sending a reply to the load balancer or the client.

If the message received is neither read nor write, the replica checks to see if it is a close message. If it is, the replica exits the loop and ends its execution.

#### The Clients

Clients are responsible for sending read and write requests. There are N clients, where N is an integer greater than or equal to 3 (considering a minimum of 6 processes). The number of clients is calculated in a 3:2 ratio to the number of replicas.

##### The Operations Files

Clients can send read or write messages. The operations that the client will perform are determined by a text file that contains the requests line by line. The file must respect the pattern `<OPERATION> <key> <value>`, where `<OPERATION>` is a string that can be `READ` or `WRITE`, `<key>` is a string of maximum length 100 that represents the key to be fetched or written and `<value>` is a string of maximum length 100 that represents the value to be written in the case of `WRITE` type operations.

Inside the `utils.c` file there is a `parse_operations` function that receives the path to the file, reads the operations and stores them in an operations vector. Each operation is represented by an `operation_t` data structure containing the type of operation, the key and the value. There are three pre-created files inside the `client_operations` directory, with 3 different sets of operations that can be used to test the system. In the `main.c` file, when determining the role of the process according to its rank for each client, one of the files is chosen using round-robin, so that each client has a different file but all clients have a file, even if there are more clients than files. Still in `main.c`, the `parse_operations` function is called to read the operations from the chosen file and the client function is called with the vector of operations ready. To add more operation files, simply create a text file with the operations in the specified format and name it `operationsN.txt`, where N is a sequential integer.

##### How it operates

With the vector of operations already in hand, each client counts how many operations are read operations and stores this value in a counter. This is useful for determining whether the client can end its execution. The client has two main functions, sending requests and receiving responses. As these are independent data flows, I decided to use the Fork Join pattern to create two threads, one to send requests and one to receive responses.

###### Sending Requests

In the request-sending thread there is a repeat loop based on the count of operations read from the file. For each operation, the client determines whether it is a write or read message. In the case of read messages, a message is created using the `new_read_message` constructor, passing the client's rank and the key to be read as parameters. The constructor returns a message of type `message_t`, compatible with the custom MPI data type `MPI_DB_MESSAGE_TYPE` that was created at the start of the process. With the message ready, the client sends the message to the load balancer using `MPI_Send` (blocking).

In the case of write messages, the client creates a message using the `new_write_message` constructor, passing the client's rank, the key and the value to be written as parameters. The constructor returns a message of type `message_t` which is sent to the load balancer using `MPI_Send` (blocking).

After sending the message the client waits between 1 and 2 seconds before continuing with the repeat loop.

###### Receiving Responses

In the thread receiving responses, the client initializes a counter of responses received and enters a `while` loop which tests whether the counter of responses received is less than the number of read operations initially calculated. As long as there are still unanswered read requests, the loop continues.

Inside the loop there is an `MPI_Recv` (blocker) which waits for a `REPLY_MESSAGE_TAG` TAG message containing the response to the read request. Any other possible message sent to the client with tags other than the response tag will be ignored. The client receives the message, prints the response and increments the counter of responses received.

###### Ending Execution

When all the requests in the vector of operations have been sent, the request-sending thread terminates and remains in the `pthread_join` waiting for the response-receiving thread to finish. When the counter of responses received equals the number of read operations, the receiving thread terminates and also goes to the `pthread_join`.

With both threads terminated, the client creates one last message using the `new_terminate_message` constructor and sends the message to the load balancer using `MPI_Send`. The client then terminates its execution. When the load balancer receives the termination message from all clients, it broadcasts a message to all replicas indicating that the program can be terminated and also terminates its execution.

#### Conclusão

O trabalho demonstra uma implementação simples para um sistema distribuído que oferece escalabilidade horizontal e alta disponibilidade através da replicação de dados e do balanceamento de carga. O programa funciona com qualquer número de processos sem precisar de nenhuma alteração, e é simples alterar ou adicionar novas operações que serão feitas pelos clientes. O sistema também é capaz de encerrar sua execução de maneira segura e ordenada, garantindo que todos os processos terminem corretamente.

A principal falha do sistema é que todo seu funcionamento depende de um único processo, o balanceador de carga, que se falhar pode comprometer todo o sistema. Uma melhoria possível seria aumentar o número de load balancers ou implementar um sistema de eleição para escolher um novo balanceador de carga caso o atual falhe. Outra melhoria importante seria adicionar um mecanismo de confirmação de escrita e consistência eventual para garantir que todas as réplicas tenham coerência de dados e que operações que falharam possam ser recuperadas.

Durante o desenvolvimento do trabalho foi possível aprender mais sobre programação paralela e distribuída, MPI, implementação de sistemas distribuídos e padrões de projeto para esse tipo de sistema. Por isso concluo que o trabalho foi uma oportunidade para entender todos estes conceitos de maneira prática e por isso o objetivo do trabalho foi alcançado.
