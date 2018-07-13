#setup wifi e.g. nmcli d wifi connect ssi pasword bdbdbdbdbd iface wlan0
sudo nmcli d wifi connect SSIDHERE password PASSWORDHERE iface INTERFACEHERE
#pull down latest updates list
sudo apt-get update
#download updates (but don't isntall yet)
sudo apt-get upgrade --download-only
#insteall updates
sudo apt-get ugprade


#get ssh key
cd  ~
wget sevun.com/key.tar.gz
#unzip ssh key
tar -xzvf ~/key.tar.gz
rm ~/key.tar.gz
sudo apt-get install gedit


#USING LIBI2C-DEV ONLY
#commands to run right at the beginning
#be sure to 'sudo apt-get install libi2c-dev' *after* i2c-tools
sudo apt-get install i2c-tools
sudo apt-get install libi2c-dev

#USING LIBSOC
#install prerequsites
sudo apt-get install autoconf
sudo apt-get install build-essential
sudo apt-get install libtool-bin
sudo apt-get install pkg-config
sudo apt-get install automake
git clone https://github.com/jackmitch/libsoc.git
cd libsoc
autoreconf -i
./configure --enable-board=dragonboard410c --with-board-configs
make
sudo make install