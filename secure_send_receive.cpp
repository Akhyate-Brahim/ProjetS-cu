#include <iostream>
#include "secure_send_receive.h"
#include "base64.h"
#include "server.h"
#include "client.h"
#include "encryption.h"

void sendDataEncrypted(const std::string& data, const std::string& publicKey, int portNumber) {
    std::string encryptedData = encryptWithPublicKey(data, publicKey);
    std::string base64Encoded = base64_encode(encryptedData);
    
    // Send the encrypted data in chunks
    const size_t chunkSize = 1024;
    for (size_t i = 0; i < base64Encoded.size(); i += chunkSize) {
        std::string chunk = base64Encoded.substr(i, std::min(chunkSize, base64Encoded.size() - i));
        sndmsg(stringToMutableCString(chunk), portNumber);
    }

    // Send end of file signal
    sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
}

std::string receiveDataDecrypted(const std::string& privateKey){
    char buffer[1024];
    std::string data;
    while (true) {
        getmsg(buffer);
        if (std::string(buffer) == "END_OF_FILE") {
            break;
        }

        data += std::string(buffer);
    }
    std::string encryptedData = base64_decode(data);
    data = decryptWithPrivateKey(encryptedData, privateKey);
    return data;
}

void sendData(const std::string& data, int portNumber){
    std::string base64Encoded = base64_encode(data);
    // Send the encrypted data in chunks
    const size_t chunkSize = 1024;
    for (size_t i = 0; i < base64Encoded.size(); i += chunkSize) {
        std::string chunk = base64Encoded.substr(i, std::min(chunkSize, base64Encoded.size() - i));
        sndmsg(stringToMutableCString(chunk), portNumber);
    }

    // Send end of file signal
    sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
}

std::string receiveData() {
    char buffer[1024];
    std::string data;

    while (true) {
        getmsg(buffer);
        if (std::string(buffer) == "END_OF_FILE") {
            break;
        }

        data += std::string(buffer);
    }

    return base64_decode(data);
}

