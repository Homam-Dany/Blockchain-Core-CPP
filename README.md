# ⛓️ Blockchain-Core-CPP

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017%20%7C%2020-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

A lightweight, high-performance, standalone blockchain implementation in modern **C++ (from scratch)** without any external dependencies.

This project implements the foundational cryptographic and distributed ledger primitives of modern blockchains, including **SHA-256**, **Merkle Trees**, **Proof of Work (PoW)**, **Proof of Stake (PoS)**, an interactive **Command Line Interface (CLI)**, and **disk file persistence**.

---

## 🌟 Key Features

- **🔐 Pure Cryptographic Core (Zero Dependencies)** :
  - Full implementation of the **SHA-256** hashing algorithm *from scratch*.
  - **Merkle Tree Root** calculation for transaction integrity aggregation.
- **⚡ Dual Consensus Mechanism** :
  - **Proof of Work (PoW)** : Difficulty-based cryptographic nonce mining with target prefix calculation.
  - **Proof of Stake (PoS)** : Stake-weighted pseudo-random validator selection with cryptographic signature generation.
- **🛡️ Tamper-Proof Audit & Integrity Verification** :
  - Instant detection of modified past transactions via Merkle root and chained hash mismatch.
- **📜 Document & Data Certification (Timestamping)** :
  - Built-in utility to hash and timestamp any contract, diploma, or text note directly into a block.
- **💾 Automatic Disk Persistence** :
  - Automatically saves blocks, transactions, and account balances to disk (`blockchain_data.txt`) and restores state seamlessly upon startup.
- **💻 Interactive Command Line Interface (CLI)** :
  - User-friendly terminal menu for creating accounts, initiating transfers, mining blocks, and verifying integrity.

---

## 🏗️ Architecture Overview

```
                      +-----------------------------+
                      |         Block Header        |
                      |-----------------------------|
                      |  Index       : uint64_t     |
                      |  Timestamp   : time_t       |
                      |  PrevHash    : SHA-256 (64h)|
                      |  MerkleRoot  : SHA-256 (64h)|
                      |  Nonce / Val : PoW / PoS    |
                      +-----------------------------+
                                     |
                         [ Merkle Tree Root ]
                                /    \
                          Hash(Tx1)  Hash(Tx2)
                             |          |
                            Tx 1       Tx 2
```

---

## 🚀 Getting Started

### 📋 Prerequisites
- Any standard C++ compiler supporting C++11 or later (`g++`, `clang++`, `MSVC`).

### 🔨 Compilation
Clone the repository and compile using `g++` :

```bash
# Clone the repository
git clone https://github.com/Homam-Dany/Blockchain-Core-CPP.git
cd Blockchain-Core-CPP

# Compile with optimization
g++ -std=c++11 -O2 main.cpp -o blockchain_core.exe
```

### ▶️ Run
```bash
# On Windows
.\blockchain_core.exe

# On Linux / macOS
./blockchain_core
```

---

## 🎮 CLI Interactive Menu

Upon launching, the interactive menu allows performing real-time operations :

```text
================================================================
        BIENVENUE DANS VOTRE LOGICIEL BLOCKCHAIN (C++)          
================================================================
------------------------- MENU PRINCIPAL -------------------------
  1. [Transaction]   Envoyer des tokens (ex: Alice -> Bob)
  2. [Certification] Horodater et certifier un document / texte
  3. [Minage]        Valider les transactions en attente (PoW ou PoS)
  4. [Comptes]       Voir les soldes et validateurs
  5. [Historique]    Afficher tous les blocs de la blockchain
  6. [Securite]      Verifier l'integrite de la blockchain
  7. [Compte]        Creer un nouvel utilisateur
  8. [Sauvegarder]   Enregistrer la blockchain dans un fichier
  9. [Recharger]     Recharger la blockchain depuis le fichier
  0. [Quitter]       Sauvegarder et quitter le programme
------------------------------------------------------------------
```

---

## 📊 Consensus Comparison : PoW vs PoS

| Metric | Proof of Work (PoW) | Proof of Stake (PoS) |
| :--- | :--- | :--- |
| **Energy Consumption** | High (intensive CPU mining) | Negligible (< 1 ms calculation) |
| **Block Creation Time** | Exponential with difficulty | Instantaneous |
| **Security Mechanism** | Hash difficulty puzzle | Stake-weighted signature |
| **Real-World Examples** | Bitcoin, Litecoin, Kaspa | Ethereum, Cardano, Solana |

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
