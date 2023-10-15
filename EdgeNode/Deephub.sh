# Copyleft (c) 2022.
# -------==========-------
# RaspberryPI
# OS: RaspberryPI OS (64 bit)
# Hostname: RaspberryPI
# Username: c1tech 
# Password: 1478963
# -------==========-------

#* ========== *# 
#   Deephub
#* ========== *# 
# https://docs.deephub.io/index.html
# 403 Iran:
export http_proxy=http://192.168.100.40:10809/
sudo systemctl stop systemd-resolved.service
mv /etc/resolv.conf /etc/resolv.conf.backup
cat << EOF > /etc/resolv.conf
nameserver 10.202.10.202
nameserver 10.202.10.102

nameserver 185.55.226.26
nameserver 185.55.225.25

nameserver 185.51.200.2
nameserver 178.22.122.100
EOF

# Docker
curl -sSL https://get.docker.com | sh
sudo usermod -aG docker $USER
sudo chown $USER /var/run/docker.sock
# Docker-Compose
sudo curl -L --fail https://raw.githubusercontent.com/linuxserver/docker-docker-compose/master/run.sh -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose
# Docker Proxy
sudo su
cat > /etc/docker/daemon.json <<EOF
{
  "registry-mirrors": ["https://registry.docker.ir"]
}
EOF
systemctl daemon-reload
systemctl restart docker

mv /etc/resolv.conf.backup /etc/resolv.conf
systemctl start systemd-resolved.service
su c1tech
# ============================== 
# Python
# --- IF NEEDED (pre-installed) ---
sudo apt update
sudo apt install python3.9 python3-pip
python -m pip install -U pip testresources setuptools
python -m pip install --upgrade pip
# ============================== 
# NodeJS
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
source ~/.bashrc
nvm install --lts
# ============================== 
# Clone Repos
git clone https://github.com/flowcate/deephub-basic-setup
git clone https://github.com/flowcate/deephub-examples
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
# Start DeepHub
# ============================== 
cd ~/deephub-basic-setup
docker-compose up -d 

# ============================== 
# Start Examples
# ============================== 
# deephub-influxdb-grafana
cd ~/deephub-examples/deephub-influxdb-grafana
docker-compose up -d 
node ./sendToInfluxDb.js
http://localhost:3000/
# ============================== 
# deephub-rest-api-basics
cd ~/deephub-examples/deephub-rest-api-basics
python deephub-rest-api-basics.py
# ============================== 
# deephub-to-mqtt-gateway
cd ~/deephub-examples/deephub-to-mqtt-gateway
docker compose up -d
node ./sub.js
# In new terminal
cd ~/deephub-examples/deephub-to-mqtt-gateway
node ./pub.js
http://raspberrypi.local:18083/
Username: admin
Password: public or brokerpw1
# ============================== 
# deephub-warehouse-use-case
cd ~/deephub-examples/deephub-warehouse-use-case
node ./addZone.js
node ./addFences.js
node ./addProviders.js
npm run start

npm run stop
curl -X DELETE localhost:8081/deephub/v1/trackables
# ============================== 
http://raspberrypi.local:8081/
http://raspberrypi.local:8081/deephub/version
# ============================== 
# On license revoked
# ============================== 
# Deephub have time based licence.(10 Days) If its expired, clone repo again.
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

# ============================== 
# Python App
# ============================== 
# SQLAlchemy – a popular Python ORM (object-relational mapper)
# Psycopg – a popular PostgreSQL driver
# Pydantic tool for reading, loading, and validating the environment variables specified in a configuration file.
pip install "uvicorn[standard]" "fastapi[all]"
pip install sqlalchemy fastapi-sqlalchemy psycopg2 python-dotenv pydantic alembic

uvicorn sql_app.main:app --reload

#* ========== *# 
# UWB Identification-of-NLOS-and-MP
#* ========== *# 
sudo apt update
sudo apt install python3 python3-pip python-is-python3 -y
pip install pandas
pip install scikit-learn
git clone https://github.com/cliansang/Identification-of-NLOS-and-MP-in-UWB-using-ML.git

# ============================== 
# TEEEEEEEEEEEEEEEEEEEEEEEEEEEMP
# ==============================

# -------==========-------
# Copyleft (c) 2022.
# -------==========-------
# Jetson Nano 2GB Developer Kit
# JetPack SDK 4.6.1
# Jetson Linux R32.6.1
# Hostname: jetson
# Username: nvidia
# Password: nvidia
# https://developer.nvidia.com/embedded/jetpack-archive
# -------==========-------

https://developer.download.nvidia.com/compute/redist/jp/v461/tensorflow/


sudo apt-get install libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev zip libjpeg8-dev liblapack-dev libblas-dev gfortran
sudo apt-get install python3-pip
sudo pip3 install -U pip testresources setuptools==49.6.0
sudo pip3 install -U numpy==1.16.1 future==0.18.2 mock==3.0.5 h5py==2.10.0 keras_preprocessing==1.1.1 keras_applications==1.0.8 gast==0.2.2 futures protobuf pybind11
# TF-2.x
sudo pip3 install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v461 tensorflow
# TF-1.15
sudo pip3 install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v461 'tensorflow<2'

sudo apt install python3.10
curl -sS https://bootstrap.pypa.io/get-pip.py | python3.10
python3.10 -m pip --version
python3.10 -m pip install --upgrade pip
pip 22.2.2 from <home>/.local/lib/python3.10/site-packages/pip (python 3.10)

python3.10 -m pip install -U pip testresources setuptools
python3.10 -m pip install -U numpy==1.16.1 future==0.18.2 mock==3.0.5 h5py==2.10.0 keras_preprocessing==1.1.1 keras_applications==1.0.8 gast==0.2.2 futures protobuf pybind11
python3.10 -m pip install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v461 'tensorflow<2'

python3.10 -m pip list

#* ========= *# 
# Jetpack
#* ========= *# 

sudo apt update && sudo apt upgrade -y
sudo apt install nano bmon software-properties-common openconnect curl -y
sudo add-apt-repository ppa:deadsnakes/ppa

echo -e "alias ocn='sudo openconnect --background --user=usr-hamid --passwd-on-stdin  goldenstarc.ir:443 --http-auth=Basic <<< "ham" > /dev/null'" | sudo tee -a ~/.bashrc  > /dev/null
echo -e "alias ocf='sudo killall -SIGINT openconnect'" | sudo tee -a ~/.bashrc > /dev/null
echo -e "alias ipinfo='curl api.ipify.org && echo -e ""'" | sudo tee -a ~/.bashrc > /dev/null

echo 14789633 | sudo openconnect --background --user=km83576 c2.kmak.us:443 --http-auth=Basic  --passwd-on-stdin
echo 14789633 | sudo openconnect --background --user=km83576 cp6.kmak.info:443 --http-auth=Basic  --passwd-on-stdin
echo hamid | sudo openconnect --background --user=usr-hamid goldenstarc.ir:443 --http-auth=Basic  --passwd-on-stdin


# Install Python 3.8 
sudo apt install python3.8 -y
curl -sS https://bootstrap.pypa.io/get-pip.py | python3.8
python3.8 -m pip install -U pip testresources setuptools
sudo reboot
python3.8
pip3.8

# Additional Tools (Optional)
python3.8 -m pip install --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v461 tensorflow

# Install Docker & Docker-Compose
# wget -O install-dc.sh https://gist.githubusercontent.com/CT83/6dbe0d9df3fd3ba4d57fd3a5347e5105/raw/4975eb302be84cf1755378523343004451c7260d/install-dc.sh
# sudo sh ./install-dc.sh

# Install Docker-Compose
pip3.8 install docker-compose
docker-compose --version
sudo usermod -aG docker $USER
sudo chown $USER /var/run/docker.sock

# This image is for JetPack 4.6.1 (L4T R32.7.1)
## Source: https://catalog.ngc.nvidia.com/orgs/nvidia/containers/l4t-ml
sudo docker run -itd --runtime nvidia --network host -v /home/nvidia/l4t-ml:/home/data nvcr.io/nvidia/l4t-ml:r32.7.1-py3

mkdir -p ~/docker/l4t-ml
cd ~/docker/l4t-ml
cat >> ~/docker/l4t-ml/docker-compose.yml << EOF
version: '3'
services:
  l4t-ml:
    image: nvcr.io/nvidia/l4t-ml:r32.7.1-py3
    container_name: l4t-ml
    restart: always
    privileged: true
    network_mode: "host"
    runtime: nvidia
    stdin_open: true # docker run -i
    tty: true        # docker run -t
    volumes:
      -  ${PWD}/data:/home/data
EOF
docker-compose up -d
# JupyterLab: Jetson.local:8888
# Password: nvidia
# Files: /home/nvidia/docker/l4t-ml/data
docker exec -it l4t-ml pip3 install <packages>