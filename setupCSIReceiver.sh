#!/usr/bin/sudo /bin/bash

wlan_interface=wlan2

echo Loading driver
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi connector_log=0x1

#echo Configuring interface
#rfkill unblock wlan
#/home/geosearchef/repo/project/scripts/ifAP.sh

#echo ""
#iwlist $wlan_interface channel | grep "Current Frequency"
#echo ""
echo "dmesg:"
dmesg | grep "CSI Tool"
echo ""

service network-manager stop

echo "Unblocking WLAN"
rfkill unblock wlan
echo "Starting hostapd..."
/home/geosearchef/workspace/Atheros-CSI-Tool-UserSpace-APP/hostapd-2.5/hostapd/start_hostapd.sh