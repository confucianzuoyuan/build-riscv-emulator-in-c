#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

void free_memory(riscv_mem *mem) {
    free(mem->mem);
}