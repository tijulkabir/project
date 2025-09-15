🔒 Secure Vault

A simple yet powerful C++ Secure Vault for managing sensitive information.
This project allows you to store passwords, backup codes, and quick notes with basic encryption, protected by a master login system.

@@ Features:

#@ Master login system (username: toha, password: toha55)

#@ Store credentials with passwords and backup codes

#@ Simple encryption & decryption support

#@ Built with Object-Oriented Programming principles

#@ Ready for future GUI expansion (OpenGL integration planned)

📂 Project Structure
secure-vault/
 ├── src/         → Source files (.cpp)
 ├── include/     → Header files (.h/.hpp)
 ├── lib/         → Extra libraries (if any)
 ├── assets/      → Screenshots & demo GIFs
 ├── CMakeLists.txt
 ├── README.md
 ├── LICENSE
 └── .gitignore

🛠️ Build & Run
🔹 Linux / MacOS
g++ src/*.cpp -I include -o vault
./vault

🔹 Windows (MinGW)
g++ src/*.cpp -I include -o vault.exe
vault.exe

📸 Preview
->Master login screen
->Adding and retrieving credentials
->Encrypted data flow in terminal
