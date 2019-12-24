#include <KAO_PAK/PakExporter.h>
#include <KAO_PAK/PakImporter.h>

bool askQuestion(bool default, std::string &answer, const std::string &question)
{
    std::cout << question
        << "\n >> ";

    std::getline(std::cin, answer);

    if (answer.length() > 0)
    {
        if ('y' == tolower(answer.at(0)))
        {
            return true;
        }
    }

    return default;
}

int main(int argc, char** argv)
{
    std::string common_path;
    std::string answer;
    bool use_log = false;
    bool pc_ver = true;
    bool no_errors = true;

    /* Welcome! */

    std::cout << "\n < KAO_PAK >"
        << "\n Simple utility to extract files from"
        << "\n \"Kao the Kangaroo\" PAK archives."
        << "\n";

    if (argc < 2)
    {
        std::cout << "\n Please close this window and drop"
            << "\n one or more of KAO PAK archives"
            << "\n onto this executable!"
            << "\n (press any key to close)"
            << "\n";

        std::cin.ignore();
        return (-1);
    }

    /* Get ready */

    std::cout << "\n Choose an option:"
        << "\n 1) passed PAK archives as arguments [UNPACK FILES]"
        << "\n 2) passed LOG lists of files as arguments [REPACK ARCHIVES]"
        << "\n >> ";

    std::getline(std::cin, common_path);

    switch (std::stoi(common_path))
    {
        case 1:
        {
            std::cout << "\n Please write a common path for all files"
                << "\n or leave empty for separate directories."
                << "\n >> ";

            std::getline(std::cin, common_path);

            if (common_path.length() > 0)
            {
                if ((common_path.back() != '/') && (common_path.back() != '\\'))
                {
                    common_path += '\\';
                }
            }

            /* Ask another question */

            use_log = askQuestion
            (
                true,
                answer,
                    "\n Do you want to create LOG files" \
                    "\n for repacking? [type `Y` for Yes, default: Yes]"
            );

            /* Check game version */

            pc_ver = !askQuestion
            (
                false,
                answer,
                    "\n Are you unpacking Dreamcast version?" \
                    "\n (which does NOT require scripts decoding)" \
                    "\n [type `Y` for Yes, default: No]"
            );

            /* Iterate through every argument */

            for (int i = 1; (i < argc) && no_errors; i++)
            {
                std::cout << "\n --------------------------------"
                    << "\n \"" << argv[i] << "\""
                    << "\n";

                PakExporter pak(argv[i], (char*)common_path.c_str(), use_log, pc_ver);

                if (pak.openAndCheckArchive())
                {
                    no_errors = pak.exportArchive();
                }
                else
                {
                    no_errors = false;
                }
            }

            break;
        }

        case 2:
        {
            /* Check game version */

            pc_ver = !askQuestion
            (
                false,
                answer,
                    "\n Are you repacking Dreamcast version?" \
                    "\n (which does NOT require scripts encoding)" \
                    "\n [type `Y` for Yes, default: No]"
            );

            /* Iterate through every argument */

            for (int i = 1; (i < argc) && no_errors; i++)
            {
                std::cout << "\n --------------------------------"
                    << "\n \"" << argv[i] << "\""
                    << "\n";

                PakImporter pak(argv[i], pc_ver);

                if (pak.createArchive())
                {
                    no_errors = pak.importData();
                }
                else
                {
                    no_errors = false;
                }
            }

            break;
        }
    }

    /* The end :) */

    std::cout << "\n --------------------------------"
        << "\n (press any key to close)"
        << "\n";

    std::cin.ignore();
    return 0;
}
