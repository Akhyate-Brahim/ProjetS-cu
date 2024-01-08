#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "server.h"
#include "client.h"
#include "base64.h"
#include "encryption.h"
#include "secure_send_receive.h"
#include "database.h"

namespace fs = std::filesystem;
const int SERVER_PORT = 50000;
const std::string FILE_STORAGE = "./files/";

void processUploadCommand(const std::string& aesKey) {
    std::string filename = sanitizeFilename(receiveDataAESDecrypted(aesKey));  // Sanitize and receive filename
    fs::path dirPath(FILE_STORAGE);

    // Check if FILE_STORAGE directory exists, if not, create it
    if (!fs::exists(dirPath)) {
        fs::create_directories(dirPath);  // Create the directory
    }
    fs::path filePath = dirPath / filename;  // Construct the full file 

    std::string decryptedData = receiveDataAESDecrypted(aesKey); // Receive file data
    // Write the received data to a temporary file
    std::ofstream tempOut(filePath.string(), std::ios::binary);
    tempOut.write(decryptedData.c_str(), decryptedData.size());
    tempOut.close();

    // Now encrypt and store the file
    encryptAndStoreFile(filePath.string()); // Encrypt and store the temporary file
    fs::remove(filePath); // Remove the temporary unencrypted file
}



void processDownloadCommand(const std::string& aesKey, std::string& requestedFilename, int portNumber) {
    requestedFilename = sanitizeFilename(requestedFilename);
    fs::path encryptedFilePath = fs::path(FILE_STORAGE) / (requestedFilename + ".enc");

    if (fs::exists(encryptedFilePath) && fs::is_regular_file(encryptedFilePath)) {
        // Decrypt the file
        decryptAndRetrieveFile(encryptedFilePath.string());

        // Read decrypted data
        fs::path decryptedFilePath = fs::path(FILE_STORAGE) / requestedFilename;
        std::ifstream fin(decryptedFilePath, std::ios::binary);
        std::string fileData((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();

        sendDataAESEncrypted(fileData, aesKey, portNumber); // Send file data
        fs::remove(decryptedFilePath); // Remove the temporary decrypted file
    } else {
        sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
    }
}

void processListCommand(const std::string& aesKey, int portNumber) {
    fs::path dirPath(FILE_STORAGE);

    // Check if FILE_STORAGE directory exists
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
        return; // Exit the function if the directory does not exist
    }

    std::stringstream fileList;
    for (const auto &entry : fs::directory_iterator(dirPath)) {
        fileList << entry.path().filename().string() << "\n";
    }

    if (fileList.str().empty()) {
        sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
    } else {
        sendDataAESEncrypted(fileList.str(), aesKey, portNumber); // Send file list
    }
}

int main(int argc, char* argv[]) {
    std::vector<unsigned char> key = readkeyFromFile("sql.key");
    std::string passphrase = toHexString(key);
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--init-users" && argc == 3) {
            const std::string usersFile = argv[2];
            initializeUsers("server.db", passphrase, usersFile);
            std::cout << "Users initialized successfully.\n";
        } else if (arg == "--add-user" && argc == 4) {
            // Add a new user with a username and password
            const std::string username = argv[2];
            const std::string password = argv[3];
            addNewUser("server.db", passphrase, username, password);
        } else {
            std::cerr << "Invalid arguments.\n";
            return 1;
        }
        return 0;
    }
    
    startserver(SERVER_PORT);
    std::string clientPubKey, publicKey, privateKey;
    generateRSAKeyPair(publicKey, privateKey); // Generate the RSA keys
    while (true) {
        // receive client RSA publicKey
        clientPubKey = receiveData();
        
        // receive client port
        int clientPort = std::stoi(receiveData());

        // sending server publicKey
        sendData(publicKey, clientPort);

        // receive aes key
        std::string aesKey = receiveDataRSADecrypted(privateKey);

        //receive client credentials
        std::string name, password;
        name = receiveDataAESDecrypted(aesKey);
        password = receiveDataAESDecrypted(aesKey);
        if (!validateUserCredentials("server.db", passphrase, name, password)){
            sendDataAESEncrypted("invalid_user", aesKey, clientPort);
            continue;
        }else{
            sendDataAESEncrypted("valid_user", aesKey, clientPort);
        }
        
        // receive the command
        std::string command = receiveDataAESDecrypted(aesKey); // Receive command

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
