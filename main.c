#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>

#include "sfx.h"

#include "utils.c"


char cwd[1024];

static const char* SFX_PATH = "AUDIO/SFX/";

const uint8_t padding[sizeof(BankHeader)] = { 0 };

void read_vag_header(uint8_t *header, uint32_t *fsize, uint32_t *rate) {
    // Check magic
    if (header[0x00] != 'V' || header[0x01] != 'A' || header[0x02] != 'G' || header[0x03] != 'p') {
        return;
    }

    // Length of data for each channel (big-endian)
    *fsize = ((uint32_t)header[0x0c] << 24) | ((uint32_t)header[0x0d] << 16) | ((uint32_t)header[0x0e] << 8) | header[0x0f];

    // Sample rate (big-endian)
    *rate = ((uint32_t)header[0x10] << 24) | ((uint32_t)header[0x11] << 16) | ((uint32_t)header[0x12] << 8) | header[0x13];
}

void write_vag_header(uint32_t fsize, uint8_t *header, uint32_t rate) {
	// Magic
	header[0x00] = 'V';
	header[0x01] = 'A';
	header[0x02] = 'G';
	header[0x03] = 'p';

	// Version (big-endian)
	header[0x04] = 0x00;
	header[0x05] = 0x00;
	header[0x06] = 0x00;
	header[0x07] = 0x03;

	// Interleave (little-endian)
	header[0x08] = (uint8_t)0;
	header[0x09] = (uint8_t)0;
	header[0x0a] = (uint8_t)0;
	header[0x0b] = (uint8_t)0;

	// Length of data for each channel (big-endian)
	header[0x0c] = (uint8_t)(fsize>>24);
	header[0x0d] = (uint8_t)(fsize>>16);
	header[0x0e] = (uint8_t)(fsize>>8);
	header[0x0f] = (uint8_t)fsize;

	// Sample rate (big-endian)
	header[0x10] = (uint8_t)(rate>>24);
	header[0x11] = (uint8_t)(rate>>16);
	header[0x12] = (uint8_t)(rate>>8);
	header[0x13] = (uint8_t)rate;

	// Number of channels (little-endian)
	header[0x1e] = (uint8_t)0x00;
	header[0x1f] = 0x00;

	// Filename
	//strncpy(header + 0x20, "psxavenc", 16);
	memset(header + 0x20, 0, 16);
}


int main(int argc, char *argv[]) {
    GenericList pak_titles = { NULL, 0 };
    GenericList bank_meta = { NULL, 0 };

    load_package_titles(&pak_titles);
    load_bank_lookup(&bank_meta);

    char path[512] = { 0 };
    SfxAppModes mode = UNPACK;
    char* fname = NULL;
    int8_t sfxPackIndex = -1;
    uint32_t packStartIndex = -1;

    printf("SFX Tool for GTA San Andreas - Created by DanielSant0s\n");
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--pack") || !strcmp(argv[i], "-p")) {
            mode = PACK;
            if (argc == i+1) {
                printf("ERROR: You are supposed to put a file name in order to build it!\n");
                return 0;
            } else {
                fname = argv[i+1];
            }
            break;
        } else if (!strcmp(argv[i], "--wav") || !strcmp(argv[i], "-w")) {
            mode = WAV;
            if (argc == i+1) {
                printf("ERROR: You are supposed to put a file name in order to build it!\n");
                return 0;
            } else {
                fname = argv[i+1];
            }
            break;
        } else if (!strcmp(argv[i], "--vag") || !strcmp(argv[i], "-v")) {
            mode = RAW;
            if (argc == i+1) {
                printf("ERROR: You are supposed to put a file name in order to build it!\n");
                return 0;
            } else {
                fname = argv[i+1];
            }
            break;
        } else {
            fname = argv[i];
            break;
        }
    }

    strcat(path, SFX_PATH);
    strcat(path, fname);

    if (!fexists(fname) && !fexists(path) ) {
        printf("ERROR: The specified file doesn't exist!\n");
        return 0;
    }

    PackageTitle* tmp_titles = pak_titles.data;
    for (i = 0; i < pak_titles.size; i++) {
        if (!strncmp(tmp_titles[i], fname, strlen(tmp_titles[i]))) {
            printf("%s detected\n", tmp_titles[i]);
            sfxPackIndex = i;
            break;
        }
    }

    if (sfxPackIndex == -1) {
        printf("ERROR: Sound pack not found on game data!\n");
        return 0;
    }

    GenericList pak_meta = { NULL, 0 };

    BankMeta* tmp_meta = bank_meta.data;
    for (i = 0; i < bank_meta.size; i++) {
        if ((sfxPackIndex == tmp_meta[i].PackageIndex)) {
            if (!pak_meta.data) {
                pak_meta.data = &tmp_meta[i];
                packStartIndex = i;
            }

            pak_meta.size++;
        }
    }

    GenericList bank_headers = { NULL, 0 };
    GenericList bank_data = { NULL, 0 };

    load_bank_data(&bank_headers, &bank_data, fname, &pak_meta);

    printf("%s %s, please wait!\n", mode? "Packing" : "Unpacking", fname);

    switch (mode)
    {
    case PACK:
        {
            char bank_path[512] = { 0 };
            char line_buf[256] = { 0 };
            BankHeader* headers = bank_headers.data;
            GenericList* adpcm_data = bank_data.data;
            long backup_pos = 0;
            long header_pos = 0;
            long curr_offset = 0;

            memset(path, 0, 512);

            sprintf(path, "%s_extracted", tmp_titles[sfxPackIndex]);

            FILE* fpack = fopen(fname, "wb");
            FILE* fbankmeta = fopen("BANKLKUP.DAT", "wb");

            BankMeta* tmp_meta = pak_meta.data;

            for (i = 0; i < bank_headers.size; i++) {
                sprintf(bank_path, "%s/bank_%03d/sfx_meta.dat", path, i+1);
                FILE* fmeta = fopen(bank_path, "r");

                header_pos = ftell(fpack);

                fseek(fpack, sizeof(BankHeader), SEEK_CUR);

                curr_offset = 0;

                int j;
                for (j = 0; j < headers[i].NumSounds; j++) {
                    sprintf(bank_path, "%s/bank_%03d/sound_%03d.vag", path, i+1, j+1);

                    FILE* f = fopen(bank_path, "rb");

                    uint32_t fileSize = 0, rate = 0;

                    char sound_header[48] = { 0 };
                    fread(&sound_header, 48, 1, f);
                    read_vag_header(&sound_header, &fileSize, &rate);

                    void* sfx_data = malloc(fileSize);
                    fread(sfx_data, fileSize, 1, f);
                    fclose(f);

                    fwrite(sfx_data, fileSize, 1, fpack);

                    free(sfx_data);

                    headers[i].Sounds[j].BufferOffset = curr_offset;

                    curr_offset += fileSize;

                    fgets(line_buf, 256, fmeta);
                    
                    sscanf(line_buf, "%d %hd %hd\n", &headers[i].Sounds[j].LoopOffset, &headers[i].Sounds[j].SampleRate, &headers[i].Sounds[j].Headroom);
                    if (rate != headers[i].Sounds[j].SampleRate) {
                        headers[i].Sounds[j].SampleRate = rate;
                        printf("New sample rate: %d, changing...\n", rate);
                    }

                }

                fclose(fmeta);

                tmp_meta[i].BankHeaderOffset = header_pos;
                tmp_meta[i].BankSize = curr_offset;

                backup_pos = ftell(fpack);
                fseek(fpack, header_pos, SEEK_SET);
                fwrite(&headers[i], sizeof(BankHeader), 1, fpack);
                fseek(fpack, backup_pos, SEEK_SET);
            }

            fwrite(bank_meta.data, bank_meta.size*sizeof(BankMeta), 1, fbankmeta);

            fclose(fpack);
            fclose(fbankmeta);
        }
        break;
    
    case UNPACK:
        {
            char bank_path[512] = { 0 };
            uint8_t* sound_ptr = NULL;

            memset(path, 0, 512);

            sprintf(path, "%s_extracted", tmp_titles[sfxPackIndex]);
            create_folder(path);

            BankHeader* headers = bank_headers.data;
            GenericList* adpcm_data = bank_data.data;

            for (i = 0; i < bank_headers.size; i++) {
                sprintf(bank_path, "%s/bank_%03d", path, i+1);
                create_folder(bank_path);

                sprintf(bank_path, "%s/sfx_meta.dat", bank_path);
                FILE* fmeta = fopen(bank_path, "w");

                int j;
                for (j = 0; j < headers[i].NumSounds; j++) {
                    sound_ptr = adpcm_data[i].data + headers[i].Sounds[j].BufferOffset;

                    sprintf(bank_path, "%s/bank_%03d/sound_%03d.vag", path, i+1, j+1);
                    FILE* f = fopen(bank_path, "wb");
                    uint32_t fsize = calculateBufferSize(&headers[i], adpcm_data[i].size, j);
                    uint8_t header[48] = { 0 };
                    write_vag_header(fsize, &header, headers[i].Sounds[j].SampleRate);
                    fwrite(&header, 48, 1, f);
                    fwrite(sound_ptr, fsize, 1, f);
                    fclose(f);

                    fprintf(fmeta, "%d %d %d\n", headers[i].Sounds[j].LoopOffset, headers[i].Sounds[j].SampleRate, headers[i].Sounds[j].Headroom);

                }

                fclose(fmeta);
            }
        }
        break;

    case WAV:
        {
            char bank_path[512] = { 0 };
            char wav_path[512] = { 0 };
            char call_path[1024] = { 0 };
            char ext_path[64] = { 0 };
            uint8_t* sound_ptr = NULL;

            memset(path, 0, 512);

            sprintf(path, "%s_wav", tmp_titles[sfxPackIndex]);
            sprintf(ext_path, "%s_extracted", tmp_titles[sfxPackIndex]);
            create_folder(path);

            BankHeader* headers = bank_headers.data;
            GenericList* adpcm_data = bank_data.data;

            _getcwd(cwd, 1024);

            for (i = 0; i < bank_headers.size; i++) {
                sprintf(wav_path, "%s\\bank_%03d", path, i+1);
                sprintf(bank_path, "%s\\bank_%03d", ext_path, i+1);
                create_folder(wav_path);
                int j;
                for (j = 0; j < headers[i].NumSounds; j++) {
                    sound_ptr = adpcm_data[i].data + headers[i].Sounds[j].BufferOffset;

                    sprintf(bank_path, "%s\\%s\\bank_%03d\\sound_%03d.vag", cwd, ext_path, i+1, j+1);
                    sprintf(wav_path, "%s\\%s\\bank_%03d\\sound_%03d.wav", cwd, path, i+1, j+1);

                    sprintf(call_path, "vgmstream-win\\vgmstream-cli.exe -i \"%s\" -o \"%s\"", bank_path, wav_path);

                    system(call_path);
                }
            }
        }
        break;

        case RAW:
        {
            char bank_path[512] = { 0 };
            char wav_path[512] = { 0 };
            char line_buf[256] = { 0 };
            char call_path[1024] = { 0 };
            char ext_path[64] = { 0 };
            char extracted_path[64] = { 0 };
            uint8_t* sound_ptr = NULL;

            memset(path, 0, 512);

            sprintf(path, "%s_wav", tmp_titles[sfxPackIndex]);
            sprintf(ext_path, "%s_vag", tmp_titles[sfxPackIndex]);
            sprintf(extracted_path, "%s_extracted", tmp_titles[sfxPackIndex]);
            create_folder(ext_path);

            BankHeader* headers = bank_headers.data;
            GenericList* adpcm_data = bank_data.data;

            _getcwd(cwd, 1024);

            for (i = 0; i < bank_headers.size; i++) {
                sprintf(bank_path, "%s\\bank_%03d\\sfx_meta.dat", extracted_path, i+1);
                FILE* fmeta = fopen(bank_path, "r");

                sprintf(wav_path, "%s\\bank_%03d", path, i+1);
                sprintf(bank_path, "%s\\bank_%03d", ext_path, i+1);
                create_folder(bank_path);
                int j;
                for (j = 0; j < headers[i].NumSounds; j++) {
                    sound_ptr = adpcm_data[i].data + headers[i].Sounds[j].BufferOffset;

                    sprintf(bank_path, "%s\\%s\\bank_%03d\\sound_%03d.vag", cwd, ext_path, i+1, j+1);
                    sprintf(wav_path, "%s\\%s\\bank_%03d\\sound_%03d.wav", cwd, path, i+1, j+1);

                    fgets(line_buf, 256, fmeta);

                    int32_t loop = -1;
                    uint16_t rate = 0;
                    int16_t headroom = 0;
                    
                    sscanf(line_buf, "%d %hd %hd\n", &loop, &rate, &headroom);
                    if (loop != -1) {
                        sprintf(call_path, "wav2vag.exe %s %s -l:%d", wav_path, bank_path, loop);
                    } else {
                        sprintf(call_path, "wav2vag.exe %s %s", wav_path, bank_path);
                    }

                    

                    system(call_path);
                }
                fclose(fmeta);
            }
        }
        break;
    }

    unload_generic_list(&pak_titles);
    unload_generic_list(&bank_meta);
    unload_generic_list(&bank_headers);
    unload_generic_list_nested(&bank_data, 1);

    return 0;
}