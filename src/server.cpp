#include "header.hpp"


/*Data struct for maintaining information of miners*/
std::unordered_map<int, struct miner_info> miner_map;

/*Data struct for storing results of each client*/
std::unordered_map<int, socketx::squeue<struct result_info>> result_map;

/*Mutex for visiting miner_map*/
std::mutex miner_mut;


/*Handle tasks from a connection*/
void handle_task(int connfd);

/*Schedule the task distribution among miners*/
std::vector<struct job_info> scheduler(struct packet pat);

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

    select_obj.FD_set(listen_client,&select_obj.readset);
    select_obj.FD_set(listen_miner,&select_obj.readset);
    fd_set backup_set = select_obj.readset;

    /*A map for maintaining communication channels of each miner*/
    std::map<int, socketx::communication> comm_map;
    /*Select a socket*/
    while(1){
        /*Put the listening fd into readset of select obj*/
        
        select_obj.readset = backup_set;
        select_obj.select_wrapper();

        /*Handle clients connections*/
        if(select_obj.FD_isset(listen_client,&select_obj.readset)){
            int connfd = conn_client.accept_from();
            std::string hostname = conn_client.get_peername(connfd);
            size_t hostport = conn_client.get_port();
            std::cout<<"A client connection from ("<<hostname<<", "<<hostport<<")"<<std::endl;
            pool.submit(std::bind(handle_task,connfd));
        }

        /*Handle miners connections*/
        if(select_obj.FD_isset(listen_miner,&select_obj.readset)){
            int connfd = conn_miner.accept_from();
            std::string hostname = conn_miner.get_peername(connfd);
            size_t hostport = conn_miner.get_port();
            std::cout<<"A miner connection from ("<<hostname<<", "<<hostport<<")"<<std::endl;
            select_obj.FD_set(connfd,&backup_set);
            comm_map[connfd] = socketx::communication();
            comm_map[connfd].communication_init(connfd);

            /*Add a miner into miner_map*/
            miner_mut.lock();
            if(!miner_map.count(connfd)){
                struct miner_info info(connfd);
                miner_map[connfd] = info;
            }else{
                std::cerr<<"Miner connection error......"<<std::endl;
            }
            miner_mut.unlock();
        }

        /*Handle results from miners*/
        for(auto it=miner_map.begin();it!=miner_map.end();++it){
            if(select_obj.FD_isset(it->first,&select_obj.readset)){
                socketx::message msg = comm_map[it->first].recvmsg(it->first);
                if(msg.get_size()<=0){
                    /*Need for further checking for failure*/
                    std::cerr<<"Error..."<<std::endl;
                    select_obj.FD_clr(it->first,&backup_set);
                    continue;
                }
                struct packet pat = deserialization(msg.get_data(),msg.get_size());
                if(pat.type == "result"){
                    /*Add a result of client pat.id into result_map*/
                    struct result_info res = result_info(it->first);
                    res.job_number = std::stoi(pat.msg);
                    res.result = pat.number;
                    result_map[pat.id].push(res);
                    std::cout<<"Received the result (result, job_number): "<<res.result<<", "<<res.job_number<<" of fd "<<pat.id<<std::endl;

                    /*The miner decrease its load by 1*/
                    miner_mut.lock();
                    it->second.load -= 1;
                    miner_mut.unlock();
                }else{
                    std::cerr<<"Wrong type: "<<pat.type<<std::endl; 
                }
            }
        }
    }
    exit(0);
}


/*Handle tasks from a connection*/
void handle_task(int connfd){
    /*A list for maintaining job information*/
    std::vector<struct job_info> job_vec;
    size_t hash_result=0;
    
    socketx::communication comm_client;
    comm_client.communication_init(connfd);
    /*Main loop for handling one client*/
    while(1){
        socketx::message msg = comm_client.recvmsg(connfd);
        if(msg.get_size()<=0) break;
        struct packet pat = deserialization(msg.get_data(),msg.get_size());
        if(pat.type == "request"){
            pat.init(connfd,"computation",pat.msg,pat.number);
            job_vec = scheduler(pat);

            /*Send data*/
            for(auto it=job_vec.begin();it!=job_vec.end();++it){
                char * data = serialization(it->pat);
                size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
                comm_client.sendmsg(it->fd,socketx::message(data,n));
                std::cout<<"Send a job (msg, number) "<<it->pat.msg<<", "<<it->pat.number<<" to fd "<<it->fd<<std::endl;
            }

            /*Wair for results*/
            while(!job_vec.empty()){
                auto ptr = result_map[connfd].wait_pop();
                auto peer = ptr->fd;
                auto job_number = ptr->job_number;
                auto result = ptr->result;
                hash_result = std::max(hash_result,result);
                /*Erase a job_info from job_vec*/
                for(auto it=job_vec.begin();it!=job_vec.end();++it){
                    if(it->pat.number == job_number){
                        job_vec.erase(it);
                        break;
                    }     
                }
            }

            /*Send the result back to client*/
            std::cout<<"Send results back to the client"<<std::endl;
            pat.init(connfd,"result",pat.msg,hash_result);
            char * data = serialization(pat);
            size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
            comm_client.sendmsg(connfd,socketx::message(data,n));
        
        }else{
            std::cerr<<"Wrong type: "<<pat.type<<std::endl; 
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
        p->second.load += 1;
        /*Construct job_info*/
        struct job_info job(p->first,pat);
        job.pat.number = i;
        job_vec.push_back(job);
    }
    return job_vec;
}