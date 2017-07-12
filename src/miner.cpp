#include "header.hpp"

/*Global results queue for all threads
* The queue is thread safe guranteed by socketx.
*/
socketx::squeue<struct packet> res_queue;

/*Compute the hash value for a certain packet*/
void computation(struct packet pat);
/*Send a result from the results queue*/
void send_results(int fd);

int main(int argc, char **argv){

    if(argc!=3){
        std::cout<<"usage: "<<argv[0]<<"<host> "<<"<port> "<<std::endl;
        exit(0);
    }
    std::string host(argv[1]);
    std::string port(argv[2]);

    /*Create a miner socket and initialize it*/
    socketx::client_socket miner;
    int miner_fd = miner.connect_to(host,port);
    miner.communication_init(miner_fd);
    /*Create a thread pool with the default number of threads*/
    socketx::thread_pool pool;
    pool.submit(std::bind(send_results,miner_fd));

    socketx::select select_obj;
    
    while(1){
        /*Put client_fd and stdin_fd into readset*/
        select_obj.FD_set(miner_fd,&select_obj.readset);
        select_obj.select_wrapper();

        /*Handle the messages from the server*/
        if(select_obj.FD_isset(miner_fd,&select_obj.readset)){
            socketx::message msg = miner.recvmsg(miner_fd);
            if(msg.get_size()==0){
                std::cerr<<"Connection interrupted...."<<std::endl;
                break;
            }
            /*Decapsulate the message*/
            struct packet pat = deserialization(msg.get_data(),msg.get_size());
            if(pat.type == "computation"){
                std::cout<<"Received a task from the server. The task's id is: "<<pat.id<<std::endl;
                pool.submit(std::bind(computation,pat));
            }
            else
                std::cout<<"error..."<<std::endl;
        }
    }
    return 0;
}

/*Compute the hash value for a certain packet*/
void computation(struct packet pat){
    std::cout<<"Computation starts: "<<pat.msg<<" "<<pat.number<<std::endl;  
    std::string msg = pat.msg;
    size_t number = pat.number;
    msg += std::to_string(number);

    /*Hash function*/
    std::hash<std::string> str_hash;
    /*Encapsulate the result packet
    * Here, we write the pat.number into pat.message for 
    * recognizing differet jobs.
    * We write the result into pat.number.
    */
    pat.init(pat.id,"result",std::to_string(number),str_hash(msg));

    /*Add it into result queue for sending*/
    res_queue.push(pat);
    std::cout<<"Computation ends: "<<pat.number<<std::endl;  
}

/*Send a result from the results queue*/
void send_results(int fd){
    socketx::communication comm;
    comm.communication_init(fd);
    while(1){
        std::shared_ptr<struct packet> p = res_queue.wait_pop();
        char * data = serialization(*p);
        size_t n = sizeof(size_t) * 4 + p->type_size + 1 + p->msg_size + 1;
        socketx::message msg(data,n);

        /*Sending*/
        std::cout<<"sending results......"<<std::endl;
        comm.sendmsg(fd,msg);
    }
}