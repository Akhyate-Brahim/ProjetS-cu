#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>
#include <sstream>
#include <iostream>
#include <openssl/sha.h>
#include <fstream>


void initializeUsers(const std::string& dbFile, const std::string& passphrase, const std::string& usersFile);
void addNewUser(const std::string& dbFile, const std::string& passphrase, const std::string& username, const std::string& password);
bool validateUserCredentials(const std::string& dbFile, const std::string& passphrase, const std::string& username, const std::string& password);

#endif // DATABASE_H
