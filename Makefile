CXX=g++
CXXFLAGS=-I.. -std=c++11

# For server
SERVER_SRC=server.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o)
SERVER_EXEC=server
SERVER_LIBS=-lserver

# For client
CLIENT_SRC=client.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o)
CLIENT_EXEC=client
CLIENT_LIBS=-lclient

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(SERVER_LIBS)

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(CLIENT_LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(SERVER_EXEC) $(SERVER_OBJ) $(CLIENT_EXEC) $(CLIENT_OBJ)

.PHONY: all clean
