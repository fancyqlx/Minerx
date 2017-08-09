#include "header.hpp"

class BitCoinClient{
    public:
        Client(socketx::EventLoop *loop, std::string hostname, std::string port)
        :loop_(loop), hostname_(hostname),port_(port),
        client_(std::make_shared<socketx::Client>(loop,hostname,port)){
            client_->setHandleConnectionFunc(std::bind(&BitCoinClient::handleConnection, this, std::placeholders::_1));
            client_->setHandleCloseEvents(std::bind(&BitCoinClient::handleCloseEvents, this, std::placeholders::_1));
            /*Get file descriptor of stdin and regist it into EventLoop*/
            std::cout<<"input string message and maximum nonce: <msg> <nonce>"<<std::endl;
            int fd = fileno(stdin);
            stdinConn = std::make_shared<socketx::Connection>(loop_,fd);
            stdinConn->setHandleReadEvents(std::bind(&BitCoinClient::stdinReadEvents, this, std::placeholders::_1));
            stdinConn->registReadEvents();
        }

        void start(){
            client_->start();
        }

        void stdinReadEvents(std::shared_ptr<socketx::Connection> conn){
            /*encapsulate the message*/
            struct packet pat;
            if(std::cin>>pat.msg>>pat.number){
                pat.init(pat.id,"request",pat.msg,pat.number);
                /*The bytes of data you need to send*/
                size_t n = sizeof(size_t) * 4 + pat.type_size + 1 + pat.msg_size + 1;
                /*Serialize the data from the struct to the bytes array*/
                char * data = serialization(pat);
                socketx::Message msg(data,n);

                /*Send the message, then wait for the result*/
                conn->sendmsg(msg);
            }else{
                std::cin.clear();
                std::cerr<<"Bad input...Please input again!"<<std::endl;
            }
            
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn){
            printf("New connection comes, we are going to set read events!!!\n");
            client_->setHandleReadEvents(std::bind(&BitCoinClient::handleReadEvents, this, std::placeholders::_1));
        }
        void handleReadEvents(std::shared_ptr<socketx::Connection> conn){
            socketx::Message msg = conn->recvmsg();
            if(msg.get_size()==0){
                conn->handleClose();
                break;
            }
            /*Decapsulate the message*/
            struct packet pat = deserialization(msg.get_data(),msg.get_size());
            if(pat.type == "result")
                std::cout<<"hash value: "<<pat.number<<std::endl;
            else
                std::cout<<"error..."<<std::endl;
        }
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn){
            printf("Close connection...\n");
            loop_->quit();
        }

    private:
        std::shared_ptr<socketx::Connection> stdinConn;
        socketx::EventLoop *loop_;
        std::shared_ptr<socketx::Client> client_;
        std::string hostname_;
        std::string port_;
};

int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);
    std::string port(argv[2]);
    socketx::EventLoop loop;
    BitCoinClient client(&loop,hostname,port);
    client.start();
    loop.loop();

    return 0;
}