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

void processUploadCommand() 
{
    char buffer[1024];

    // Receive the file name
    getmsg(buffer);
    string filename = "./files/" + string(buffer);
    fs::path dirPath("./files");
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
    string path = "./files";
    for (const auto &entry : fs::directory_iterator(path)) {
        buffer << entry.path().filename().string() << "\n";
    }
    sndmsg(stringToMutableCString(buffer.str()), portNumber);
    char* endFile = stringToMutableCString("END_OF_LIST");
    sndmsg(endFile, portNumber);
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
    }

    stopserver();
    return 0;
}