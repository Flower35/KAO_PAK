#include <KAO_PAK/PakExporter.h>
#include <KAO_PAK/TextFileReader.h>


////////////////////////////////////////////////////////////////
// PAK EXPORTER - initialize
////////////////////////////////////////////////////////////////

PakExporter::PakExporter(char* pak, char* directory, bool log, bool pc_ver)
{
    /* Reset data */

    FilesCount = 0;
    FileTableOffset = 0;

    Data = nullptr;

    /* Save paths */

    OutputDir = directory;
    PakName = pak;

    SaveLog = log;

    /* Does this game version use decryted script files? */

    Win32Version = pc_ver;
}


////////////////////////////////////////////////////////////////
// PAK EXPORTER - close
////////////////////////////////////////////////////////////////

PakExporter::~PakExporter()
{
    if (PakFile.is_open())
    {
        PakFile.close();
    }

    if (ItemFile.is_open())
    {
        ItemFile.close();
    }

    if (LogFile.is_open())
    {
        LogFile.close();
    }

    if (nullptr != Data)
    {
        delete[](Data);
    }
}

////////////////////////////////////////////////////////////////
// Check PAK archive file extension
////////////////////////////////////////////////////////////////

bool PakExporter::checkPakFileExtension()
{
    int l = PakName.length();

    /* At least one char, dot and PAK ext. */

    if (l >= (1 + 1 + 3))
    {
        return
        (
            ('.' == PakName.at(l - 4))
            && ('p' == std::tolower(PakName.at(l - 3)))
            && ('a' == std::tolower(PakName.at(l - 2)))
            && ('k' == std::tolower(PakName.at(l - 1)))
        );
    }

    return false;
}


////////////////////////////////////////////////////////////////
// Get PAK archive filename + check output directory
////////////////////////////////////////////////////////////////

void PakExporter::getPakFilenameFromPath()
{
    std::string temp = PakName;

    int c1 = PakName.rfind('/') + 1;
    int c2 = PakName.rfind('\\') + 1;
    PakName = PakName.substr(c1 > c2 ? c1 : c2);

    if (checkPakFileExtension())
    {
        PakName = PakName.substr(0, (PakName.length() - 4));
    }

    if (OutputDir.length() <= 0)
    {
        OutputDir = temp.substr(0, c1 > c2 ? c1 : c2) + PakName + '\\';
    }
}


////////////////////////////////////////////////////////////////
// Open and check PAK archive
////////////////////////////////////////////////////////////////

bool PakExporter::openAndCheckArchive()
{
    int32_t size;
    int32_t test;

    /* Try to open the PAK archive */

    PakFile.open(PakName, std::ios::in | std::ios::binary | std::ios::ate);

    if (!PakFile.is_open())
    {
        std::cout << "\n [ERROR] Cannot open this file:"
            << "\n * \"" << PakName << "\""
            << "\n";

        return false;
    }

    /* Read file size and set file pointer to the last 12 bytes */

    size = (int32_t)PakFile.tellg();
    PakFile.seekg(size - 12);

    /* Read archive info and check PAK header */

    PakFile.read((char*)&FilesCount, 0x04);
    PakFile.read((char*)&FileTableOffset, 0x04);
    PakFile.read((char*)&test, 0x04);

    if (*(int32_t*)"T8FM" != test)
    {
        std::cout << "\n [ERROR] Incorrect PAK header! (expected: \"T8FM\")"
            << "\n";

        return false;
    }

    /* Validate offsets and get output name */

    if ((FileTableOffset + (0x58 * FilesCount)) > (size - 12))
    {
        std::cout << "\n [ERROR] Incorrect file table offset or too many files!"
            << "\n";

        return false;
    }

    getPakFilenameFromPath();

    /* Write info to console window */

    std::cout << "\n --------------------------------"
        << "\n * " << PakName
        << "\n * Output folder: \"" << OutputDir << "\""
        << "\n";

    /* Try to open log file */
    
    _mkdir(OutputDir.c_str());

    if (SaveLog)
    {
        std::string path = OutputDir + PakName + ".log";

        LogFile.open(path.c_str(), std::ios::out | std::ios::trunc);

        if (!LogFile.is_open())
        {
            std::cout << "\n [WARNING] Cannot create this file:"
                << "\n * \"" << path << "\""
                << "\n";

            SaveLog = false;
        }
        else
        {
            std::cout << " * Log file: \"" << path << "\""
                << "\n";

            LogFile << "STREAM: " << PakName << "\n"
                << "FOLDER: " << OutputDir << "\n";
        }
    }

    return true;
}


////////////////////////////////////////////////////////////////
// Export whole PAK archive
////////////////////////////////////////////////////////////////

bool PakExporter::exportArchive()
{
    int32_t current_item = 0;

    char item_name[0x50];
    int32_t item_offset;
    int32_t item_size;

    /* Jump between file table and file offsets */

    std::cout << "\n Extracting archive..."
        << "\n";

    if (SaveLog)
    {
        LogFile << "\n -tate"
            << "\n";
    }

    for (current_item = 0; current_item < FilesCount; current_item++)
    {
        PakFile.seekg(FileTableOffset + (0x58 * current_item));

        PakFile.read(item_name, 0x50);
        PakFile.read((char*)&item_offset, 0x04);
        PakFile.read((char*)&item_size, 0x04);

        item_name[0x50 - 1] = 0x00;

        if ((item_offset + item_size) > FileTableOffset)
        {
            std::cout << "\n [ERROR] Incorrect file offset or size!"
                << "\n \"" << item_name << "\""
                << "\n (0x" << std::hex << item_offset << ", " << std::dec << item_size << ")"
                << "\n";

            return false;
        }

        PakFile.seekg(item_offset);

        if (!saveItem(item_size, item_name))
        {
            return false;
        }
    }

    /* The end :) */

    return true;
}


////////////////////////////////////////////////////////////////
// Save a single file to your hard drive
////////////////////////////////////////////////////////////////

bool PakExporter::saveItem(int32_t filesize, char* filename)
{
    std::string path = OutputDir + filename;
    char temp;

    /* Write info to console */

    std::cout << filename << "\n";

    if (SaveLog)
    {
        LogFile << filename << "\n";
    }

    /* Try to allocate memory for this item */

    if (nullptr != Data)
    {
        delete[](Data);
    }

    try
    {
        Data = new uint8_t [filesize];
    }
    catch (std::bad_alloc)
    {
        std::cout << "\n [ERROR] Could not allocate " << filesize << " bytes!"
            << "\n";

        return false;
    }

    /* Read item data */

    PakFile.read((char*)Data, filesize);

    if (Win32Version)
    {
        TextFileReader::decode(filename, Data, filesize);
    }

    /* Create output directories */

    for (char* p = (char*)path.c_str(); (*p); p++)
    {
        if (((*p) == '/') || ((*p) == '\\'))
        {
            temp = (*p);
            (*p) = 0x00;

            _mkdir(path.c_str());

            (*p) = temp;
        }
    }

    /* Open file for saving */

    if (ItemFile.is_open())
    {
        ItemFile.close();
    }

    ItemFile.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!ItemFile.is_open())
    {
        std::cout << "\n [ERROR] Could not create file:"
            << "\n\"" << path << "\""
            << "\n";

        return false;
    }

    /* Save data and close item */

    ItemFile.write((char*)Data, filesize);

    ItemFile.close();

    delete[](Data);
    Data = nullptr;

    return true;
}
