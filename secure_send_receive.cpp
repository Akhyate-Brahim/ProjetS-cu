#include <iostream>
#include "secure_send_receive.h"


void sendDataRSAEncrypted(const std::string& data, const std::string& publicKey, int portNumber) {
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

std::string receiveDataRSADecrypted(const std::string& privateKey){
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


void extractIvTagAndCiphertext(const std::vector<unsigned char>& combinedData,
                               std::vector<unsigned char>& iv,
                               std::vector<unsigned char>& tag,
                               std::vector<unsigned char>& ciphertext) {
    const size_t ivSize = 12; // 96 bits for IV
    const size_t tagSize = 16; // 128 bits for Tag

    if (combinedData.size() < ivSize + tagSize) {
        throw std::runtime_error("Combined data is too short to contain IV and tag.");
    }

    size_t ciphertextSize = combinedData.size() - ivSize - tagSize;

    // Extract IV (12 bytes before the tag)
    iv.assign(combinedData.begin() + ciphertextSize, combinedData.begin() + ciphertextSize + ivSize);

    // Extract tag (last 16 bytes)
    tag.assign(combinedData.end() - tagSize, combinedData.end());

    // The rest is ciphertext
    ciphertext.assign(combinedData.begin(), combinedData.begin() + ciphertextSize);
}

void sendDataAESEncrypted(const std::string& data, const std::string& aesKey, int portNumber) {
    // Convert string data and key to vector<unsigned char>
    std::vector<unsigned char> plaintext(data.begin(), data.end());
    std::vector<unsigned char> key(aesKey.begin(), aesKey.end());
    std::string ivVectorString;
    generateParameter(ivVectorString, 12);
    std::vector<unsigned char> ivVector(ivVectorString.begin(), ivVectorString.end());

    // Prepare additional parameters for encryption
    std::vector<unsigned char> aad; // Optional: Additional Authenticated Data
    std::vector<unsigned char> ciphertext;
    std::vector<unsigned char> tag(16); // Tag for GCM

    // Encrypt the data
    gcm_encrypt(plaintext, aad, key, ivVector, ciphertext, tag);

    // Append the IV and tag to the ciphertext
    ciphertext.insert(ciphertext.end(), ivVector.begin(), ivVector.end());
    ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());

    // Convert the ciphertext to a Base64-encoded string
    std::string base64Encoded = base64_encode(ciphertext.data(), ciphertext.size());

    // Send the encrypted data in chunks
    const size_t chunkSize = 1024;
    for (size_t i = 0; i < base64Encoded.size(); i += chunkSize) {
        std::string chunk = base64Encoded.substr(i, std::min(chunkSize, base64Encoded.size() - i));
        sndmsg(stringToMutableCString(chunk), portNumber);
    }

    // Send end of file signal
    sndmsg(stringToMutableCString("END_OF_FILE"), portNumber);
}


std::string receiveDataAESDecrypted(const std::string& aesKey) {
    char buffer[1024];
    std::string base64EncodedData;

    while (true) {
        getmsg(buffer);
        std::string bufferStr(buffer);
        if (bufferStr == "END_OF_FILE") {
            break;
        }

        base64EncodedData += bufferStr;
    }

    // Decode from Base64
    std::string decodedData = base64_decode(base64EncodedData);
    std::vector<unsigned char> combinedData(decodedData.begin(), decodedData.end());

    // Extract IV, Tag, and Ciphertext
    std::vector<unsigned char> iv, tag, ciphertext;
    extractIvTagAndCiphertext(combinedData, iv, tag, ciphertext);

    // Convert aesKey string to vector
    std::vector<unsigned char> key(aesKey.begin(), aesKey.end());

    // Decrypt the data
    std::vector<unsigned char> decryptedData;
    std::vector<unsigned char> aad; // AAD is not used in this context, so it's empty
    gcm_decrypt(ciphertext, aad, tag, key, iv, decryptedData);

    // Convert decrypted data back to string
    std::string decryptedStr(decryptedData.begin(), decryptedData.end());

    return decryptedStr;
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
std::vector<unsigned char> readAESKeyFromFile(const std::string& keyFilename = "aes.key") {
    std::ifstream keyFile(keyFilename, std::ios::binary);
    if (!keyFile) {
        throw std::runtime_error("Unable to open key file.");
    }

    std::vector<unsigned char> key(std::istreambuf_iterator<char>(keyFile), {});
    keyFile.close();
    return key;
}

void encryptAndStoreFile(const std::string& filename) {
    // Read file data
    std::ifstream fileIn(filename, std::ios::binary);
    std::string fileData((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
    fileIn.close();

    // Convert string data and key to vector<unsigned char>
    std::vector<unsigned char> plaintext(fileData.begin(), fileData.end());
    std::vector<unsigned char> key = readAESKeyFromFile();
    // Generate IV
    std::string ivVectorString;
    generateParameter(ivVectorString, 12); // Assuming generateParameter is a defined function
    std::vector<unsigned char> ivVector(ivVectorString.begin(), ivVectorString.end());

    // Prepare additional parameters for encryption
    std::vector<unsigned char> aad; // Optional: Additional Authenticated Data
    std::vector<unsigned char> ciphertext;
    std::vector<unsigned char> tag(16); // Tag for GCM

    // Encrypt the data
    gcm_encrypt(plaintext, aad, key, ivVector, ciphertext, tag);

    // Append IV and tag to the ciphertext
    ciphertext.insert(ciphertext.end(), ivVector.begin(), ivVector.end());
    ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());

    // Write the encrypted data to a file
    std::ofstream fileOut(filename + ".enc", std::ios::binary);
    fileOut.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
    fileOut.close();
}

void decryptAndRetrieveFile(const std::string& encryptedFilename) {
    // Read encrypted data from file
    std::ifstream fileIn(encryptedFilename, std::ios::binary);
    std::string encryptedData((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
    fileIn.close();
    
    // Convert encrypted data to vector<unsigned char>
    std::vector<unsigned char> combinedData(encryptedData.begin(), encryptedData.end());

    // Extract IV, Tag, and Ciphertext
    std::vector<unsigned char> iv, tag, ciphertext;
    extractIvTagAndCiphertext(combinedData, iv, tag, ciphertext);

    // Convert aesKey string to vector
    std::vector<unsigned char> key = readAESKeyFromFile();

    // Decrypt the data
    std::vector<unsigned char> decryptedData;
    std::vector<unsigned char> aad; // AAD is not used in this context, so it's empty
    gcm_decrypt(ciphertext, aad, tag, key, iv, decryptedData);

    // Write decrypted data back to a file (with original filename)
    std::string originalFilename = encryptedFilename.substr(0, encryptedFilename.find(".enc"));
    std::ofstream fileOut(originalFilename, std::ios::binary);
    fileOut.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
    fileOut.close();
}

std::string sanitizeFilename(const std::string& filename) {
    std::string sanitized;
    for (char ch : filename) {
        // Skip path traversal characters and non-printable characters
        if (ch == '/' || ch == '\\' || ch < 32 || ch > 126) continue;
        sanitized.push_back(ch);
    }
    return sanitized;
}