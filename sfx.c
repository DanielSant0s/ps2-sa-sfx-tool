#include "sfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t getPCMOffset(SoundMeta *sound, BankMeta *bankInfo) {
	// BankHeaderOffset is the offset of the start of the header,
	//  so the end of the header is 4804 bytes after.
	return bankInfo->BankHeaderOffset + sizeof(BankHeader) + sound->BufferOffset;
}

uint32_t calculateBufferSize(BankHeader *bankHeader, uint32_t bankAudioSize, int soundIndex) {
	SoundMeta sound = bankHeader->Sounds[soundIndex];

	uint32_t length;
	if (soundIndex >= (int)(bankHeader->NumSounds-1)) {
		// This is the final sound in the bank, so we need to use the size of the bank
		//  to calculate the length of the sound's buffer.
		length = bankAudioSize - sound.BufferOffset;
	} else {
		// length = this sound offset - next sound offset
		// This works because the buffers are stored contiguously in the file.
		length = bankHeader->Sounds[soundIndex+1].BufferOffset - sound.BufferOffset;
	}

	return length;
}

void load_package_titles(GenericList* list) {
    FILE* f = NULL;

    f = fopen("AUDIO/CONFIG/PAKFILES.DAT", "rb");

    fseek(f, 0, SEEK_END);

    long fileSize = ftell(f);

    fseek(f, 0, SEEK_SET);

    list->data = malloc(fileSize);

    fread(list->data, fileSize, 1, f);

    list->size = fileSize/sizeof(PackageTitle);

    fclose(f);
}

void load_bank_lookup(GenericList* list) {
    FILE* f = NULL;

    f = fopen("AUDIO/CONFIG/BANKLKUP.DAT", "rb");

    fseek(f, 0, SEEK_END);

    long fileSize = ftell(f);

    fseek(f, 0, SEEK_SET);

    list->data = malloc(fileSize);

    fread(list->data, fileSize, 1, f);

    list->size = fileSize/sizeof(BankMeta);

    fclose(f);
}

void unload_generic_list(GenericList* list) {
    free(list->data);
}

void unload_generic_list_nested(GenericList* list, int levels) {
    if (levels > 0) {
        int nextLevels = levels - 1;
        GenericList* subList = (GenericList*)list->data;

        for (uint32_t i = 0; i < list->size; ++i) {
            unload_generic_list_nested(&subList[i], nextLevels);
        }
    }

    free(list->data);
}

void load_bank_data(GenericList* bank_headers, GenericList* bank_data, const char* path, GenericList* curr_pack) {
    uint8_t* pack_data = NULL;
    FILE *f = fopen(path, "rb");

    fseek(f, 0, SEEK_END);

    long fileSize = ftell(f);

    fseek(f, 0, SEEK_SET);

    pack_data = malloc(fileSize);

    fread(pack_data, fileSize, 1, f);

    fclose(f);

    bank_headers->size = curr_pack->size;
    bank_data->size = curr_pack->size;

    bank_headers->data = malloc(bank_headers->size * sizeof(BankHeader));
    bank_data->data = malloc(bank_headers->size * sizeof(GenericList));

    BankMeta* tmp_meta = curr_pack->data;
    BankHeader* tmp_headers = bank_headers->data;
    GenericList* tmp_data = bank_data->data;

    for (int i = 0; i < bank_headers->size; i++) {
        memcpy(&tmp_headers[i], &pack_data[tmp_meta[i].BankHeaderOffset], sizeof(BankHeader));

        tmp_data[i].data = malloc(tmp_meta[i].BankSize);
        tmp_data[i].size = tmp_meta[i].BankSize;
        memcpy(tmp_data[i].data, &pack_data[tmp_meta[i].BankHeaderOffset + sizeof(BankHeader)], tmp_meta[i].BankSize);
    }

    free(pack_data);
}