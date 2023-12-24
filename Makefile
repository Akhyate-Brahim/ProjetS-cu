CXX=g++
CXXFLAGS=-I.. -std=c++17

# Base64 library
BASE64_SRC=base64.cpp
BASE64_OBJ=$(BASE64_SRC:.cpp=.o)

# For server
SERVER_SRC=server.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o) $(BASE64_OBJ)
SERVER_EXEC=server
SERVER_LIBS=-lserver -lclient

# For client
CLIENT_SRC=sectrans.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o) $(BASE64_OBJ)
CLIENT_EXEC=sectrans
CLIENT_LIBS=-lclient -lserver

# Local directory for libraries
LIB_DIR=.

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) -L$(LIB_DIR) $(SERVER_LIBS) -Wl,-rpath,$(LIB_DIR)

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) -L$(LIB_DIR) $(CLIENT_LIBS) -Wl,-rpath,$(LIB_DIR)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(SERVER_EXEC) $(SERVER_OBJ) $(CLIENT_EXEC) $(CLIENT_OBJ) $(BASE64_OBJ)

.PHONY: all clean
