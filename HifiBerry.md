#Shairport-Sync w/ Auxillary Cable Input

A brief background about the project:
Currently, I have a 13 month daughter who listens to lullabies while she sleeps. Currently, we have an iPad playing Pandora in her room, each night when my wife and I go to bed we are tasked with going into her room to turn it off with the risk of waking her up.
One night, we went in and turned it off only to find that we had woken her up in the process. Needless to say it was a long night and was the birth of this project.

The project will require the following:

* microSD card 8GB or larger
* Raspberry Pi 3 or Pi Zero,A,B+,B + WiFi Adapter
* OSMC or Raspbian Jessie
* HiFiBerry AMP+ or HiFiBerry of your choice
* An additional USB Sound Card (I used a Sabrent, with a mic input)
* Spare USB Keyboard (needed just for setup)
* Speakers

### Goal of the Project:

The goal of this project is to be able to stream music from an iDevice and Android Device (rooted and running a similar app to AirCast) from anywhere in your wireless network. In addition, I wanted to have the retro ability to plug in any device that has a 3.5mm headphone jack.

### Current Limitations:

For the time being any audio played through the Aux Cable will be in mono due to the USB sound card's mic input is mono. If/when a USB sound card comes out with stereo input you can follow this guide and eliminate this limitation.

### Getting started:

First we will need to load the SD card with the OS of your choice, if you need assistance you can go here: https://www.raspberrypi.org/documentation/installation/installing-images/README.md

Next, will require setting up SSH or using a Plugged in Keyboard to get to the terminal.

If using OSMC, you will have the option to set SSH in the setup process. 
If using Raspbian, you will need to run the following command : `sudo raspi-config`. Navigate to `Interfacing options`, then to `ssh`, press `Enter` and select `Enable or disable ssh server`. The Raspberry Pi Foundation has also mentioned placing a file named `ssh` without any extension onto the boot partition of the SD card will allow for SSH to be enabled for headless setups. ( I have not tried this before)

Now with your spare keyboard, lets first setup WiFi access. Most WiFi dongles I have come across are now supported on the latest OS's and won't need any special drivers.

So lets first scan for the WiFi network we want:

```
sudo iwlist wlan0 scan | grep ESSID
```
You should see output similar to : 

```
ESSID: TWC21721
```
Next we will need to add the following information to our wpa_supplicant file:

```
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
 
network={
  ssid="TWC21721"
  psk="YOUR_WIFI_PASSWORD"
  }
```
Now save and exit. Run the following commands to get the WiFi to register if it doesn't automatically. 

```
sudo ifdown wlan0
sudo ifup wlan0
```
Now we need to setup our soundcards:

```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install alsa-utils
sudo nano /etc/asound.conf
```
Add the following to the asound.conf:

```
pcm.!default {
  type hw card 1
}
ctl.!default {
  type hw card 1
}
```
Next we will add the indexing for our sound cards:

```
sudo nano /etc/modprobe.d/alsa-base.conf

options snd_bcm2835 index = 0
options snd_rpi_hifiberry_amp index = 1
```
Since I have had issues including the USB sound card in the base configuration file I typically will exclude it, this way it will show up as the last slot.

```
sudo reboot
```
You can do a quick check after the reboot to ensure the sound cards have been setup properly, 

```
aplay -l
```


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
We need to create 3 files `shairportstart.sh` and `shairportend.sh` and `shairportfade.sh` and should be located in your home directory in a folder called shScripts:

```
cd ~
sudo mkdir shScripts
cd shScripts
sudo nano shairportstart.sh
```

shairportstart.sh should include:

```
#!/bin/sh
/bin/pkill arecord
if [ $(date +%H) -ge "18" -o $(date +%H) -le "7" ]; then
        /usr/bin/amixer set Master 40%
else
        /usr/bin/amixer set Master 100%
fi
# ----------- Be sure to change your directory path here
#/home/osmc/shScripts/shairportfade.sh& #if using OSMC
/home/pi/shScripts/shairportfade.sh& #if using raspbian

exit 0
```
shairportend.sh should include:

```
#!/bin/sh
/usr/bin/amixer set Master 70%
/usr/bin/arecord -D plughw:2 -f dat | /usr/bin/aplay -D plughw:2 -f dat&
exit 0
```
shairportfade.sh should include:

```
#!/bin/sh
/usr/bin/amixer set Master 30-
sleep 2
for (( i=0; i<30; i++))
do  
    /usr/bin/amixer set Master 1+
done
exit 0
```


Now we will setup the `shairport-sync` configuration file:

```
sudo nano /etc/shairport-sync.conf
```
Under the General you will want to add the following:

```
general =
{
    name = "Front Room"; // This can be any name you want your device to show up as
    password = "secret"; // you can remove password if you don't want a password
    interpolation = "soxr";
    
};

```
Then under Session control you will want to add the following:


```
sessioncontrol =
{
// If using Raspbian
  run_this_before_play_begins = "/home/pi/shScripts/shairportstart.sh";
  run_this_after_play_ends = "/home/pi/shScripts/shairportend.sh";
// If using OSMC
  //run_this_before_play_begins = "/home/osmc/shScripts/shairportstart.sh";
  //run_this_after_play_ends = "/home/osmc/shScripts/shairportend.sh";
  
  wait_for_completion = "yes";
};

```
If you would like to enable aux input on boot simply add the following to 

`/etc/rc.local`:


```
/usr/bin/arecord -D plughw:2 -f dat | /usr/bin/aplay -D plughw:2 -f dat&
```

Now we are done with shairport-sync and done setting up the Pi to run as an AirPlay receiver.

As far as shairport-sync goes you can use any scripts that you so choose and do not need to use shell scripts I have included here you are free to create your own scripts and have them do whatever you see fit.

### Please note:
in the scripts you will need to use full path names `/bin/kill` instead of `kill` or `/usr/bin/amixer` instead of `amixer`. In addition, do not use `sudo` as these scripts are run with root permission and you will likely have a bad time if you include `sudo`.


### Pitfalls and Debugging:
Which leads me to some of the issues I have encountered during the inital testing and setup.

During my initial project, I hadn't decided to use an AUX input and liked the idea of using a script that made sure that during the hours my daughter would be sleeping to set the max volume to a point where if the device connected would accidentally be turned all the way up it would not be loud enough to wake her. Then during the hours she was awake the max volume would be set higher to be heard through the house.

After this I decided to add a fade script that would slowly fade the volume to the set based on the time. Had everything working and the only mistakes I had made thus far were in regards to how to program in shell.
Now comes the introduction of the second sound card. I was able to setup the second sound card, play to it and from it no problem with the following line :

```
arecord -D plughw:2 -f dat | aplay -D plughw:2 -f dat&
```
And I was able to kill it with the following code:

```
sudo pkill arecord
```
I then decided this would be a perfect implementation to have `sudo pkill arecord` command added to the beginning of `shairportstart.sh` and have the `arecord -D plughw:2 -f dat | aplay -D plughw:2 -f dat&` added to the end of `shairportend.sh`.

After I implemented it it restarted the pi and connected my device via the aux cable and grabbed another iPad to connect via AirPlay. First the AirPlay connected successfully, then I disconnected from AirPlay and BOOM! My aux connected device started playing. I was sitting here thinking to myself "This is sooooo cool it actually worked". Then I connected the iPad back to AirPlay, as I sit still listening the my aux device. I couldn't couldn't understand where I went wrong. I immediately went back to the `pkill command` and checked that syntax was correct, everything checked out. Next, execute the script via `bash -x shairportstart.sh` it properly executes everything and now no noise. I spend some time researching `shairport-sync` and how I should use the sessioncontrol functionality. I see that they suggest ensuring that a shebang (`#!/bin/sh or #!/bin/bash or #!/usr/bin/python`) is being used and that you use full path names for each command (`/bin/pkill`). So naturally I use full pathnames and ensure the shebang is error free. Reboot and try again, still nothing...

Now I'm beginning to get some what flustered and begin trying some other ways of killing scripts, thinking that pkill was the issue.
I tried:

```
grep arecord | sed s/' arecord'// | xargs sudo kill
pgrep -l arecord | sed s/' arecord'// | xargs sudo kill
pgrep arecord | sudo kill
/bin/ps -ef | /bin/grep are | /bin/grep -v /bin/grep | /usr/bin/awk '{print $2}'| /usr/bin/xargs /usr/bin/sudo /bin/kill
```
Still none of them work. Then I add a loggin portion to the script to see what usr is running it and its working directory. I run the script and no logging occurs. Eventually, I stumble upon something and kill the instance of shairport-sync then start it with the command line. I test it and the scripts work properly! Finally, I fixed it!

I then disabled shairport-sync through systemctl and added shairport-sync to my `/etc/rc.local` file, rebooted and tested it again. It works again. Success!

I left it this way for about a day, then the thought of why it wasn't working prior started nagging at me. So, I start doing research the best way I know how, Google! Hours and hours have gone by and I still wasn't any closer to figuring out why, so naturally I call the "Justice League" aka stackoverflow, obviously. I get some answers back, but nothing to really explain to me why it happens. Since, both systemd and rc.local run programs as root it just didn't make sense to me. So after a day or so of back and forth with some users who are far smarter than I am, I was finally able to get a grasp on what was going on and why things weren't working out and I finally thought to look at the syslog in `/var/log/syslog` to find the following error:

```
Jan 24 00:38:45 raspberrypi shairport-sync[617]: sudo: no tty present and no askpass program specified
```
I first had to look at how to combat this issue, so I read that it needed a password passed to it. Well since shairport-sync doesn't have a passwd I initially gave it one and echoed my password to it and that worked, but that solution seemed unsecure and nagged at me. 
I took a step back and finally realized the issue is that I am using `sudo` when its completely unnecessary. The script is already being run as root, since each child process inherits the permissions and ownerships of its parent process. Meaning `shairportstart.sh` has root permissions, further meaning `sudo` is unnecessary! After I realized this I removed `sudo` from the script and simply changed the first line to `pkill arecord`, removed the line from `/etc/rc.local`, then re-enabled shairport-sync `sudo systemctl enable shairport-sync` and rebooted the pi. I tested it out and it worked like it should have the first time had I remembered a simple lesson that everyone should know. 
#### "Do not call sudo in a shell script it is better to run sudo shairportstart.sh. There is no need to sudo as root."

