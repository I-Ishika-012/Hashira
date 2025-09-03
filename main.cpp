#include <bits/stdc++.h>
using namespace std;

// ---------- Big integer (base 1e9) ----------
struct BigInt {
    static const uint32_t BASE = 1000000000u; // 1e9
    vector<uint32_t> d; // little-endian limbs

    BigInt() { d.clear(); d.push_back(0); }

    void normalize() {
        while (d.size() > 1 && d.back() == 0) d.pop_back();
    }

    // multiply by small integer (m <= 1e9)
    void mul_small(uint32_t m) {
        if (m == 0) { d.assign(1, 0); return; }
        uint64_t carry = 0;
        for (size_t i = 0; i < d.size(); ++i) {
            uint64_t cur = (uint64_t)d[i] * m + carry;
            d[i] = (uint32_t)(cur % BASE);
            carry = cur / BASE;
        }
        while (carry) {
            d.push_back((uint32_t)(carry % BASE));
            carry /= BASE;
        }
        normalize();
    }

    // add small integer (v < BASE)
    void add_small(uint32_t v) {
        uint64_t carry = v;
        size_t i = 0;
        while (carry && i < d.size()) {
            uint64_t cur = (uint64_t)d[i] + carry;
            d[i] = (uint32_t)(cur % BASE);
            carry = cur / BASE;
            ++i;
        }
        if (carry) d.push_back((uint32_t)carry);
        normalize();
    }

    // multiply BigInt * BigInt
    BigInt mul(const BigInt& other) const {
        size_t n = d.size(), m = other.d.size();
        vector<uint64_t> tmp(n + m, 0);
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < m; ++j)
                tmp[i + j] += (uint64_t)d[i] * (uint64_t)other.d[j];

        BigInt res; res.d.assign(n + m, 0);
        uint64_t carry = 0;
        for (size_t i = 0; i < tmp.size(); ++i) {
            uint64_t cur = tmp[i] + carry;
            res.d[i] = (uint32_t)(cur % BASE);
            carry = cur / BASE;
        }
        while (carry) {
            res.d.push_back((uint32_t)(carry % BASE));
            carry /= BASE;
        }
        res.normalize();
        return res;
    }

    string toString() const {
        // print most significant limb, then the rest zero-padded 9 digits
        ostringstream oss;
        if (d.empty()) return "0";
        oss << d.back();
        for (int i = (int)d.size() - 2; i >= 0; --i) {
            oss << setw(9) << setfill('0') << d[i];
        }
        return oss.str();
    }
};

// ---------- utilities ----------
static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static inline string stripTrailingCommaBrace(const string &s) {
    string t = trim(s);
    while (!t.empty() && (t.back() == ',' || t.back() == '}')) t.pop_back();
    return trim(t);
}

static inline string extractValue(const string &line, const string &key) {
    size_t pos = line.find("\"" + key + "\"");
    if (pos == string::npos) return "";
    size_t colon = line.find(':', pos);
    if (colon == string::npos) return "";
    string val = line.substr(colon + 1);
    val = stripTrailingCommaBrace(val);
    if (!val.empty() && val.front() == '"' && val.back() == '"') {
        val = val.substr(1, val.size() - 2);
    }
    return trim(val);
}

int charToDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    return -1;
}

// parse a value-string in given base into BigInt (throws on invalid digit)
BigInt parseValueToBigInt(const string &digitsStr, int base) {
    BigInt res; // starts at 0
    // Remove possible leading/trailing spaces:
    string s = trim(digitsStr);
    if (s.empty()) throw runtime_error("Empty value string when decoding number.");
    for (char ch : s) {
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
        int dv = charToDigit(ch);
        if (dv < 0 || dv >= base) {
            ostringstream e;
            e << "Digit '" << ch << "' out of range for base " << base;
            throw runtime_error(e.str());
        }
        res.mul_small((uint32_t)base);
        res.add_small((uint32_t)dv);
    }
    return res;
}

// ---------- main parser and logic ----------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // read the whole file "input.json"
    ifstream file("input.json");
    if (!file) {
        cerr << "Could not open input.json\n";
        return 1;
    }

    string line;
    int n = -1, k = -1;
    struct Entry { int base = -1; string value = ""; bool hasBase = false; bool hasValue = false; };
    map<int, Entry> entries;

    int currentKey = -1;
    while (getline(file, line)) {
        string t = stripTrailingCommaBrace(line);
        if (t.find("\"n\"") != string::npos) {
            string val = extractValue(line, "n");
            if (!val.empty()) n = stoi(val);
        } else if (t.find("\"k\"") != string::npos) {
            string val = extractValue(line, "k");
            if (!val.empty()) k = stoi(val);
        } else {
            // detect a new object key like "1": {
            string s = trim(line);
            size_t q1 = s.find('"');
            if (q1 != string::npos) {
                size_t q2 = s.find('"', q1 + 1);
                if (q2 != string::npos) {
                    // Capture key string
                    string keyStr = s.substr(q1 + 1, q2 - q1 - 1);
                    // ignore "keys" object itself
                    if (keyStr != "keys") {
                        // check if line contains '{' (start of object)
                        if (s.find('{') != string::npos) {
                            try {
                                currentKey = stoi(keyStr);
                            } catch (...) {
                                currentKey = -1;
                            }
                        }
                    }
                }
            }

            if (currentKey != -1) {
                if (t.find("\"base\"") != string::npos) {
                    string b = extractValue(line, "base");
                    if (!b.empty()) {
                        try {
                            entries[currentKey].base = stoi(b);
                            entries[currentKey].hasBase = true;
                        } catch (...) {
                            cerr << "Invalid base for key " << currentKey << "\n";
                            return 1;
                        }
                    }
                } else if (t.find("\"value\"") != string::npos) {
                    string v = extractValue(line, "value");
                    if (!v.empty()) {
                        entries[currentKey].value = v;
                        entries[currentKey].hasValue = true;
                    }
                }
                // if line closes the object, reset currentKey
                if (line.find("}") != string::npos && line.find("{") == string::npos) {
                    currentKey = -1;
                }
            }
        }
    }

    // Validate keys
    if (n < 0 || k < 0) {
        cerr << "Missing keys.n or keys.k\n";
        return 1;
    }
    if (entries.count(1) == 0) {
        cerr << "Missing entry for key 1\n";
        return 1;
    }
    if (entries.count(k) == 0) {
        cerr << "Missing entry for key " << k << "\n";
        return 1;
    }
    // Ensure both entries have base and value
    if (!entries[1].hasBase || !entries[1].hasValue) {
        cerr << "Entry 1 incomplete\n";
        return 1;
    }
    if (!entries[k].hasBase || !entries[k].hasValue) {
        cerr << "Entry " << k << " incomplete\n";
        return 1;
    }

    try {
        BigInt x = parseValueToBigInt(entries[1].value, entries[1].base);
        BigInt y = parseValueToBigInt(entries[k].value, entries[k].base);
        BigInt c = x.mul(y);
        cout << c.toString() << "\n";
    } catch (const exception &ex) {
        cerr << "Error decoding numbers: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
