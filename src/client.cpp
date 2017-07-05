#include "header.hpp"

int main(int argc, char **argv){

    if(argc!=3){
        std::cout<<"usage: "<<argv[0]<<"<host> "<<"<port> "<<std::endl;
        exit(0);
    }
    std::string host(argv[1]);
    std::string port(argv[2]);

    /*Create a client socket and initialize it*/
    socketx::client_socket client;
    int client_fd = client.connect_to(host,port);
    client.communication_init(client_fd);

    int stdin_fd = fileno(stdin);
    socketx::select select_obj;

    /*input msg and nonce for hashing*/
    struct packet pat("request");
    std::cout<<"input string message and maximum nonce: <msg> <nonce>"<<std::endl;
    
    while(1){
        /*Put client_fd and stdin_fd into readset*/
        select_obj.fd_set(client_fd,select_obj.readset);
        select_obj.fd_set(stdin_fd,select_obj.readset);

        select_obj.select_wrapper();

        /*Handle the messages from the server*/
        if(select_obj.fd_isset(client_fd,select_obj.readset)){
            socketx::message msg = client.recvmsg();

            /*Decapsulate the message*/
            pat = deserialization(msg.get_data(),msg.get_size());
            if(pat.type == "result")
                std::cout<<"hash value: "<<pat.number<<std::endl;
            else
                std::cout<<"error..."<<std::endl;
        }

        /*Handle the data from the stdin*/
        if(select_obj.fd_isset(stdin_fd,select_obj.readset)){
            /*encapsulate the message*/
            std::cin>>pat.msg>>pat.number;
            char * data = serialization(pat);
            socketx::message msg(data,sizeof(data));

            /*Send the message, then wait for the result*/
            client.sendmsg(client_fd,msg);
        }
    }
    
    return 0;
}