sudo apt-get install build-essential git uuid-dev iasl nasm python3-distutils python3-apt
git clone https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init --recursive
make -C BaseTools
. edksetup.sh
cp ../misc/targetMde.txt Conf/target.txt
build
rm -r MdeModulePkg
cp ../misc/MdeModulePkg.zip .
unzip MdeModulePkg.zip
rm MdeModulePkg.zip
