# M-Series Mac MuJoCo C Installation Tutorial

To get MuJoCo up and running on your M-series Mac follow this tutorial.

Packages Required:

1. Command line tools - go to  developer account, install latest version of tools, open package and follow install instructions.
    1. xcode-select —install
2. Homebrew -  https://brew.sh
    1. /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
3. cmake
    1. brew install cmake

MuJoCo M1 Tutorial:

1. Install mujoco instructions are from, or just read the substeps below: 

C: https://mujoco.readthedocs.io/en/stable/programming/index.html?highlight=build#building-mujoco-from-source

Python: https://mujoco.readthedocs.io/en/stable/python.html?highlight=build#building-from-source

	Steps to build
    1. Clone the mujoco repository from GitHub: 
        1. git clone https://github.com/deepmind/mujoco.git
    2. Create a new build directory somewhere, and cd into it.
    3. Run cmake $PATH_TO_CLONED_REPO to configure the build.
    4. Run cmake --build . to build.
    5. cmake --install .

2. Download the .dmg file: https://github.com/deepmind/mujoco/releases
3. Insert template folder inside of mujoco-X.X.X
4. Navigate to the template folder:

	cd mujoco/mujoco-X.X.X/template

5. To run via a CMakeLists.txt run “cmake .” in the terminal. Makefile will appear, run "make" in the terminal.
6. In the terminal run ./main, you should see a ball bounce in MuJoCo. Congratulations, MuJoCo is running on your Mac!
