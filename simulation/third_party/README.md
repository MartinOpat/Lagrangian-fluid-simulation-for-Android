## To re-compile and re-build all 3rd party libraries You can run the following:
**Note that recompiling these libraries is (usually) not necessary or recommended!
Doing this will include the header files for the compiled libraries in your local NDK!**

### Installing prerequisites First, make sure all the necessary packages are installed on your system. On Ubuntu, you can run:
```bash
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install libxml2-dev
sudo apt-get install curl
sudo apt-get install patchelf
sudo apt-get install clang
sudo apt-get install m4
```

### Installing 3rd party libraries
It is recommended to use the provided `compileThirdPartyLibs.sh` bash script, which can be used like so:
```bash
chmod +x compileThirdPartyLibs.sh
sudo ./compileThirdPartyLibs.sh $ABI $NDK
```

The 3rd party libraries can also be installed manually see `individual_libraries_installation_isntructions.md`, however this is not recommended.
