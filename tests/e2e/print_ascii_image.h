#include "core/image.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#include <algorithm>

using namespace glimpse;

// Setup console for Unicode output (for Windows)
void setup_console_for_unicode() {
#ifdef _WIN32
  // Set console code page to UTF-8
  SetConsoleOutputCP(CP_UTF8);

  // Enable Unicode in C++ standard streams
  _setmode(_fileno(stdout), _O_U16TEXT);

  // Alternative approach if the above doesn't work
  // SetConsoleOutputCP(65001); // UTF-8 code page
#endif
}

int intMin(int a, int b) { return a < b ? a : b; }
int intMax(int a, int b) { return a > b ? a : b; }

// Print an image to console using ASCII characters
void print_ascii_image(const Image& img, int max_width = 80, int max_height = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height);

  // Calculate sampling step (how many pixels to skip)
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  // ASCII character set from dark to bright
  const std::string chars = " .:-=+*#%@";

  std::cout << "\nASCII Image Preview (" << display_width << "x" << display_height << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < display_height; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel and calculate brightness
      ImageColor pixel = img.get(orig_x, orig_y);

      // Calculate perceived brightness (human eye is more sensitive to green)
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      // Map brightness to ASCII character
      int char_index = static_cast<int>(brightness * (chars.length() - 1));
      char_index = intMin(0, intMax(static_cast<int>(chars.length()) - 1, char_index));

      // TODO: Experiment with character sets for different images
      //  Get dominant color (R, G, B)
      char color_code = ' ';
      if (pixel.rgb[0] > pixel.rgb[1] && pixel.rgb[0] > pixel.rgb[2] && pixel.rgb[0] > 100)
        color_code = 'R';
      else if (pixel.rgb[1] > pixel.rgb[0] && pixel.rgb[1] > pixel.rgb[2] && pixel.rgb[1] > 100)
        color_code = 'G';
      else if (pixel.rgb[2] > pixel.rgb[0] && pixel.rgb[2] > pixel.rgb[1] && pixel.rgb[2] > 100)
        color_code = 'B';

      // Alternative: use ASCII color code with brightness
      if (color_code != ' ' && chars[char_index] != ' ') {
        std::cout << color_code;
      } else {
        std::cout << chars[char_index];
      }
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print an image to console using colored ASCII characters (with ANSI colors)
void print_color_ascii_image(const Image& img, int max_width = 120, int max_height = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height);

  // Calculate sampling step (how many pixels to skip)
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  // Use characters that are more likely to work across different terminals
  // ASCII only version as fallback
  const std::string ascii_chars = " .:-=+*#%@";

  // Block characters - using simple blocks more likely to be supported
  const std::string block_chars = "\xDB\xDB";  // Space and full block

  // ASCII character set from dark to bright
  //   const std::string chars = " .:-=+*#%@";
  //   const std::string chars = "■■■";                       // \u0444
  // Choose character set (use ascii_chars if having problems with block_chars)
  const std::string& chars = block_chars;

  std::cout << "\nColored ASCII Image Preview (" << display_width << "x" << display_height << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < display_height; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel/
      // TODO: Actually downsample around the pixel and calculate average color, or gaussian blur
      ImageColor pixel = img.get(orig_x, orig_y);

      // Calculate brightness and select character
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      int char_index = static_cast<int>(brightness * (chars.length() - 1));
      char_index = intMax(0, intMin(static_cast<int>(chars.length()) - 1, char_index));

      // More accurate color determination
      int r = pixel.rgb[0];
      int g = pixel.rgb[1];
      int b = pixel.rgb[2];

      // Calculate color ratios
      double total = r + g + b;
      if (total < 30) {
        // Very dark - just use black text
        std::cout << "\033[30m" << chars[char_index] << "\033[0m";
        continue;
      }

      double r_ratio = r / total;
      double g_ratio = g / total;
      double b_ratio = b / total;

      // Set thresholds for color detection
      const double color_threshold = 0.4;     // Color needs to be at least this % of the total
      const double neutral_threshold = 0.32;  // If all colors are about this even, it's grayscale

      // Determine ANSI color code (simplifying to 8 colors)
      int ansi_color;
#if 0
      // Simplified color mapping - prioritize the dominant color channel
      if (pixel.rgb[0] > pixel.rgb[1] && pixel.rgb[0] > pixel.rgb[2]) {
        // Red dominant
        if (pixel.rgb[1] > 128 && pixel.rgb[2] > 128)
          ansi_color = 97;  // Bright white
        else if (pixel.rgb[1] > 128)
          ansi_color = 93;  // Bright yellow
        else if (pixel.rgb[2] > 128)
          ansi_color = 95;  // Bright magenta
        else
          ansi_color = 91;  // Bright red
      } else if (pixel.rgb[1] > pixel.rgb[0] && pixel.rgb[1] > pixel.rgb[2]) {
        // Green dominant
        if (pixel.rgb[0] > 128 && pixel.rgb[2] > 128)
          ansi_color = 97;  // Bright white
        else if (pixel.rgb[0] > 128)
          ansi_color = 93;  // Bright yellow
        else if (pixel.rgb[2] > 128)
          ansi_color = 96;  // Bright cyan
        else
          ansi_color = 92;  // Bright green
      } else if (pixel.rgb[2] > pixel.rgb[0] && pixel.rgb[2] > pixel.rgb[1]) {
        // Blue dominant
        if (pixel.rgb[0] > 128 && pixel.rgb[1] > 128)
          ansi_color = 97;  // Bright white
        else if (pixel.rgb[0] > 128)
          ansi_color = 95;  // Bright magenta
        else if (pixel.rgb[1] > 128)
          ansi_color = 96;  // Bright cyan
        else
          ansi_color = 94;  // Bright blue
      } else {
        // No clear dominant - grayscale
        if (brightness > 0.7)
          ansi_color = 97;  // Bright white
        else if (brightness > 0.4)
          ansi_color = 37;  // White
        else if (brightness > 0.2)
          ansi_color = 90;  // Bright black (gray)
        else
          ansi_color = 30;  // Black
      }
#endif

#if 1

      // Check if it's grayscale (all colors relatively even)
      if (r_ratio > neutral_threshold && g_ratio > neutral_threshold && b_ratio > neutral_threshold) {
        // Grayscale - choose based on brightness
        if (brightness > 0.8)
          ansi_color = 97;  // Bright white
        else if (brightness > 0.5)
          ansi_color = 37;  // White
        else if (brightness > 0.25)
          ansi_color = 90;  // Bright black (gray)
        else
          ansi_color = 30;  // Black
      }
      // Check for primary and secondary colors
      else if (r_ratio > color_threshold && g_ratio > color_threshold && b_ratio < 0.2) {
        // Yellow (red + green)
        ansi_color = (brightness > 0.6) ? 93 : 33;
      } else if (r_ratio > color_threshold && b_ratio > color_threshold && g_ratio < 0.2) {
        // Magenta (red + blue)
        ansi_color = (brightness > 0.6) ? 95 : 35;
      } else if (g_ratio > color_threshold && b_ratio > color_threshold && r_ratio < 0.2) {
        // Cyan (green + blue)
        ansi_color = (brightness > 0.6) ? 96 : 36;
      } else if (r_ratio > color_threshold && r_ratio > g_ratio && r_ratio > b_ratio) {
        // Red dominant
        ansi_color = (brightness > 0.6) ? 91 : 31;
      } else if (g_ratio > color_threshold && g_ratio > r_ratio && g_ratio > b_ratio) {
        // Green dominant
        ansi_color = (brightness > 0.6) ? 92 : 32;
      } else if (b_ratio > color_threshold && b_ratio > r_ratio && b_ratio > g_ratio) {
        // Blue dominant
        ansi_color = (brightness > 0.6) ? 94 : 34;
      } else {
        // Fallback to grayscale
        if (brightness > 0.7)
          ansi_color = 97;  // Bright white
        else if (brightness > 0.4)
          ansi_color = 37;  // White
        else if (brightness > 0.2)
          ansi_color = 90;  // Bright black (gray)
        else
          ansi_color = 30;  // Black
      }

#endif
      // Print colored character using ANSI escape codes   ■
      std::cout << "\033[" << ansi_color << "m" << chars[char_index] << "\033[0m";
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print an image to console with enhanced contrast
void print_enhanced_ascii_image(const Image& img, int max_width = 80, int max_height = 40) {
  // If image is too large, we'll downsample it to fit terminal
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height);

  // Calculate sampling step (how many pixels to skip)
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  // Analyze the image first to determine brightness range
  double min_brightness = 1.0;
  double max_brightness = 0.0;

  // First pass - find brightness range
  for (int y = 0; y < display_height; y++) {
    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel
      ImageColor pixel = img.get(orig_x, orig_y);

      // Calculate brightness
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      //   std::cout << "Color: " << static_cast<int>(pixel.rgb[0]) << "," << static_cast<int>(pixel.rgb[1]) << ","
      //             << static_cast<int>(pixel.rgb[2]) << " Brightness: " << brightness << std::endl;
      brightness /= 255.0;  // Normalize to 0-1

      // Update min/max
      min_brightness = min(min_brightness, brightness);
      max_brightness = max(max_brightness, brightness);
    }
  }

  // Prevent division by zero
  double brightness_range = max_brightness - min_brightness;
  if (brightness_range < 0.05) {
    brightness_range = 1.0;
    min_brightness = 0.0;
  }

  std::cout << "\nEnhanced ASCII Image Preview (" << display_width << "x" << display_height << ")\n";
  std::cout << "Brightness range: " << min_brightness << " - " << max_brightness << "\n";

  // Wider range of characters for better differentiation
  const std::string chars = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < display_height; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel
      ImageColor pixel = img.get(orig_x, orig_y);

      // Calculate brightness with enhanced contrast
      double brightness = 0.299 * pixel.rgb[0] + 0.587 * pixel.rgb[1] + 0.114 * pixel.rgb[2];
      brightness /= 255.0;  // Normalize to 0-1

      // Apply contrast enhancement
      double enhanced_brightness = (brightness - min_brightness) / brightness_range;
      enhanced_brightness = max(0.0, min(1.0, enhanced_brightness));  // Clamp to 0-1

      // Map brightness to character
      int char_index = static_cast<int>(enhanced_brightness * (chars.length() - 1));
      char_index = intMax(0, intMin(static_cast<int>(chars.length() - 1), char_index));

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

      double r_ratio = r / total;
      double g_ratio = g / total;
      double b_ratio = b / total;

      const double color_threshold = 0.4;
      const double neutral_threshold = 0.3;

      // Print character with color indication
      if (r_ratio > neutral_threshold && g_ratio > neutral_threshold && b_ratio > neutral_threshold) {
        // Grayscale - just use the character
        std::cout << chars[char_index];
      } else {
        // Color-coding
        char color_indicator;
        if (r_ratio > color_threshold && g_ratio > color_threshold)
          color_indicator = 'Y';
        else if (r_ratio > color_threshold && b_ratio > color_threshold)
          color_indicator = 'M';
        else if (g_ratio > color_threshold && b_ratio > color_threshold)
          color_indicator = 'C';
        else if (r_ratio > color_threshold && r_ratio > g_ratio && r_ratio > b_ratio)
          color_indicator = 'R';
        else if (g_ratio > color_threshold && g_ratio > r_ratio && g_ratio > b_ratio)
          color_indicator = 'G';
        else if (b_ratio > color_threshold && b_ratio > r_ratio && b_ratio > g_ratio)
          color_indicator = 'B';
        else
          color_indicator = chars[char_index];

        // Only replace with color code if the character has enough "weight"
        if (chars[char_index] != ' ' && chars[char_index] != '.') {
          std::cout << color_indicator;
        } else {
          std::cout << chars[char_index];
        }
      }
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n\n";

  std::cout << "Color Legend: R=Red, G=Green, B=Blue, Y=Yellow, M=Magenta, C=Cyan\n\n";
}

// Print an image using true color (24-bit RGB) background colors
void print_truecolor_image(const Image& img, int max_width = 60, int max_height = 30) {
  // Background colors need more horizontal space, so we use smaller defaults
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height);

  // Calculate sampling step (how many pixels to skip)
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  std::cout << "\nTrue Color Image Preview (" << display_width << "x" << display_height << "):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width * 2; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < display_height; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel
      ImageColor pixel = img.get(orig_x, orig_y);
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
  for (int x = 0; x < display_width * 2; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Enable Windows ANSI colors
void enable_windows_ansi_colors() {
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
void print_pixel_image(const Image& img, int max_width = 60, int max_height = 30) {
  // Background colors need more horizontal space, so we use smaller defaults
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height);

  // Calculate sampling step (how many pixels to skip)
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  std::cout << "\nPixel Image Preview (" << display_width << "x" << display_height << "):\n";

  // Each console "pixel" is two spaces with background color to make it more square
  const std::string pixel = "  ";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width * 2; x++) std::cout << "-";
  std::cout << "+\n";

  // Print each row
  for (int y = 0; y < display_height; y++) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel
      ImageColor pixel_color = img.get(orig_x, orig_y);
      int r = pixel_color.rgb[0];
      int g = pixel_color.rgb[1];
      int b = pixel_color.rgb[2];

      // We need to map RGB to console colors
      // This maps RGB to the closest ANSI background color
      int bg_color;

      // Very dark pixels - black background
      if (r < 30 && g < 30 && b < 30) {
        bg_color = 40;  // Black background
      }
      // Bright white pixels
      else if (r > 200 && g > 200 && b > 200) {
        bg_color = 47;  // White background
      }
      // Determine dominant color and map to ANSI background code
      else {
        double total = r + g + b;
        double r_ratio = r / total;
        double g_ratio = g / total;
        double b_ratio = b / total;

        const double color_threshold = 0.38;

        if (r_ratio > color_threshold && g_ratio > color_threshold && b_ratio < 0.2) {
          bg_color = 43;  // Yellow background
        } else if (r_ratio > color_threshold && b_ratio > color_threshold && g_ratio < 0.2) {
          bg_color = 45;  // Magenta background
        } else if (g_ratio > color_threshold && b_ratio > color_threshold && r_ratio < 0.2) {
          bg_color = 46;  // Cyan background
        } else if (r_ratio > color_threshold && r_ratio > g_ratio && r_ratio > b_ratio) {
          bg_color = 41;  // Red background
        } else if (g_ratio > color_threshold && g_ratio > r_ratio && g_ratio > b_ratio) {
          bg_color = 42;  // Green background
        } else if (b_ratio > color_threshold && b_ratio > r_ratio && b_ratio > g_ratio) {
          bg_color = 44;  // Blue background
        } else {
          // Fallback to grayscale
          double brightness = 0.299 * r + 0.587 * g + 0.114 * b;
          brightness /= 255.0;

          if (brightness > 0.7) {
            bg_color = 47;  // White background
          } else if (brightness > 0.4) {
            bg_color = 47;  // White background (use same as above for simplicity)
          } else if (brightness > 0.2) {
            bg_color = 100;  // Bright black (gray) background
          } else {
            bg_color = 40;  // Black background
          }
        }
      }

      // Use bright background colors for more vibrant display
      if (bg_color >= 40 && bg_color <= 47) {
        double brightness = 0.299 * r + 0.587 * g + 0.114 * b;
        brightness /= 255.0;

        if (brightness > 0.5 && bg_color != 47) {  // Not already white
          bg_color += 60;                          // Convert to bright background color
        }
      }

      // Print a space with the selected background color
      std::cout << "\033[" << bg_color << "m  \033[0m";
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < display_width * 2; x++) std::cout << "-";
  std::cout << "+\n\n";
}

// Print image choosing the best method for the terminal
void print_best_image(const Image& img, int max_width = 60, int max_height = 30) {
  // Enable ANSI colors for Windows
  enable_windows_ansi_colors();

  // Attempt to determine terminal capabilities
  bool supports_truecolor = false;
  bool supports_ansi_colors = true;  // Assume basic ANSI colors are supported

  // Check environment variables for true color support
  char* colorterm = getenv("COLORTERM");
  if (colorterm && (strcmp(colorterm, "truecolor") == 0 || strcmp(colorterm, "24bit") == 0)) {
    supports_truecolor = true;
  }

  // Check for TERM variable indicating advanced terminal
  char* term = getenv("TERM");
  if (term && (strstr(term, "256color") != NULL || strstr(term, "xterm") != NULL || strstr(term, "konsole") != NULL)) {
    supports_ansi_colors = true;  // Definitely supports ANSI colors
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
    supports_truecolor = true;
  }
#endif

  // Choose the best rendering method
  if (supports_truecolor) {
    print_truecolor_image(img, max_width, max_height);
  } else if (supports_ansi_colors) {
    print_pixel_image(img, max_width, max_height);
  } else {
    // Fallback to basic ASCII
    print_enhanced_ascii_image(img, max_width * 2, max_height);
  }
}

// High-resolution console renderer for Windows
void print_highres_image(const Image& img, int max_width = 120, int max_height = 60) {
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
  int console_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  int console_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

  // Determine display dimensions (leave space for borders)
  int display_width = intMin(img.width, intMin(max_width, console_width - 2));
  int display_height = intMin(img.height, intMin(max_height, console_height - 3));

  // Calculate sampling step
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  std::cout << "\nHigh Resolution Console Image (" << display_width << "x" << display_height << "):\n";

  // Get current cursor position
  COORD cursorPos = csbi.dwCursorPosition;
  cursorPos.Y += 2;  // Move down two lines for the header

  // Create buffers for characters and attributes
  std::vector<CHAR_INFO> buffer(display_width * display_height);

  // Fill the buffer with pixels
  for (int y = 0; y < display_height; y++) {
    for (int x = 0; x < display_width; x++) {
      // Sample the original image
      int orig_x = static_cast<int>(x * x_step);
      int orig_y = static_cast<int>(y * y_step);

      // Get pixel
      ImageColor pixel = img.get(orig_x, orig_y);
      CHAR_INFO& charInfo = buffer[y * display_width + x];

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
  writeRegion.Right = static_cast<SHORT>(cursorPos.X) + display_width - 1;
  writeRegion.Bottom = static_cast<SHORT>(cursorPos.Y) + display_height - 1;

  // Write the buffer to the console
  if (!WriteConsoleOutput(hConsole, buffer.data(),
                          {static_cast<SHORT>(display_width), static_cast<SHORT>(display_height)}, {0, 0},
                          &writeRegion)) {
    std::cerr << "Failed to write to console buffer" << std::endl;
  }

  // Move cursor below the image
  cursorPos.Y += display_height + 1;
  SetConsoleCursorPosition(hConsole, cursorPos);
  std::cout << std::endl;
#else
  // Fallback for non-Windows platforms
  std::cout << "High-resolution console rendering is only supported on Windows" << std::endl;
  print_truecolor_image(img, max_width, max_height);
#endif
}

// Ultra-high resolution console renderer using half-block characters
void print_ultrahd_console_image(const Image& img, int max_width = 80, int max_height = 40) {
#ifdef _WIN32
  // Enable ANSI colors first
  enable_windows_ansi_colors();

  // Calculate dimensions
  int display_width = intMin(img.width, max_width);
  int display_height = intMin(img.height, max_height) * 2;  // Double vertical res with half-blocks

  // Calculate sampling step
  double x_step = static_cast<double>(img.width) / display_width;
  double y_step = static_cast<double>(img.height) / display_height;

  std::cout << "\nUltra HD Console Image (" << display_width << "x" << (display_height / 2) << " doubled):\n";

  // Top border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n";

  // Process two rows at a time using upper and lower half blocks
  //   for (int y = 0; y < display_height; y += 2) {
  for (int y = display_height - 1; y >= 0; y -= 2) {
    std::cout << "|";  // Left border

    for (int x = 0; x < display_width; x++) {
      // Get upper and lower pixels
      int orig_x = static_cast<int>(x * x_step);

      int orig_y_upper = static_cast<int>(y * y_step);
      int orig_y_lower = static_cast<int>((y + 1) * y_step);

      // Handle edge case for odd height images
      if (orig_y_lower >= img.height) {
        orig_y_lower = img.height - 1;
      }

      ImageColor upper_pixel = img.get(orig_x, orig_y_upper);
      ImageColor lower_pixel = img.get(orig_x, orig_y_lower);

      // Extract RGB values
      int upper_r = upper_pixel.rgb[0];
      int upper_g = upper_pixel.rgb[1];
      int upper_b = upper_pixel.rgb[2];

      int lower_r = lower_pixel.rgb[0];
      int lower_g = lower_pixel.rgb[1];
      int lower_b = lower_pixel.rgb[2];

      // Use half block characters and true color ANSI codes
      // ▀ (upper half block) for upper pixel
      // Set foreground color to upper pixel and background color to lower pixel

      // ESC[ 38;2;⟨r⟩;⟨g⟩;⟨b⟩ m - Set foreground to RGB color
      // ESC[ 48;2;⟨r⟩;⟨g⟩;⟨b⟩ m - Set background to RGB color
      std::cout << "\033[38;2;" << upper_r << ";" << upper_g << ";" << upper_b << "m"
                << "\033[48;2;" << lower_r << ";" << lower_g << ";" << lower_b << "m"
                << "▀"         // Upper half block character
                << "\033[0m";  // Reset formatting
    }

    std::cout << "|\n";  // Right border
  }

  // Bottom border
  std::cout << "+";
  for (int x = 0; x < display_width; x++) std::cout << "-";
  std::cout << "+\n\n";
#else
  // Fallback for non-Windows platforms
  std::cout << "Ultra HD console rendering is using true color ANSI codes" << std::endl;
  print_truecolor_image(img, max_width, max_height);
#endif
}