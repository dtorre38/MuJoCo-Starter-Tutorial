
# Setting Up VS Code to Run C/C++ Code with a Shell Script

## Step 1: Ensure Your Shell Script is Executable
Before running the script, make sure it has execution permissions.

```bash
chmod +x <your_script_name>
```

## Step 2: Create a Task in VS Code
1. Open **VS Code Command Palette** (`Ctrl + Shift + P` or `Cmd + Shift + P` on Mac).
2. Search for **"Tasks: Configure Tasks"** and select it.
3. Click **"Create tasks.json file from template"**, then select **"Others"**.
4. Modify `tasks.json` inside `.vscode/`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build and Run C++ Project",
            "type": "shell",
            "command": "${workspaceFolder}/<your_script_name>",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"],
            "detail": "Runs the build script"
        }
    ]
}
```

## Step 3: Configure Debugging (launch.json)
1. Open **Command Palette** (`Ctrl + Shift + P`).
2. Search for **"Debug: Add Configuration..."**.
3. Select **"C++ (GDB/LLDB)"**.
4. Modify `launch.json` inside `.vscode/`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Run C++ Executable",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/<your_executable_name>",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "console": "integratedTerminal",
            "MIMode": "lldb", 
            "preLaunchTask": "Build and Run C++ Project"
        }
    ]
}
```

## Step 4: Running with the Run Button

### **Important: You Must Be on the Shell Script File (`<your_script_name>`)**
When running the program, **ensure that the active file in VS Code is your shell script (`<your_script_name>`)**.  
If you attempt to run from another file (e.g., `main.c` or `main.cpp`), VS Code might not execute the correct commands.

To run:
1. **Open `<your_script_name>` in VS Code**.
2. Click **Run and Debug** (`Ctrl + Shift + D` or `Cmd + Shift + D` on Mac).
3. Select **"Run C++ Executable"**.
4. Click the **Run > button** or press **F5**.

Your C++ project should now build and run correctly using the **Run button in VS Code**!

