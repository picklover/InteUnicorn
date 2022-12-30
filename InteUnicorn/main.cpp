#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <unicorn/unicorn.h>

// addr:
//      INT 21h;
//      jmp addr;
const char* cmd = "\xcd\x21\xeb\xfc";
int count = 1;

void cb(uc_engine* uc, uint32_t intno, void* user_data) {
	printf("Callback count: %d\n", count);
	count += 1;
}

int main() {
	uc_engine* uc;
	uc_err err;
	printf("Start\n");
	err = uc_open(UC_ARCH_X86, UC_MODE_16, &uc);
	if (err != UC_ERR_OK) {
		printf("Failed to open uc\n");
		return -1;
	}
	err = uc_mem_map(uc, 0x7000, 4 * 1024, UC_PROT_ALL);
	if (err != UC_ERR_OK) {
		printf("Failed to allocate memory with %d\n", err);
		return -1;
	}
	int ip = 0x100;
	int cs = 0x75a;
	int address_to_load = cs * 16 + ip;
	err = uc_mem_write(uc, address_to_load, cmd, 4);
	if (err != UC_ERR_OK) {
		printf("Failed to write memory with %d\n", err);
		return -1;
	}
	err = uc_reg_write(uc, UC_X86_REG_IP, &ip);
	if (err != UC_ERR_OK) {
		printf("Failed to write register with %d\n", err);
		return -1;
	}
	err = uc_reg_write(uc, UC_X86_REG_CS, &cs);
	if (err != UC_ERR_OK) {
		printf("Failed to write register with %d\n", err);
		return -1;
	}
	uc_hook hook;
	err = uc_hook_add(uc, &hook, UC_HOOK_INTR, (void*)cb, nullptr, 0, -1);
	if (err != UC_ERR_OK) {
		printf("Hook failed with %d\n", err);
		return -1;
	}
	printf("Before emulation.\n");
	err = uc_emu_start(uc, address_to_load, address_to_load + 4, 0, 0);
	if (err != UC_ERR_OK) {
		printf("Emulation error %d\n", err);
		return -1;
	}
	printf("After emulation.\n");
	uc_close(uc);

	return 0;
}