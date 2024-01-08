#!/bin/bash

# Function to fuzz the 'sectrans -up' command with a specified file
fuzz_upload_command() {
    file_name=$1
    echo "Fuzzing upload command with file: $file_name"

    # Apply zzuf to the sectrans command with the given file
    zzuf -r 0.01 sectrans -up "$file_name"
    echo "--------------------------------"
}

# Function to fuzz the filename for the 'sectrans -down' command
fuzz_download_command() {
    file_name=$1
    echo "Fuzzing download command with file: $file_name"

    # Fuzz the filename
    fuzzed_file_name=$(echo "$file_name" | zzuf -r 0.01)

    # Attempt to download using the fuzzed filename
    sectrans -down "$fuzzed_file_name"
    echo "--------------------------------"
}

# Main script execution

# Prepare a file for the '-up' command
upload_file="uploadTestFile.txt"
echo "Test content for upload" > "$upload_file"
# Fuzz the 'sectrans -up' command with the specified file
fuzz_upload_command "$upload_file"

# Fuzz the 'sectrans -down' command with a fuzzed filename
download_file="downloadTestFile.txt"
fuzz_download_command "$download_file"
