# Lagrangian-fluid-simulation-for-Android
## Initial setup

### Installing prerequisites First, make sure all the necessary packages are installed on your system. On Ubuntu, you can run:
```bash
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install libxml2-dev
sudo apt-get install curl
sudo apt-get install patchelf
```

### Installing 3rd party libraries
It is recommended to use the provided `configure.sh` bash script, which can be used like so:
```bash
chmod +x configure.sh
sudo ./configure.sh $ABI $NDK
```

The 3rd party libraries can also be installed manually by going into the `simulation/third_party` folder and following the provided `readme.md`