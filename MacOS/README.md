# Install Gabedit on MacOS

## Install Gabedit using port

* Install [MacPorts](https://www.macports.org/install.php)
* Under a terminal (Press CmdSpace to open spotlight search, and type terminal and hit return.) , type:
```console
mkdir tmpGabedit
cd tmpGabedit
wget https://github.com/allouchear/gabedit/raw/main/MacOS/xBuildGabeditPort
chmod u+x xBuildGabeditPort
./xBuildGabeditPort
```
Please note, that install : wget, mesa and gtk2 using port, download and install gtkglext and finally Gabedit.
After installation, you can remove tmpGabedit directory

