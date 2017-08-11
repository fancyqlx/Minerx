#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <assert.h>

#include "utilx.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Threadx.hpp"

struct packet{
    size_t id;
    size_t type_size;
    std::string type;
    size_t msg_size;
    std::string msg;
    size_t number;
    packet(std::string str):type(str),id(0),msg_size(0),msg(),number(0){
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
    std::shared_ptr<socketx::Connection> conn;
    miner_info(std::shared_ptr<socketx::Connection> conn_):load(0),conn(conn_){     
    }
    miner_info():load(0){     
    }
};

struct job_info{
    std::shared_ptr<socketx::Connection> conn;
    struct packet pat;
    job_info(std::shared_ptr<socketx::Connection> conn_, struct packet pat_):conn(conn_),pat(pat_){
    };
};

/*Result infomation.
* job_number is the number for recognizing jobs,
* it is wrapped in packet.message sent from miners. Noting
* that this number is the original packet.number sent from server
* to miners.
*/
struct result_info{
    std::shared_ptr<socketx::Connection> conn;
    size_t job_number;
    size_t result;
    result_info( std::shared_ptr<socketx::Connection> conn_):conn(conn_),result(0){
    }
};

/*Compare function for miner_info*/
bool less_info(const struct miner_info &info1, const struct miner_info &info2);

bool less_value(const std::pair<std::shared_ptr<socketx::Connection>, struct miner_info> &lhs,
                 const std::pair<std::shared_ptr<socketx::Connection>, struct miner_info> &rhs);

/*Serialize data from packet to char* */
char * serialization(struct packet &data);

/*Deserialize char* to packet */
struct packet deserialization(char * data, size_t n);

#endif