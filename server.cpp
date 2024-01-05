#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "server.h"
#include "client.h"
#include "base64.h"
#include "encryption.h"
#include "secure_send_receive.h"

namespace fs = std::filesystem;
const int SERVER_PORT = 50000;
const std::string FILE_STORAGE = "./files/";

// Global variables for keys
std::string clientPubKey, publicKey, privateKey;

void processUploadCommand() {
    std::string filename = receiveDataDecrypted(privateKey);  // Receive filename
    filename = FILE_STORAGE + filename;
    fs::path dirPath(FILE_STORAGE);
    std::ofstream fout(filename, std::ios::binary);

    std::string decryptedData = receiveDataDecrypted(privateKey); // Receive file data
    fout.write(decryptedData.c_str(), decryptedData.size());
    fout.close();
}

void processListCommand(int portNumber) {
    std::stringstream fileList;
    for (const auto &entry : fs::directory_iterator(FILE_STORAGE)) {
        fileList << entry.path().filename().string() << "\n";
    }
    sendDataEncrypted(fileList.str(), clientPubKey, portNumber); // Send file list
}

void processDownloadCommand(const std::string& requestedFilename, int portNumber) {
    fs::path filePath = fs::path(FILE_STORAGE) / requestedFilename;

    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        std::ifstream fin(filePath, std::ios::binary);
        std::string fileData((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();

        sendDataEncrypted(fileData, clientPubKey, portNumber); // Send file data
    } else {
        sendDataEncrypted("FILE_NOT_FOUND", clientPubKey, portNumber); // Send error message
    }
}

int main() {
    startserver(SERVER_PORT);
    generateRSAKeyPair(publicKey, privateKey); // Generate or load the keys

    while (true) {
        // receive client publicKey
        clientPubKey = receiveData();
        
        // receive client port
        int clientPort = std::stoi(receiveData());

        // sending server publicKey
        sendData(publicKey, clientPort);

        // receive the command
        std::string command = receiveDataDecrypted(privateKey); // Receive command

        if (command.find("upload") != std::string::npos) {
            processUploadCommand();
        } else if (command.find("list") != std::string::npos) {
            processListCommand(clientPort);
        } else if (command.find("download") != std::string::npos) {
            std::string filename = receiveDataDecrypted(privateKey); // Receive filename
            processDownloadCommand(filename, clientPort);
        }
    }

    stopserver();
    return 0;
}
