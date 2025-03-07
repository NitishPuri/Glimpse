clear


# Install required dependencies if not present
# if [ -f /etc/lsb-release ] || [ -f /etc/debian_version ]; then
#     # For Ubuntu/Debian
#     sudo apt-get update
#     sudo apt-get install -y libgl1-mesa-dev xorg-dev
# elif [ -f /etc/fedora-release ]; then
#     # For Fedora
#     sudo dnf install -y mesa-libGL-devel libXi-devel libXinerama-devel libXcursor-devel libXrandr-devel
# fi



# Configure
# rm -rf buildLinux
# cmake --preset wsl

# build
cmake --build buildLinux

# cd buildLinux
