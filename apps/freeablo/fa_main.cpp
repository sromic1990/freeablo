// clang-format off
#include <misc/disablewarn.h>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <misc/enablewarn.h>
// clang-format on
#include <iostream>
#include <faio/fafileobject.h>
#include <settings/settings.h>
#include <diabloexe/diabloexe.h>
#include "engine/enginemain.h"

namespace bpo = boost::program_options;
bool parseOptions(int argc, char** argv, bpo::variables_map& variables)
{
    boost::program_options::options_description desc("Options");
    desc.add_options()("help,h", "Print help")
        // -1 represents the main menu
        ("level,l", bpo::value<int32_t>()->default_value(-1), "Level number to load (0-16)")(
            "character,c", bpo::value<std::string>()->default_value("Warrior"), "Choose Warrior, Rogue or Sorcerer")(
            "invuln", bpo::value<std::string>()->default_value("off"), "on or off")("client", bpo::bool_switch()->default_value(false));

    try
    {
        bpo::store(bpo::parse_command_line(argc, argv, desc), variables);

        if (variables.count("help"))
        {
            std::cout << desc << std::endl;
            return false;
        }

        bpo::notify(variables);

        const int32_t dLvl = variables["level"].as<int32_t>();

        if (dLvl > 16)
            throw bpo::error("There is no level after 16");
    }
    catch (bpo::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return false;
    }

    return true;
}

bool dataFilesSetUp(const Settings::Settings& settings)
{
    std::string mpqPath = settings.get<std::string>("Game", "PathMPQ");
    std::string exePath = settings.get<std::string>("Game", "PathEXE");

    if (mpqPath.empty() || exePath.empty())
        return false;

    if (DiabloExe::DiabloExe::getVersion(exePath).empty())
        return false;

    // TODO: validate mpq MD5 as well.
    // Not sure if this changes across versions.

    return true;
}

int fa_main(int argc, char** argv)
{
    Settings::Settings settings;

    // Check if we've been configured with data files, and if we haven't, run the launcher to prompt configuration
    if (!(settings.loadUserSettings() && dataFilesSetUp(settings)))
    {
        system((boost::filesystem::system_complete(argv[0]).parent_path() / "launcher").string().c_str());
        return EXIT_SUCCESS;
    }

    if (!FAIO::init(settings.get<std::string>("Game", "PathMPQ")))
        return EXIT_FAILURE;

    Engine::EngineMain engine;

    int retval = EXIT_SUCCESS;

    boost::program_options::variables_map variables;
    if (parseOptions(argc, argv, variables))
        engine.run(variables);
    else
        retval = EXIT_FAILURE;

    FAIO::FAFileObject::quit();
    return retval;
}
