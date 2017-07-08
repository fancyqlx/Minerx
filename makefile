DIR_SRC = ./src
DIR_LIB = ./libs

SRC = $(wildcard ${DIR_SRC}/*.cpp)
LIB = $(wildcard ${DIR_LIB}/*.cpp)

VPATH = $(DIR_SRC):$(DIR_LIB)
CFLAGS += -I$(DIR_SRC) -I$(DIR_LIB) -std=c++11 -pthread -g

SRC_OBJ = $(patsubst %.cpp, %.o, ${SRC})
LIB_OBJ = $(patsubst %.cpp, %.o, ${LIB})

all: client miner server

client: ${DIR_SRC}/client.o ${DIR_SRC}/header.o $(LIB_OBJ)
	g++ $(CFLAGS) ${DIR_SRC}/client.o ${DIR_SRC}/header.o $(LIB_OBJ) -o ${DIR_SRC}/client

miner: ${DIR_SRC}/miner.o ${DIR_SRC}/header.o $(LIB_OBJ)
	g++ $(CFLAGS) ${DIR_SRC}/miner.o ${DIR_SRC}/header.o $(LIB_OBJ) -o ${DIR_SRC}/miner

server: ${DIR_SRC}/server.o ${DIR_SRC}/header.o $(LIB_OBJ)
	g++ $(CFLAGS) ${DIR_SRC}/server.o ${DIR_SRC}/header.o $(LIB_OBJ) -o ${DIR_SRC}/server

${DIR_SRC}/%.o:$(DIR_SRC)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

${DIR_LIB}/%.o:$(DIR_LIB)/%.cpp
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(LIB_OBJ) client miner server