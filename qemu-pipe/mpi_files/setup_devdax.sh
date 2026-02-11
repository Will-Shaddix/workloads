#!bin/bash
sudo apt-get install ndctl pmem-tools libpmem-dev
# show regions and existing namespaces
sudo ndctl list
# a more readable form:
sudo ndctl list --reconfigure
# create a devdax namespace for region0
# sudo ndctl create-namespace --region=region0 --mode=devdax
sudo ndctl create-namespace --region=region0 --mode=devdax --size=1G
# (temporary) make it readable/writable by current user
# sudo chmod 666 /dev/dax0.0
# or add your user to a group that owns the device

# sudo apt-get update
# sudo apt-get install -y linux-modules-extra-$(uname -r) ndctl daxctl jq

# sudo apt-get install -y daxctl jq

