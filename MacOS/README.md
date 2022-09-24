# Install Gabedit on MacOS

## Install Gabedit using port

* Install [MacPorts](https://www.macports.org/install.php)
* Under a terminal (Press Cmd+Space to open spotlight search, and type terminal and hit return) , type:
```console
mkdir tmpGabedit
cd tmpGabedit
sudo port install wget
wget https://github.com/allouchear/gabedit/raw/main/MacOS/xBuildGabeditPort
chmod u+x xBuildGabeditPort
./xBuildGabeditPort
```
Please note, that install : wget, mesa , GLU, libtool, pkgconfig and gtk2 using port, download and install gtkglext and finally Gabedit.
After installation, you can remove tmpGabedit directory

