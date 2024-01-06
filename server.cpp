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

void processUploadCommand(const std::string& aesKey) {
    std::string filename = receiveDataAESDecrypted(aesKey);  // Receive filename
    filename = FILE_STORAGE + filename;
    fs::path dirPath(FILE_STORAGE);
    std::ofstream fout(filename, std::ios::binary);

    std::string decryptedData = receiveDataAESDecrypted(aesKey); // Receive file data
    fout.write(decryptedData.c_str(), decryptedData.size());
    fout.close();
}

void processListCommand(const std::string& aesKey, int portNumber) {
    std::stringstream fileList;
    for (const auto &entry : fs::directory_iterator(FILE_STORAGE)) {
        fileList << entry.path().filename().string() << "\n";
    }
    sendDataAESEncrypted(fileList.str(), aesKey, portNumber); // Send file list
}

void processDownloadCommand(const std::string& aesKey, const std::string& requestedFilename, int portNumber) {
    fs::path filePath = fs::path(FILE_STORAGE) / requestedFilename;

    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        std::ifstream fin(filePath, std::ios::binary);
        std::string fileData((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();

        sendDataAESEncrypted(fileData, aesKey, portNumber); // Send file data
    } else {
        sendDataAESEncrypted("FILE_NOT_FOUND", aesKey, portNumber); // Send error message
    }
}

int main() {
    startserver(SERVER_PORT);
    std::string clientPubKey, publicKey, privateKey;
    generateRSAKeyPair(publicKey, privateKey); // Generate or load the keys

    while (true) {
        // receive client publicKey
        clientPubKey = receiveData();
        
        // receive client port
        int clientPort = std::stoi(receiveData());

        // sending server publicKey
        sendData(publicKey, clientPort);

        // receive aes key
        std::string aesKey = receiveDataRSADecrypted(privateKey);
        // receive the command
        std::string command = receiveDataAESDecrypted(aesKey); // Receive command
        std::cout << "command : " << command << std::endl;
        if (command == "upload") {
            processUploadCommand(aesKey);
        } else if (command == "list") {
            processListCommand(aesKey, clientPort);
        } else if (command == "download") {
            std::string filename = receiveDataAESDecrypted(aesKey); // Receive filename
            processDownloadCommand(aesKey, filename, clientPort);
        }
    }

    stopserver();
    return 0;
}
