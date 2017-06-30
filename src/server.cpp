#include "socket.hpp"
#include "threadx.hpp"
#include <iostream>
#include <string>


/*Handle tasks from a connection*/
void handle_task(int connfd);

/*Schedule the task distribution among miners*/
void scheduler();



int main(int argc,char** argv){

    if(argc!=3){
        fprintf(stderr,"usage: %s <client port> <miner port>\n", argv[0]);
        exit(0);
    }
    
    std::string client_port = argv[1];
    std::string miner_port = argv[2];

    /*Create a thread pool for handling clients requests*/
    socketx::thread_pool pool(5);


    /*Select a socket*/


    /*Handle clients connections*/

    /*Handle miners connections*/

    /*Manage miners*/


    pool.submit(echo_send);

    while(1){
        int connfd = server.accept_from();
        if(connfd>=0){
            std::string hostname = server.get_peername(connfd);
            size_t hostport = server.get_port();
            std::cout<<"connected to ("<<hostname<<", "<<hostport<<")"<<std::endl;
            pool.submit(std::bind(echo_receive,connfd));
            mut.lock();
            fdlist.push_back(connfd);
            mut.unlock();
        }
    }
    exit(0);
}