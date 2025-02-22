#pragma once
#include <string>
#include <vector>
#include <iostream>

struct CmdOptions
{
    int width = 500;
    int height = 500;
    int samples = 100;
    int max_depth = 50;
    int scene = 1;
};

CmdOptions ParseCommandLine(int argc, char *argv[])
{
    CmdOptions options;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--width" && i + 1 < argc)
        {
            options.width = std::stoi(argv[++i]);
        }
        else if (arg == "--height" && i + 1 < argc)
        {
            options.height = std::stoi(argv[++i]);
        }
        else if (arg == "--samples" && i + 1 < argc)
        {
            options.samples = std::stoi(argv[++i]);
        }
        else if (arg == "--scene" && i + 1 < argc)
        {
            options.scene = std::stoi(argv[++i]);
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << std::endl;
        }
    }

    return options;
}
