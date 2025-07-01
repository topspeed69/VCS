#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

class HuffmanNode {
public:
    char character;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(char c, int freq, HuffmanNode* l = nullptr, HuffmanNode* r = nullptr)
        : character(c), frequency(freq), left(l), right(r) {}
    bool operator<(const HuffmanNode& other) const {
        return frequency > other.frequency;
    }
};

class HuffmanCoder {
public:
    // Compress file and return encoded string with header
    static std::string compress(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::unordered_map<char, int> charFrequency;
        for (char c : data) charFrequency[c]++;
        std::priority_queue<HuffmanNode> pq;
        for (const auto& [c, freq] : charFrequency) pq.push(HuffmanNode(c, freq));
        while (pq.size() > 1) {
            HuffmanNode* left = new HuffmanNode(pq.top()); pq.pop();
            HuffmanNode* right = new HuffmanNode(pq.top()); pq.pop();
            pq.push(HuffmanNode('\0', left->frequency + right->frequency, left, right));
        }
        HuffmanNode* root = nullptr;
        if (!charFrequency.empty()) {
            root = new HuffmanNode(pq.top()); pq.pop();
        }
        std::unordered_map<char, std::string> huffmanCodes;
        if (root) encodeData(root, "", huffmanCodes);
        // Header: number of unique chars, then (char, freq) pairs
        std::ostringstream header;
        header << (uint32_t)charFrequency.size() << " ";
        for (const auto& [c, freq] : charFrequency) header << (int)(unsigned char)c << ":" << freq << " ";
        // Encode data
        std::string bitString;
        for (char c : data) bitString += huffmanCodes[c];
        // Pad to byte boundary
        int pad = 8 - (bitString.size() % 8);
        if (pad != 8) bitString.append(pad, '0');
        header << pad << " ";
        // Convert bitString to bytes
        std::string compressedData;
        for (size_t i = 0; i < bitString.size(); i += 8) {
            std::bitset<8> byte(bitString.substr(i, 8));
            compressedData += static_cast<char>(byte.to_ulong());
        }
        std::string result = header.str() + "\n" + compressedData;
        deleteTree(root);
        return result;
    }
    // Decompress encoded string with header
    static std::string decompress(const std::string& compressedData) {
        std::istringstream in(compressedData);
        uint32_t unique;
        in >> unique;
        std::unordered_map<char, int> charFrequency;
        for (uint32_t i = 0; i < unique; ++i) {
            int c, freq;
            char colon;
            in >> c >> colon >> freq;
            charFrequency[(char)c] = freq;
        }
        int pad;
        in >> pad;
        std::string dummy;
        std::getline(in, dummy); // skip to data
        std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        // Rebuild Huffman tree
        std::priority_queue<HuffmanNode> pq;
        for (const auto& [c, freq] : charFrequency) pq.push(HuffmanNode(c, freq));
        while (pq.size() > 1) {
            HuffmanNode* left = new HuffmanNode(pq.top()); pq.pop();
            HuffmanNode* right = new HuffmanNode(pq.top()); pq.pop();
            pq.push(HuffmanNode('\0', left->frequency + right->frequency, left, right));
        }
        HuffmanNode* root = nullptr;
        if (!charFrequency.empty()) {
            root = new HuffmanNode(pq.top()); pq.pop();
        }
        // Convert bytes to bit string
        std::string bitString;
        for (unsigned char ch : bytes) {
            std::bitset<8> bits(ch);
            bitString += bits.to_string();
        }
        if (pad != 8 && pad > 0) bitString.erase(bitString.end() - pad, bitString.end());
        // Decode
        std::string result;
        if (root) {
            HuffmanNode* node = root;
            for (char bit : bitString) {
                if (bit == '0') node = node->left;
                else node = node->right;
                if (!node->left && !node->right) {
                    result += node->character;
                    node = root;
                }
            }
        }
        deleteTree(root);
        return result;
    }
private:
    static void encodeData(HuffmanNode* node, const std::string& code, std::unordered_map<char, std::string>& huffmanCodes) {
        if (!node->left && !node->right) {
            huffmanCodes[node->character] = code;
            return;
        }
        encodeData(node->left, code + "0", huffmanCodes);
        encodeData(node->right, code + "1", huffmanCodes);
    }
    static void deleteTree(HuffmanNode* node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
};