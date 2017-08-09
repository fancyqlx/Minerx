## Minerx

**TODO: Update this project by new socketx......**

Minerx is a simple project for implementing a distributed bitcoin miner. The project is writed by C/C++, and it heavily utilized library [socketx](https://github.com/fancyqlx/socketx) which is an underlying library for network programming. In addition, this project is originated from a lesson of CMU, you can find more details from [cmu440/p1](https://github.com/cmu440/p1/blob/master/p1.pdf).

### Client
The client sends a user-specified request to the server, receives and prints the results.

### Miner
The miner is a computation entity. It received a part of jobs from the server, then performs computation.

### Server
The server is the core of this system. It receives requests from clients, process the requests and splits the job into multiple small jobs for miners. The server needs to manage miners, detect and handle failures during the process of tasks. Finally, it should collect results among all miners and send the results to the corresponding client.