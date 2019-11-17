#ifndef H_PAK_IMPORTER
#define H_PAK_IMPORTER

#include <KAO_PAK/ConsoleApp.h>


////////////////////////////////////////////////////////////////
// PAK IMPORTER CLASS
////////////////////////////////////////////////////////////////

const int PAK_MAX_FILES = 4096;


enum ParseOptions
{
    GET_STREAM_NAME,
    GET_DIRECTORY,
    GET_NEXT_FILE
};


class PakImporter
{
    public:

    /*** Properties ***/

        std::string MediaDir;
        char PakName[32];

        int32_t FilesCount;
        int32_t FileTableOffset;
        uint8_t* FileTable;

        std::ofstream PakFile;
        std::ifstream ItemFile;
        uint8_t* Data;

        std::ifstream ListFile;
        std::string ListName;

    /*** Methods ***/

        void trimLine(std::string &line);

        bool parseList(std::string &result, int what);

        bool readItem(int32_t &filesize, char* filename);

        bool createArchive();

        bool importData();

        PakImporter(char* log);

        ~PakImporter();
};

#endif
