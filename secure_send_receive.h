#ifndef SECURE_SEND_RECEIVE_H
#define SECURE_SEND_RECEIVE_H

#include <string>

void sendDataEncrypted(const std::string& data, const std::string& publicKey, int portNumber);
std::string receiveDataDecrypted(const std::string& privateKey);
void sendData(const std::string& data, int portNumber);
std::string receiveData();


#endif