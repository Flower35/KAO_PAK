#ifndef H_TEXTFILEREADER
#define H_TEXTFILEREADER

#include <KAO_PAK/ConsoleApp.h>


////////////////////////////////////////////////////////////////
// SCRIPTS DECODING
////////////////////////////////////////////////////////////////

namespace TextFileReader
{
    bool compareNames(char* filename, const char* test_str);
    bool nameStartsWith(char* filename, const char* test_str);
    bool nameEndsWith(char* filename, const char* test_str);

    bool isSuitableForDecoding(char* filename);

    void decode(char* filename, uint8_t* data, int32_t filesize);
    void encode(char* filename, uint8_t* data, int32_t filesize);
}

#endif
