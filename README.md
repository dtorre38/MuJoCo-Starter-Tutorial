# M-Series Mac MuJoCo C Installation Tutorial

To get MuJoCo up and running on your M-series Mac follow this tutorial.

Packages Required, install via terminal:

1. Command line tools
    1. ```xcode-select —install```
2. Homebrew -  https://brew.sh
    1. ```/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"```
3. CMake
    1. ```brew install cmake```
4. GLFW
    1. ```brew install glfw```

MuJoCo M-Series Tutorial:

1. Download the .dmg file: https://github.com/deepmind/mujoco/releases.
	1. Transfer MuJoCo app into Applications folder.
	2. Open MuJoCo app. 	
2. Install MuJoCo:
    1. Open terminal window and clone the mujoco repository from GitHub: 
        1. ```git clone https://github.com/deepmind/mujoco.git```
    2. Create a new build directory inside of the mujoco folder and cd into it.
    	1. ```mkdir build```
     	2. ```cd build```
    3. Configure:
     	1. ```cmake ..```
    5. Build:
    	1. ```cmake --build .```
    7. Install:
       	1. ```cmake --install .```
3. Download folder and insert inside of mujoco-X.X.X
4. Navigate to the template folder:
	1. ```cd mujoco/mujoco-X.X.X/MuJoCo-Starter-Template-Main```
5. To run via CMakeLists.txt:
	1. ```cmake .```
 	2. ```make```
  	3. ```./main```
6. To run via run file:
	1. ```chmod +x run_unix```
 	2. ```./run_unix```
7. A MuJoCo window will appear and a ball will bounce. Congratulations, MuJoCo is running on your Mac!
