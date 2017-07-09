#include "header.hpp"

std::unordered_map<int, struct miner_info> miner_map;
std::mutex mut;

/*Handle tasks from a connection*/
void handle_task(int connfd);

/*Schedule the task distribution among miners*/
vector<struct job_info> scheduler(struct packet pat);

int main(int argc,char** argv){

    if(argc!=3){
        fprintf(stderr,"usage: %s <client port> <miner port>\n", argv[0]);
        exit(0);
    }
    
    std::string client_port = std::string(argv[1]);
    std::string miner_port = std::string(argv[2]);

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
        select_obj.FD_set(listen_client,&select_obj.readset);
        select_obj.FD_set(listen_miner,&select_obj.readset);

        select_obj.select_wrapper();

        /*Handle clients connections*/
        if(select_obj.FD_isset(listen_client,&select_obj.readset)){
            int connfd = conn_client.accept_from();
            std::string hostname = conn_client.get_peername(connfd);
            size_t hostport = conn_client.get_port();
            std::cout<<"recieve a computation request from ("<<hostname<<", "<<hostport<<")"<<std::endl;
            pool.submit(std::bind(handle_task,connfd));
        }

        /*Handle miners connections*/
        if(select_obj.FD_isset(listen_miner,&select_obj.readset)){
            int connfd = conn_miner.accept_from();
            std::string hostname = conn_miner.get_peername(connfd);
            size_t hostport = conn_miner.get_port();
            std::cout<<"recieve a join request from ("<<hostname<<", "<<hostport<<")"<<std::endl;

            /*Add a miner into miner_map*/
            mut.lock();
            if(!miner_map.count(connfd)){
                struct miner_info info = miner_info(connfd);
                miner_map[connfd] = info;
            }else{
                std::cerr<<"Miner connection error......"<<std::endl;
            }
            mut.unlock();
        }
    }
    exit(0);
}


/*Handle tasks from a connection*/
void handle_task(int connfd){
    /*A list for maintaining job information*/
    std::vector<struct job_info> job_vec;
    std::map<int, socketx::communication> comm_map;
    
    socketx::communication comm_client;
    comm_client.communication_init(connfd);
    while(1){
        socketx::message msg = comm.recvmsg();
        if(msg.get_size()<=0) break;
        struct packet pat = deserialization(msg.get_data(),msg.get_size());
        if(pat.type == "request"){
            job_vec = scheduler(pat);

            /*Initialize communication and send data*/
            for(auto it=job_vec.begin();it!=<job_vec.end();++it){
                if(!comm_map.count(job_vec[i].fd)){
                    comm_map[it->fd] = socketx::communication();
                    comm_map[it->fd].communication_init(it->fd);
                }
                char * data = serialization(it->pat);
                size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
                comm_map[it->fd].send(it->fd,socketx::message(data,n));
            }

            /*Wair for results*/



        }else{
            std::cerr<<"Wrong type: "pat.type<<std::endl; 
        }
        
        
   
    }   
}

/*Schedule the task distribution among miners*/
std::vector<struct job_info> scheduler(struct packet pat){
    /*Nonce of the request from the client*/
    int n = pat.number;
    std::vector<struct job_info> job_vec;

    /*Distribute jobs*/
    for(int i=0;i<n;++i){
        auto p = std::min_element(miner_map.begin(),miner_map.end(),less_value);
        /*Increase load of miner fd*/
        p->first += 1;
        /*Construct job_info*/
        struct job_info job;
        job.fd = p->second;
        job.pat = pat;
        job.pat.number = i;
        job_vec.push_back(job);
    }
    return job_vec;
}