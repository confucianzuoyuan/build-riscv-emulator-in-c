#include <stdlib.h>

#include "emu.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        LOG_ERROR("没有二进制文件！\n");
        return -1;
    }

    riscv_emu *emu = malloc(sizeof(riscv_emu));

    if (!init_emu(emu, argv[1])) {
        exit(1);
    }

    // 取出一条指令，执行一条指令
    uint64_t start_pc = emu->cpu.pc;
    while (emu->cpu.pc < start_pc + emu->cpu.bus.memory.code_size) {
        // 取出指令
        fetch(&emu->cpu);
        // 将pc指向将要执行的下一条指令
        emu->cpu.pc += 4;
        // 解码将要执行的指令
        bool ret = decode(&emu->cpu);
        if (ret == false)
            break;
        // 执行指令
        exec(&emu->cpu);
    }

    dump_reg(&emu->cpu);

    close_emu(emu);

    return 0;
}