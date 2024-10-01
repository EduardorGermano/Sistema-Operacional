#include <stdexcept>
#include <string>
#include <string_view>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "config.h"
#include "lib.h"
#include "arq-sim.h"
#include "os.h"

namespace OS {

enum TaskState {
    READY,
    EXECUTING
};

class Task {
public:
    uint16_t regs[8] = {0};  // Registradores
    uint16_t reg_pc = 0;     // Program counter
    uint16_t stack = 0;
    uint16_t paddr_offset = 0;
    uint16_t paddr_max = 0;
    std::string bin_name;
    size_t bin_size = 0;
    int tid = 0;
    TaskState state = READY;
};

class OS {
public:
    Arch::Cpu* cpu;
    Arch::Memory* memory;
    Arch::Terminal* terminal;
    std::vector<Task*> tasks;
    Task* current_task = nullptr;
    Task* idle_task = nullptr;
    int next_sched_task = 0;
    int next_task_id = 0;
    size_t memory_page_size = 4096;
    size_t blocksize = 256;
    size_t number_blocks;
    std::vector<int> memory_blocks;

    OS(Arch::Cpu* cpu, Arch::Memory* memory, Arch::Terminal* terminal)
        : cpu(cpu), memory(memory), terminal(terminal) {
        number_blocks = memory->get_size() / memory_page_size;
        memory_blocks.resize(number_blocks, 0);
        idle_task = load_task("idle.bin");
        if (!idle_task) {
            panic("Could not load idle.bin task");
        }
        sched(idle_task);
        terminal->println(Arch::Terminal::Type::Command, "This is the console, type the commands here");
    }

    Task* load_task(const std::string& bin_name) {
        if (!file_exists(bin_name)) {
            printk("File " + bin_name + " does not exist");
            return nullptr;
        }

        Task* task = new Task();
        task->bin_name = bin_name;
        task->bin_size = file_size(bin_name) / 2;  // Tamanho em palavras

        task->paddr_offset, task->paddr_max = allocate_contiguous_physical_memory_to_task(task->bin_size);
        if (task->paddr_offset == -1) {
            return nullptr;
        }

        task->regs[0] = 0;
        task->reg_pc = 1;
        task->state = READY;
        read_binary_to_memory(task->paddr_offset, task->paddr_max, bin_name);
        printk("Task " + bin_name + " successfully loaded");

        task->tid = next_task_id++;
        return task;
    }

    void sched(Task* task) {
        if (current_task != nullptr) {
            panic("current_task must be nullptr when scheduling a new one");
        }
        if (task->state != READY) {
            panic("Task " + task->bin_name + " must be in READY state to be scheduled");
        }

        for (int i = 0; i < 8; ++i) {
            cpu->set_reg(i, task->regs[i]);
        }

        cpu->set_pc(task->reg_pc);
        task->state = EXECUTING;
        current_task = task;
        printk("Scheduling task " + task->bin_name);
    }

    void un_sched(Task* task) {
        if (task->state != EXECUTING) {
            panic("Task must be in EXECUTING state to be unscheduled");
        }

        for (int i = 0; i < 8; ++i) {
            task->regs[i] = cpu->get_reg(i);
        }

        task->reg_pc = cpu->get_pc();
        task->state = READY;
        current_task = nullptr;
        printk("Unscheduling task " + task->bin_name);
    }

    void printk(const std::string& msg) {
        terminal->println(Arch::Terminal::Type::Kernel, "kernel: " + msg);
    }

    void panic(const std::string& msg) {
        terminal->println(Arch::Terminal::Type::Kernel, "Kernel panic: " + msg);
        cpu->halt();
    }

    void read_binary_to_memory(uint16_t paddr_offset, uint16_t paddr_max, const std::string& bin_name) {
        // Código para carregar binário na memória
    }

    std::pair<int, int> allocate_contiguous_physical_memory_to_task(size_t words) {
        // Lógica para alocar memória física
        return {0, 0};  // Substituir por lógica real
    }

    void syscall() {
        // Implementar chamadas de sistema
    }

    void interrupt(const Arch::InterruptCode interrupt) {
        // Implementar tratamento de interrupções
    }

private:
    bool file_exists(const std::string& bin_name) {
        // Verifica se o arquivo existe
        return true;
    }

    size_t file_size(const std::string& bin_name) {
        // Retorna o tamanho do arquivo
        return 1024;
    }
};

}  // end namespace OS
