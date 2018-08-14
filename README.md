![Logo](resources/adonis_logo.png)


# Quick Start Guide

Adonis is a spiking neural network simulator coded using C++. There are currently two versions:  
**1. Adonis_t** : a clock-based version of the simulator which includes current dynamics  
**2. Adonis_e** : an event-based version of the simulator without current dynamics

## Dependencies

#### On macOS

###### Homebrew
Homebrew is used to easily install macOS dependencies. Open a terminal and run ``/usr/bin/ruby -e “$(curl -fsSL [https://raw.githubusercontent.com/Homebrew/install/master](https://raw.githubusercontent.com/Homebrew/install/master) install)”``.

###### Premake 4
Premake 4 is used to build the project. Open a terminal and run ``brew install premake``.

###### Qt (optional if no GUI is needed)
The Qt framework is needed when using the GUI to visualise the output of a neural network. The following has been tested with **Qt 5.11.1** and support cannot be guaranteed for other versions.

**first option**  
Open a terminal and run ``brew install qt5``

**second option**

1. Download directly from https://www.qt.io/download/
2. Select the correct version of Qt
3. Make sure the Qt Charts add-on is selected
4. Open the premake4.lua file and modify the moc, include and library paths depending on where Qt was installed

#### On Linux (Debian and Ubuntu)

###### Premake 4
Premake 4 is used to build the project. Open a terminal and run ``sudo apt-get install premake4``.

###### Qt (optional if no GUI is needed)
The Qt framework version 5.10 or newer is needed when using the GUI to visualise the output of a neural network. To install qt5 on Debian Buster, type the following:
``sudo apt-get install qt5-default libqt5charts5 libqt5charts5-dev libqt5qml5 qtdeclarative5-dev qml-module-qtcharts``

This should get you going in terms of dependencies. If your distribution does not support that version (Debian Stretch bundles 5.7), consider downloading the latest Qt manually. The following has been tested with **Qt 5.11.1**.

1. Download directly from https://www.qt.io/download/
2. Select the correct version of Qt
3. Make sure the Qt Charts add-on is selected
4. Open the premake4.lua file and modify the moc, include and library paths depending on where Qt was installed
5. Open the .bashrc file in your home directory and add these lines:
```
LD\_LIBRARY\_PATH=[path to the Qt dynamic lib]
export LD\_LIBRARY\_PATH
```

## Testing

1. Go to the Adonis directory and run ``premake4 gmake && cd build && make``
2. execute ``cd release && ./testNetwork`` to run the spiking neural network.

**_Disclaimer: some of the applications bundled in with the simulator use a path relative to the executable to use one of the files present in the data folder. As such, executing ``./release/testNetwork`` instead of ``cd release && ./testNetwork`` could lead to an error message when the relative path is set incorrectly_**

#### Building Without Qt
In case you do not want to use the Qt GUI, you can build Adonis without any Qt dependencies by running ``premake4 --without-qt gmake`` instead of ``premake4 gmake``

#### Other Premake Actions and Options
To use Xcode as an IDE on macOS, go the Adonis base directory and run ``premake4 xcode4``.

Run ``premake4 --help`` for more information

## Using the simulator

The Adonis simulator is a header-only C++ library with 12 classes

![flowChart](resources/flowchart.svg)
