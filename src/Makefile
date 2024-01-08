CXX=g++
CXXFLAGS=-I. -std=c++17

# OpenSSL libraries
OPENSSL_LIBS=-lssl -lcrypto -lsqlcipher

# Server and Client sources
SERVER_SRCS=server.cpp base64.cpp encryption.cpp secure_send_receive.cpp database.cpp
CLIENT_SRCS=sectrans.cpp base64.cpp encryption.cpp secure_send_receive.cpp database.cpp

# Server and Client executables
SERVER_EXEC=server
CLIENT_EXEC=sectrans

# Local directory for libraries
LIB_DIR=.

all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC):
	$(CXX) $(CXXFLAGS) -o $(SERVER_EXEC) $(SERVER_SRCS) -L$(LIB_DIR) -lserver -lclient $(OPENSSL_LIBS) -Wl,-rpath,$(LIB_DIR)

$(CLIENT_EXEC):
	$(CXX) $(CXXFLAGS) -o $(CLIENT_EXEC) $(CLIENT_SRCS) -L$(LIB_DIR) -lclient -lserver $(OPENSSL_LIBS) -Wl,-rpath,$(LIB_DIR)

clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)

.PHONY: all clean
