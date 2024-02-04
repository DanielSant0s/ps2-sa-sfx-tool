#ifndef SFX_FORMATS
#define SFX_FORMATS

#include <stdint.h>

typedef enum {
    UNPACK,
    PACK,
	WAV,
	RAW,
} SfxAppModes;

//PakFiles.dat
typedef char PackageTitle[52];

typedef struct {
    void* data;
    uint32_t size;
} GenericList;

// BankLkup.dat
typedef struct {
	// Index of package in PakFiles.dat
	uint8_t PackageIndex;
	uint8_t Padding[3];

	// Bank header location in package file.
	uint32_t BankHeaderOffset;

	// Total size of sounds in bank.
	uint32_t BankSize;
} BankMeta;

typedef struct {
	// Offset of the PCM buffer from the end of the bank header.
	uint32_t BufferOffset;

	// Where the start of the loop is (in samples).
	int32_t LoopOffset;

	// Sample rate (measured in Hz).
	uint16_t SampleRate;

	// Audio headroom. Defines how much louder than average
	//  this sound effect is expected to go.
	int16_t Headroom;
} SoundMeta;

// 4804 bytes (including SoundMeta array)
typedef struct {
	// The number of sounds in the bank. Must be <= 400.
	uint16_t NumSounds;
	uint16_t Padding;

	// Sound metadata.
	SoundMeta Sounds[400];
} BankHeader;

typedef struct {
	// Sum of all buffer sizes before this slot (i.e. the offset).
	uint32_t BufferOffset;

	// Buffer size for this slot.
	uint32_t BufferSize;

	// {-1, -1} on disk. Related to feet sounds?
	int32_t Unknown[2];

    short BankId;
    short NumSoundInfo;
	SoundMeta Sounds[400];
} Slot;

typedef struct {
	// Always 46
	uint16_t NumSlots;
	Slot Slots[46];
} SlotFile;

uint32_t getPCMOffset(SoundMeta *sound, BankMeta *bankInfo);
uint32_t calculateBufferSize(BankHeader *bankHeader, uint32_t bankAudioSize, int soundIndex);

void load_package_titles(GenericList* list);
void load_bank_lookup(GenericList* list);
void load_bank_data(GenericList* bank_headers, GenericList* bank_data, const char* fname, GenericList* curr_pack);

void unload_generic_list(GenericList* list);
void unload_generic_list_nested(GenericList* list, int levels);

#endif