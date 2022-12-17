#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
chmod +x ./**/*.sh
chmod +x ./**/**/*.sh
echo -e "\n\nBuilding projects...\n\n"
make -C ./consola
make -C ./kernel
make -C ./cpu
make -C ./memoria
echo -e "\n\nDeploy done!\n\n"