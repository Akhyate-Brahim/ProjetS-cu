#include "database.h"

// Function to hash a password using SHA-256
std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (unsigned char i : hash) {
        ss << std::hex << static_cast<int>(i);
    }
    return ss.str();
}

// Modified function to open a SQLCipher database
sqlite3* openDatabase(const std::string& dbFile, const std::string& passphrase) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbFile.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return nullptr;
    }

    // Set up the key for SQLCipher encryption
    std::string keyQuery = "PRAGMA key = '" + passphrase + "';";
    rc = sqlite3_exec(db, keyQuery.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error setting SQLCipher key: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return nullptr;
    }

    return db;
}

void createUsersTable(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, hashed_password TEXT NOT NULL);";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool addUser(sqlite3* db, const std::string& username, const std::string& hashedPassword) {
    std::string sql = "INSERT INTO users (username, hashed_password) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    }
    return false;
}

bool authenticateUser(sqlite3* db, const std::string& username, const std::string& password) {
    std::string sql = "SELECT hashed_password FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* hashedPassword = sqlite3_column_text(stmt, 0);
            std::string providedHashedPassword = hashPassword(password);
            bool isAuthenticated = (providedHashedPassword == reinterpret_cast<const char*>(hashedPassword)); // Secure comparison
            sqlite3_finalize(stmt);
            return isAuthenticated;
        }
        sqlite3_finalize(stmt);
    }
    return false;
}
// Function to initialize users from a file
void initializeUsers(const std::string& dbFile, const std::string& passphrase, const std::string& usersFile) {
    sqlite3* db = openDatabase(dbFile, passphrase);
    if (db) {
        createUsersTable(db);
        std::ifstream file(usersFile);
        std::string line;
        while (std::getline(file, line)) {
            auto separator = line.find(':');
            if (separator != std::string::npos) {
                std::string username = line.substr(0, separator);
                std::string password = line.substr(separator + 1);
                addUser(db, username, hashPassword(password));
            }
        }
        sqlite3_close(db);
    }
}

// Function to add a new user
void addNewUser(const std::string& dbFile, const std::string& passphrase, const std::string& username, const std::string& password) {
    sqlite3* db = openDatabase(dbFile, passphrase);
    if (db) {
        createUsersTable(db);
        addUser(db, username, hashPassword(password));
        sqlite3_close(db);
    }
}
bool validateUserCredentials(const std::string& dbFile, const std::string& passphrase, const std::string& username, const std::string& password) {
    // Open the database with the given passphrase
    sqlite3* db = openDatabase(dbFile, passphrase);
    if (!db) {
        return false; // Database opening failed
    }

    // Attempt to authenticate the user
    bool isAuthenticated = authenticateUser(db, username, password);

    // Close the database connection
    sqlite3_close(db);

    return isAuthenticated;
}
