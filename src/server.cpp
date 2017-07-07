#include "header.hpp"

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

    /*Create select object*/
    socketx::select select_obj;
    /*Create two server socket, one for listening clients and 
    * another for listening miners.
    */
    socketx::server_socket conn_client;
    socketx::server_socket conn_miner;
    int listen_client = conn_client.listen_to(client_port);
    int listen_miner = conn_miner.listen_to(miner_port);

    /*Select a socket*/
    while(1){
        /*Put the listening fd into readset of select obj*/
        select_obj.FD_set(listen_client,select_obj.readset);
        select_obj.FD_set(listen_miner,select_obj.readset);

        select_obj.select_wrapper();

        /*Handle clients connections*/
        if(select_obj.FD_isset(listen_client,select_obj.readset)){
            int connfd = conn_client.accept_from();
            std::string hostname = conn_client.get_peername(connfd);
            size_t hostport = conn_client.get_port();
            std::cout<<"recieve a request from ("<<hostname<<", "<<hostport<<")"<<std::endl;
            pool.submit(std::bind(handle_task,connfd));
        }

        /*Handle miners connections*/
        if(select_obj.FD_isset(listen_miner,select_obj.readset)){
            int connfd = conn_miner.accept_from();
            std::string hostname = conn_miner.get_peername(connfd);
            size_t hostport = conn_miner.get_port();
            std::cout<<"recieve a request from ("<<hostname<<", "<<hostport<<")"<<std::endl;
        }
    }
    exit(0);
}