#include <iostream>
#include <fstream>
#include <cstring>
#include "server.h"
#include "base64.h" // Include your Base64 header
using namespace std;

void processUploadCommand() {
    char buffer[1024];

    // Receive the file name
    getmsg(buffer);
    string filename = "./files/" + string(buffer);
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

int main() {
    startserver(1234);

    while (true) {
        char commandBuffer[1024];
        cout<<"before get command"<<endl;
        getmsg(commandBuffer);
        cout<<"after get command "<< commandBuffer << endl;
        string command(commandBuffer);
        if (command == "file upload") {
            processUploadCommand();
        }
    }

    stopserver();
    return 0;
}

