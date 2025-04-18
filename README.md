## About

This project is the **ELEC4900 Final Year Project (FYP)** Group MZ01a-24 at **The Hong Kong University of Science and Technology (HKUST)**, and support only Windows machines, and we developed this project under Windows 10 environment.

# Project Setup Assumptions

This document outlines the key assumptions and requirements for setting up and running the project successfully. Please follow these directory and file structure guidelines strictly to ensure compatibility.

## Pre-requisites

Make sure the following tools are installed **before** running the project:

- **Vagrant**: Version `2.4.1`  

- **VirtualBox**: Version `7.0.20r163906`  

Other versions may work but are not guaranteed to be compatible with the current Vagrant configuration and virtual machines used in this project.
You may need to modify the bash scripts that exist in this project to match the behavior of newer or older versions.
Although the pre-requisite is stated above, this project also support other virtual environments, as long as they support command-line, and you may need to modify .sh scripts mentioned in the next section. 

## Folder Structure Requirements

1. **Main Folder Name**  
   - The main project folder **must be named `CPEP`**.

2. **Script File Placement**  
   - The following shell script files must be placed **directly inside the `CPEP` folder**:
     - `client_vm_login.sh`
     - `server_vm_login.sh`
     - `boot_vm.sh`

3. **3120 Project Directory Naming**  
   - For the ELEC3120 course project, the folder must be named exactly **`foggytcpTest`**.

4. **Required Files in `foggytcpTest`**  
   - The following files must exist **inside the `foggytcpTest` folder**, with these exact filenames:
     - `client_script_cpep.sh`
     - `server_script_cpep.sh`
     - `client.pcap`
     - `server.pcap`

5. **Parser Files**  
   - All parser-related `.h` and `.cpp` files should be placed **inside a folder named `Parser`**.
   - The folder name **must be `Parser`** and **must not be changed**.
   - Any related `.dll` files **must also be located inside the `Parser` folder**.

6. **Naming Convention**  
   - The entire project directory and all folder/file names **contains ideally only English characters**. Avoid using any special or non-English characters.
   - We have tested project directory with Chinese symbols, it works but still avoid special or non-English characters.

7. **Vagrantfile location**
   - `Vagrantfile` must exist inside the `CPEP` folder
  
8. **CPEP-exe folder**
   - **Please move all the files and sub-folder into `CPEP` folder, as they are the files for Qt application to be executed successfully**.
    
## Caution

If you are working on a Windows system, please make sure that all `.sh` files (shell scripts) use **Unix-style line endings**.  
Windows often saves files with CRLF (`\r\n`) endings, which can cause issues on Unix-based systems (like Linux or macOS).

Before running `.sh` scripts in this project on Unix-based environments, it is recommended to run:

dos2unix (name of the script).sh
