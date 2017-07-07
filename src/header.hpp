#ifndef HEADER_HPP
#define HEADER_HPP

#include "socket.hpp"
#include "threadx.hpp"
#include <iostream>
#include <string>

struct packet{
    size_t type_size;
    std::string type;
    size_t msg_size;
    std::string msg;
    size_t number;
    packet(std::string str):type(str),msg_size(0),msg(),number(0){
        type_size = type.size();
    }
};

/*Serialize data from packet to char* */
char * serialization(struct packet &data);

/*Deserialize char* to packet */
struct packet deserialization(char * data, size_t n);

#endif