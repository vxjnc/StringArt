# StringArt: Image to Thread Art Generator

**Author:** [vxjnc](https://github.com/vxjnc)
**License:** [MIT License](LICENSE)

**StringArt** is a high-performance command-line tool written in C++ that transforms standard raster images into stunning representations of thread art (string art) by calculating the optimal sequence of threads (lines) between fixed points (pegs).

## ⚙️ Building the Project

This project uses a standard `Makefile` for compilation.

### Prerequisites (Debian/Ubuntu)

Ensure you have a C++ compiler and the `make` utility installed:

```bash
sudo apt update
sudo apt install build-essential
```

### Compilation

Navigate to the project's root directory and simply run:

```bash
make
```

This command compiles the source files and generates the main executable file named `string_art`.

## 🚀 Usage

The generated executable string_art processes images based on command-line arguments, specifying input, output, and processing parameters.

Command-Line Options

You can view the full list of options by running the program with the -h or --help flag:

./string_art -h

| Option | Description | Default Value |
| :---: | :---: | :---: |
| --input-image <path> | Input image path. | input.png |
| --output-image <path> | Output image path. | output.png |
| --input-sequence <path> | Input sequence file. | sequence.txt |
| --output-sequence <path> | Output sequence file. | sequence.txt |
| --count-nails <number> | Number of nails (pegs) around the circle. | 300 |
| --max-iterations <number> | Maximum number of lines (threads) to draw. | 4500
| --k-density <number> | Density parameter, influencing line selection. | 500.0 |
| --alpha <number> | Alpha blending value for the drawn threads (controls line visibility). | 0.15 |
| --colors "r:g:b;r:g:b..." | Custom colors in RGB format. (e.g., "0:0:0;255:255:255" for black and white). | (None/Internal default) |
| --load-sequence | Load existing sequence from --input-sequence instead of calculating a new one. | (Flag) |
| --sobel | Apply the Sobel edge detection filter to the input image before processing. | (Flag) |
| --help / -h | Show this help message. | |

### Basic Execution

To run the program, use the following syntax. The program will typically look for a configuration file or use default settings if no specific parameters are provided.

### Command Examples

Here are examples demonstrating how to generate different styles of String Art from a source image.

| Input Image | Output Image | Command Executed |
| :---: | :---: | :---: |
| <img src="assets/input.png" width="256" height="256"> | <img src="assets/output.png" width="256" height="256"> | `./string_art --input-image assets/input.png --output-image assets/output.png` |
| <img src="assets/input.png" width="256" height="256"> |<img src="assets/output-gray.png" width="256" height="256">  | `./string_art --input-image assets/input.png --output-image assets/output-gray.png --output-sequence sequence-gray.txt --colors "0:0:0"` |


## 📂 Project Structure

```
.
├── assets/                  # Example input/output images for documentation
├── build/                   # Compiled object files (ignored by Git)
├── include/                 # Third-party headers (stb)
├── src/                     # C++ Source files
├── .gitignore               # List of files to ignore
├── LICENSE                  # MIT License details
├── main.cpp                 # Main program entry point
└── Makefile                 # Build instructions
```

## ✨ Contributing

Feel free to open issues or submit pull requests if you find bugs or want to implement new features.
