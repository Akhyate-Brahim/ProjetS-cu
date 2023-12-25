#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>
#include "client.h"
#include "server.h"
#include "base64.h"

using namespace std;
namespace fs = std::filesystem;
const int SERVER_PORT = 50000;
const int CLIENT_PORT = 1235;

void uploadFile(const string& filename) 
{
    // Send the 'up' command first
    char* upCommand = stringToMutableCString("file upload");
    sndmsg(upCommand, SERVER_PORT);

    // Open the file for reading in binary mode
    ifstream fin(filename, ios::binary);

    // Read the file into a string
    string fileData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    // Encode the file data to Base64
    string encodedData = base64_encode(fileData);

    // Send the filename
    char* mutableFilename = stringToMutableCString(filename);
    sndmsg(mutableFilename, SERVER_PORT);
    delete[] mutableFilename; // Free the allocated memory for filename

    // Send the encoded data in chunks
    const size_t chunkSize = 1024; // Define the maximum chunk size
    for (size_t i = 0; i < encodedData.size(); i += chunkSize) {
        string chunk = encodedData.substr(i, min(chunkSize, encodedData.size() - i));
        char* mutableChunk = stringToMutableCString(chunk);
        sndmsg(mutableChunk, SERVER_PORT);
        delete[] mutableChunk; // Free the allocated memory for each chunk
    }

    // Send end of file
    char* endFile = stringToMutableCString("END_OF_FILE");
    sndmsg(endFile, SERVER_PORT);
    cout<< filename << " " << upCommand << " " << endFile << endl;

    // Free the allocated memory
    delete[] endFile;
    delete[] upCommand;
}

void listFiles(){
    char buffer[1024];

    // Send the 'list' command first
    char* listCommand = stringToMutableCString("list files");
    sndmsg(listCommand, SERVER_PORT);
    delete[] listCommand;

    // send client port
    startserver(CLIENT_PORT);
    sndmsg(stringToMutableCString(to_string(CLIENT_PORT)), SERVER_PORT);

    // receive the files
    getmsg(buffer);
    stopserver();

    // print files on client console
    cout << buffer << endl;
}

void downloadFile(string filename)
{
    // Send the download command first
    char* downCommand = stringToMutableCString("file download");
    sndmsg(downCommand, SERVER_PORT);

    // send client port
    startserver(CLIENT_PORT);
    sndmsg(stringToMutableCString(to_string(CLIENT_PORT)), SERVER_PORT);
    
    // Send the filename
    char* mutableFilename = stringToMutableCString(filename);
    sndmsg(mutableFilename, SERVER_PORT);
    delete[] mutableFilename;

    ///
    char buffer[1024];
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <command> [arguments]\n";
        return 1;
    }
    string command = argv[1];
    if (command == "-up") {
        string filename = argv[2];
        uploadFile(filename);
    } else if (command == "-list"){
        listFiles();
    } else if (command == "-down"){
        string filename = argv[2];
        downloadFile(filename);
    }

    return 0;
}
