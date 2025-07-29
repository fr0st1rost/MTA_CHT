#pragma once
#include <string>
#include <vector>
#include <algorithm>

class Security {
public:
    static std::string RuntimeEncrypt(const std::string& input) {
        std::string output = input;
        std::vector<unsigned char> key = GenerateRuntimeKey();

        for (size_t i = 0; i < input.size(); ++i) {
            output[i] ^= key[i % key.size()] + (i % 256);
        }
        return output;
    }

    static std::string RuntimeDecrypt(const std::string& input) {
        return RuntimeEncrypt(input); // Шифрование = Дешифрование для XOR
    }

private:
    static std::vector<unsigned char> GenerateRuntimeKey() {
        // Генерация ключа на основе системных характеристик
        std::vector<unsigned char> key = {
            static_cast<unsigned char>(GetTickCount() & 0xFF),
            static_cast<unsigned char>(__rdtsc() & 0xFF),
            static_cast<unsigned char>(GetCurrentProcessId() & 0xFF)
        };

        // Добавляем случайные байты
        for (int i = 0; i < 5; i++) {
            key.push_back(rand() % 256);
        }
        return key;
    }
};

// Макросы для удобства
#define ENCRYPT_STR(str) Security::RuntimeEncrypt(str)
#define DECRYPT_STR(str) Security::RuntimeDecrypt(str)