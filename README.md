# CryptPass

**CryptPass** is an application for secure password storage that uses OpenSSL. The application allows users to add, delete, and edit password entries while protecting them using modern encryption techniques.

## Overview

CryptPass uses:
- **Qt** for the graphical user interface (GUI).
- **SQLite** for storing password data.
- **AES-256 encryption in CBC mode** for secure password storage.
- **Scrypt and PBKDF2** for hashing the master password.

The application provides:
- **Password encryption:** Encrypts passwords using the AES-256 algorithm in CBC mode.
- **Secure password management:** A user-friendly GUI for managing your passwords safely.
- **CSV import/export:** Ability to import and export passwords in CSV format.
- **Rainbow table attack prevention:** Utilizes a unique salt during hashing.

## Features

- **On-the-fly encryption:** Passwords are encrypted before saving, and a secure master password is used for decryption.
- **Flexibility:** Users can add, edit, and delete password entries.
- **Import/Export:** Easily import and export passwords to and from CSV files.
- **Master password hashing:** Implements Scrypt and PBKDF2 with a unique salt for enhanced security.
- **Planned SQLCipher support:** In the future, the entire database may be encrypted using SQLCipher.

## Technologies

- **C++17**
- **Qt 6**
- **SQLite** (with planned support for on-the-fly encryption via SQLCipher in the future)
- **OpenSSL**

## Installation

### Requirements

1. **Qt 6** (with C++17 support and above)
2. **OpenSSL**
3. **SQLCipher** (planned for future full database encryption)

### Build Instructions

#### To compile from source:

1. Download the source code on the [Realeses](https://github.com/inkogniito/CryptPass/releases) page or clone the repository:
   
   ```bash
   git clone https://github.com/inkogniito/CryptPass.git
   cd CryptPass
   
2. Build the project using CMake:
   
   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release

3. Ensure that you have all dependencies installed, including Qt, OpenSSL, and (in the future) SQLCipher.

4. Run the application:
  In the build/Release folder, execute CryptPass.exe.

#### To use an installer:
  Go to the [Releases](https://github.com/yourusername/CryptPass/releases) page and download the latest version.

## Usage
1. Master Password Setup:
  On first launch, the user is prompted to set a master password, which is used to encrypt and decrypt all other passwords profiles.

2. Login:
  After setting up the master password, log in using the same password.

3. Password Management:
  In the main window, you can:

    - Add a new password entry by specifying URL, login, and password.

    - Edit or delete existing entries.

    - Import or export passwords in CSV format.

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

It incorporates the following third-party libraries, which are distributed under their own licenses:

*   **Qt 6:** LGPL v3.0
*   **OpenSSL:** Apache License 2.0
*   **SQLCipher Community Edition:** BSD License

Full copyright and license notices for these dependencies can be found in the **[`NOTICE`](NOTICE)** file.