#include "header.hpp"

/*Global results queue for all threads
* The queue is thread safe guranteed by socketx.
*/
socketx::squeue<struct packet> res_queue;

/*Compute the hash value for a certain packet*/
void computation(struct packet pat);

/*Send a result from the results queue*/
void send_results(std::shared_ptr<socketx::Connection> conn);

class Miner{
    public:
        Miner(socketx::EventLoop *loop, std::string hostname, std::string port)
        :loop_(loop), hostname_(hostname),port_(port),
        miner_(std::make_shared<socketx::Client>(loop,hostname,port)){
            miner_->setHandleConnectionFunc(std::bind(&Miner::handleConnection, this, std::placeholders::_1));
            miner_->setHandleCloseEvents(std::bind(&Miner::handleCloseEvents, this, std::placeholders::_1));
        }

        void start(){
            miner_->start();
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes, we are going to regist to server!!!\n");
            miner_->setHandleReadEvents(std::bind(&Miner::handleReadEvents, this, std::placeholders::_1));
            pool.submit(std::bind(send_results,conn));
            /*Regist as a miner*/
            struct packet pat("miner");
            /*The bytes of data you need to send*/
            size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
            /*Serialize the data from the struct to the bytes array*/
            char * data = serialization(pat);
            socketx::Message msg(data,n);
            /*Send the message to regist as a client*/
            conn->sendmsg(msg);
             printf("Registed!!!\n");
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            socketx::Message msg = conn->recvmsg();
            if(msg.getSize()==0){
                conn->handleClose();
                return;
            }
            /*Decapsulate the message*/
            struct packet pat = deserialization(msg.getData(),msg.getSize());
            if(pat.type == "computation"){
                std::cout<<"Received a task from the server. The task's id is: "<<pat.id<<std::endl;
                pool.submit(std::bind(computation,pat));
            }
            else
                std::cout<<"error..."<<std::endl;
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            loop_->quit();
        }

    private:
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Client> miner_;
        std::string hostname_;
        std::string port_;

        /*Create a thread pool with the default number of threads*/
        socketx::ThreadPool pool;
};


int main(int argc, char **argv){
if(argc!=3){
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);
    std::string port(argv[2]);
    socketx::EventLoop loop;
    Miner miner(&loop,hostname,port);
    miner.start();
    loop.loop();

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
void send_results(std::shared_ptr<socketx::Connection> conn){
    while(1){
        std::shared_ptr<struct packet> p = res_queue.wait_pop();
        char * data = serialization(*p);
        size_t n = sizeof(size_t) * 4 + p->type_size + 1 + p->msg_size + 1;
        socketx::Message msg(data,n);

        /*Sending*/
        std::cout<<"sending results......"<<std::endl;
        conn->sendmsg(msg);
    }
}