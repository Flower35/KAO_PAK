#include <KAO_PAK/TextFileReader.h>


////////////////////////////////////////////////////////////////
// TextFileReader: checking filename
////////////////////////////////////////////////////////////////

bool TextFileReader::compareNames(char* filename, const char* test_str)
{
    int32_t i;
    int32_t size_a = std::strlen(filename);
    int32_t size_b = std::strlen(test_str);

    int32_t char_a;
    int32_t char_b;
    bool dir_separator_a;
    bool dir_separator_b;

    for (i = 0; i < size_b; i++)
    {
        char_a = std::tolower(filename[i]);
        char_b = std::tolower(test_str[i]);

        dir_separator_a = (('/' == char_a) || ('\\' == char_a));
        dir_separator_b = (('/' == char_b) || ('\\' == char_b));

        if (!(dir_separator_a && dir_separator_b))
        {
            if (char_a != char_b)
            {
                return false;
            }
        }
    }

    return true;
}

bool TextFileReader::nameStartsWith(char* filename, const char* test_str)
{
    int32_t size_a = std::strlen(filename);
    int32_t size_b = std::strlen(test_str);

    if (size_b > size_a)
    {
        return false;
    }

    return compareNames(filename, test_str);
}

bool TextFileReader::nameEndsWith(char* filename, const char* test_str)
{
    int32_t size_a = std::strlen(filename);
    int32_t size_b = std::strlen(test_str);

    if (size_b > size_a)
    {
        return false;
    }

    return compareNames((filename + size_a - size_b), test_str);
}

bool TextFileReader::isSuitableForDecoding(char* filename)
{
    if (nameEndsWith(filename, ".def"))
    {
        return true;
    }

    if (nameEndsWith(filename, ".at"))
    {
        return true;
    }

    if (nameStartsWith(filename, "text/localize.win/denis."))
    {
        return true;
    }

    if (nameEndsWith(filename, "/scene.cut"))
    {
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////
// TextFileReader: decoding
////////////////////////////////////////////////////////////////

void TextFileReader::decode(char* filename, uint8_t* data, int32_t filesize)
{
    int32_t i; // file data loop

    int32_t t; // magic word
    const char tate[4] = {'t','a','t','e'};

    uint8_t b; // decoding helper
    uint8_t c; // temp byte

    if (isSuitableForDecoding(filename))
    {
        t = 0;
        b = 0x12;

        for (i = 0; i < filesize; i++)
        {
            c = (tate[t] ^ data[i]) - b;
            b = data[i];
            data[i] = c;

            t++;

            if (t >= 4)
            {
                t = 0;
            }
        }
    }
}


////////////////////////////////////////////////////////////////
// TextFileReader: encoding
////////////////////////////////////////////////////////////////

void TextFileReader::encode(char* filename, uint8_t* data, int32_t filesize)
{
    int32_t i; // file data loop

    int32_t t; // magic word
    const char tate[4] = {'t','a','t','e'};

    uint8_t b; // encoding helper

    if (isSuitableForDecoding(filename))
    {
        t = 0;
        b = 0x12;

        for (i = 0; i < filesize; i++)
        {
            b = (b + data[i]) ^ tate[t];
            data[i] = b;

            t++;

            if (t >= 4)
            {
                t = 0;
            }
        }
    }
}
