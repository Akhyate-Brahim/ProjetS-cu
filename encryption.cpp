#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>
#include <iostream>

void generateRSAKeyPair(std::string &publicKey, std::string &privateKey, int keyLength = 2048) {
    // Generating the key pair
    RSA *keypair = RSA_new();
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);

    // Key pair generation
    RSA_generate_key_ex(keypair, keyLength, bne, NULL);

    // To store the keys
    BIO *pub = BIO_new(BIO_s_mem());
    BIO *priv = BIO_new(BIO_s_mem());

    // Writing keys to bio
    PEM_write_bio_RSAPublicKey(pub, keypair);
    PEM_write_bio_RSAPrivateKey(priv, keypair, NULL, NULL, 0, NULL, NULL);

    // Key to string
    size_t pub_len = BIO_pending(pub);
    size_t priv_len = BIO_pending(priv);
    publicKey.resize(pub_len);
    privateKey.resize(priv_len);

    BIO_read(pub, &publicKey[0], pub_len);
    BIO_read(priv, &privateKey[0], priv_len);

    // Free resources
    RSA_free(keypair);
    BIO_free_all(pub);
    BIO_free_all(priv);
    BN_free(bne);
}

std::string encryptWithPublicKey(const std::string& data, const std::string& publicKey) {
    RSA *rsa = RSA_new();
    BIO *keybio = BIO_new_mem_buf((void*)publicKey.c_str(), -1);
    if (keybio == NULL) {
        std::cout << "Error creating key BIO" << std::endl;
        return "";
    }

    if (PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL) == NULL) {
        std::cout << "Error loading public key" << std::endl;
        BIO_free(keybio);
        return "";
    }
    int rsaSize = RSA_size(rsa);
    std::string encryptedData;
    encryptedData.resize(rsaSize);

    int result = RSA_public_encrypt(data.size(), (unsigned char*)data.c_str(), (unsigned char*)&encryptedData[0], rsa, RSA_PKCS1_PADDING);
    if (result == -1) {
        char *err = (char*)malloc(130);
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        std::cout << "Encryption Error: " << err << std::endl;
        free(err);
    } else {
        encryptedData.resize(result); // Resize string to actual size of encrypted data
    }
    RSA_free(rsa);
    BIO_free(keybio);

    return encryptedData;
}

std::string decryptWithPrivateKey(const std::string& encryptedData, const std::string& privateKey) {
    RSA *rsa = RSA_new();
    BIO *keybio = BIO_new_mem_buf((void*)privateKey.c_str(), -1);
    PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

    std::string decryptedData;
    decryptedData.resize(RSA_size(rsa));

    RSA_private_decrypt(encryptedData.size(), (unsigned char*)encryptedData.c_str(), (unsigned char*)&decryptedData[0], rsa, RSA_PKCS1_PADDING);

    RSA_free(rsa);
    BIO_free(keybio);

    return decryptedData;
}