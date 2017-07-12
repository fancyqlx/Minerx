#ifndef HEADER_HPP
#define HEADER_HPP

#include "socket.hpp"
#include "threadx.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>

struct packet{
    size_t id;
    size_t type_size;
    std::string type;
    size_t msg_size;
    std::string msg;
    size_t number;
    packet(std::string str):type(str),msg_size(0),msg(),number(0){
        type_size = type.size();
    }
    void init(size_t id_, std::string type_, std::string msg_, size_t number_){
        id = id_;
        type = type_;
        msg = msg_;
        number = number_;
        type_size = type.size();
        msg_size = msg.size();
    }
};

struct miner_info{
    size_t load;
    int fd;
    miner_info(int fd_):load(0),fd(fd_){     
    }
    miner_info():load(0),fd(0){     
    }
};

struct job_info{
    int fd;
    struct packet pat;
    job_info(int fd_, struct packet pat_):fd(fd_),pat(pat_){
    };
};

/*Result infomation.
* fd is peer host file descriptor.
* job_number is the number for recognizing jobs,
* it is wrapped in packet.message sent from miners. Noting
* that this number is the original packet.number sent from server
* to miners.
*/
struct result_info{
    int fd;
    size_t job_number;
    size_t result;
    result_info(int fd_):fd(fd_),result(0){
    }
};

/*Compare function for miner_info*/
bool less_info(const struct miner_info &info1, const struct miner_info &info2);

bool less_value(const std::pair<int, struct miner_info> &lhs, const std::pair<int, struct miner_info> &rhs);

/*Serialize data from packet to char* */
char * serialization(struct packet &data);

/*Deserialize char* to packet */
struct packet deserialization(char * data, size_t n);

#endif