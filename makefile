DIR_SRC = ./src
DIR_LIB = ./socketx

SRC = $(wildcard ${DIR_SRC}/*.cpp)
LIB = $(wildcard ${DIR_LIB}/*.cpp)

VPATH = $(DIR_LIB)/src
CFLAGS += -I$(DIR_LIB)/src -std=c++11 -pthread -L$(DIR_LIB)/libs -lsocketx -g

SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})

all: client miner server

client: ${DIR_SRC}/client.o ${DIR_SRC}/header.o 
	g++ ${DIR_SRC}/client.o ${DIR_SRC}/header.o $(CFLAGS) -o ${DIR_SRC}/client

miner: ${DIR_SRC}/miner.o ${DIR_SRC}/header.o 
	g++ ${DIR_SRC}/miner.o ${DIR_SRC}/header.o $(CFLAGS) -o ${DIR_SRC}/miner

server: ${DIR_SRC}/server.o ${DIR_SRC}/header.o
	g++ ${DIR_SRC}/server.o ${DIR_SRC}/header.o $(CFLAGS) -o ${DIR_SRC}/server

${DIR_SRC}/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(DIR_SRC)/client $(DIR_SRC)/miner $(DIR_SRC)/server