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

#commands to run right at the beginning
#be sure to 'sudo apt-get install libi2c-dev' *after* i2c-tools
sudo apt-get install gedit
sudo apt-get install i2c-tools
sudo apt-get install libi2c-dev