#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <endian.h>

#include "memory.h"

bool init_mem(riscv_mem *mem, const char *filename) {
    if (!filename) {
        LOG_ERROR("需要一个二进制文件！\n");
        return false;
    }

    // 以二进制的方式打开文件
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        LOG_ERROR("无效的二进制文件路径！\n");
        return false;
    }

    // fp指针指向文件的开头，偏移量为0，一直移动到文件末尾。
    fseek(fp, 0, SEEK_END);
    // 二进制文件一共多少个字节
    size_t sz = ftell(fp) * sizeof(uint8_t);
    // 将文件指针重新指向文件开头
    rewind(fp);

    mem->mem = malloc(sz);
    if (!mem->mem) {
        LOG_ERROR("没有足够的内存！\n");
        fclose(fp);
        return false;
    }

    size_t read_size = fread(mem->mem, sizeof(uint8_t), sz, fp);

    if (read_size != sz) {
        LOG_ERROR("使用fread读取二进制文件出现错误！\n");
        fclose(fp);
        return false;
    }
    fclose(fp);
    mem->code_size = read_size;
    return true;
}

// 如果本机是小端序，那么直接读取就行了
// 例如bit是16，那么下面将翻译为*(uint16_t *) (ptr)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define read_len(bit, ptr, value) value = *(uint##bit##_t *) (ptr)
#else
#define read_len(bit, ptr, value) \
    value = __builtin_bswap##bit(*(uint##bit##_t *) (prt))
#endif

uint64_t read_mem(riscv_mem *mem, uint64_t addr, uint64_t size) {
    uint64_t index = (addr - DRAM_BASE);
    uint64_t value;

    switch (size) {
    case 8: // 8 个 bit
        value = mem->mem[index];
        break;
    case 16: // 16 个 bit
        read_len(16, &mem->mem[index], value);
        break;
    case 32:
        read_len(32, &mem->mem[index], value);
        break;
    case 64:
        read_len(64, &mem->mem[index], value);
        break;
    default:
        LOG_ERROR("无效的内存大小！\n");
        return -1;
    }
    return value;
}

// 在我们的模拟器中，内存是小端序，
// 所以如果我们的宿主机器的架构也是小端序的，
// 可以直接将内存强制类型转换到目标指针类型。
// 如果我们的架构是大端序的话
// 需要先将小端序调整为大端序
// 注意：我不确定内存的索引会不会溢出，如果溢出，那么下面的实现会报错。

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define write_len(bit, ptr, value) \
    *(uint##bit##_t *) (ptr) = (uint##bit##_t)(value)
#else
#define write_len(bit, ptr, value) \
    *(uint##bit##_t *) (ptr) = (uint##bit##_t) __builtin_bswap##bit((value))
#endif

void write_mem(riscv_mem *mem, uint64_t addr, uint64_t value, uint8_t size) {
    uint64_t index = (addr - DRAM_BASE);

    switch (size) {
    case 8:
        mem->mem[index] = (uint8_t) value;
        break;
    case 16:
        write_len(16, &mem->mem[index], value);
        break;
    case 32:
        write_len(32, &mem->mem[index], value);
        break;
    case 64:
        write_len(64, &mem->mem[index], value);
        break;
    default:
        LOG_ERROR("无效的内存大小！\n");
    }
}

void free_memory(riscv_mem *mem) {
    free(mem->mem);
}