#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "base64.h"
#include "client.h"
#include "server.h"
#include "encryption.h"
#include "secure_send_receive.h"

using namespace std;
namespace fs = std::filesystem;
const int SERVER_PORT = 50000;
const int CLIENT_PORT = 1235;

// Assuming global variables for keys
std::string publicKey, privateKey, serverPubKey;

void uploadFile(const string& filename) {
    // Send the 'up' command first
    sendDataEncrypted("upload", serverPubKey, SERVER_PORT);

    // Open the file for reading in binary mode
    ifstream fin(filename, ios::binary);
    string fileData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    // Encode and encrypt the file data
    sendDataEncrypted(fileData, serverPubKey, SERVER_PORT);

    // Send the filename
    sendDataEncrypted(filename, serverPubKey, SERVER_PORT);
}

void listFiles() {
    // Send the 'list' command first
    sendDataEncrypted("list", serverPubKey, SERVER_PORT);

    // Receive and decrypt the file list
    string filesList = receiveDataDecrypted(privateKey);

    // Print files on client console
    std::cout << filesList << std::endl;
}

void downloadFile(string filename) {
    // Send the download command first
    sendDataEncrypted("download", serverPubKey, SERVER_PORT);

    // Send the filename
    sendDataEncrypted(filename, serverPubKey, SERVER_PORT);

    string fileData = receiveDataDecrypted(privateKey);
    ofstream fout(filename, ios::binary);
    fout.write(fileData.c_str(), fileData.size());
    fout.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <command> [arguments]\n";
        return 1;
    }
    // start client server
    startserver(CLIENT_PORT);

    
    // Generate or load the keys
    generateRSAKeyPair(publicKey, privateKey);
    
    // send the client public key
    sendData(publicKey, SERVER_PORT);
    
    // send client port to the server
    sendData(std::to_string(CLIENT_PORT), SERVER_PORT);

    // receive server public key
    serverPubKey = receiveData();

    string command = argv[1];
    if (command == "-up") {
        string filename = argv[2];
        uploadFile(filename);
    } else if (command == "-list") {
        listFiles();
    } else if (command == "-down") {
        string filename = argv[2];
        downloadFile(filename);
    }
    stopserver();

    return 0;
}
