#include "pch.hh"

#include "imy.hh"

#pragma pack(push,1)

struct imy_header{
    unsigned int magic_number; // 4

    unsigned int size; // 2
    unsigned short stream_offset; //was width // 2
    unsigned char comp_type; // 1
    unsigned char unknown06; // 1
    unsigned short unknown07; //was height // 2
    unsigned short unknown08; //was paletteSize // 2 -> 16
    unsigned int zero0; //padding // 4
    unsigned int zero1; //padding // 4 -> 24
    unsigned int zero2; //padding // 4
    unsigned int zero3; //padding // 4 -> 32
    unsigned short number_of_info_bytes;// 2 -> 34
}
#ifndef _MSC_VER
__attribute__((packed, aligned(1)));
#endif
#pragma pack(pop)

imy::~imy() {
    for (int i = 0; i < child_count; ++i) delete child_file[i];
    free(child_file);
}

bool imy::extract(const char* filename, const char* outfile) {
    FILE* f = fopen(filename, "rb");
    if (f == nullptr) return false;
    fseek(f, 0, SEEK_END);
    uint32_t sz = ftell(f);
    if (sz < 0x20) {
        fclose(f);
        return false;
    }
    fseek(f, 0, SEEK_SET);
    uint8_t* data = new uint8_t[sz];
    fread(data, 1, sz, f);
    fclose(f);
    uint32_t imy_count = ((uint32_t*)data)[0];
    uint32_t* imy_offset = ((uint32_t*)data) + 2;
    fread(imy_offset, 4, imy_count, f);
    FILE* fout = fopen(outfile, "wb");
    uint32_t dec_file_size = 0;
    for (uint32_t i = 0; i < imy_count; ++i) {
        uint32_t start_offset = imy_offset[i];
        imy_header* header = (imy_header*)(data + start_offset);
        if(header->magic_number != 0x00594D49){
            fprintf(stderr, "IMY file magic number is wrong!\n");
            delete[] data;
            return false;
        }
        dec_file_size += (header->size & ~0xFF);
    }
    uint8_t* out_data = new uint8_t[dec_file_size];
    uint32_t out_offset = 0;
    uint32_t end_offset = 0;
    for (uint32_t i = 0; i < imy_count; ++i) {
        fprintf(stdout, "Extracting index %u at offset %X...\n", i, imy_offset[i]);
        uint32_t start_offset = imy_offset[i];
        imy_header* header = (imy_header*)(data + start_offset);
        end_offset += header->size & ~0xFF;
        if ((header->comp_type >> 4) == 1) {
            unsigned int info_byte_offset = start_offset + sizeof(imy_header);
            const unsigned int info_byte_offset_end = info_byte_offset + header->number_of_info_bytes;
            unsigned int data_offset = info_byte_offset_end;

            //create look up table
            uint32_t lookup_table[4];
            lookup_table[0] = 2; // go back one short
            lookup_table[1] = header->stream_offset;
            lookup_table[2] = lookup_table[1] + 2;
            lookup_table[3] = lookup_table[1] - 2;

            while( out_offset < end_offset && info_byte_offset < info_byte_offset_end){
                // read every info byte
                unsigned char info_byte = data[info_byte_offset++];

                if (info_byte & 0xF0) {
                    if ((info_byte & 0xC0) == 0xC0) {
                        //copy shorts from the already uncompressed stream by looking back
                        const int index = (info_byte & 0x30) >> 4; // the value can only be 0-3
                        const int bytes_to_copy = ((info_byte & 0x0F) + 1) * 2;
                        memcpy(out_data + out_offset, out_data + out_offset - lookup_table[index], bytes_to_copy);
                        out_offset += bytes_to_copy;
                    } else {
                        //copy a short from the compressed stream by looking back to a short
                        const unsigned int lookback_bytes = (info_byte - 16)*2 + 2;
                        memcpy(out_data + out_offset, data + data_offset - lookback_bytes, 2);
                        out_offset += 2;
                    }
                } else {
                    // just copy shorts (2 byte)
                    //you always copy at least one short
                    const unsigned int copy_bytes = (info_byte+1)*2;
                    memcpy(out_data + out_offset, data + data_offset, copy_bytes);
                    data_offset += copy_bytes;
                    out_offset += copy_bytes;
                }
            }
            fprintf(stdout, "%u %u %u\n", out_offset, info_byte_offset, info_byte_offset_end);
        } else {
            fprintf(stderr, "IMY compression type '%u' is not supported!\n", header->comp_type);
            delete[] out_data;
            fclose(fout);
            return false;
        }
    }
    fwrite(out_data, 1, dec_file_size, fout);
    delete[] out_data;
    fclose(fout);
    delete[] data;
    return true;

/*
    if( (header->comp_type >> 4) == 1){

        const unsigned int dec_file_size = header->stream_offset * header->unknown07; //not sure

        unsigned int info_byte_offset = instream->pos();
        const unsigned int info_byte_offset_end = info_byte_offset + header->number_of_info_bytes;
        unsigned int data_offset = info_byte_offset_end;

        //create look up table
        PG::UTIL::Array<unsigned int, 4> lookup_table;
        lookup_table[0] = 2; // go back one short
        lookup_table[1] = header->stream_offset;
        lookup_table[2] = lookup_table[1] + 2;
        lookup_table[3] = lookup_table[1] - 2;

        while( (outstream->pos()-start_offset) < dec_file_size && info_byte_offset < info_byte_offset_end){

            // read every info byte
            instream->seek(info_byte_offset);
            unsigned char info_byte = instream->readChar();
            info_byte_offset++;

            if( info_byte & 0xF0 ){
                if( (info_byte & 0x80) && (info_byte & 0x40)){
                    //copy shorts from the already uncompressed stream by looking back
                    const int index = (info_byte & 0x30) >> 4; // the value can only be 0-3
                    const int shorts_to_copy = (info_byte & 0x0F) + 1;

                    for (int i = 0; i < shorts_to_copy; i++){
                        //read
                        const unsigned int curr_end = outstream->pos();
                        outstream->seek(curr_end - lookup_table[index]);
                        const short s = outstream->readShort();
                        outstream->seek(curr_end);
                        outstream->writeShort(s);
                    }

                } else {
                    //copy a short from the compressed stream by looking back to a short
                    const unsigned int lookback_bytes = (info_byte - 16)*2 + 2;
                    instream->seek(data_offset-lookback_bytes);
                    outstream->writeShort(instream->readShort());
                }
            } else {
                // just copy shorts (2 byte)
                //you always copy at least one short
                const unsigned int copy_bytes = (info_byte+1)*2;
                char c[copy_bytes];
                instream->seek(data_offset);
                instream->read(&c[0], copy_bytes);
                outstream->write(&c[0], copy_bytes);
                data_offset += copy_bytes;
            }
        }
    } else {
        std::cerr << "IMY compression type '"<<header->comp_type<<"' is not supported!" << std::endl;
        return FAILURE;
    }

    return SUCCESS;
    */
}

bool imy::build(const char* filename) {
    return true;
}

void imy::add(const char* file) {
    child_file = (char**)realloc(child_file, sizeof(char*) * (child_count + 1));
    child_file[child_count++] = strdup(file);
}
