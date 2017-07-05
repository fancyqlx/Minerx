DIR_SRC = ./src
DIR_LIB = ./libs

SRC = $(wildcard ${DIR_SRC}/*.cpp)
LIB = $(wildcard ${DIR_LIB}/*.cpp)

VPATH = $(DIR_SRC):$(DIR_LIB)
CFLAGS += -I$(DIR_SRC) -I$(DIR_LIB) -std=c++11 -pthread

SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})
LIB_OBJ = $(patsubst %.cpp, %.o, ${LIB})

all: client miner server 

client: client.o $(LIB_OBJ)
	g++ $(CFLAGS) client.o $(LIB_OBJ) -o client

miner: miner.o $(LIB_OBJ)
	g++ $(CFLAGS) miner.o $(LIB_OBJ) -o miner

server: server.o $(LIB_OBJ)
	g++ $(CFLAGS) server.o $(LIB_OBJ) -o server

${DIR_SRC}/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

${DIR_LIB}/%.o:$(DIR_LIB)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(LIB_OBJ) client miner server