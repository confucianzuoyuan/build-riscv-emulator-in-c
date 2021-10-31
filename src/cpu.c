#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define asr(value, amount) (value < 0 ? ~(~value >> amount) : value >> amount)

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

static bool inst_ld(riscv_cpu *cpu, uint32_t inst) {
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t rs1 = ((inst >> 15) & 0x1f);
    uint8_t funct3 = (inst >> 12) & 0x7;

    uint64_t imm = asr((int32_t)(inst & 0xfff00000), 20);
    uint64_t addr = cpu->xreg[rs1] + imm;

    switch (funct3) {
    // lb
    case 0: {
        uint64_t value = read_bus(&cpu->bus, addr, 8);
        cpu->xreg[rd] = ((int8_t)(value));
    } break;
    // lh
    case 1: {
        uint64_t value = read_bus(&cpu->bus, addr, 16);
        cpu->xreg[rd] = ((int16_t)(value));
    } break;
    // lw
    case 2: {
        uint64_t value = read_bus(&cpu->bus, addr, 32);
        cpu->xreg[rd] = ((int32_t)(value));
    } break;
    // ld
    case 3: {
        uint64_t value = read_bus(&cpu->bus, addr, 64);
        cpu->xreg[rd] = ((int64_t)(value));
    } break;
    // lbu
    case 4: {
        uint64_t value = read_bus(&cpu->bus, addr, 8);
        cpu->xreg[rd] = value;
    } break;
    // lhu
    case 5: {
        uint64_t value = read_bus(&cpu->bus, addr, 16);
        cpu->xreg[rd] = value;
    } break;
    case 6: {
        uint64_t value = read_bus(&cpu->bus, addr, 32);
        cpu->xreg[rd] = value;
    } break;
    default:
        LOG_ERROR("对于操作码 0x3 来说，funct3 0x%x 未知。\n", funct3);
        return false;
    }

    return true;
}

static bool inst_imm(riscv_cpu *cpu, uint32_t inst) {
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t rs1 = ((inst >> 15) & 0x1f);
    uint8_t funct3 = (inst >> 12) & 0x7;
    uint8_t funct7 = (inst >> 25) & 0x7f;

    // 立即数是指令的高12位
    uint64_t imm = asr((int32_t)(inst & 0xfff00000), 20);
    // shift amount 移动位数是立即数的低六位。
    uint32_t shamt = (imm & 0x3f);

    switch (funct3) {
    // addi
    case 0x0:
        cpu->xreg[rd] = cpu->xreg[rs1] + imm;
        break;
    // slli
    case 0x1:
        cpu->xreg[rd] = cpu->xreg[rs1] << shamt;
        break;
    // slti
    case 0x2:
        cpu->xreg[rd] = ((int64_t) cpu->xreg[rs1] < (int64_t) imm) ? 1 : 0;
        break;
    // sltiu
    case 0x3:
        cpu->xreg[rd] = (cpu->xreg[rs1] < imm) ? 1 : 0;
        break;
    // xori
    case 0x4:
        cpu->xreg[rd] = cpu->xreg[rs1] ^ imm;
        break;
    // sr..
    case 0x5: {
        switch (funct7) {
        // srli
        case 0x0:
            cpu->xreg[rd] = cpu->xreg[rs1] >> shamt;
            break;
        // srai
        case 0x20:
            cpu->xreg[rd] = asr((int64_t)(cpu->xreg[rs1]), shamt);
            break;
        default:
            LOG_ERROR("对于操作码0x13，funct7 0x%x 未知。\n", funct7);
            return false;
        }
    } break;
    // ori
    case 0x6:
        cpu->xreg[rd] = cpu->xreg[rs1] | imm;
        break;
    default:
        LOG_ERROR("对于操作码0x13，funct3 0x%x 未知。\n", funct3);
        return false;
    }

    return true;
}

static bool inst_reg(riscv_cpu *cpu, uint32_t inst) {
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t rs1 = ((inst >> 15) & 0x1f);
    uint8_t rs2 = ((inst >> 20) & 0x1f);
    uint8_t funct3 = (inst >> 12) & 0x7;
    uint8_t funct7 = (inst >> 25) & 0x7f;

    // shift amount is the low 6 bits of rs2
    uint32_t shamt = (cpu->xreg[rs2] & 0x3f);
    uint16_t funct = (uint16_t) funct3 << 8 | funct7;

    switch (funct) {
    // add
    case 0x0000:
        cpu->xreg[rd] = cpu->xreg[rs1] + cpu->xreg[rs2];
        break;
    // mul
    case 0x0001:
        cpu->xreg[rd] = cpu->xreg[rs1] * cpu->xreg[rs2];
        break;
    // sub
    case 0x0020:
        cpu->xreg[rd] = cpu->xreg[rs1] - cpu->xreg[rs2];
        break;
    // sll
    case 0x0100:
        cpu->xreg[rd] = cpu->xreg[rs1] << cpu->xreg[rs2];
        break;
    // slt
    case 0x0200:
        cpu->xreg[rd] = ((int64_t) cpu->xreg[rs1] < (int64_t) cpu->xreg[rs2]) ? 1 : 0;
        break;
    // sltu
    case 0x0300:
        cpu->xreg[rd] = (cpu->xreg[rs1] < cpu->xreg[rs2]) ? 1 : 0;
        break;
    // xor
    case 0x0400:
        cpu->xreg[rd] = cpu->xreg[rs1] ^ cpu->xreg[rs2];
        break;
    // sra
    case 0x0520:
        cpu->xreg[rd] = asr((int64_t) cpu->xreg[rs1], shamt);
        break;
    // or
    case 0x0600:
        cpu->xreg[rd] = cpu->xreg[rs1] | cpu->xreg[rs2];
        break;
    // and
    case 0x0700:
        cpu->xreg[rd] = cpu->xreg[rs1] & cpu->xreg[rs2];
        break;
    default:
        LOG_ERROR("对于操作码 0x33，funct3 0x%x 或者 funct7 0x%x 未知。\n", funct3, funct7);
        return false;
    }
    return true;
}

bool exec(riscv_cpu *cpu, uint32_t inst) {
    // risc-v的指令占32-bit的内存大小，也就是4个字节。
    // opcode占索引为0～6的bit，也就是7个bit，所以下面的语句是取出opcode
    uint8_t opcode = inst & 0x7f;

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
    // load
    case 0x3:
        return inst_ld(cpu, inst);
    // imm
    case 0x13:
        return inst_imm(cpu, inst);
    // reg
    case 0x33:
        return inst_reg(cpu, inst);
    default:
        LOG_ERROR("未实现的或者无效的操作码 0x%x\n", opcode);
        return false;
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