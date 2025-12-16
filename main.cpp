#include <random>
#include <string_view>
#include <fstream>
#include <iostream>

#include "src/string_art/StringArt.h"
#include "src/utils/io_utils.h"

using namespace std::string_view_literals;

int main(int argc, char *argv[])
{
    std::string_view inputImage = "input.png";
    std::string_view outputImage = "output.png";
    std::string_view inputSequence = "sequence.txt";
    std::string_view outputSequence = "sequence.txt";
    int countNails = 300;
    int maxIterations = 4500;
    int kDensity = 500;
    bool isLoadSequence = false;
    bool isApplySobel = false;
    float alpha = 0.15f;

    std::vector<Color> colors =
        {
            Color{{0, 0, 0}},       // Black
            Color{{255, 255, 255}}, // White
            Color{{255, 0, 0}},     // Red
            Color{{0, 255, 0}},     // Green
            Color{{0, 0, 255}},     // Blue
            Color{{255, 0, 255}},   // Magenta
            Color{{0, 255, 255}},   // Cyan
            Color{{255, 255, 0}}    // Yellow
        };

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: ./StringArt.exe [options]\n\n"
                      << "Options:\n"
                      << "  --input-image <path>       Input image path (default: input.png)\n"
                      << "  --output-image <path>      Output image path (default: output.png)\n"
                      << "  --input-sequence <path>    Input sequence file (default: sequence.txt)\n"
                      << "  --output-sequence <path>   Output sequence file (default: sequence.txt)\n"
                      << "  --count-nails <number>     Number of nails (default: 300)\n"
                      << "  --max-iterations <number>  Maximum iterations (default: 4500)\n"
                      << "  --k-density <number>       Density parameter (default: 500)\n"
                      << "  --load-sequence            Load existing sequence\n"
                      << "  --sobel                    Apply Sobel filter\n"
                      << "  --alpha <number>           Alpha blending value (default: 0.15)\n"
                      << "  --colors \"r:g:b;r:g:b...\"  Custom colors in RGB format\n"
                      << "  --help -h                  Show this help message\n\n"
                      << "Color format example: \"0:0:0;255:255:255\" for black and white\n";
            return 0;
        }
        else if (arg == "--input-image" && i + 1 < argc)
        {
            inputImage = argv[++i];
        }
        else if (arg == "--output-image" && i + 1 < argc)
        {
            outputImage = argv[++i];
        }
        else if (arg == "--input-sequence" && i + 1 < argc)
        {
            inputSequence = argv[++i];
        }
        else if (arg == "--output-sequence" && i + 1 < argc)
        {
            outputSequence = argv[++i];
        }
        else if (arg == "--count-nails" && i + 1 < argc)
        {
            countNails = std::stoi(argv[++i]);
        }
        else if (arg == "--max-iterations" && i + 1 < argc)
        {
            maxIterations = std::stoi(argv[++i]);
        }
        else if (arg == "--k-density" && i + 1 < argc)
        {
            kDensity = std::stoi(argv[++i]);
        }
        else if (arg == "--load-sequence")
        {
            isLoadSequence = true;
        }
        else if (arg == "--sobel")
        {
            isApplySobel = true;
        }
        else if (arg == "--alpha" && i + 1 < argc)
        {
            alpha = std::stof(argv[++i]);
        }
        else if (arg == "--colors" && i + 1 < argc)
        {
            std::string colorArg = argv[++i];
            std::vector<std::string> colorStrings = split(colorArg, ';');
            colors.clear();
            for (const auto &colorStr : colorStrings)
            {
                std::vector<std::string> components = split(colorStr, ':');
                if (components.size() != 3)
                {
                    std::cerr << "Invalid color format: " << colorStr << std::endl;
                    return 1;
                }
                uint8_t r = static_cast<uint8_t>(std::stoi(components[0]));
                uint8_t g = static_cast<uint8_t>(std::stoi(components[1]));
                uint8_t b = static_cast<uint8_t>(std::stoi(components[2]));
                colors.emplace_back(Color{{r, g, b}});
            }
        }
        else
        {
            std::cerr << "Unknown argument or missing value: " << arg << std::endl;
        }
    }

    Image input;
    input.load(inputImage);

    std::mt19937 gen(std::random_device{}());
    StringArtGenerator generator(
        input,
        countNails,
        maxIterations,
        colors,
        kDensity,
        isApplySobel,
        gen);

    if (isLoadSequence)
    {
        auto sequences = generator.loadSequence(inputSequence);
        auto result = generator.rebuildFromSequence(sequences, alpha);
        result.save(outputImage);
    }
    else
    {
        auto result = generator.generate(alpha);

        result.first.save(outputImage);

        std::ofstream fout(outputSequence.data());
        for (auto &&[color, nailInd] : result.second)
        {
            int r = color[0];
            int g = color[1];
            int b = color[2];
            fout << r << ' ' << g << ' ' << b << ' ' << nailInd << '\n';
        }
    }

    return 0;
}
