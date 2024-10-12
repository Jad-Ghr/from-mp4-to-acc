#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

uint32_t take_first_4_bits(FILE *file){

uint8_t octa[4];
if(fread(octa,1,4,file)==4){
printf("Raw size bytes: %02x %02x %02x %02x\n", octa[0], octa[1], octa[2], octa[3]);
return (octa[0] << 24) | (octa[1] << 16) |(octa[2] << 8) |octa[3] ;
}
else{
    return 0;
}
}
uint64_t take_first_8_bits(FILE *file){
uint8_t octa[8];
if(fread(octa,1,8,file) == 8){
   return ((uint64_t)octa[0]<< 56) | ((uint64_t)octa[1]<< 48)|
          ((uint64_t)octa[2]<< 40) | ((uint64_t)octa[3]<< 32)|
          ((uint64_t)octa[4]<< 24) | ((uint64_t)octa[5]<< 16)|
          ((uint64_t)octa[6]<< 8) | ((uint64_t)octa[7]);
   }
else {
        return 0;
        }
}

void type_mp4(FILE *file,char *type){
    if(fread(type,1,4,file)==4){
        type[4]='\0';
    }
    else{
        type[0]='\0';
    }
}

void write_adts_header(FILE *aac_file, uint32_t frame_length) {
    uint16_t syncword = 0xFFF; // ADTS syncword
    uint8_t id = 0; // MPEG version (0 = MPEG-4)
    uint8_t layer = 0; // Layer (always 0)
    uint8_t protection_absent = 1; // 1 if no CRC is present
    uint8_t profile = 2; // AAC LC
    uint8_t sampling_frequency_index = 4; // 44100 Hz
    uint8_t channel_configuration = 2; // 2 channels (stereo)

    // Write ADTS header
    uint16_t adts_length = frame_length + 7; // Add 7 for ADTS header length
    fwrite(&syncword, 2, 1, aac_file);
    uint8_t header[7];
    header[0] = (id << 3) | (layer << 1) | protection_absent;
    header[1] = (profile << 6) | (sampling_frequency_index << 2) | (channel_configuration >> 2);
    header[2] = (channel_configuration << 6) & 0xC0;
    header[2] |= (adts_length >> 11) & 0x1F;
    header[3] = (adts_length >> 3) & 0xFF;
    header[4] = ((adts_length & 0x7) << 5) | 0x1F; // The last 5 bits
    header[5] = 0; // No CRC
    header[6] = 0;

    fwrite(header, sizeof(header), 1, aac_file);
}

void ProAudDat(uint8_t *audio_spl,uint32_t size){


    FILE *aac_file = fopen("C:\\Users\\kakois\\Downloads\\audio_output.aac", "wb");


    if (aac_file == NULL) {

        printf("Error: Unable to create AAC file.\n");

        return;

    }

printf("Processing audio data (first 16 bytes): ");

    for (uint32_t i = 0; i < size && i < 16; i++) {
        printf("%02x \n ", audio_spl[i]);
    }

    printf("\n");

    // Write the audio data to the MP3 file
    write_adts_header(aac_file, size - 8);
    fwrite(audio_spl, 1, size, aac_file);
    fclose(aac_file);
    printf("Audio data saved to audio_output.aac\n");
}


int main()
{
    //this how you make a poiter to file

    FILE *main_file;

    // Reading file;

    char file_read[127];
    printf("write me the path for folder :");
    fgets(file_read,sizeof(file_read),stdin);//exp for file C:\Users\kakois\Downloads\2 Broke Girls Season 3 WEB-DL 720p x264 [Pahe.in]\2 Broke Girls S03 EP 001.mp4
    file_read[strcspn(file_read,"\n")]='\0';


    if( strlen(file_read) < 4 |strcmp(file_read + strlen(file_read) - 4 ,".mp4") != 0 ){
        printf("This format is not suported :");
        return 1;
    }


    main_file = fopen(file_read,"rb");


    if(main_file == NULL){
            printf("Error: Unable to access the file '%s'.\n", file_read);
            return 1;
    }

    else{

        while(1){// f gets(ch,size of(ch),main_file) does not work because we don't want to read we want just to se the end

            uint32_t size = take_first_4_bits(main_file);

            if(size == 1){
                uint64_t size_64 = take_first_8_bits(main_file);
                size = (uint64_t)size_64;
                if( size_64 >UINT32_MAX){
                    printf("Error: Atom size exceeds 32-bit range (%llu bytes).\n", size_64);
                    break;
                }

            }


            if (size < 8) {  // Atom size must be at least 8 bytes
            printf("Error: Invalid atom size (%u bytes).\n", size);
            break;
        }


        char atom_type[5];
            type_mp4(main_file,atom_type);


        if (atom_type[0] == '\0') {
            printf("Error reading atom type.\n");
            break;
        }


            else{

                if(strcmp(atom_type,"mdat")==0){
                    printf("Found atom type: ('mdat ') (size: %u bytes)\n", size);

                            uint8_t *audio_spl = malloc( size - 8 ) ;

                            if( audio_spl == NULL){
                                printf("can not access ?\n");
                                break;
                            }

                            else{

                                if(fread(audio_spl,1,size-8,main_file)!=size -8){
                                    printf("Error reading \n");
                                    free(audio_spl);
                                    break;
                                }

                                else{
                                    //step 4

                                    ProAudDat(audio_spl,size-8);
                                    free(audio_spl);\
                                }


                                }
                                }

                else {

                        if(strcmp(atom_type,"moov")==0){

                                printf("Found atom type: ('moov') (size: %u bytes)\n", size);
                            }



                        else{

                            printf("Found atom: %s (size: %u bytes)\n", atom_type, size);

                        }}

                }


            if (fseek(main_file, size - 8, SEEK_CUR) != 0) {
            printf("Error: Failed to skip atom data.\n");
            break;

        }

            if (feof(main_file)) {
            break;
        }
        }
    }
    fclose(main_file);
    return 0;
}

