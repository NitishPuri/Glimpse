#include "glimpse/util/image.h"

#ifdef _WIN32
#define NOMINMAX
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#include <algorithm>
#include <cstring>
#include <string>

using namespace glimpse;

// Setup console for Unicode output (for Windows)
void setupConsoleForUnicode() {
#ifdef _WIN32
  // Set console code page to UTF-8
  SetConsoleOutputCP(CP_UTF8);

  // Enable Unicode in C++ standard streams
  _setmode(_fileno(stdout), _O_U16TEXT);

  // Alternative approach if the above doesn't work
  // SetConsoleOutputCP(65001); // UTF-8 code page
#endif
}

// Print an image to console using ASCII characters
void printAsciiImage(const Image& img, int maxWidth = 80, int maxHeight = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight);

  // Calculate sampling step (how many pixels to skip)
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  // ASCII character set from dark to bright
  const std::string chars = " .:-=+*#%@";

  std::cout << "\nASCII Image Preview (" << displayWidth << "x" << displayHeight << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < displayHeight; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel and calculate brightness
      ImageColor pixel = img.get(origX, origY);

      // Calculate perceived brightness (human eye is more sensitive to green)
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      // Map brightness to ASCII character
      int charIndex = static_cast<int>(brightness * (chars.length() - 1));
      charIndex = std::min(0, std::max(static_cast<int>(chars.length()) - 1, charIndex));

      // TODO: Experiment with character sets for different images
      //  Get dominant color (R, G, B)
      char colorCode = ' ';
      if (pixel.rgb[0] > pixel.rgb[1] && pixel.rgb[0] > pixel.rgb[2] && pixel.rgb[0] > 100)
        colorCode = 'R';
      else if (pixel.rgb[1] > pixel.rgb[0] && pixel.rgb[1] > pixel.rgb[2] && pixel.rgb[1] > 100)
        colorCode = 'G';
      else if (pixel.rgb[2] > pixel.rgb[0] && pixel.rgb[2] > pixel.rgb[1] && pixel.rgb[2] > 100)
        colorCode = 'B';

      // Alternative: use ASCII color code with brightness
      if (colorCode != ' ' && chars[charIndex] != ' ') {
        std::cout << colorCode;
      } else {
        std::cout << chars[charIndex];
      }
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print an image to console using colored ASCII characters (with ANSI colors)
void printColorAsciiImage(const Image& img, int maxWidth = 120, int maxHeight = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight);

  // Calculate sampling step (how many pixels to skip)
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  // Use characters that are more likely to work across different terminals
  // ASCII only version as fallback
  const std::string asciiChars = " .:-=+*#%@";

  // Block characters - using simple blocks more likely to be supported
  const std::string blockChars = "\xDB\xDB";  // Space and full block

  // ASCII character set from dark to bright
  //   const std::string chars = " .:-=+*#%@";
  //   const std::string chars = "■■■";                       // \u0444
  // Choose character set (use asciiChars if having problems with blockChars)
  const std::string& chars = blockChars;

  std::cout << "\nColored ASCII Image Preview (" << displayWidth << "x" << displayHeight << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < displayHeight; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel/
      // TODO: Actually downsample around the pixel and calculate average color, or gaussian blur
      ImageColor pixel = img.get(origX, origY);

      // Calculate brightness and select character
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      int charIndex = static_cast<int>(brightness * (chars.length() - 1));
      charIndex = std::max(0, std::min(static_cast<int>(chars.length()) - 1, charIndex));

      // More accurate color determination
      int r = pixel.rgb[0];
      int g = pixel.rgb[1];
      int b = pixel.rgb[2];

      // Calculate color ratios
      double total = r + g + b;
      if (total < 30) {
        // Very dark - just use black text
        std::cout << "\033[30m" << chars[charIndex] << "\033[0m";
        continue;
      }

      double rRatio = r / total;
      double gRatio = g / total;
      double bRatio = b / total;

      // Set thresholds for color detection
      const double colorThreshold = 0.4;     // Color needs to be at least this % of the total
      const double neutralThreshold = 0.32;  // If all colors are about this even, it's grayscale

      // Determine ANSI color code (simplifying to 8 colors)
      int ansiColor;
#if 0
      // Simplified color mapping - prioritize the dominant color channel
      if (pixel.rgb[0] > pixel.rgb[1] && pixel.rgb[0] > pixel.rgb[2]) {
        // Red dominant
        if (pixel.rgb[1] > 128 && pixel.rgb[2] > 128)
          ansiColor = 97;  // Bright white
        else if (pixel.rgb[1] > 128)
          ansiColor = 93;  // Bright yellow
        else if (pixel.rgb[2] > 128)
          ansiColor = 95;  // Bright magenta
        else
          ansiColor = 91;  // Bright red
      } else if (pixel.rgb[1] > pixel.rgb[0] && pixel.rgb[1] > pixel.rgb[2]) {
        // Green dominant
        if (pixel.rgb[0] > 128 && pixel.rgb[2] > 128)
          ansiColor = 97;  // Bright white
        else if (pixel.rgb[0] > 128)
          ansiColor = 93;  // Bright yellow
        else if (pixel.rgb[2] > 128)
          ansiColor = 96;  // Bright cyan
        else
          ansiColor = 92;  // Bright green
      } else if (pixel.rgb[2] > pixel.rgb[0] && pixel.rgb[2] > pixel.rgb[1]) {
        // Blue dominant
        if (pixel.rgb[0] > 128 && pixel.rgb[1] > 128)
          ansiColor = 97;  // Bright white
        else if (pixel.rgb[0] > 128)
          ansiColor = 95;  // Bright magenta
        else if (pixel.rgb[1] > 128)
          ansiColor = 96;  // Bright cyan
        else
          ansiColor = 94;  // Bright blue
      } else {
        // No clear dominant - grayscale
        if (brightness > 0.7)
          ansiColor = 97;  // Bright white
        else if (brightness > 0.4)
          ansiColor = 37;  // White
        else if (brightness > 0.2)
          ansiColor = 90;  // Bright black (gray)
        else
          ansiColor = 30;  // Black
      }
#endif

#if 1

      // Check if it's grayscale (all colors relatively even)
      if (rRatio > neutralThreshold && gRatio > neutralThreshold && bRatio > neutralThreshold) {
        // Grayscale - choose based on brightness
        if (brightness > 0.8)
          ansiColor = 97;  // Bright white
        else if (brightness > 0.5)
          ansiColor = 37;  // White
        else if (brightness > 0.25)
          ansiColor = 90;  // Bright black (gray)
        else
          ansiColor = 30;  // Black
      }
      // Check for primary and secondary colors
      else if (rRatio > colorThreshold && gRatio > colorThreshold && bRatio < 0.2) {
        // Yellow (red + green)
        ansiColor = (brightness > 0.6) ? 93 : 33;
      } else if (rRatio > colorThreshold && bRatio > colorThreshold && gRatio < 0.2) {
        // Magenta (red + blue)
        ansiColor = (brightness > 0.6) ? 95 : 35;
      } else if (gRatio > colorThreshold && bRatio > colorThreshold && rRatio < 0.2) {
        // Cyan (green + blue)
        ansiColor = (brightness > 0.6) ? 96 : 36;
      } else if (rRatio > colorThreshold && rRatio > gRatio && rRatio > bRatio) {
        // Red dominant
        ansiColor = (brightness > 0.6) ? 91 : 31;
      } else if (gRatio > colorThreshold && gRatio > rRatio && gRatio > bRatio) {
        // Green dominant
        ansiColor = (brightness > 0.6) ? 92 : 32;
      } else if (bRatio > colorThreshold && bRatio > rRatio && bRatio > gRatio) {
        // Blue dominant
        ansiColor = (brightness > 0.6) ? 94 : 34;
      } else {
        // Fallback to grayscale
        if (brightness > 0.7)
          ansiColor = 97;  // Bright white
        else if (brightness > 0.4)
          ansiColor = 37;  // White
        else if (brightness > 0.2)
          ansiColor = 90;  // Bright black (gray)
        else
          ansiColor = 30;  // Black
      }

#endif
      // Print colored character using ANSI escape codes   ■
      std::cout << "\033[" << ansiColor << "m" << chars[charIndex] << "\033[0m";
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print an image to console with enhanced contrast
void printEnhancedAsciiImage(const Image& img, int maxWidth = 80, int maxHeight = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight);

  // Calculate sampling step (how many pixels to skip)
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  // Analyze the image first to determine brightness range
  double minBrightness = 1.0;
  double maxBrightness = 0.0;

  // First pass - find brightness range
  for (int y = 0; y < displayHeight; y++) {
    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel
      ImageColor pixel = img.get(origX, origY);

      // Calculate brightness
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      //   std::cout << "Color: " << static_cast<int>(pixel.rgb[0]) << "," << static_cast<int>(pixel.rgb[1]) << ","
      //             << static_cast<int>(pixel.rgb[2]) << " Brightness: " << brightness << std::endl;
      brightness /= 255.0;  // Normalize to 0-1

      // Update min/max
      minBrightness = std::min(minBrightness, brightness);
      maxBrightness = std::max(maxBrightness, brightness);
    }
  }

  // Prevent division by zero
  double brightnessRange = maxBrightness - minBrightness;
  if (brightnessRange < 0.05) {
    brightnessRange = 1.0;
    minBrightness = 0.0;
  }

  std::cout << "\nEnhanced ASCII Image Preview (" << displayWidth << "x" << displayHeight << ")\n";
  std::cout << "Brightness range: " << minBrightness << " - " << maxBrightness << "\n";

  // Wider range of characters for better differentiation
  const std::string chars = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < displayHeight; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel
      ImageColor pixel = img.get(origX, origY);

      // Calculate brightness with enhanced contrast
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      // Apply contrast enhancement
      double enhancedBrightness = (brightness - minBrightness) / brightnessRange;
      enhancedBrightness = std::max(0.0, std::min(1.0, enhancedBrightness));  // Clamp to 0-1

      // Map brightness to character
      int charIndex = static_cast<int>(enhancedBrightness * (chars.length() - 1));
      charIndex = std::max(0, std::min(static_cast<int>(chars.length() - 1), charIndex));

      // Determine colors
      int r = pixel.rgb[0];
      int g = pixel.rgb[1];
      int b = pixel.rgb[2];

      // Calculate color ratios
      double total = r + g + b;
      if (total < 30) {
        // Very dark
        std::cout << ' ';
        continue;
      }

      double rRatio = r / total;
      double gRatio = g / total;
      double bRatio = b / total;

      const double colorThreshold = 0.4;
      const double neutralThreshold = 0.3;

      // Print character with color indication
      if (rRatio > neutralThreshold && gRatio > neutralThreshold && bRatio > neutralThreshold) {
        // Grayscale - just use the character
        std::cout << chars[charIndex];
      } else {
        // Color-coding
        char colorIndicator;
        if (rRatio > colorThreshold && gRatio > colorThreshold)
          colorIndicator = 'Y';
        else if (rRatio > colorThreshold && bRatio > colorThreshold)
          colorIndicator = 'M';
        else if (gRatio > colorThreshold && bRatio > colorThreshold)
          colorIndicator = 'C';
        else if (rRatio > colorThreshold && rRatio > gRatio && rRatio > bRatio)
          colorIndicator = 'R';
        else if (gRatio > colorThreshold && gRatio > rRatio && gRatio > bRatio)
          colorIndicator = 'G';
        else if (bRatio > colorThreshold && bRatio > rRatio && bRatio > gRatio)
          colorIndicator = 'B';
        else
          colorIndicator = chars[charIndex];

        // Only replace with color code if the character has enough "weight"
        if (chars[charIndex] != ' ' && chars[charIndex] != '.') {
          std::cout << colorIndicator;
        } else {
          std::cout << chars[charIndex];
        }
      }
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n\n";

  std::cout << "Color Legend: R=Red, G=Green, B=Blue, Y=Yellow, M=Magenta, C=Cyan\n\n";
}

// Print an image using true color (24-bit RGB) background colors
void printTruecolorImage(const Image& img, int maxWidth = 60, int maxHeight = 30) {
  // Background colors need more horizontal space, so we use smaller defaults
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight);

  // Calculate sampling step (how many pixels to skip)
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  std::cout << "\nTrue Color Image Preview (" << displayWidth << "x" << displayHeight << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth * 2; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < displayHeight; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel
      ImageColor pixel = img.get(origX, origY);
      int r = pixel.rgb[0];
      int g = pixel.rgb[1];
      int b = pixel.rgb[2];

      // Print a space with true color background
      // ESC[ 48;2;⟨r⟩;⟨g⟩;⟨b⟩ m - Set background to RGB color
      std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m  \033[0m";
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth * 2; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Enable Windows ANSI colors
void enableWindowsAnsiColors() {
#ifdef _WIN32
  // For Windows 10+, enable ANSI processing
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }
  }
#endif
}

// Print an image using console background colors
void printPixelImage(const Image& img, int maxWidth = 60, int maxHeight = 30) {
  // Background colors need more horizontal space, so we use smaller defaults
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight);

  // Calculate sampling step (how many pixels to skip)
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  std::cout << "\nPixel Image Preview (" << displayWidth << "x" << displayHeight << "):\n";

  // Each console "pixel" is two spaces with background color to make it more square
  const std::string pixel = "  ";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth * 2; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < displayHeight; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel
      ImageColor pixelColor = img.get(origX, origY);
      int r = pixelColor.rgb[0];
      int g = pixelColor.rgb[1];
      int b = pixelColor.rgb[2];

      // We need to map RGB to console colors
      // This maps RGB to the closest ANSI background color
      int bgColor;

      // Very dark pixels - black background
      if (r < 30 && g < 30 && b < 30) {
        bgColor = 40;  // Black background
      }
      // Bright white pixels
      else if (r > 200 && g > 200 && b > 200) {
        bgColor = 47;  // White background
      }
      // Determine dominant color and map to ANSI background code
      else {
        double total = r + g + b;
        double rRatio = r / total;
        double gRatio = g / total;
        double bRatio = b / total;

        const double colorThreshold = 0.38;

        if (rRatio > colorThreshold && gRatio > colorThreshold && bRatio < 0.2) {
          bgColor = 43;  // Yellow background
        } else if (rRatio > colorThreshold && bRatio > colorThreshold && gRatio < 0.2) {
          bgColor = 45;  // Magenta background
        } else if (gRatio > colorThreshold && bRatio > colorThreshold && rRatio < 0.2) {
          bgColor = 46;  // Cyan background
        } else if (rRatio > colorThreshold && rRatio > gRatio && rRatio > bRatio) {
          bgColor = 41;  // Red background
        } else if (gRatio > colorThreshold && gRatio > rRatio && gRatio > bRatio) {
          bgColor = 42;  // Green background
        } else if (bRatio > colorThreshold && bRatio > rRatio && bRatio > gRatio) {
          bgColor = 44;  // Blue background
        } else {
          // Fallback to grayscale
          double brightness = 0.299 * r + 0.587 * g + 0.114 * b;
          brightness /= 255.0;

          if (brightness > 0.7) {
            bgColor = 47;  // White background
          } else if (brightness > 0.4) {
            bgColor = 47;  // White background (use same as above for simplicity)
          } else if (brightness > 0.2) {
            bgColor = 100;  // Bright black (gray) background
          } else {
            bgColor = 40;  // Black background
          }
        }
      }

      // Use bright background colors for more vibrant display
      if (bgColor >= 40 && bgColor <= 47) {
        double brightness = 0.299 * r + 0.587 * g + 0.114 * b;
        brightness /= 255.0;

        if (brightness > 0.5 && bgColor != 47) {  // Not already white
          bgColor += 60;                          // Convert to bright background color
        }
      }

      // Print a space with the selected background color
      std::cout << "\033[" << bgColor << "m  \033[0m";
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth * 2; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print image choosing the best method for the terminal
void printBestImage(const Image& img, int maxWidth = 60, int maxHeight = 30) {
  // Enable ANSI colors for Windows
  enableWindowsAnsiColors();

  // Attempt to determine terminal capabilities
  bool supportsTruecolor = false;
  bool supportsAnsiColors = true;  // Assume basic ANSI colors are supported

  // Check environment variables for true color support
  char* colorterm = getenv("COLORTERM");
  if (colorterm && (strcmp(colorterm, "truecolor") == 0 || strcmp(colorterm, "24bit") == 0)) {
    supportsTruecolor = true;
  }

  // Check for TERM variable indicating advanced terminal
  char* term = getenv("TERM");
  if (term && (strstr(term, "256color") != NULL || strstr(term, "xterm") != NULL || strstr(term, "konsole") != NULL)) {
    supportsAnsiColors = true;  // Definitely supports ANSI colors
  }

  // Windows Terminal and newer Windows 10+ consoles often support true color
#ifdef _WIN32
  // Windows 10 version 1703+ should support true color in compatible terminals
  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  // Using the Windows Terminal app is a good indication of true color support
  char* wt = getenv("WT_SESSION");
  if (wt != NULL) {
    supportsTruecolor = true;
  }
#endif

  // Choose the best rendering method
  if (supportsTruecolor) {
    printTruecolorImage(img, maxWidth, maxHeight);
  } else if (supportsAnsiColors) {
    printPixelImage(img, maxWidth, maxHeight);
  } else {
    // Fallback to basic ASCII
    printEnhancedAsciiImage(img, maxWidth * 2, maxHeight);
  }
}

// High-resolution console renderer for Windows
void printHighresImage(const Image& img, int maxWidth = 120, int maxHeight = 60) {
#ifdef _WIN32
  // Get console handle
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to get console handle" << std::endl;
    return;
  }

  // Get console information
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
    std::cerr << "Failed to get console info" << std::endl;
    return;
  }

  // Calculate available space
  int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  int consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

  // Determine display dimensions (leave space for borders)
  int displayWidth = std::min(img.width, std::min(maxWidth, consoleWidth - 2));
  int displayHeight = std::min(img.height, std::min(maxHeight, consoleHeight - 3));

  // Calculate sampling step
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  std::cout << "\nHigh Resolution Console Image (" << displayWidth << "x" << displayHeight << "):\n";

  // Get current cursor position
  COORD cursorPos = csbi.dwCursorPosition;
  cursorPos.Y += 2;  // Move down two lines for the header

  // Create buffers for characters and attributes
  std::vector<CHAR_INFO> buffer(displayWidth * displayHeight);

  // Fill the buffer with pixels
  for (int y = 0; y < displayHeight; y++) {
    for (int x = 0; x < displayWidth; x++) {
      // Sample the original image
      int origX = static_cast<int>(x * xStep);
      int origY = static_cast<int>(y * yStep);

      // Get pixel
      ImageColor pixel = img.get(origX, origY);
      CHAR_INFO& charInfo = buffer[y * displayWidth + x];

      // Calculate color indices (map 24-bit color to console colors)
      int r = pixel.rgb[0];
      int g = pixel.rgb[1];
      int b = pixel.rgb[2];

      // Map RGB to console color attributes - 16 color mode
      int fgColor = 0;
      int bgColor = 0;

      // Determine foreground color (for the character)
      if (r > 192 && g > 192 && b > 192) {
        fgColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      } else if (r > 192 && g > 192) {
        fgColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
      } else if (r > 192 && b > 192) {
        fgColor = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      } else if (g > 192 && b > 192) {
        fgColor = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      } else if (r > 192) {
        fgColor = FOREGROUND_RED | FOREGROUND_INTENSITY;
      } else if (g > 192) {
        fgColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
      } else if (b > 192) {
        fgColor = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      } else if (r > 128 || g > 128 || b > 128) {
        fgColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      } else {
        fgColor = 0;
      }

      // Determine background color
      if (r > 128 && g > 128 && b > 128) {
        bgColor = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
      } else if (r > 128 && g > 128) {
        bgColor = BACKGROUND_RED | BACKGROUND_GREEN;
      } else if (r > 128 && b > 128) {
        bgColor = BACKGROUND_RED | BACKGROUND_BLUE;
      } else if (g > 128 && b > 128) {
        bgColor = BACKGROUND_GREEN | BACKGROUND_BLUE;
      } else if (r > 128) {
        bgColor = BACKGROUND_RED;
      } else if (g > 128) {
        bgColor = BACKGROUND_GREEN;
      } else if (b > 128) {
        bgColor = BACKGROUND_BLUE;
      } else {
        bgColor = 0;
      }

      // Use different characters based on brightness to increase perceived resolution
      double brightness = 0.299 * r + 0.587 * g + 0.114 * b;
      brightness /= 255.0;

      // Choose character based on brightness
      char displayChar;
      if (brightness > 0.80)
        displayChar = static_cast<char>(219);  // █ Full block
      else if (brightness > 0.60)
        displayChar = static_cast<char>(178);  // ▓ Dark shade
      else if (brightness > 0.40)
        displayChar = static_cast<char>(177);  // ▒ Medium shade
      else if (brightness > 0.20)
        displayChar = static_cast<char>(176);  // ░ Light shade
      else
        displayChar = ' ';  // Space

      // Set character and attributes
      charInfo.Char.AsciiChar = displayChar;
      charInfo.Attributes = static_cast<WORD>(fgColor | bgColor);
    }
  }

  // Define the area to write to
  SMALL_RECT writeRegion;
  writeRegion.Left = static_cast<SHORT>(cursorPos.X);
  writeRegion.Top = static_cast<SHORT>(cursorPos.Y);
  writeRegion.Right = static_cast<SHORT>(cursorPos.X + displayWidth - 1);
  writeRegion.Bottom = static_cast<SHORT>(cursorPos.Y + displayHeight - 1);

  // Write the buffer to the console
  if (!WriteConsoleOutput(hConsole, buffer.data(),
                          {static_cast<SHORT>(displayWidth), static_cast<SHORT>(displayHeight)}, {0, 0},
                          &writeRegion)) {
    std::cerr << "Failed to write to console buffer" << std::endl;
  }

  // Move cursor below the image
  cursorPos.Y += static_cast<SHORT>(displayHeight + 1);
  SetConsoleCursorPosition(hConsole, cursorPos);
  std::cout << std::endl;
#else
  // Fallback for non-Windows platforms
  std::cout << "High-resolution console rendering is only supported on Windows" << std::endl;
  printTruecolorImage(img, maxWidth, maxHeight);
#endif
}

// Ultra-high resolution console renderer using half-block characters
void printUltrahdConsoleImage(const Image& img, int maxWidth = 80, int maxHeight = 40) {
#ifdef _WIN32
  // Enable ANSI colors first
  enableWindowsAnsiColors();

  // Calculate dimensions
  int displayWidth = std::min(img.width, maxWidth);
  int displayHeight = std::min(img.height, maxHeight) * 2;  // Double vertical res with half-blocks

  // Calculate sampling step
  double xStep = static_cast<double>(img.width) / displayWidth;
  double yStep = static_cast<double>(img.height) / displayHeight;

  std::cout << "\nUltra HD Console Image (" << displayWidth << "x" << (displayHeight / 2) << " doubled):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n";

  // Process two rows at a time using upper and lower half blocks
  //   for (int y = 0; y < displayHeight; y += 2) {
  for (int y = displayHeight - 1; y >= 0; y -= 2) {
    std::cout << "|";  // Left border

    for (int x = 0; x < displayWidth; x++) {
      // Get upper and lower pixels
      int origX = static_cast<int>(x * xStep);

      int origYUpper = static_cast<int>(y * yStep);
      int origYLower = static_cast<int>((y + 1) * yStep);

      // Handle edge case for odd height images
      if (origYLower >= img.height) {
        origYLower = img.height - 1;
      }

      ImageColor upperPixel = img.get(origX, origYUpper);
      ImageColor lowerPixel = img.get(origX, origYLower);

      // Extract RGB values
      int upperR = upperPixel.rgb[0];
      int upperG = upperPixel.rgb[1];
      int upperB = upperPixel.rgb[2];

      int lowerR = lowerPixel.rgb[0];
      int lowerG = lowerPixel.rgb[1];
      int lowerB = lowerPixel.rgb[2];

      // Use half block characters and true color ANSI codes
      // ▀ (upper half block) for upper pixel
      // Set foreground color to upper pixel and background color to lower pixel

      // ESC[ 38;2;⟨r⟩;⟨g⟩;⟨b⟩ m - Set foreground to RGB color
      // ESC[ 48;2;⟨r⟩;⟨g⟩;⟨b⟩ m - Set background to RGB color
      std::cout << "\033[38;2;" << upperR << ";" << upperG << ";" << upperB << "m"
                << "\033[48;2;" << lowerR << ";" << lowerG << ";" << lowerB << "m"
                << "▀"         // Upper half block character
                << "\033[0m";  // Reset formatting
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < displayWidth; x++) std::cout << "-";
  std::cout << "+\n\n";
#else
  // Fallback for non-Windows platforms
  std::cout << "Ultra HD console rendering is using true color ANSI codes" << std::endl;
  printTruecolorImage(img, maxWidth, maxHeight);
#endif
}