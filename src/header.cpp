#include "header.hpp"

/*Serialize data from packet to char* */
char * serialization(struct packet &data){
    size_t n = sizeof(size_t) * 3 + data.type_size+ data.msg_size;
    char * ret = new char[n];
    char * p = ret;

    /*Copy type_size to the bytes array*/
    memcpy(p,&data.type_size,sizeof(size_t));
    p += sizeof(size_t);

    /*Copy type*/
    const char * s1 = data.type.c_str();
    memcpy(p,s1,data.type_size);
    p += data.type_size;
    delete s1;

    /*Copy msg_size*/
    memcpy(p,&data.msg_size,sizeof(size_t));
    p += sizeof(size_t);

    /*Copy msg*/
    const char * s2 = data.msg.c_str();
    memcpy(p,s2,data.msg_size);
    p += data.msg_size;
    delete s2;

    /*Copy number*/
    memcpy(p,&data.number,sizeof(size_t));
    
    return ret;
}

/*Deserialize char* to packet */
struct packet deserialization(char * data, size_t n){
    size_t type_size=0, msg_size=0, number=0;
    std::string type, msg;
    char * p = data;
    size_t count = 0;

    /*Get type_size and type*/
    if(count <= n){
        memcpy(&type_size,p,sizeof(size_t));
        p += sizeof(size_t);
        count += sizeof(size_t);
    }
    if(count <= n){
        char *s = new char[type_size];
        memcpy(s,p,type_size);
        type = new std::string(s);
        p += type_size;
        count += type_size;
    }
    
    /*Get msg_size and msg*/
    if(count <= n){
        memcpy(&msg_size,p,sizeof(size_t));
        p += sizeof(size_t);
        count += sizeof(size_t);
    }
    if(count <= n){
        s = new char[msg_size];
        memcpy(s,p,msg_size);
        msg = new std::string(s);
        p += msg_size;
        count += msg_size;
    }

    /*Get the number*/
    if(count <= n){
        memcpy(&number,p,sizeof(size_t));
    }

    /*Construct return packet*/
    struct packet pat(type);
    pat.type_size = type_size;
    pat.msg_size = msg_size;
    pat.msg = msg;
    pat.number = number;

    return pat;

}