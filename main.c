////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2022 Antonio Fermiano
//
// This file is part of rawaudioutil.
//
// rawaudioutil is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// rawaudioutil is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with rawaudioutil.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RAW2WAV "--raw2wav"
#define WAV2RAW "--wav2raw"
#define BUFFER_SIZE (512 * 1024)
#define WAVE_HEADER_FILE_SIZE_OFFSET 4
#define DATA_CHUNK_FILE_SIZE_OFFSET 40

typedef enum _mode {MODE_NONE, MODE_RAW2WAV, MODE_WAV2RAW} mode;

void show_usage(){
    fprintf(stderr,
            "Usage:\n"
            "     rawaudioutil [" WAV2RAW "] [" RAW2WAV "] <filesize>\n"
            "\n"
            );
}

char wave_header[] = {0x52, 0x49, 0x46, 0x46,  // "RIFF"
                      0x00, 0x00, 0x00, 0x00,  // File size
                      0x57, 0x41, 0x56, 0x45,  // "WAVE"
                      0x66, 0x6D, 0x74, 0x20,  //"fmt "
                      0x10, 0x00, 0x00, 0x00,  // Length of format data (16)
                      0x01, 0x00,              // Format (PCM)
                      0x02, 0x00,              // Number of channels (2)
                      0x44, 0xAC, 0x00, 0x00,  // Sample rate (44100)
                      0x10, 0xB1, 0x02, 0x00,  // Bytes per second (1411200)
                      0x04, 0x00,              // Bytes per sample (4)
                      0x10, 0x00,              // Bits per sample (16)
                      0x64, 0x61, 0x74, 0x61,  // "data"
                      0x00, 0x00, 0x00, 0x00}; // Data chunk size

static char buffer[BUFFER_SIZE];

int redirect_data(){

    while(feof(stdin) == 0){
        int bytes_read = fread(buffer, 1, sizeof(buffer), stdin);

        if(ferror(stdin) != 0){
            fprintf(stderr, "Error reading data\n");
            return 2;
        }

        if(fwrite(buffer, 1, bytes_read, stdout) != bytes_read){
            fprintf(stderr, "Error writing data\n");
            return 3;
        }
    }

    return 0;
}

int wav2raw(){
    if(fread(buffer, 1, sizeof(wave_header), stdin) != sizeof(wave_header)){
        fprintf(stderr, "Error reading wave header\n");
        return 4;
    }

    return redirect_data();
}

int raw2wav(int filesize){
    int file_size_in_header = filesize + sizeof(wave_header) - 8;

    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        memcpy(wave_header + WAVE_HEADER_FILE_SIZE_OFFSET, &file_size_in_header, sizeof(file_size_in_header));
        memcpy(wave_header + DATA_CHUNK_FILE_SIZE_OFFSET, &filesize, sizeof(filesize));
    #else
        char *raw_file_size = (char *)file_size_in_header;
        char *raw_data_chunk_size = (char *)filesize;
        for(int i = 0; i < sizeof(file_size_in_header); i++){
            *(wave_header + WAVE_HEADER_FILE_SIZE_OFFSET + i) = raw_file_size[sizeof(file_size_in_header) - 1];
            *(wave_header + DATA_CHUNK_FILE_SIZE_OFFSET + i) = raw_data_chunk_size[sizeof(filesize) - 1];
        }
    #endif

    if(fwrite(wave_header, 1, sizeof(wave_header), stdout) != sizeof(wave_header)){
        fprintf(stderr, "Error writing wave header\n");
        return 5;
    }

    return redirect_data();
}

int main(int argc, char *argv[])
{
    int filesize = -1;
    mode mode = MODE_NONE;
    for(int i = 1; i < argc; i++){
        char *current_arg = argv[i];

        if(strcmp(WAV2RAW, current_arg) == 0){
            mode = MODE_WAV2RAW;
        }
        else if(strcmp(RAW2WAV, current_arg) == 0){
            mode = MODE_RAW2WAV;
        }
        else if(filesize < 0){
            filesize = atoi(current_arg);
        }
        else{
            show_usage();
            exit(1);
        }
    }

    if(mode == MODE_NONE || filesize < 0){
        show_usage();
        exit(1);
    }

    int ret = 0;
    if(mode == MODE_WAV2RAW){
        ret = wav2raw();
    }
    else if(mode == MODE_RAW2WAV){
        ret = raw2wav(filesize);
    }

    fflush(stdin);
    fflush(stdout);

    return ret;
}
