CXX=g++
CXXFLAGS=-I.. -std=c++11

# Base64 library
BASE64_SRC=base64.cpp
BASE64_OBJ=$(BASE64_SRC:.cpp=.o)

# For server
SERVER_SRC=server.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o) $(BASE64_OBJ)
SERVER_EXEC=server
SERVER_LIBS=-lserver

# For client
CLIENT_SRC=sectrans.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o) $(BASE64_OBJ)
CLIENT_EXEC=sectrans
CLIENT_LIBS=-lclient

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(SERVER_LIBS)

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(CLIENT_LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(SERVER_EXEC) $(SERVER_OBJ) $(CLIENT_EXEC) $(CLIENT_OBJ) $(BASE64_OBJ)

.PHONY: all clean
