#ifndef HEADER_HPP
#define HEADER_HPP

#include "socket.hpp"
#include "threadx.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

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
};

struct miner_info{
    size_t load;
    int fd;
    miner_info(int fd_):load(0),fd(fd_){     
    }
};

struct job_info{
    int fd;
    struct packet pat;
}

struct result_info{
    int fd;
    size_t result;
    result_info(int fd_):fd(fd_),result(0){
    }
};

/*Compare function for miner_info*/
bool less_info(struct miner_info &info1, struct miner_info &info2);

bool less_value(std::pair<int, struct miner_info> &lhs, std::pair<int, struct miner_info> &rhs);

/*Serialize data from packet to char* */
char * serialization(struct packet &data);

/*Deserialize char* to packet */
struct packet deserialization(char * data, size_t n);

#endif