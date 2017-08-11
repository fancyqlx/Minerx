#include "header.hpp"


/*Data struct for maintaining information of miners*/
std::unordered_map<std::shared_ptr<socketx::Connection>, struct miner_info> miner_map;

/*Data struct for storing results of each client.
* Here we use fd as id of each client, this id will be
* stored in packet.
*/
std::unordered_map<int, socketx::squeue<struct result_info>> result_map;

/*Mutex for visiting miner_map*/
std::mutex miner_mut;

/*Handle tasks from a connection*/
void handle_task(std::shared_ptr<socketx::Connection> conn);

/*Schedule the task distribution among miners*/
std::vector<struct job_info> scheduler(struct packet pat);

class MinerServer{
    public:
        MinerServer(socketx::EventLoop *loop, std::string port)
        :loop_(loop), port_(port),
        server_(std::make_shared<socketx::Server>(loop,port)){
            server_->setHandleConnectionFunc(std::bind(&MinerServer::handleConnection, this, std::placeholders::_1));
            server_->setHandleCloseEvents(std::bind(&MinerServer::handleCloseEvents, this, std::placeholders::_1));
        }

        void start(){
            server_->start();
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes!!!\n");
            server_->setHandleReadEvents(std::bind(&MinerServer::handleReadEvents, this, std::placeholders::_1));
        }

        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            socketx::Message msg = conn->recvmsg();
            if(msg.getSize()==0){
                conn->handleClose();
                return;
            }
            /*Decapsulate the message*/
            struct packet pat = deserialization(msg.getData(),msg.getSize());
            /*Messages from client*/
            if(pat.type == "client"){
                std::string hostname = conn->getPeername();
                size_t hostport = conn->getPort();
                std::cout<<"A client connection from ("<<hostname<<", "<<hostport<<")"<<std::endl;
                conn->unregist();
                pool.submit(std::bind(handle_task,conn));
            }
            /*Messages from miner*/
            else if(pat.type == "miner"){
                std::string hostname = conn->getPeername();
                size_t hostport = conn->getPort();
                std::cout<<"A miner connection from ("<<hostname<<", "<<hostport<<")"<<std::endl;
                
                /*Add a miner into miner_map*/
                miner_mut.lock();
                if(!miner_map.count(conn)){
                    struct miner_info info(conn);
                    miner_map[conn] = info;
                }else{
                    std::cerr<<"Miner connection error......"<<std::endl;
                }
                miner_mut.unlock();
            }
            /*Results from miners*/
            else if(pat.type == "result"){
                /*Add a result of client pat.id into result_map*/
                struct result_info res = result_info(conn);
                res.job_number = std::stoi(pat.msg);
                res.result = pat.number;
                result_map[pat.id].push(res);
                std::cout<<"Received the result (result, job_number): "<<res.result<<", "<<res.job_number<<" of fd "<<pat.id<<std::endl;

                /*The miner decrease its load by 1*/
                miner_mut.lock();
                miner_map[conn].load -= 1;
                miner_mut.unlock();
            }else{
                std::cerr<<"Wrong type: "<<pat.type<<std::endl; 
            }
        }
        
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            loop_->quit();
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Server> server_;
        std::string port_;
        /*Create a thread pool with the default number of threads*/
        socketx::ThreadPool pool;
};

int main(int argc,char** argv){

    if(argc!=2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }

    std::string port(argv[1]);
    socketx::EventLoop loop;
    MinerServer server(&loop,port);
    server.start(); 
    loop.loop();

    return 0;
}

/*Handle tasks from a connection*/
void handle_task(std::shared_ptr<socketx::Connection> conn){
    printf("New thread for handling tasks.....\n");
    /*A list for maintaining job information*/
    std::vector<struct job_info> job_vec;
    size_t hash_result=0;
    
    /*Main loop for handling one client*/
    while(1){
        socketx::Message msg = conn->recvmsg();
        if(msg.getSize()<=0) break;
        struct packet pat = deserialization(msg.getData(),msg.getSize());
        if(pat.type == "request"){
            printf("Received a request from clients\n");
            int connfd = conn->getFD();
            pat.init(connfd,"computation",pat.msg,pat.number);
            job_vec = scheduler(pat);

            /*Send data*/
            for(auto it=job_vec.begin();it!=job_vec.end();++it){
                char * data = serialization(it->pat);
                size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
                it->conn->sendmsg(socketx::Message(data,n));
                std::cout<<"Send a job (msg, number) "<<it->pat.msg<<", "<<it->pat.number<<" to fd "<<it->conn->getFD()<<std::endl;
            }

            /*Wair for results*/
            while(!job_vec.empty()){
                auto ptr = result_map[connfd].wait_pop();
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
            conn->sendmsg(socketx::Message(data,n));
        
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
        if(p==miner_map.end()) break;
        /*Increase load of miner fd*/
        p->second.load += 1;
        /*Construct job_info*/
        struct job_info job(p->first,pat);
        job.pat.number = i;
        job_vec.push_back(job);
    }
    return job_vec;
}