#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <iostream>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/evperr.h>
#include <openssl/err.h>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <openssl/ossl_typ.h>
#include <sstream>

// RSA Key Pair Generation
void generateRSAKeyPair(std::string &publicKey, std::string &privateKey, int keyLength = 2048);

// RSA Encryption/Decryption
std::string encryptWithPublicKey(const std::string& data, const std::string& publicKey);
std::string decryptWithPrivateKey(const std::string& encryptedData, const std::string& privateKey);

// AES Encryption/Decryption
int gcm_encrypt(const std::vector<unsigned char>& plaintext,
                const std::vector<unsigned char>& aad,
                const std::vector<unsigned char>& key,
                const std::vector<unsigned char>& iv,
                std::vector<unsigned char>& ciphertext,
                std::vector<unsigned char>& tag);
int gcm_decrypt(const std::vector<unsigned char>& ciphertext,
                const std::vector<unsigned char>& aad,
                const std::vector<unsigned char>& tag,
                const std::vector<unsigned char>& key,
                const std::vector<unsigned char>& iv,
                std::vector<unsigned char>& plaintext);
void generateParameter(std::string& key, int size);
std::vector<unsigned char> readStorageKey(const std::string& filepath);

#endif
