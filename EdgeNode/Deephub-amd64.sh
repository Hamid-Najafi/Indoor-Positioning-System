# Copyleft (c) 2022.
# -------==========-------
# Ubuntu 22.04
# Hostname: ILMA 
# Username: ilma
# Password: 1478963
# https://developer.nvidia.com/embedded/jetpack-archive
#* ========== *# 
#     UWB
#* ========== *# 
sudo apt update
sudo apt install python3 python3-pip python-is-python3 -y
pip install pandas
pip install scikit-learn
git clone https://github.com/cliansang/Identification-of-NLOS-and-MP-in-UWB-using-ML.git

#* ========== *# 
#   Deephub
#* ========== *# 
# https://docs.deephub.io/index.html

# 403 Iran:
sudo systemctl stop systemd-resolved.service
mv /etc/resolv.conf /etc/resolv.conf.backup
cat << EOF > /etc/resolv.conf
nameserver 185.51.200.2
nameserver 178.22.122.100
EOF

# Docker
curl -sSL https://get.docker.com/ | sh
sudo usermod -aG docker $USER
sudo chown $USER /var/run/docker.sock
# Docker-Compose
sudo curl -L --fail https://raw.githubusercontent.com/linuxserver/docker-docker-compose/master/run.sh -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose
# Docker Proxy
cat > /etc/docker/daemon.json <<EOF
{
  # "registry-mirrors": ["https://registry.docker.ir"]
  "registry-mirrors": ["https://docker.iranrepo.ir"]
}
EOF
sudo systemctl daemon-reload
sudo systemctl restart docker

mv /etc/resolv.conf.backup /etc/resolv.conf
sudo systemctl start systemd-resolved.service

# ============================== 
# Python
if [ ! -d /home/$USER/.pip ]
then
mkdir /home/$USER/.pip
chown $USER:$USER /home/$USER/.pip
cat >> /home/$USER/.pip/pip.conf << EOF
[global]
index-url = https://pypi.iranrepo.ir/simple
EOF
fi

sudo apt install -y python3.10
curl -sS https://bootstrap.pypa.io/get-pip.py | python3.10
python3.10 -m pip install -U pip testresources setuptools
python3.10 -m pip install --upgrade pip
python3.10 -m pip --version
# ============================== 
# NodeJS
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.1/install.sh | bash
source ~/.bashrc

export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion
nvm install --lts

# ============================== 
# DeepHub® Basic Setup
# ============================== 
# Clone Repos
git clone https://github.com/flowcate/deephub-basic-setup
git clone https://github.com/flowcate/deephub-examples
sudo chown -R 1000:1000 ~/deephub-basic-setup
sudo chown -R 1000:1000 ~/deephub-examples
# ============================== 
# Setup Environments
cd ~/deephub-basic-setup
docker-compose pull

cd ~/deephub-examples/deephub-influxdb-grafana
docker-compose pull

cd ~/deephub-examples/deephub-rest-api-basics
pip install ujson requests

cd ~/deephub-examples/deephub-to-mqtt-gateway
docker-compose pull
npm i

cd ~/deephub-examples/deephub-warehouse-use-case
npm i

# ============================== 
# Start Services
cd ~/deephub-basic-setup
docker-compose up -d 
# ========
cd ~/deephub-examples/deephub-influxdb-grafana
docker-compose up -d 
node ./sendToInfluxDb.js
http://localhost:3000/
# ========
cd ~/deephub-examples/deephub-rest-api-basics
python3 deephub-rest-api-basics.py
# ========
cd ~/deephub-examples/deephub-to-mqtt-gateway
node ./sub.js
node ./pub.js
# ========
cd ~/deephub-examples/deephub-warehouse-use-case
# To Start
node ./addZone.js
node ./addFences.js
node ./addProviders.js
npm run start
# To Stop
npm run stop
curl -X DELETE localhost:8081/deephub/v1/trackables
curl -X DELETE hpthinclient.local:8081/deephub/v1/trackables
# ============================== 
http://37.32.6.18:8081

# Deephub have time based licence.(10 Days) If its expired, clone repo again.
http://localhost:8081
http://localhost:8081/deephub/version
http://localhost:8081/deephub-ui/buildinfo.json

http://hpthinclient.local:8081/deephub/version
http://hpthinclient.local:8081/deephub-ui/buildinfo.json
http://hpthinclient.local:8081/swagger
# Error
# This instance of DeepHub® does not have a valid lease from the license server. The trial period has expired.

# ============================== 
# On license revoked
cd ~/deephub-basic-setup
docker-compose down
cd ~
sudo rm -rf ~/deephub-basic-setup/
git clone https://github.com/flowcate/deephub-basic-setup
cd ~/deephub-basic-setup
docker-compose up -d 

# ============================== 
# DeepHub® Advanced Setup
# ============================== 
git clone https://github.com/flowcate/deephub-advanced-setup.git
sudo chown -R 1000:1000 ~/deephub-advanced-setup
cd ~/deephub-advanced-setup
docker-compose pull
docker compose up -d

# CREATE REALM & USER ...
docker exec -it deephub-advanced-setup_keycloak_1 /bin/bash
cd /opt/bitnami/keycloak/bin
kcadm.sh update realms/master -s sslRequired=NONE --server http://localhost:8080/ --realm master --user user --password bitnami
kcadm.sh update realms/omlox -s sslRequired=NONE --server http://localhost:8080/ --realm master --user user --password bitnami