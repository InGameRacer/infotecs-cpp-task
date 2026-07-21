#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include "../library/dynamic_lib.h"

const int PORT = 5001;

int main() {
    std::cout << "=== Запущена Программа №2 ===" << std::endl;
    std::cout << "Используется динамическая библиотека функций." << std::endl;

    // Ignore SIGPIPE to avoid crashing if writing to a closed socket
    #ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
    #endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[Ошибка] Не удалось создать сокет сервера." << std::endl;
        return 1;
    }

    // Allow quick socket reuse to avoid "Address already in use" errors on restarts
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[Ошибка] Не удалось настроить опции сокета SO_REUSEADDR." << std::endl;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[Ошибка] Не удалось привязать сокет к порту " << PORT << "." << std::endl;
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "[Ошибка] Не удалось начать прослушивание портов." << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "Ожидание подключений от Программы №1 на порту " << PORT << "..." << std::endl;

    while (true) {
        sockaddr_in client_address;
        socklen_t addrlen = sizeof(client_address);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &addrlen);
        if (client_fd < 0) {
            std::cerr << "[Ошибка] Ошибка при принятии подключения (accept)." << std::endl;
            // Sleep slightly to avoid busy loop if accept fails continuously
            usleep(100000); 
            continue;
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, ip_str, INET_ADDRSTRLEN);
        std::cout << "\n[Подключение] Программа №1 подключилась (" << ip_str << ")." << std::endl;

        char buffer[1024] = {0};
        while (true) {
            // Read from client
            std::memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read <= 0) {
                std::cout << "[Соединение] Программа №1 отключилась или связь потеряна. Ожидание повторного подключения..." << std::endl;
                close(client_fd);
                break;
            }

            std::string received_str(buffer);
            // Strip any trailing newlines or whitespaces
            while (!received_str.empty() && (received_str.back() == '\n' || received_str.back() == '\r' || received_str.back() == ' ')) {
                received_str.pop_back();
            }

            if (received_str.empty()) {
                continue;
            }

            std::cout << "[Данные] Получено значение суммы: \"" << received_str << "\"" << std::endl;

            // Analyze received value with Function 3
            bool result = function3(received_str);
            if (result) {
                std::cout << "[УСПЕХ] Значение \"" << received_str << "\" корректно: больше 2-ух символов (или > 2) и кратно 32!" << std::endl;
            } else {
                std::cout << "[ОШИБКА] Несоответствие критериям: значение \"" << received_str << "\" не больше 2-ух или не кратно 32." << std::endl;
            }
        }
    }

    close(server_fd);
    return 0;
}
