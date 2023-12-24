#include <iostream>
#include <fstream>
#include <vector>
#include "client.h"
#include "base64.h"
using namespace std;

void uploadFile(const string& filename) {

    // Send the 'up' command first
    char* upCommand = stringToMutableCString("file upload");
    sndmsg(upCommand, 1234);

    // Open the file for reading in binary mode
    ifstream fin(filename, ios::binary);

    // Read the file into a string
    string fileData((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    // Encode the file data to Base64
    string encodedData = base64_encode(fileData);

    // Send the filename
    char* mutableFilename = stringToMutableCString(filename);
    sndmsg(mutableFilename, 1234);
    delete[] mutableFilename; // Free the allocated memory for filename

    // Send the encoded data in chunks
    const size_t chunkSize = 1024; // Define the maximum chunk size
    for (size_t i = 0; i < encodedData.size(); i += chunkSize) {
        string chunk = encodedData.substr(i, min(chunkSize, encodedData.size() - i));
        char* mutableChunk = stringToMutableCString(chunk);
        sndmsg(mutableChunk, 1234);
        delete[] mutableChunk; // Free the allocated memory for each chunk
    }

    // Send end of file
    char* endFile = stringToMutableCString("END_OF_FILE");
    sndmsg(endFile, 1234);
    cout<< filename << " " << upCommand << " " << endFile << endl;

    // Free the allocated memory
    delete[] endFile;
    delete[] upCommand;

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <command> [arguments]\n";
        return 1;
    }
    string command = argv[1];
    if (command == "-up" && argc == 3) {
        string filename = argv[2];
        uploadFile(filename);
    }

    return 0;
}
