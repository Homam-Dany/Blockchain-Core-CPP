#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <limits>

using namespace std;
using Clock = chrono::steady_clock;

// ============================================================================
// 1. MODULE CRYPTOGRAPHIQUE : SHA-256 (implémentation from scratch)
// ============================================================================
class SHA256 {
    static inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
    static void transform(uint32_t h[8], const uint8_t block[64]) {
        static const uint32_t K[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
        uint32_t w[64];
        for (int i = 0; i < 16; i++)
            w[i] = (uint32_t)block[i*4] << 24 | (uint32_t)block[i*4+1] << 16 |
                   (uint32_t)block[i*4+2] << 8 | (uint32_t)block[i*4+3];
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15] >> 3);
            uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
        for (int i = 0; i < 64; i++) {
            uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
            uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + mj;
            hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
public:
    static string hash(const string& msg) {
        uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                         0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
        vector<uint8_t> data(msg.begin(), msg.end());
        uint64_t bitLen = (uint64_t)msg.size() * 8;
        data.push_back(0x80);
        while (data.size() % 64 != 56) data.push_back(0x00);
        for (int i = 7; i >= 0; i--) data.push_back((uint8_t)(bitLen >> (i * 8)));
        for (size_t off = 0; off < data.size(); off += 64) transform(h, &data[off]);
        ostringstream oss;
        for (int i = 0; i < 8; i++) oss << hex << setw(8) << setfill('0') << h[i];
        return oss.str();
    }
};

// ============================================================================
// 2. STRUCTURES DE DONNÉES (TRANSACTION, ARBRE DE MERKLE, BLOC)
// ============================================================================
struct Transaction {
    string sender;
    string receiver;
    double amount;
    string type; // "TRANSFER" ou "CERTIFICATION"

    string toString() const {
        ostringstream oss;
        if (type == "CERTIFICATION") {
            oss << "[CERTIF] " << sender << " -> " << receiver;
        } else {
            oss << sender << " -> " << receiver << " : " << fixed << setprecision(2) << amount << " tokens";
        }
        return oss.str();
    }
};

string merkleRoot(const vector<Transaction>& txs) {
    if (txs.empty()) return SHA256::hash("");
    vector<string> level;
    for (const auto& t : txs) level.push_back(SHA256::hash(t.toString()));
    while (level.size() > 1) {
        if (level.size() % 2 == 1) level.push_back(level.back());
        vector<string> next;
        for (size_t i = 0; i < level.size(); i += 2)
            next.push_back(SHA256::hash(level[i] + level[i+1]));
        level = next;
    }
    return level[0];
}

enum class Consensus { POW, POS };

struct Block {
    int index = 0;
    time_t timestamp = 0;
    string prevHash;
    string merkle;
    vector<Transaction> txs;
    uint64_t nonce = 0;
    string validator;
    string signature;
    string hash;

    string headerData() const {
        ostringstream oss;
        oss << index << timestamp << prevHash << merkle << nonce << validator;
        return oss.str();
    }
    string computeHash() const { return SHA256::hash(headerData()); }
};

struct Validator { 
    string name; 
    double stake; 
    string secret; 
};

// ============================================================================
// 3. CLASSE BLOCKCHAIN
// ============================================================================
class Blockchain {
public:
    vector<Block> chain;
    vector<Transaction> pending;
    map<string, double> balances;
    vector<Validator> validators;
    Consensus defaultMode;
    int difficulty;

    Blockchain(Consensus m = Consensus::POW, int diff = 3) 
        : defaultMode(m), difficulty(diff) {
        // Utilisateurs par défaut avec soldes initiaux
        balances["Alice"] = 100.0;
        balances["Bob"] = 60.0;
        balances["Charlie"] = 40.0;

        // Validateurs PoS
        validators.push_back({"Alice", 50.0, "key_alice_secret"});
        validators.push_back({"Bob", 30.0, "key_bob_secret"});
        validators.push_back({"Charlie", 20.0, "key_charlie_secret"});

        // Bloc Genesis
        createGenesisBlock();
    }

    void createGenesisBlock() {
        chain.clear();
        Block g;
        g.index = 0;
        g.timestamp = 1700000000;
        g.prevHash = string(64, '0');
        g.merkle = merkleRoot({});
        g.validator = "GENESIS";
        g.hash = g.computeHash();
        chain.push_back(g);
    }

    bool addAccount(const string& name, double initialBalance) {
        if (balances.find(name) != balances.end()) return false;
        balances[name] = initialBalance;
        return true;
    }

    bool addTransaction(const string& from, const string& to, double amt) {
        if (balances.find(from) == balances.end()) {
            cout << "  [ERREUR] Expéditeur inconnu : " << from << "\n";
            return false;
        }
        if (balances.find(to) == balances.end()) {
            balances[to] = 0.0; // Création automatique du compte destinataire
        }

        double spent = 0;
        for (const auto& t : pending) {
            if (t.sender == from && t.type != "CERTIFICATION") spent += t.amount;
        }

        if (amt <= 0 || (balances[from] - spent) < amt) {
            cout << "  [REJETEE] Solde insuffisant pour " << from 
                 << " (Disponible : " << (balances[from] - spent) << " | Requis : " << amt << ")\n";
            return false;
        }

        pending.push_back({from, to, amt, "TRANSFER"});
        cout << "  [OK] Transaction ajoutee dans le pool en attente.\n";
        return true;
    }

    void addCertification(const string& owner, const string& docDescription, const string& docHash) {
        Transaction tx;
        tx.sender = owner;
        tx.receiver = docDescription + " [SHA256: " + docHash.substr(0, 16) + "...]";
        tx.amount = 0.0;
        tx.type = "CERTIFICATION";
        pending.push_back(tx);
        cout << "  [OK] Document certifie en attente de validation dans le prochain bloc.\n";
    }

    void mineBlockPoW(Block& b) {
        string target(difficulty, '0');
        b.nonce = 0;
        cout << "  Minage en cours (Difficulte " << difficulty << ", cible: " << target << ")...\n";
        auto t0 = Clock::now();
        do {
            b.nonce++;
            b.hash = b.computeHash();
        } while (b.hash.compare(0, difficulty, target) != 0);
        double ms = chrono::duration<double, milli>(Clock::now() - t0).count();
        cout << "  [SUCCES PoW] Bloc mine en " << ms << " ms | Nonce = " << b.nonce << "\n";
    }

    Validator& selectValidator(const string& prevHash) {
        mt19937_64 rng(stoull(prevHash.substr(0, 15), nullptr, 16));
        double total = 0;
        for (const auto& v : validators) total += v.stake;
        double pick = uniform_real_distribution<double>(0, total)(rng), cum = 0;
        for (auto& v : validators) {
            cum += v.stake;
            if (pick <= cum) return v;
        }
        return validators.back();
    }

    static string sign(const Validator& v, const string& hash) {
        return SHA256::hash(v.secret + hash);
    }

    void forgePoS(Block& b) {
        Validator& v = selectValidator(b.prevHash);
        b.validator = v.name;
        b.nonce = 0;
        b.hash = b.computeHash();
        b.signature = sign(v, b.hash);
        v.stake += 1.0; // Récompense de validation
        cout << "  [SUCCES PoS] Bloc forge instantanement par le validateur : " << v.name << " (Mise actuelle: " << v.stake << ")\n";
    }

    bool minePending(Consensus mode) {
        if (pending.empty()) {
            cout << "  [INFO] Aucune transaction en attente a miner.\n";
            return false;
        }

        Block b;
        b.index = (int)chain.size();
        b.timestamp = time(nullptr);
        b.prevHash = chain.back().hash;
        b.txs = pending;
        b.merkle = merkleRoot(b.txs);

        if (mode == Consensus::POW) {
            mineBlockPoW(b);
        } else {
            forgePoS(b);
        }

        // Mise à jour des soldes
        for (const auto& t : b.txs) {
            if (t.type == "TRANSFER") {
                balances[t.sender] -= t.amount;
                balances[t.receiver] += t.amount;
            }
        }

        chain.push_back(b);
        pending.clear();
        cout << "  [BLOCKCHAIN] Bloc #" << b.index << " valide et enregistre dans la chaine !\n";
        cout << "  Hash : " << b.hash << "\n";
        return true;
    }

    int firstInvalidBlock(string* reason = nullptr) const {
        auto fail = [&](int i, const string& r) { if (reason) *reason = r; return i; };
        for (size_t i = 1; i < chain.size(); i++) {
            const Block& b = chain[i];
            const Block& p = chain[i-1];
            if (b.merkle != merkleRoot(b.txs)) return fail((int)i, "Racine de Merkle invalide (donnees de transactions alterees)");
            if (b.hash != b.computeHash()) return fail((int)i, "Hash invalide");
            if (b.prevHash != p.hash) return fail((int)i, "Chainage corrompu (prevHash different du hash precedent)");
            if (b.nonce > 0) {
                if (b.hash.compare(0, difficulty, string(difficulty, '0')) != 0)
                    return fail((int)i, "Proof of Work invalide (difficulte non atteinte)");
            }
        }
        return -1;
    }

    bool checkIntegrity() const {
        string reason;
        int bad = firstInvalidBlock(&reason);
        if (bad == -1) {
            cout << "  [INTEGRITE] 100% VALIDE - Tous les " << chain.size() << " blocs sont intacts et inviolables.\n";
            return true;
        } else {
            cout << "  [ALERTE SECURITE] Bloc #" << bad << " corrompu ! Raison : " << reason << "\n";
            return false;
        }
    }

    void displayChain() const {
        cout << "\n======================== HISTORIQUE DE LA BLOCKCHAIN ========================\n";
        for (const auto& b : chain) {
            cout << "+-- Bloc #" << b.index << (b.index == 0 ? " [GENESIS]" : "") << "\n";
            cout << "|  Date        : " << ctime(&b.timestamp);
            cout << "|  Prev Hash   : " << b.prevHash << "\n";
            cout << "|  Merkle Root : " << b.merkle << "\n";
            cout << "|  Bloc Hash   : " << b.hash << "\n";
            if (b.index > 0) {
                if (!b.validator.empty()) cout << "|  Validateur  : " << b.validator << " (PoS)\n";
                if (b.nonce > 0)          cout << "|  Nonce PoW   : " << b.nonce << "\n";
                cout << "|  Transactions (" << b.txs.size() << ") :\n";
                for (const auto& t : b.txs) {
                    cout << "|    * " << t.toString() << "\n";
                }
            }
            cout << "+----------------------------------------------------------------------------\n";
        }
    }

    void displayBalances() const {
        cout << "\n================= REGISTRE DES COMPTES & SOLDES =================\n";
        for (const auto& kv : balances) {
            cout << "  * " << left << setw(15) << kv.first << " : " << fixed << setprecision(2) << kv.second << " tokens\n";
        }
        cout << "----------------- VALIDATEURS PROOF OF STAKE --------------------\n";
        for (const auto& v : validators) {
            cout << "  * " << left << setw(15) << v.name << " : Mise (Stake) = " << v.stake << "\n";
        }
        cout << "=================================================================\n";
    }

    // ========================================================================
    // PERSISTANCE SUR FICHIER (Sauvegarder et Charger)
    // ========================================================================
    bool saveToFile(const string& filename) const {
        ofstream out(filename);
        if (!out) {
            cout << "  [ERREUR] Impossible d'ouvrir le fichier en ecriture : " << filename << "\n";
            return false;
        }

        // Sauvegarde des comptes
        out << "BALANCES " << balances.size() << "\n";
        for (const auto& kv : balances) {
            out << kv.first << " " << kv.second << "\n";
        }

        // Sauvegarde des blocs
        out << "BLOCKS " << chain.size() << "\n";
        for (const auto& b : chain) {
            out << b.index << "|" << b.timestamp << "|" << b.prevHash << "|" << b.merkle 
                << "|" << b.nonce << "|" << b.validator << "|" << b.signature << "|" << b.hash << "\n";
            out << "TXS " << b.txs.size() << "\n";
            for (const auto& t : b.txs) {
                out << t.sender << "|" << t.receiver << "|" << t.amount << "|" << t.type << "\n";
            }
        }

        cout << "  [SUCCES] Blockchain sauvegardee avec succes dans '" << filename << "' !\n";
        return true;
    }

    bool loadFromFile(const string& filename) {
        ifstream in(filename);
        if (!in) {
            return false;
        }

        string tag;
        size_t nBalances = 0;
        if (!(in >> tag >> nBalances) || tag != "BALANCES") return false;

        map<string, double> newBalances;
        for (size_t i = 0; i < nBalances; i++) {
            string u; double s;
            in >> u >> s;
            newBalances[u] = s;
        }

        size_t nBlocks = 0;
        if (!(in >> tag >> nBlocks) || tag != "BLOCKS") return false;

        string dummy;
        getline(in, dummy); // vider la fin de ligne

        vector<Block> newChain;
        for (size_t i = 0; i < nBlocks; i++) {
            string line;
            if (!getline(in, line)) return false;
            stringstream ss(line);
            Block b;
            string sIdx, sTime, sNonce;
            getline(ss, sIdx, '|');
            getline(ss, sTime, '|');
            getline(ss, b.prevHash, '|');
            getline(ss, b.merkle, '|');
            getline(ss, sNonce, '|');
            getline(ss, b.validator, '|');
            getline(ss, b.signature, '|');
            getline(ss, b.hash, '|');

            b.index = stoi(sIdx);
            b.timestamp = stoll(sTime);
            b.nonce = stoull(sNonce);

            string txTag;
            size_t nTxs = 0;
            in >> txTag >> nTxs;
            getline(in, dummy);

            for (size_t j = 0; j < nTxs; j++) {
                string txLine;
                getline(in, txLine);
                stringstream txSs(txLine);
                Transaction t;
                string sAmt;
                getline(txSs, t.sender, '|');
                getline(txSs, t.receiver, '|');
                getline(txSs, sAmt, '|');
                getline(txSs, t.type, '|');
                t.amount = stod(sAmt);
                b.txs.push_back(t);
            }
            newChain.push_back(b);
        }

        balances = newBalances;
        chain = newChain;
        cout << "  [SUCCES] " << chain.size() << " blocs charges depuis '" << filename << "' !\n";
        checkIntegrity();
        return true;
    }
};

// ============================================================================
// 4. MENU INTERACTIF UTILISATEUR
// ============================================================================
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    Blockchain bc(Consensus::POW, 3);
    const string DATA_FILE = "blockchain_data.txt";

    // Chargement automatique si le fichier existe
    cout << "================================================================\n";
    cout << "        BIENVENUE DANS VOTRE LOGICIEL BLOCKCHAIN (C++)          \n";
    cout << "================================================================\n";

    if (bc.loadFromFile(DATA_FILE)) {
        cout << "  -> Donnees precedentes restaurees automatiquement.\n";
    } else {
        cout << "  -> Nouvelle blockchain initialisee (Bloc Genesis cree).\n";
    }

    int choice = -1;
    while (choice != 0) {
        cout << "\n------------------------- MENU PRINCIPAL -------------------------\n";
        cout << "  1. [Transaction] Envoyer des tokens (ex: Alice -> Bob)\n";
        cout << "  2. [Certification] Horodater et certifier un document / texte\n";
        cout << "  3. [Minage] Valider les transactions en attente (PoW ou PoS)\n";
        cout << "  4. [Comptes] Voir les soldes et validateurs\n";
        cout << "  5. [Historique] Afficher tous les blocs de la blockchain\n";
        cout << "  6. [Securite] Verifier l'integrite de la blockchain\n";
        cout << "  7. [Compte] Creer un nouvel utilisateur\n";
        cout << "  8. [Sauvegarder] Enregistrer la blockchain dans un fichier\n";
        cout << "  9. [Recharger] Recharger la blockchain depuis le fichier\n";
        cout << "  0. [Quitter] Sauvegarder et quitter le programme\n";
        cout << "------------------------------------------------------------------\n";
        cout << "Votre choix : ";

        if (!(cin >> choice)) {
            clearInput();
            continue;
        }

        switch (choice) {
            case 1: {
                string from, to;
                double amount;
                cout << "\n-- Envoyer des tokens --\n";
                cout << "Nom de l'expediteur : ";
                cin >> from;
                cout << "Nom du destinataire : ";
                cin >> to;
                cout << "Montant en tokens   : ";
                if (!(cin >> amount)) {
                    cout << "Montant invalide !\n";
                    clearInput();
                    break;
                }
                bc.addTransaction(from, to, amount);
                break;
            }
            case 2: {
                string owner, docName, docContent;
                cout << "\n-- Certifier un document / texte --\n";
                cout << "Proprietaire du document : ";
                cin >> owner;
                clearInput();
                cout << "Titre / Description du document : ";
                getline(cin, docName);
                cout << "Texte ou contenu a certifier    : ";
                getline(cin, docContent);
                string docHash = SHA256::hash(docContent);
                cout << "Empreinte SHA-256 calculee : " << docHash << "\n";
                bc.addCertification(owner, docName, docHash);
                break;
            }
            case 3: {
                int modeChoice = 1;
                cout << "\n-- Choisir le mecanisme de consensus --\n";
                cout << "1. Proof of Work (Minage avec difficulte " << bc.difficulty << ")\n";
                cout << "2. Proof of Stake (Validation ponderee par mise)\n";
                cout << "Votre choix (1 ou 2) : ";
                cin >> modeChoice;
                if (modeChoice == 2) {
                    bc.minePending(Consensus::POS);
                } else {
                    bc.minePending(Consensus::POW);
                }
                break;
            }
            case 4:
                bc.displayBalances();
                break;
            case 5:
                bc.displayChain();
                break;
            case 6:
                bc.checkIntegrity();
                break;
            case 7: {
                string name;
                double initialBalance;
                cout << "\n-- Creer un nouveau compte --\n";
                cout << "Nom d'utilisateur : ";
                cin >> name;
                cout << "Solde initial     : ";
                if (!(cin >> initialBalance)) {
                    cout << "Solde invalide !\n";
                    clearInput();
                    break;
                }
                if (bc.addAccount(name, initialBalance)) {
                    cout << "  [OK] Compte " << name << " cree avec " << initialBalance << " tokens.\n";
                } else {
                    cout << "  [ERREUR] Ce compte existe deja !\n";
                }
                break;
            }
            case 8:
                bc.saveToFile(DATA_FILE);
                break;
            case 9:
                bc.loadFromFile(DATA_FILE);
                break;
            case 0:
                bc.saveToFile(DATA_FILE);
                cout << "\nMerci d'avoir utilise le logiciel Blockchain. A bientot !\n";
                break;
            default:
                cout << "Option inconnue. Veuillez reessayer.\n";
                break;
        }
    }

    return 0;
}
