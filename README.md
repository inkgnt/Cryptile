# Cryptile

Cryptile is a secure cross-platform vault for storing sensitive personal data. It supports multiple authentication methods, including a smart card and a master password, and is designed around envelope encryption with per-record key separation.

## Overview
TODO :)
This project is unfinished, and if you want to participate, well, let me know :) mrqirll@gmail.com

## Security model

Cryptile separates each record into two logical parts:
- **non-sensitive data**
- **sensitive data**

However, non-sensitive data is still sensitive. The distinction is purely relative and reflects different handling levels, where non-sensitive data is considered less sensitive than sensitive data. The only practical difference is the access pattern: non-sensitive data is loaded on application unlock, while sensitive data is loaded on demand. Both are handled using secure buffers and secure memory practices.

Each record contains a randomly generated `object_id`, per-record salts, and initialization vectors.
Cryptographic keys are derived using HKDF from a smart-card (FIDO2) HMAC challenge response and record-specific data. In the recommended configuration, the smart card acts as the root of trust. If a smart card is not available, the user may use a master password; however, this mode is considered much weaker, since the entropy source is human-generated rather than hardware-backed.

The database encryption key is initialized once per application session, after which SQLCipher manages its lifecycle internally.

There are two layers of encryption. The first layer is SQLCipher, and the second layer is AES-256-GCM. Each record has its own pair of CEKs, which are themselves encrypted with their KEKs. The actual data is encrypted using these CEKs: first the CEKs are decrypted, and only then the data is decrypted.

When the user opens a record, the application derives the required keys, decrypts the data, displays it on screen, and wipes sensitive buffers as soon as possible.

At any given time, only one record is decrypted in memory. Cryptile uses libsodium’s secure memory primitives to reduce the risk of sensitive data exposure.


The application provides:
TODO :)

## Technologies
- **Qt 6** for the graphical user interface (GUI).
- **SQLite** with **SQLCipher** for encrypted database storage.
- **OpenSSL** and **libsodium** for cryptographic primitives.
- **OpenSC** for smart-card communication via APDU commands.
- **vcpkg** for dependency management.

## Installation

### Requirements

1. **Qt 6** (with C++17 support and above)
2. **vcpkg**

### Build Instructions

#### To compile from source:

1. Download the source code on the [Realeses](https://github.com/inkgnt/Cryptile/releases) page or clone the repository:
   
   ```bash
   git clone https://github.com/inkgnt/Cryptile.git
   cd Cryptile

2. Ensure that you have all dependencies installed.
   TODO :)
   
3. Build the project using CMake:
   TODO :)
   
4. Run the application:
   TODO :)
   
#### To use an installer:
  Go to the [Releases](https://github.com/inkgnt/Cryptile/releases) page and download the latest version.

## Usage
TODO :)

## License

This project is licensed under the Apache License, Version 2.0.  
See the [LICENSE](./LICENSE.txt) file for details.

You may obtain a copy of the license at:
http://www.apache.org/licenses/LICENSE-2.0

Copyright (c) 2026 The Cryptile contributors.

## Third-party components

This project includes third-party software components, each subject to its own license terms.

Full copyright and licensing information is provided in the [NOTICE](./NOTICE.txt) file.
