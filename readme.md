# SecTrans Project

## Getting Started

This project is built using C++17 and comprises a server (`server`) and a client (`sectrans`), both pre-compiled and ready to use.

### Starting the Server

To start the server, simply run:

```
server
```

### Adding a User

To add a new user to the user database, use:

```
server --add-user <name> <password>
```

*Note: The password must be at least 8 characters long.*

### Running the Client

To interact with the server using the client, you have the following commands:

- List files:
  ```
  sectrans -list
  ```
- Upload a file:
  ```
  sectrans -up <file>
  ```
- Download a file:
  ```
  sectrans -down <file>
  ```

You will be prompted to enter a username and password for each command.

An already registered user for testing:

- Username: `admin`
- Password: `admin1234`

### File Storage

The files are securely stored in the `./files` directory. When downloading, files are saved to the current directory where `sectrans` is located.

### Source code

- the client main is in [./src/sectrans.cpp](./src/sectrans.cpp)

- the server main is in [./src/server.cpp](./src/server.cpp)

- [./src/base64.h](./src/base64.h) concerns base64 encoding and decoding

- [./src/database.h](./src/database.h)concerns sqlite operations

- [./src/encryption.h](./src/encryption.h) concerns RSA and AES encryption


