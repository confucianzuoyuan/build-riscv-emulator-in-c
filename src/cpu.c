#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

bool init_cpu(riscv_cpu *cpu, const char *filename) {
    if (!init_bus(&cpu->bus, filename)) return false;

    memset(&cpu->xreg[0], 0, sizeof(uint64_t) * 32);

    cpu->pc = DRAM_BASE;
    cpu->xreg[2] = DRAM_BASE + DRAM_SIZE; // 1GiB
    return true;
}

// 从内存中取出一条指令
// risc-v指令的大小是4个字节，32-bit
// 由于二进制文件是以小端序存储的
// 所以需要转换成大端序
uint32_t fetch(riscv_cpu *cpu) {
    return read_bus(&cpu->bus, cpu->pc, 32);
}

void exec(riscv_cpu *cpu, uint32_t inst) {
    // risc-v的指令占32-bit的内存大小，也就是4个字节。
    // opcode占索引为0～6的bit，也就是7个bit，所以下面的语句是取出opcode
    uint8_t opcode = inst & 0x7f;
    // 向右移位7个bit，然后按位与，可以取出7～11的bit，也就是5个bit
    uint8_t rd = (inst >> 7) & 0x1f;
    // 向右移位15个bit，然后按位与，可以取出15～19的bit，也就是5个bit
    uint8_t rs1 = ((inst >> 15) & 0x1f);
    // 向右移位20个bit，然后按位与，可以取出20～24的bit，也就是5个bit
    uint8_t rs2 = ((inst >> 20) & 0x1f);

    // 取出高位的12个bit，就是立即数
    uint64_t imm = asr_i64((int) (inst & 0xfff00000), 20);

    // 对于unsigned整型溢出，C的规范是有定义的——“溢出后的数会以2^(8*sizeof(type))作模运算”，
    // 也就是说，如果一个unsigned char（1字符，8bits）溢出了，会把溢出的值与256求模。
    // #include <stdio.h>
    // int main(int argc, const char * argv[]) {
    //   // insert code here...
    //   unsigned char x;
    //   x = 128 + 130;
    //   printf("%d\n",x);  
    // }
    // 上面代码会输出：2，258以256为模的结果值是2。
    switch (opcode) {
        //addi
        case 0x13:
            cpu->xreg[rd] = cpu->xreg[rd] + imm;
            break;
        case 0x33:
            cpu->xreg[rd] = cpu->xreg[rs1] + cpu->xreg[rs2];
            break;
        default:
            LOG_ERROR("未实现或无效的操作码：0x%x\n", opcode);
            exit(1);
    }
}

void dump_reg(riscv_cpu *cpu) {
    for (size_t i = 0; i < 32; i++) {
        printf("x%ld = 0x%lx, ", i, cpu->xreg[i]);
        if (!((i + 1) & 3)) printf("\n");
    }
    printf("\n");
}

void free_cpu(riscv_cpu *cpu) {
    free_bus(&cpu->bus);
}