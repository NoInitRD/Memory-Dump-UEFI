cd edk2
cp ../app/* MdeModulePkg/Application/Application/
cp ../misc/targetMde.txt Conf/target.txt
. edksetup.sh
build
cp Build/MdeModule/DEBUG_GCC5/X64/Application.efi ../boot.efi
