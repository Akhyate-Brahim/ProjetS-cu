#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>
#include <vector>
#include <openssl/ossl_typ.h>

// RSA Key Pair Generation
void generateRSAKeyPair(std::string &publicKey, std::string &privateKey, int keyLength = 2048);

// RSA Encryption/Decryption
std::string encryptWithPublicKey(const std::string& data, const std::string& publicKey);
std::string decryptWithPrivateKey(const std::string& encryptedData, const std::string& privateKey);

// AES Encryption/Decryption


#endif
