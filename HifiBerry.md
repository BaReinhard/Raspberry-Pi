Shairport-Sync w/ Auxillary Cable Input

A brief background about the project:
Currently, I have a 13 month daughter who listens to lullabies while she sleeps. Currently, we have an iPad playing Pandora in her room, each night when my wife and I go to bed we are tasked with going into her room to turn it off with the risk of waking her up.
One night, we went in and turned it off only to find that we had woken her up in the process. Needless to say it was a long night and was the birth of this project.

The project will require the following:
microSD card 8GB or larger
Raspberry Pi 3 or Pi Zero,A,B+,B + WiFi Adapter
OSMC or Raspbian Jessie
HiFiBerry AMP+ or HiFiBerry of your choice
An additional USB Sound Card (I used a Sabrent, with a mic input)
Spare USB Keyboard (needed just for setup)
Speakers

Goal of the Project:

The goal of this project is to be able to stream music from an iDevice and Android Device (rooted and running a similar app to AirCast) from anywhere in your wireless network. In addition, I wanted to have the retro ability to plug in any device that has a 3.5mm headphone jack.

Current Limitations:

For the time being any audio played through the Aux Cable will be in mono due to the USB sound card's mic input is mono. If/when a USB sound card comes out with stereo input you can follow this guide and eliminate this limitation.

Getting started:

First we will need to load the SD card with the OS of your choice, if you need assistance you can go here: https://www.raspberrypi.org/documentation/installation/installing-images/README.md

Next, will require setting up SSH or using a Plugged in Keyboard to get to the terminal.

If using OSMC, you will have the option to set SSH in the setup process. 
If using Raspbian, you will need to run the following command : `sudo raspi-config`. Navigate to `Interfacing options`, then to `ssh`, press `Enter` and select `Enable or disable ssh server`. The Raspberry Pi Foundation has also mentioned placing a file named `ssh` without any extension onto the boot partition of the SD card will allow for SSH to be enabled for headless setups. ( I have not tried this before)

Now with your spare keyboard, lets first setup WiFi access. Most WiFi dongles I have come across are now supported on the latest OS's and won't need any special drivers.

So lets first scan for the WiFi network we want:
`sudo iwlist wlan0 scan | grep ESSID`
You should see output similar to : `ESSID: TWC21721`. Next we will need to add the following information to our wpa_supplicant file: `sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
```
network={
  ssid="TWC21721"
  psk="YOUR_WIFI_PASSWORD"
  }
```
Now save and exit. Run the following commands to get the WiFi to register if it doesn't automatically. `sudo ifdown wlan0` then `sudo ifup wlan0`.

Now we need to setup our AirPlay functionality through `shairport-sync` a fabulous open source project headed by Mike Brady at https://github.com/mikebrady/shairport-sync
Some of the following steps aren't necessary, but I decided to do them all in case I ran into any trouble with a different setup.
I will list the following commands you will need to execute:

```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential git
sudo apt-get install autoconf automake libtool libdaemon-dev libasound2-dev libpopt-dev libconfig-dev
sudo apt-get install avahi-daemon libavahi-client-dev
sudo apt-get install libssl-dev
sudo apt-get install libpolarssl-dev
sudo apt-get install libsoxr-dev
```
With all the dependencies now met we will need to setup `shairport-sync`

```
cd ~
sudo git clone https://github.com/mikebrady/shairport-sync.git
cd shairport-sync
sudo autoreconf -i -f
sudo ./configure --sysconfdir=/etc --with-alsa --with-avahi --with-ssl=openssl --with-metadata --with-soxr --with-systemd
sudo make
getent group shairport-sync &>/dev/null || sudo groupadd -r shairport-sync >/dev/null
getent passwd shairport-sync &> /dev/null || sudo useradd -r -M -g shairport-sync -s /usr/bin/nologin -G audio shairport-sync >/dev/null
sudo make install
```
Now we can enable `shairport-sync` through systemctl so it will be running after each reboot:

```
sudo systemctl enable shairport-sync
```
Next, we will setup some basic settings:

```
sudo nano /etc/shairport-sync.conf
```
Under the General you will want to add the following:

```
general =
{
    name = "Front Room";
    password = "secret"; // you can remove password if you don't want a password
    interpolation = "soxr";
    
};

```
Now we are done with shairport-sync.
Next, we will be setting up the sound cards so that can we properly
