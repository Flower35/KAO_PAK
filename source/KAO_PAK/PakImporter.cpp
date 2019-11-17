#include <KAO_PAK/PakImporter.h>


////////////////////////////////////////////////////////////////
// PAK IMPORTER - initialize
////////////////////////////////////////////////////////////////

PakImporter::PakImporter(char* log)
{
    /* Reset data */

    std::memset(PakName, 0x00, sizeof(PakName));
    FilesCount = (-1);
    FileTableOffset = 0;
    Data = nullptr;
    FileTable = nullptr;

    /* Save file path */

    ListName = log;
}


////////////////////////////////////////////////////////////////
// PAK IMPORTER - close
////////////////////////////////////////////////////////////////

PakImporter::~PakImporter()
{
    if (PakFile.is_open())
    {
        PakFile.close();
    }

    if (ItemFile.is_open())
    {
        ItemFile.close();
    }

    if (nullptr != Data)
    {
        delete[](Data);
    }

     if (nullptr != FileTable)
    {
        delete[](FileTable);
    }
}


////////////////////////////////////////////////////////////////
// Trim line (delete special characters on both ends)
////////////////////////////////////////////////////////////////

void PakImporter::trimLine(std::string &line)
{
    int start = 0;
    int end = (line.length() - 1);

    if (end >= 0)
    {
        while ((start < end) && (line.at(start) <= 0x20))
        {
            start++;
        }

        while ((end > start) && (line.at(end) < 0x20))
        {
            end--;
        }

        line = line.substr(start, (end - start + 1));
    }
}


////////////////////////////////////////////////////////////////
// Parse files list
////////////////////////////////////////////////////////////////

bool PakImporter::parseList(std::string &result, int what)
{
    switch(what)
    {
        case ParseOptions::GET_STREAM_NAME:
        {
            ListFile.seekg(0);

            while (!ListFile.eof())
            {
                std::getline(ListFile, result);

                trimLine(result);

                if (0 == result.compare(0, 7, "STREAM:"))
                {
                    result = result.substr(7);
                    trimLine(result);

                    return true;
                }
            }

            return false;
        }

        case ParseOptions::GET_DIRECTORY:
        {
            ListFile.seekg(0);

            while (!ListFile.eof())
            {
                std::getline(ListFile, result);

                trimLine(result);

                if (0 == result.compare(0, 7, "FOLDER:"))
                {
                    result = result.substr(7);
                    trimLine(result);

                    return true;
                }
            }

            return false;
        }

        case ParseOptions::GET_NEXT_FILE:
        {
            if (FilesCount < 0)
            {
                /* Search for the first file on the list */

                ListFile.seekg(0);

                while (!ListFile.eof())
                {
                    std::getline(ListFile, result);

                    trimLine(result);

                    if (0 == result.compare("-tate"))
                    {
                        FilesCount = 0;

                        result = "";
                        return true;
                    }
                }

                return false;
            }
            else
            {
                /* Finish checking list if ending was reached */

                if (ListFile.eof())
                {
                    return false;
                }

                std::getline(ListFile, result);

                trimLine(result);

                return true;
            }

            return false;
        }

        default:
        {
            std::cout << "\n [ERROR] Invalid parser argument."
                << "\n";

            return false;
        }
    }
}


////////////////////////////////////////////////////////////////
// Create a brand new PAK archive
////////////////////////////////////////////////////////////////

bool PakImporter::createArchive()
{
    int32_t test;
    std::string temp_str;

    /* Allocate memory for file table */

    try
    {
        FileTable = new uint8_t [0x58 * PAK_MAX_FILES];
    }
    catch (std::bad_alloc)
    {
        std::cout << "\n [ERROR] Could not allocate " << (0x58 * PAK_MAX_FILES) << " bytes for file table!"
            << "\n";

        FileTable = nullptr;
        return false;
    }

    /* Try to open list file */

    ListFile.open(ListName, std::ios::in);

    if (!ListFile.is_open())
    {
        std::cout << "\n [ERROR] Cannot open this file:"
            << "\n * \"" << ListName << "\""
            << "\n";

        return false;
    }

    /* Get stream name and input/output directory */

    if (!parseList(temp_str, ParseOptions::GET_STREAM_NAME))
    {
        std::cout << "\n [ERROR] Could not find \"STREAM:\" line in list file!"
            << "\n";

        return false;
    }

    test = temp_str.length();
    test = (test > 32) ? 32 : test;
    std::memcpy(PakName, temp_str.c_str(), test);

    if (!parseList(MediaDir, ParseOptions::GET_DIRECTORY))
    {
        std::cout << "\n [ERROR] Could not find \"FOLDER:\" line in list file!"
            << "\n";

        return false;
    }

    if ((MediaDir.back() != '/') && (MediaDir.back() != '\\'))
    {
        MediaDir += '\\';
    }

    /* Try to open the PAK archive */

    temp_str = MediaDir + PakName + ".pak";

    PakFile.open(temp_str, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!PakFile.is_open())
    {
        std::cout << "\n [ERROR] Cannot create this file:"
            << "\n * \"" << temp_str << "\""
            << "\n";

        return false;
    }

    /* Write info to console window */

    std::cout << "\n --------------------------------"
        << "\n * " << PakName
        << "\n * Output folder: \"" << MediaDir << "\""
        << "\n";

    return true;
}


////////////////////////////////////////////////////////////////
// Import files into PAK archive
////////////////////////////////////////////////////////////////

bool PakImporter::importData()
{
    int32_t test;
    int32_t item_size;

    std::string temp_str;

    char empty[128];
    std::memset(empty, 0x00, 128);

    std::cout << "\n Importing data..."
        << "\n";

    while (parseList(temp_str, ParseOptions::GET_NEXT_FILE))
    {
        if (temp_str.length() > 0)
        {
            if (FilesCount >= PAK_MAX_FILES)
            {
                break;
            }

            /* Add new item */

            if (!readItem(item_size, (char*)temp_str.c_str()))
            {
                return false;
            }

            /* Save item info to file table */

            std::memset(&(FileTable[0x58 * FilesCount]), 0x00, 0x50);

            test = temp_str.length();
            test = (test > 0x50) ? 0x50 : test;
            std::memcpy(&(FileTable[0x58 * FilesCount]), temp_str.c_str(), test);

            *(uint32_t*)&(FileTable[0x58 * FilesCount + 0x50]) = FileTableOffset;
            *(uint32_t*)&(FileTable[0x58 * FilesCount + 0x54]) = item_size;

            /* Update offset for next file */

            FileTableOffset += item_size;

            /* Increase items counter */

            FilesCount++;
        }
    }

    /* Complete archive header */

    PakFile.write((char*)FileTable, (0x58 * FilesCount));

    PakFile.write((char*)&FilesCount, 0x04);
    PakFile.write((char*)&FileTableOffset, 0x04);
    PakFile.write("T8FM", 0x04);

    /* The end :) */

    PakFile.close();

    return true;
}


////////////////////////////////////////////////////////////////
// Copy a single file content
////////////////////////////////////////////////////////////////

bool PakImporter::readItem(int32_t &filesize, char* filename)
{
    std::string path = MediaDir + filename;

    /* Write info to console */

    std::cout << filename << "\n";

    /* Try to open new file */

    if (ItemFile.is_open())
    {
        ItemFile.close();
    }

    ItemFile.open(path, std::ios::in | std::ios::binary | std::ios::ate);

    if (!ItemFile.is_open())
    {
        std::cout << "\n [ERROR] Cannot open this file:"
            << "\n * \"" << path << "\""
            << "\n";

        return false;
    }

    /* Get file size and reset file pointer */

    filesize = (int32_t)ItemFile.tellg();
    ItemFile.seekg(0);

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

    /* Read item data and close file */

    ItemFile.read((char*)Data, filesize);
    ItemFile.close();

    /* Save data to archive */

    PakFile.write((char*)Data, filesize);

    delete[](Data);
    Data = nullptr;

    return true;
}
