#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "server.h"
#include "client.h"
#include "base64.h"

using namespace std;
namespace fs = std::filesystem;
const int SERVER_PORT = 50000;
const string FILE_STORAGE = "./files/";

void processUploadCommand() 
{
    char buffer[1024];

    // Receive the file name
    getmsg(buffer);
    string filename = FILE_STORAGE + string(buffer);
    fs::path dirPath(FILE_STORAGE);
    ofstream fout(filename, ios::binary);

    // Keep receiving and writing data until the end signal is received
    while (true) {
        getmsg(buffer);

        // Check for signal indicating the end of data transmission
        if (strcmp(buffer, "END_OF_FILE") == 0) {
            break;
        }

        string encodedData(buffer);

        // Decode the Base64 data and write to the file
        string decodedData = base64_decode(encodedData);
        fout.write(decodedData.c_str(), decodedData.size());
    }

    fout.close();
}

void processListCommand(int portNumber)
{
    stringstream buffer;
    for (const auto &entry : fs::directory_iterator(FILE_STORAGE)) {
        buffer << entry.path().filename().string() << "\n";
    }
    sndmsg(stringToMutableCString(buffer.str()), portNumber);
    char* endFile = stringToMutableCString("END_OF_LIST");
    sndmsg(endFile, portNumber);
}

void processDownloadCommand(const string& requestedFilename, int portNumber) 
{
    fs::path filePath = fs::path(FILE_STORAGE) / requestedFilename;

    // Check if the file exists and is not a directory
    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        // Open the file for reading in binary mode
        ifstream fin(filePath, ios::binary);

        // Read and send the file data
        string fileData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
        fin.close();
        string encodedData = base64_encode(fileData);

        // Send the encoded data in chunks
        const size_t chunkSize = 1024; // Define the maximum chunk size
        for (size_t i = 0; i < encodedData.size(); i += chunkSize) {
            string chunk = encodedData.substr(i, min(chunkSize, encodedData.size() - i));
            char* mutableChunk = stringToMutableCString(chunk);
            sndmsg(mutableChunk, portNumber);
            delete[] mutableChunk;
        }

        // Send end of file signal
        char* endFile = stringToMutableCString("END_OF_FILE");
        sndmsg(endFile, portNumber);
        delete[] endFile;
    } else {
        // Handle the case where the file is not found
        char* fileNotFoundMsg = stringToMutableCString("FILE_NOT_FOUND");
        sndmsg(fileNotFoundMsg, portNumber);
        delete[] fileNotFoundMsg;
    }
}

int main() 
{
    startserver(SERVER_PORT);

    while (true) {
        char commandBuffer[1024];
        getmsg(commandBuffer);
        string command(commandBuffer);
        cout << command;
        if (command == "file upload") {
            processUploadCommand();
        }
        else if (command == "list files"){
            char port[1024];
            getmsg(port);
            int portNumber = atoi(port);
            processListCommand(portNumber);
        } 
        else if (command == "file download"){
            char buffer[1024];
            getmsg(buffer);
            int portNumber = atoi(buffer);
            getmsg(buffer);
            string filename = buffer;
            processDownloadCommand(filename, portNumber);
        }
    }

    stopserver();
    return 0;
}