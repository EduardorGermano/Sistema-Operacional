#include <iostream>
#include <fstream>
#include <vector>
#include "os.h"
#include "config.h"
#include "lib.h"
#include "arq-sim.h"

namespace OS {

static Arch::Terminal* terminal = nullptr;
static Arch::Cpu* cpu = nullptr;
static bool is_running = true;
static const uint16_t PROCESS_START_ADDRESS = 0x0000; // Endereço de início do processo na memória

// Função para carregar o programa na memória
bool load_program(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::vector<uint16_t> buffer;
    uint16_t value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        buffer.push_back(value);
    }

    // Verifica se o tamanho do programa cabe na memória
    if (buffer.size() > Config::memsize_words) {
        std::cerr << "Program size exceeds memory size" << std::endl;
        return false;
    }

    // Carregar o programa na memória
    for (size_t i = 0; i < buffer.size(); ++i) {
        cpu->pmem_write(PROCESS_START_ADDRESS + i, buffer[i]);
    }

    // Configura o registrador de programa (PC) para o início do processo
    cpu->set_gpr(0, PROCESS_START_ADDRESS); // Supondo que o PC seja o registrador 0

    return true;
}

void boot(Arch::Terminal* term, Arch::Cpu* processor) {
    terminal = term;
    cpu = processor;

    std::cout << "Booting Simple Monotask OS..." << std::endl;
    terminal->println(Arch::Terminal::Type::Kernel, "Kernel initialized");

    // Carregar o processo (exemplo com nome de arquivo fictício)
    if (!load_program("program.bin")) {
        std::cerr << "Failed to load program" << std::endl;
        is_running = false;
        return;
    }

    // Iniciar a execução do processo
    while (is_running) {
        cpu->run_cycle();  // Executa um ciclo de CPU
        // Processar interrupções ou syscalls se necessário
    }

    std::cout << "Shutting down..." << std::endl;
}

void interrupt(const Arch::InterruptCode interrupt) {
    switch (interrupt) {
        case Arch::InterruptCode::Keyboard:
            if (terminal->has_char) {
                int c = terminal->read_typed_char();
                if (c == 27) {  // Código ASCII para a tecla 'Esc'
                    is_running = false;
                    std::cout << "Shutdown command received." << std::endl;
                } else {
                    std::cout << "Key pressed: " << static_cast<char>(c) << std::endl;
                }
            }
            break;

        case Arch::InterruptCode::Timer:
            // Lógica do temporizador (se necessário)
            break;

        case Arch::InterruptCode::GPF:
            std::cout << "General Protection Fault - Halting CPU" << std::endl;
            is_running = false;
            cpu->turn_off();
            break;

        default:
            std::cout << "Unknown interrupt" << std::endl;
            break;
    }
}

void syscall() {
    // Implementar chamadas de sistema básicas aqui
    // Exemplo de chamada de sistema que pode ser expandida conforme necessário
    std::cout << "Syscall invoked" << std::endl;
}

} // end namespace OS
