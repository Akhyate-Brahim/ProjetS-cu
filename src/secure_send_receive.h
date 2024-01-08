#ifndef SECURE_SEND_RECEIVE_H
#define SECURE_SEND_RECEIVE_H

#include <string>
#include <vector>
#include <iomanip>
#include "base64.h"
#include "server.h"
#include "client.h"
#include "encryption.h"

void sendData(const std::string& data, int portNumber);
std::string receiveData();
void sendDataRSAEncrypted(const std::string& data, const std::string& publicKey, int portNumber);
std::string receiveDataRSADecrypted(const std::string& privateKey);
void sendDataAESEncrypted(const std::string& data, const std::string& key, int portNumber);
std::string receiveDataAESDecrypted(const std::string& key);
void encryptAndStoreFile(const std::string& filename);
void decryptAndRetrieveFile(const std::string& encryptedFilename);
std::vector<unsigned char> readkeyFromFile(const std::string& keyFilename = "aes.key");
std::string sanitizeFilename(const std::string& filename);
bool validateCredentials(const std::string& name, const std::string& password);

#endif