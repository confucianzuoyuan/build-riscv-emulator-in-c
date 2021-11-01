#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define asr(value, amount) (value < 0 ? ~(~value >> amount) : value >> amount)

static void instr_lb(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 8);
    cpu->xreg[cpu->instr.rd] = ((int8_t)(value));
}

static void instr_lh(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 16);
    cpu->xreg[cpu->instr.rd] = ((int16_t)(value));
}

static void instr_lw(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 32);
    cpu->xreg[cpu->instr.rd] = ((int32_t)(value));
}

static void instr_ld(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 64);
    cpu->xreg[cpu->instr.rd] = value;
}

static void instr_lbu(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 16);
    cpu->xreg[cpu->instr.rd] = value;
}

static void instr_lhu(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 32);
    cpu->xreg[cpu->instr.rd] = value;
}

static void instr_lwu(riscv_cpu *cpu) {
    uint64_t addr = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
    uint64_t value = read_bus(&cpu->bus, addr, 64);
    cpu->xreg[cpu->instr.rd] = value;
}

static void instr_addi(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] + cpu->instr.imm;
}

static void instr_slli(riscv_cpu *cpu) {
    // shift amount is the lower 6 bits of immediate
    uint32_t shamt = (cpu->instr.imm & 0x3f);
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] << shamt;
}

static void instr_slti(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = ((int64_t) cpu->xreg[cpu->instr.rs1] < (int64_t) cpu->instr.imm) ? 1 : 0;
}

static void instr_sltiu(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = (cpu->xreg[cpu->instr.rs1] < cpu->instr.imm) ? 1 : 0;
}

static void instr_xori(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] ^ cpu->instr.imm;
}

static void instr_srli(riscv_cpu *cpu) {
    // shift amount is the lower 6 bits of immediate
    uint32_t shamt = (cpu->instr.imm & 0x3f);
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] >> shamt;
}

static void instr_srai(riscv_cpu *cpu) {
    // shift amount is the lower 6 bits of immediate
    uint32_t shamt = (cpu->instr.imm & 0x3f);
    cpu->xreg[cpu->instr.rd] = asr((int64_t)(cpu->xreg[cpu->instr.rs1]), shamt);
}

static void instr_ori(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] | cpu->instr.imm;
}

static void instr_andi(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] & cpu->instr.imm;
}

static void instr_add(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] + cpu->xreg[cpu->instr.rs2];
}

static void instr_mul(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] * cpu->xreg[cpu->instr.rs2];
}

static void instr_sub(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] - cpu->xreg[cpu->instr.rs2];
}

static void instr_sll(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] << cpu->xreg[cpu->instr.rs2];
}

static void instr_slt(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = ((int64_t) cpu->xreg[cpu->instr.rs1] < (int64_t) cpu->xreg[cpu->instr.rs2]) ? 1 : 0;
}

static void instr_sltu(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = (cpu->xreg[cpu->instr.rs1] < cpu->xreg[cpu->instr.rs2]) ? 1 : 0;
}

static void instr_xor(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] ^ cpu->xreg[cpu->instr.rs2];
}

static void instr_sra(riscv_cpu *cpu) {
    // shift amount is the low 6 bits of rs2
    uint32_t shamt = (cpu->xreg[cpu->instr.rs2] & 0x3f);
    cpu->xreg[cpu->instr.rd] = asr((int64_t) cpu->xreg[cpu->instr.rs1], shamt);
}

static void instr_or(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] | cpu->xreg[cpu->instr.rs2];
}

static void instr_and(riscv_cpu *cpu) {
    cpu->xreg[cpu->instr.rd] = cpu->xreg[cpu->instr.rs1] & cpu->xreg[cpu->instr.rs2];
}

static riscv_instr_entry instr_load_type[] = {
    [0x0] = {instr_lb, NULL},  
    [0x1] = {instr_lh, NULL},
    [0x2] = {instr_lw, NULL},  
    [0x3] = {instr_ld, NULL},
    [0x4] = {instr_lbu, NULL}, 
    [0x5] = {instr_lhu, NULL},
    [0x6] = {instr_lwu, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC3, instr_load_type);

static riscv_instr_entry instr_srli_srai_type[] = {
    [0x0] = {instr_srli, NULL},
    [0x20] = {instr_srai, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_srli_srai_type);

static riscv_instr_entry instr_imm_type[] = {
    [0x0] = {instr_addi, NULL},
    [0x1] = {instr_slli, NULL},
    [0x2] = {instr_slti, NULL},
    [0x3] = {instr_sltiu, NULL},
    [0x4] = {instr_xori, NULL},
    [0x5] = {NULL, &instr_srli_srai_type_list},
    [0x6] = {instr_ori, NULL},
    [0x7] = {instr_andi, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC3, instr_imm_type);

static riscv_instr_entry instr_add_mul_sub_type[] = {
    [0x00] = {instr_add, NULL},
    [0x01] = {instr_mul, NULL},
    [0x20] = {instr_sub, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_add_mul_sub_type);

static riscv_instr_entry instr_sll_type[] = {
    [0x00] = {instr_sll, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_sll_type);

static riscv_instr_entry instr_slt_type[] = {
    [0x00] = {instr_slt, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_slt_type);

static riscv_instr_entry instr_sltu_type[] = {
    [0x00] = {instr_sltu, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_sltu_type);

static riscv_instr_entry instr_xor_type[] = {
    [0x00] = {instr_xor, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_xor_type);

static riscv_instr_entry instr_sra_type[] = {
    [0x20] = {instr_sra, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_sra_type);

static riscv_instr_entry instr_or_type[] = {
    [0x00] = {instr_or, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_or_type);

static riscv_instr_entry instr_and_type[] = {
    [0x00] = {instr_and, NULL}
};
INIT_RISCV_INSTR_LIST(FUNC7, instr_and_type);

static riscv_instr_entry instr_reg_type[] = {
    [0x0] = {NULL, &instr_add_mul_sub_type_list},
    [0x1] = {NULL, &instr_sll_type_list},
    [0x2] = {NULL, &instr_slt_type_list},
    [0x3] = {NULL, &instr_sltu_type_list},
    [0x4] = {NULL, &instr_xor_type_list},
    [0x5] = {NULL, &instr_sra_type_list},
    [0x6] = {NULL, &instr_or_type_list},
    [0x7] = {NULL, &instr_and_type_list}
};
INIT_RISCV_INSTR_LIST(FUNC3, instr_reg_type);

static riscv_instr_entry opcode_type[] = {
    [0x03] = {NULL, &instr_load_type_list},
    [0x13] = {NULL, &instr_imm_type_list},
    [0x33] = {NULL, &instr_reg_type_list}
};
INIT_RISCV_INSTR_LIST(OPCODE, opcode_type);

static bool __decode(riscv_cpu *cpu, riscv_instr_desc *instr_desc)
{
    uint8_t index;

    switch (instr_desc->type.type) {
    case OPCODE:
        index = cpu->instr.opcode;
        break;
    case FUNC3:
        index = cpu->instr.funct3;
        break;
    case FUNC7:
        index = cpu->instr.funct3;
        break;
    default:
        LOG_ERROR("Invalid index type\n");
        return false;
    }

    if (index > instr_desc->size) {
        LOG_ERROR("未实现或无效的指令 0x%x\n",
                  cpu->instr.instr);
        return false;
    }

    riscv_instr_entry entry = instr_desc->instr_list[index];
    if (entry.exec_func == NULL && entry.next == NULL) {
        LOG_ERROR("未实现或无效的指令类型 0x%x\n",
                  cpu->instr.instr);
        return false;
    }

    if (entry.next != NULL)
        __decode(cpu, entry.next);
    else
        cpu->exec_func = entry.exec_func;

    return true;
}

bool init_cpu(riscv_cpu *cpu, const char *filename) {
    if (!init_bus(&cpu->bus, filename))
        return false;
    memset(&cpu->instr, 0, sizeof(riscv_instr));
    memset(&cpu->xreg[0], 0, sizeof(uint64_t) * 32);
    cpu->pc = DRAM_BASE;
    cpu->xreg[2] = DRAM_BASE + DRAM_SIZE;
    cpu->exec_func = NULL;
    return true;
}

// 取指令
void fetch(riscv_cpu *cpu) {
    uint32_t instr = read_bus(&cpu->bus, cpu->pc, 32);
    cpu->instr.instr = instr;
    cpu->instr.opcode = instr & 0x7f;
    cpu->instr.rd = (instr >> 7) & 0x1f;
    cpu->instr.rs1 = ((instr >> 15) & 0x1f);
    cpu->instr.rs2 = ((instr >> 20) & 0x1f);
    cpu->instr.imm = asr((int32_t)(instr & 0xfff00000), 20);
    cpu->instr.funct3 = (instr >> 12) & 0x7;
    cpu->instr.funct7 = (instr >> 25) & 0x7f;
}

bool decode(riscv_cpu *cpu) {
    return __decode(cpu, &opcode_type_list);
}

void exec(riscv_cpu *cpu) {
    // x0寄存器永远为0
    cpu->xreg[0] = 0;
    cpu->exec_func(cpu);

    memset(&cpu->instr, 0, sizeof(riscv_instr));
    cpu->exec_func = NULL;
}

void dump_reg(riscv_cpu *cpu)
{
    for (size_t i = 0; i < 32; i++) {
        printf("x%ld = 0x%lx, ", i, cpu->xreg[i]);
        if (!((i + 1) & 3))
            printf("\n");
    }
    printf("\n");
}

void free_cpu(riscv_cpu *cpu)
{
    free_bus(&cpu->bus);
}