#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "sfx.h"

#include "utils.c"

#define INITIAL_OFFSET 0x5010

void load_bank_slot_file(SlotFile* ptr) {
    FILE* f = NULL;

    f = fopen("AUDIO/CONFIG/BANKSLOT.DAT", "rb");

    fseek(f, 0, SEEK_END);

    long fileSize = ftell(f);

    fseek(f, 0, SEEK_SET);

    fread(&ptr->NumSlots, 1, sizeof(uint16_t), f);

    fread(ptr->Slots, 46*sizeof(Slot), 1, f);

    fclose(f);
}

int main(int argc, char *argv[]) {
    bool build = false;
    SlotFile bank_slot;
    char line_buf[256];

    printf("SFX Bank slot manager for GTA San Andreas - Created by DanielSant0s\n");
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--build") || !strcmp(argv[i], "-b")) {
            build = true;
            break;
        }
    }

    load_bank_slot_file(&bank_slot);

    if (build) {
        FILE* f  = fopen("slots_dump.dat", "r");

        uint32_t offset = INITIAL_OFFSET;
        int i;
        for (i = 0; i < 46; i++) {
            fgets(line_buf, 256, f);
            sscanf(line_buf, "%d\n", &bank_slot.Slots[i].BufferSize);
            bank_slot.Slots[i].BufferOffset = offset;
            offset += bank_slot.Slots[i].BufferSize;
        }

        fclose(f);

        f  = fopen("BANKSLOT.DAT", "wb");
        fwrite(&bank_slot.NumSlots, 2, 1, f);
        fwrite(bank_slot.Slots, sizeof(Slot)*46, 1, f);
        fclose(f);

        return 0;
    }

    FILE* out = fopen("slots_dump.dat", "w");
    for (int i = 0; i < 46; i++) {
        fprintf(out, "%d\n", bank_slot.Slots[i].BufferSize);
    }
    fclose(out);

    printf("Bank slots dump generated at slots_dump.dat.\n");

    return 0;
}