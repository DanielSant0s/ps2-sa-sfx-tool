import os
import subprocess

root = os.getcwd()
conv_root = os.getcwd()

with os.scandir(root) as root_entries:
    for root_entry in root_entries:
        if root_entry.name.endswith("_mod"):
            convdir_name = root_entry.name.replace("_mod", "_converted")
            conv_bank = root + "\\" + convdir_name + "\\"
            bank = root + "\\" + root_entry.name + "\\"
    
            if not os.path.exists(convdir_name):
                os.mkdir(convdir_name, 777)

            with os.scandir(bank) as banks_entries:
               for bank_entry in banks_entries:
                    
                    if not os.path.exists(conv_bank + "\\" + bank_entry.name):
                        os.mkdir(conv_bank + "\\" + bank_entry.name, 777)

                    sound_path = bank + "\\" + bank_entry.name + "\\"
                    sound_conv_path = conv_bank + "\\" + bank_entry.name + "\\"

                    with os.scandir(bank + "\\" + bank_entry.name) as sounds_entries:
                        for sound_entry in sounds_entries:
                            subprocess.run(["wav2vag.exe", sound_path+sound_entry.name, sound_conv_path+sound_entry.name.replace(".wav", ".vag")])

                        print(sound_entry.name)
