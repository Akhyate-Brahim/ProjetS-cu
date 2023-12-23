#include <iostream>
#include <fstream>
#include <cstring>
#include "server.h"  // Assuming this is a C++ compatible header
using namespace std;

int main() {
    char buffer[1024];
    startserver(1234);
    getmsg(buffer);
    printf("received message %s \n",buffer);
    stopserver();
    return 0;
}
