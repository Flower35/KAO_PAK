#ifndef H_PAK_EXPORTER
#define H_PAK_EXPORTER

#include <KAO_PAK/ConsoleApp.h>


////////////////////////////////////////////////////////////////
// PAK EXPORTER CLASS
////////////////////////////////////////////////////////////////

class PakExporter
{
    public:

    /*** Properties ***/

        std::string OutputDir;

        std::string PakName;

        int32_t FilesCount;
        int32_t FileTableOffset;

        std::ifstream PakFile;
        std::ofstream ItemFile;
        uint8_t* Data;

        std::ofstream LogFile;
        bool SaveLog;

        bool Win32Version;

    /*** Methods ***/

        bool saveItem(int32_t filesize, char* filename);

        bool exportArchive();

        bool checkStreamSize(int32_t size);

        bool openAndCheckArchive();

        bool checkPakFileExtension();

        void getPakFilenameFromPath();

        PakExporter(char* pak, char* directory, bool log, bool pc_ver);

        ~PakExporter();
};

#endif
