# 🖥️ Mac MuJoCo C Installation Tutorial

This guide will help you **set up MuJoCo on macOS** (both M-Series and Intel Macs) and run a simple C project.

---

## 📦 Required Packages
Before installing MuJoCo, install the following packages via **Terminal**:

### 1️⃣ Command Line Tools (Xcode)
```sh
xcode-select --install
```

### 2️⃣ Homebrew (if not installed) [https://brew.sh]
```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3️⃣ Install Required Dependencies
```sh
brew install cmake glfw
```

### 4️⃣ Link GLFW (Required for some systems)
```sh
brew link glfw
```
**Note:** This step is especially important on Intel Macs and may be required on some M-Series Macs as well.

---

### 🛠️ Installing MuJoCo on macOS (M-Series & Intel)

1️⃣ Download & Install MuJoCo
1.	Download the latest MuJoCo .dmg file → MuJoCo Releases: https://github.com/google-deepmind/mujoco/releases.
2.	Open the .dmg file and move the MuJoCo app to the Applications folder.
3.	Launch the MuJoCo app (this is required to complete the setup).

2️⃣ Install MuJoCo via Terminal
1.	Clone the MuJoCo repository from GitHub:
```sh
git clone https://github.com/deepmind/mujoco.git
```
2.	Navigate into the MuJoCo directory:
```sh
cd mujoco
```
3.	Create a build directory and enter it:
```sh
mkdir build && cd build
```
4.	Configure the build:
```sh
cmake ..
```
5.	Compile MuJoCo:
```sh
cmake --build .
```
6.	Install MuJoCo (`sudo` is needed because MuJoCo installs into `/usr/local`; enter your Mac password when asked):
```sh
sudo cmake --install .
```

**Note:** This tutorial works with MuJoCo 2.3.3 and newer, including the 3.x and 4.x releases.

---

### 🚀 Running the MuJoCo Starter Tutorial

1️⃣ Clone This Repository

You can clone this tutorial into any folder you like (it does not need to be inside the MuJoCo folder):
```sh
git clone https://github.com/dtorre38/MuJoCo-Starter-Tutorial.git
```

2️⃣ Navigate to the Project Directory
```sh
cd MuJoCo-Starter-Tutorial
```

---

### ▶️ Running the Code

You can run the project in two ways: using CMake or the run script.

🔹 Option 1: Run via CMake
```sh
cmake .
make
./bin/main model/ball.xml
```

🔹 Option 2: Run via the run Script
1.	Give execution permissions (only needed once):
```sh
chmod +x run
chmod +x run_advanced
```
2.	Run the simple script:
```sh
./run
```

---

### 🎉 What Happens Next?

A MuJoCo window will appear, and you’ll see a ball bouncing. Congratulations! You’ve successfully set up and run MuJoCo on your Mac! 🎊

---

## 📖 Additional Documentation

This repository includes additional tutorials:

- **VSCode Setup Guide** → [docs/vscode-guides/README.md](docs/vscode-guides/README.md)
- **GitHub Usage Guide** → [docs/github-guides/README.md](docs/github-guides/README.md)

Each guide contains a PDF and setup-by-step instructions.