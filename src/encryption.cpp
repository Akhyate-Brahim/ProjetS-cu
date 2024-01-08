#include "encryption.h"
#include <filesystem>
namespace fs = std::filesystem;

void generateRSAKeyPair(std::string &publicKey, std::string &privateKey, int keyLength) {
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

    // Create a buffer for the decrypted data
    std::vector<unsigned char> buffer(RSA_size(rsa));

    // Perform the decryption
    int decryptedSize = RSA_private_decrypt(encryptedData.size(), 
                                            reinterpret_cast<const unsigned char*>(encryptedData.c_str()), 
                                            buffer.data(), rsa, RSA_PKCS1_PADDING);
    if (decryptedSize == -1) {
        // Handle decryption error
        RSA_free(rsa);
        BIO_free(keybio);
        throw std::runtime_error("RSA decryption failed.");
    }

    // Create a string from the decrypted data buffer
    std::string decryptedData(buffer.begin(), buffer.begin() + decryptedSize);

    RSA_free(rsa);
    BIO_free(keybio);

    return decryptedData;
}


void generateParameter(std::string& key, int size) {
    unsigned char key_bytes[size];
    RAND_bytes(key_bytes, sizeof(key_bytes));;
    key = std::string((char *)key_bytes, sizeof(key_bytes));
}


void handleErrors() {
    throw std::runtime_error("OpenSSL error occurred.");
}

int gcm_encrypt(const std::vector<unsigned char>& plaintext,
                const std::vector<unsigned char>& aad,
                const std::vector<unsigned char>& key,
                const std::vector<unsigned char>& iv,
                std::vector<unsigned char>& ciphertext,
                std::vector<unsigned char>& tag) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL))
        handleErrors();

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()))
        handleErrors();

    if(!aad.empty()) {
        if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad.data(), aad.size()))
            handleErrors();
    }

    ciphertext.resize(plaintext.size());
    if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()))
        handleErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len))
        handleErrors();
    ciphertext_len += len;

    tag.resize(16); // GCM tag is typically 16 bytes
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()))
        handleErrors();

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int gcm_decrypt(const std::vector<unsigned char>& ciphertext,
                const std::vector<unsigned char>& aad,
                const std::vector<unsigned char>& tag,
                const std::vector<unsigned char>& key,
                const std::vector<unsigned char>& iv,
                std::vector<unsigned char>& plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(!EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL))
        handleErrors();

    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL))
        handleErrors();

    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()))
        handleErrors();

    if(!aad.empty()) {
        if(!EVP_DecryptUpdate(ctx, NULL, &len, aad.data(), aad.size()))
            handleErrors();
    }

    plaintext.resize(ciphertext.size());
    if(!EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()))
        handleErrors();
    plaintext_len = len;

    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), (void*)tag.data()))
        handleErrors();

    ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);

    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0) {
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        return plaintext_len;
    } else {
        return -1; // Decryption failed
    }
}
std::vector<unsigned char> readStorageKey(const std::string& filepath) {
    std::ifstream keyFile(filepath, std::ios::in | std::ios::binary);
    if (!keyFile.is_open()) {
        throw std::runtime_error("Unable to open key file: " + filepath);
    }

    std::vector<unsigned char> key(std::istreambuf_iterator<char>(keyFile), {});
    keyFile.close();

    if (key.size() != 16) { // Check if the key size is 32 bytes (256 bits)
        throw std::runtime_error("Key size is incorrect. Expected 32 bytes.");
    }

    return key;
}