# Documentation

## Viewing
You can view the current documentaiton in the browser by either opening the `html/index.html` file or simply by running:
```bash
xdg-open ./html/index.html
```

## Updating
## Prerequisites
Firstly, make sure all the necessary packages are available on your machine.
```bash
sudo apt-get update
sudo apt-get install doxygen
sudo apt-get install graphviz
```

To update the documentation, simply run:
```bash
doxygen Doxyfile
```
