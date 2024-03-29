# ps2-sa-sfx-tool

Requires: Windows 7^, Python 3.7^

## How to use SFX Tool:  

    1. Copy your entire AUDIO folder from PlayStation 2 version of GTA San Andreas and put it here.

    2. Open cmd and type the file you want to extract. E.g.: sfx.exe GENRL01.PAK

    3. It will generate a folder called "GENRL_extracted" sou you can see your banks and sounds there.
        a. Inside each bank folder there is an sfx_meta.dat, a text file that contains 
           sample rate, loop start (if there is no loop it will be -1) and average volume, 
           where each line represents a sound_XXX.vag.

        b. The sound output is in .vag format, a typical PS2 format, use vgmstream or MFAudio if you 
           want to convert it to .wav to edit or whatever.

    4. Create a folder called "GENRL_mod" and put the bank folders and your modified sounds with the correct structure.
        a. Remember to create a .wav file with the same sample rate as the original. 
           If you change that, you have to change on each sfx_meta.dat at "GENRL_extracted".

        b. Run "convert_mods.py", it will generate a folder called "GENRL_converted" with your vags.

    5. Replace GENRL_extracted sounds by the sounds you converted on step 4, change sfx_meta.dat if necessary.

    6. Reopen cmd again and type "sfx.exe --pack GENRL01.PAK", and it will compile a new GENRL01.PAK and BANKLKUP.DAT on sfx-tool folder.
        a. Remember to create a GENRL02.PAK, which is just a copy of the first file. Every SFX pack needs that.

## How to use Bank Slot Manager:  

    When you replace a sound with a better sample rate or a longer sound, you need to increase bank slots to make the game
    allocate the right amount of memory. Put the files SFX Tool generated and run with PCSX2 (REMEMBER TO ENABLE LOG CONSOLE).
    If you see any "IOPAudio Error: Cannot load sound bank X ( Y bytes) into bank slot Z (W bytes)" on console, you need to do
    this process in order to get your sounds playing correctly. I included a slightly better calibrated BANKSLOT.DAT, but 
    sometimes you need to do it yourself, I included a script called slot_increaser.py too, it increases all slots by 1500 bytes
    everytime you run it.

    1. Do "How to use SFX Tool" step 1 if you don't.

    2. Open cmd and type "slots.exe".
    
    3. It will generate a file called slots_dump.dat
        a. Increase the bank slots as indicated on PCSX2 log 
           ("IOPAudio Error: Cannot load sound bank X (Y bytes) into bank slot Z (W bytes)").

        b. Open slots_dump.dat with a text editor, search W (amount of bytes the slot stores) and 
           replace it for Y (the size your new bank needs), save and close.

    4. Open cmd and type "slots.exe --build".
        a. It will generate a new BANKSLOT.DAT at slots.exe folder.
