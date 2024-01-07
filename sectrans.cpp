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
const int CLIENT_PORT = 50001;


void uploadFile(const string& aesKey, const string& filename) {
    // Send the 'up' command first
    sendDataAESEncrypted("upload", aesKey,SERVER_PORT);

    // Open the file for reading in binary mode
    ifstream fin(filename, ios::binary);
    string fileData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();
    // Send the filename
    sendDataAESEncrypted(filename, aesKey,SERVER_PORT);
    // Encode and encrypt the file data
    sendDataAESEncrypted(fileData, aesKey,SERVER_PORT);
}

void downloadFile(const string& aesKey, const string& filename) {
    // Send the download command first
    sendDataAESEncrypted("download", aesKey, SERVER_PORT);

    // Send the filename
    sendDataAESEncrypted(filename, aesKey, SERVER_PORT);

    string fileData = receiveDataAESDecrypted(aesKey);
    if (fileData == "FILE_NOT_FOUND"){
        std::cout<< "file does not exist"<<std::endl;
        throw new runtime_error("FILE_NOT_FOUND");
    }
    ofstream fout(filename, ios::binary);
    fout.write(fileData.c_str(), fileData.size());
    fout.close();
}

void listFiles(const string& aesKey) {
    // Send the 'list' command first
    sendDataAESEncrypted("list", aesKey, SERVER_PORT);

    // Receive and decrypt the file list
    string filesList = receiveDataAESDecrypted(aesKey);

    // Print files on client console
    std::cout << filesList << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <command> [arguments]\n";
        return 1;
    }
    std::string publicKey, privateKey, serverPubKey;
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

    // send aes key
    std::string aesKey;
    generateParameter(aesKey, 16);
    sendDataRSAEncrypted(aesKey, serverPubKey, SERVER_PORT);

    string command = argv[1];

    if (command == "-up") {
        string filename = argv[2];
        uploadFile(aesKey,filename);
    } else if (command == "-list") {
        listFiles(aesKey);
    } else if (command == "-down") {
        string filename = argv[2];
        downloadFile(aesKey, filename);
    }
    stopserver();

    return 0;
}
