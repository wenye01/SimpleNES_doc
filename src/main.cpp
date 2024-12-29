#include "Emulator.h"
#include "Log.h"
#include <string>
#include <sstream>

namespace _NES
{
    void parseControllerConf(std::string filepath,
                            std::vector<sf::Keyboard::Key>& p1,
                            std::vector<sf::Keyboard::Key>& p2);
}

int main(int argc, char** argv)
{
    std::ofstream logFile ("simplenes.log"), cpuTraceFile;
    _NES::TeeStream logTee (logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        _NES::Log::get().setLogStream(logTee);
    else
        _NES::Log::get().setLogStream(std::cout);

    _NES::Log::get().setLevel(_NES::Info);

    std::string path;
    std::string keybindingsPath = "keybindings.conf";

    //Default keybindings
    std::vector<sf::Keyboard::Key> p1 {sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::RShift, sf::Keyboard::Return,
                                       sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D},
                                   p2 {sf::Keyboard::Numpad5, sf::Keyboard::Numpad6, sf::Keyboard::Numpad8, sf::Keyboard::Numpad9,
                                       sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right};
    _NES::Emulator emulator;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg (argv[i]);
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "SimpleNES is a simple NES emulator.\n"
                      << "It can run off .nes images.\n"
                      << "Set keybindings with keybindings.conf\n\n"
                      << "Usage: SimpleNES [options] rom-path\n\n"
                      << "Options:\n"
                      << "-h, --help             Print this help text and exit\n"
                      << "-s, --scale            Set video scale. Default: 3.\n"
                      << "                       Scale of 1 corresponds to " << _NES::NESVideoWidth << "x" << _NES::NESVideoHeight << std::endl
                      << "-w, --width            Set the width of the emulation screen (height is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "-H, --height           Set the height of the emulation screen (width is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "                       This option is mutually exclusive to --width\n"
                      << "-C, --conf             Set the keybindings file's path. The default \n"
                      << "                       keybindings file is keybindings.conf.\n"
                      << std::endl;
            return 0;
        }
        else if (std::strcmp(argv[i], "--log-cpu") == 0)
        {
            _NES::Log::get().setLevel(_NES::CpuTrace);
            cpuTraceFile.open("_NES.cpudump");
            _NES::Log::get().setCpuTraceStream(cpuTraceFile);
            LOG(_NES::Info) << "CPU logging set." << std::endl;
        }
        else if (std::strcmp(argv[i], "-s") == 0 || std::strcmp(argv[i], "--scale") == 0)
        {
            float scale;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> scale)
                emulator.setVideoScale(scale);
            else
                LOG(_NES::Error) << "Setting scale from argument failed" << std::endl;
            ++i;
        }
        else if (std::strcmp(argv[i], "-w") == 0 || std::strcmp(argv[i], "--width") == 0)
        {
            int width;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> width)
                emulator.setVideoWidth(width);
            else
                LOG(_NES::Error) << "Setting width from argument failed" << std::endl;
            ++i;
        }
        else if (std::strcmp(argv[i], "-H") == 0 || std::strcmp(argv[i], "--height") == 0)
        {
            int height;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> height)
                emulator.setVideoHeight(height);
            else
                LOG(_NES::Error) << "Setting height from argument failed" << std::endl;
            ++i;
        }
        else if (std::strcmp(argv[i], "-C") == 0 || std::strcmp(argv[i], "--conf") == 0) {
            if (i + 1 < argc)
                keybindingsPath = argv[i + 1];
            else
                LOG(_NES::Error) << "Setting keybindings.conf's path from argument failed" << std::endl;
            ++i;
        }
        else if (argv[i][0] != '-')
            path = argv[i];
        else
            std::cerr << "Unrecognized argument: " << argv[i] << std::endl;
    }

    if (path.empty())
    {
        std::cout << "Argument required: ROM path" << std::endl;
        return 1;
    }

    _NES::parseControllerConf(std::move(keybindingsPath), p1, p2);
    emulator.setKeys(p1, p2);
    emulator.run(path);
    return 0;
}
