CXX=g++
CXXFLAGS=-I. -std=c++17

# OpenSSL libraries
OPENSSL_LIBS=-lssl -lcrypto

# Base64 objects
BASE64_SRC=base64.cpp
BASE64_OBJ=$(BASE64_SRC:.cpp=.o)

# Encryption objects
ENCRYPTION_SRC=encryption.cpp
ENCRYPTION_OBJ=$(ENCRYPTION_SRC:.cpp=.o)

# Secure Send/Receive objects
SECURE_SEND_RECEIVE_SRC=secure_send_receive.cpp
SECURE_SEND_RECEIVE_OBJ=$(SECURE_SEND_RECEIVE_SRC:.cpp=.o)

# Server executable
SERVER_SRC=server.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o) $(BASE64_OBJ) $(ENCRYPTION_OBJ) $(SECURE_SEND_RECEIVE_OBJ)
SERVER_EXEC=server

# Client executable
CLIENT_SRC=sectrans.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o) $(BASE64_OBJ) $(ENCRYPTION_OBJ) $(SECURE_SEND_RECEIVE_OBJ)
CLIENT_EXEC=sectrans

# Local directory for libraries
LIB_DIR=.

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) -L$(LIB_DIR) -lserver -lclient $(OPENSSL_LIBS) -Wl,-rpath,$(LIB_DIR)

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CXX) -g -o $@ $^ $(CXXFLAGS) -L$(LIB_DIR) -lclient -lserver $(OPENSSL_LIBS) -Wl,-rpath,$(LIB_DIR)

%.o: %.cpp %.h
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(SERVER_EXEC) $(SERVER_OBJ) $(CLIENT_EXEC) $(CLIENT_OBJ) $(BASE64_OBJ) $(ENCRYPTION_OBJ) $(SECURE_SEND_RECEIVE_OBJ)

.PHONY: all clean
