#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <cctype>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include "../library/dynamic_lib.h"

// Shared buffer and synchronization primitives
std::string shared_buffer = "";
bool data_ready = false;
std::mutex buffer_mutex;
std::condition_variable cv;

bool keep_running = true;
const int PORT = 5001;
const char* SERVER_IP = "127.0.0.1";

// Socket file descriptor
int client_fd = -1;

// Function to safely try connecting to Program 2
bool connect_to_program2() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cerr << "[Thread 2] Ошибка создания сокета." << std::endl;
        return false;
    }

    // Set a short timeout for connection (e.g., 2 seconds) to avoid long blocking
    struct timeval tv_timeout;
    tv_timeout.tv_sec = 2;
    tv_timeout.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv_timeout, sizeof(tv_timeout));

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "[Thread 2] Неверный адрес сервера." << std::endl;
        close(client_fd);
        client_fd = -1;
        return false;
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        // Silent or brief error, we don't want to spam the console constantly
        close(client_fd);
        client_fd = -1;
        return false;
    }

    std::cout << "[Thread 2] Успешно подключено к Программе №2!" << std::endl;
    return true;
}

// Thread 1: Accepts user input and validates it
void thread1_input_handler() {
    std::string input;
    while (keep_running) {
        std::cout << "\nВведите строку (только цифры, макс. 64 символа): " << std::flush;
        if (!std::getline(std::cin, input)) {
            // EOF reached, stop running
            keep_running = false;
            cv.notify_all();
            break;
        }

        // Validate: non-empty, max 64 characters, only digits
        if (input.empty()) {
            std::cout << "[Ошибка] Строка не должна быть пустой!" << std::endl;
            continue;
        }
        if (input.length() > 64) {
            std::cout << "[Ошибка] Длина строки превышает 64 символа! (Текущая длина: " << input.length() << ")" << std::endl;
            continue;
        }
        
        bool is_only_digits = std::all_of(input.begin(), input.end(), [](char c) {
            return std::isdigit(c);
        });

        if (!is_only_digits) {
            std::cout << "[Ошибка] Строка должна содержать только цифры!" << std::endl;
            continue;
        }

        // Process with Function 1 (from dynamic library)
        function1(input);

        // Put in shared buffer and notify Thread 2
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            shared_buffer = input;
            data_ready = true;
        }
        cv.notify_one();
    }
}

// Thread 2: Processes shared buffer and sends data to Program 2
void thread2_sender_handler() {
    // Ignore SIGPIPE to prevent crash when writing to a broken socket
    #ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
    #endif

    while (keep_running) {
        std::string local_data;

        // Synchronized retrieval from shared buffer
        {
            std::unique_lock<std::mutex> lock(buffer_mutex);
            cv.wait(lock, [] { return data_ready || !keep_running; });

            if (!keep_running) break;

            local_data = shared_buffer;
            shared_buffer.clear();
            data_ready = false;
        }

        // Print retrieved data
        std::cout << "\n[Thread 2] Получены данные из буфера: " << local_data << std::endl;

        // Process with Function 2 (from dynamic library)
        int sum = function2(local_data);
        std::string sum_str = std::to_string(sum);
        std::cout << "[Thread 2] Сумма численных элементов: " << sum << " (отправка в Программу №2...)" << std::endl;

        // Attempt sending over socket with automatic connection/reconnection
        bool sent = false;
        int retries = 0;

        while (!sent && keep_running) {
            if (client_fd < 0) {
                std::cout << "[Thread 2] Нет соединения с Программой №2. Попытка подключения..." << std::endl;
                if (!connect_to_program2()) {
                    std::cout << "[Thread 2] Не удалось подключиться к Программе №2. Повтор через 2 сек..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    retries++;
                    if (retries >= 2) {
                        std::cout << "[Thread 2] Пропуск отправки из-за отсутствия соединения. Данные будут потеряны, ожидаем новый ввод." << std::endl;
                        break;
                    }
                    continue;
                }
            }

            // Send data (sum_str)
            // MSG_NOSIGNAL prevents SIGPIPE if the connection is closed
            ssize_t bytes_sent = send(client_fd, sum_str.c_str(), sum_str.length(), 0);
            if (bytes_sent < 0) {
                std::cout << "[Thread 2] Ошибка отправки. Возможно, Программа №2 была остановлена. Сброс соединения..." << std::endl;
                close(client_fd);
                client_fd = -1;
                retries++;
                if (retries >= 2) {
                    break;
                }
            } else {
                std::cout << "[Thread 2] Успешно отправлено значение \"" << sum_str << "\" в Программу №2." << std::endl;
                sent = true;
            }
        }
    }

    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
}

int main() {
    std::cout << "=== Запущена Программа №1 ===" << std::endl;
    std::cout << "Используется динамическая библиотека функций." << std::endl;

    // Start threads
    std::thread t1(thread1_input_handler);
    std::thread t2(thread2_sender_handler);

    t1.join();
    t2.join();

    std::cout << "Программа №1 завершила работу." << std::endl;
    return 0;
}
