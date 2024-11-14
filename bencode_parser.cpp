#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <map>

using namespace std;

typedef struct {
    string announce;
    vector<unsigned char> info_hash;
    string name;
    int piece_length;
    vector<unsigned char> pieces;
} TorrentInfo;

int parseBencodeInteger(const string &data, size_t &pos);
string parseBencodeString(const string &data, size_t &pos);
vector<string> parseBencodeList(const string &data, size_t &pos);
map<string, string> parseBencodeDict(const string &data, size_t &pos);

void parseTorrentFile(const string &filePath, TorrentInfo &torrentInfo);
void computeInfoHash(const string &data, vector<unsigned char> &infoHash);
void printHex(const vector<unsigned char> &data);

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <torrent-file>" << endl;
        return 1;
    }

    string torrentFile = argv[1];
    TorrentInfo torrentInfo;

    parseTorrentFile(torrentFile, torrentInfo);

    cout << "Announce URL: " << torrentInfo.announce << endl;
    cout << "Name: " << torrentInfo.name << endl;
    cout << "Piece Length: " << torrentInfo.piece_length << endl;
    cout << "Pieces: ";
    printHex(torrentInfo.pieces);
    cout << "Info Hash: ";
    printHex(torrentInfo.info_hash);
    
    return 0;
}

void parseTorrentFile(const string &filePath, TorrentInfo &torrentInfo) {
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        exit(1);
    }

    string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    size_t pos = 0;
    map<string, string> dict = parseBencodeDict(data, pos);

    torrentInfo.announce = dict["announce"];
    torrentInfo.name = dict["name"];
    torrentInfo.piece_length = stoi(dict["piece length"]);
    torrentInfo.pieces = vector<unsigned char>(dict["pieces"].begin(), dict["pieces"].end());

    string infoHashStr = dict["info"];
    computeInfoHash(infoHashStr, torrentInfo.info_hash);

    file.close();
}

map<string, string> parseBencodeDict(const string &data, size_t &pos) {
    map<string, string> dict;

    pos++;  
    while (data[pos] != 'e') {
        string key = parseBencodeString(data, pos);
        string value;
        
        if (data[pos] == 'i') {
            int integerValue = parseBencodeInteger(data, pos);
            value = to_string(integerValue);
        } else if (data[pos] == 'l') {
            vector<string> listValue = parseBencodeList(data, pos);
            value = "List of " + to_string(listValue.size()) + " items";
        } else if (data[pos] == 'd') {
            map<string, string> dictValue = parseBencodeDict(data, pos);
            value = "Nested dictionary";
        } else {
            value = parseBencodeString(data, pos);
        }
        
        dict[key] = value;
    }
    
    pos++;

    return dict;
}

vector<string> parseBencodeList(const string &data, size_t &pos) {
    vector<string> list;
    pos++;  

    while (data[pos] != 'e') {
        string value;
        if (data[pos] == 'i') {
            int integerValue = parseBencodeInteger(data, pos);
            value = to_string(integerValue);
        } else if (data[pos] == 'l') {
            vector<string> listValue = parseBencodeList(data, pos);
            value = "List of " + to_string(listValue.size()) + " items";
        } else if (data[pos] == 'd') {
            map<string, string> dictValue = parseBencodeDict(data, pos);
            value = "Nested dictionary";
        } else {
            value = parseBencodeString(data, pos);
        }
        
        list.push_back(value);
    }

    pos++;  
    return list;
}

string parseBencodeString(const string &data, size_t &pos) {
    size_t length = 0;

    while (isdigit(data[pos])) {
        length = length * 10 + (data[pos] - '0');
        pos++;
    }

    pos++;

    string str = data.substr(pos, length);
    pos += length;

    return str;
}

int parseBencodeInteger(const string &data, size_t &pos) {
    pos++;  
    int value = 0;
    
    while (isdigit(data[pos])) {
        value = value * 10 + (data[pos] - '0');
        pos++;
    }
    
    pos++;  
    return value;
}

void computeInfoHash(const string &data, vector<unsigned char> &infoHash) {
    infoHash.clear();
    for (char c : data) {
        infoHash.push_back(static_cast<unsigned char>(c));
    }
}

void printHex(const vector<unsigned char> &data) {
    for (unsigned char byte : data) {
        printf("%02x", byte);
    }
    cout << endl;
}

