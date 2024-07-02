# Lagrangian-fluid-simulation-for-Android

## Table of contents
1. [Raw data](#raw-data)
2. [Processed data](#processed-data)
3. [Measurement tools](#measurement-tools)
4. [Data processing tools](#processing-tools)

## Raw data
The raw data can be found at `simulation/measurements/files`. The first file in this folder named `1.measurement.plan` explains the naming of the raw data files and what measurements they correspond to.

## Processed data
The processed data only exists in the form of Python Panda's dataframe. The dataframe are contained within jupyter notebooks present in the `utils` folder.

## Measurement tools
Bash script utilizing [logcat](https://developer.android.com/tools/logcat) and [adb](https://developer.android.com/tools/adb) were used in the process of taking the measurements. These scripts can be found in `simulation/`measurements/scripts`

## Processing tools
For processing the collected data Python 3.12 [Jupyter](https://jupyter.org/) notebook was used in combination with the following libraries:
- [NumPy](https://numpy.org/)
- [Pandas](https://pandas.pydata.org/)
- [Matplotlib](https://matplotlib.org/)