# Andino X2

Homepage of [Andino X2](https://andino.systems/andino-x2/)

Tabe of Content   
   
1. [Digital inputs](README.md#digital-inputs)
2. [Relay outputs](README.md#relay-output)
3. [Prepare the Debian](README.md#prepare-the-debian)
4. [Enable UART to GPIO](README.md#enable-uart-to-gpio)
5. [Add the Driver for the SPI UART](README.md#add-the-driver-for-the-spi-uart)
6. [Add a driver for the SPI Ethernet controller](README.md#add-a-driver-for-the-spi-ethernet-controller)
7. [Add the Driver for the RTC DS3231](README.md#add-the-driver-for-the-rtc-ds3231)
8. [Communication with the Atmel Controller](README.md#communication-with-the-atmel-controller)
9. [Atmel Controller <-> Raspberry Pi](README.md#atmel-controller-<->-raspberry-pi)
10. [Atmel Controller <-> PC via USB](README.md#atmel-controller-<->-pc-via-usb)
11. [Communication with Modem (optional)](README.md#communication-with-modem-(optional))
12. [Drive the LED from Raspberry](README.md#drive-the-led-from-raspberry)
13. [Firmware for the Atmel Controller](README.md#firmware-for-the-atmel-controller)
14. [Setup the 2G / EDGE Driver SIMCom800L](README.md#setup-the-2g-edge-driver-simcom800l)
15. [Schemaics of the Andino X2](README.md#schematics)


## Documentation

![Andino X2 - Raspberry Pi on DIN Rail - Front view](andino-x2-front.png)
![Andino X2 - Raspberry Pi on DIN Rail - PCB Overview](andino-x2-pcb-overview.png)
![Andino X2 - Raspberry Pi on DIN Rail - Block Diagram](blockdiagram-andino-x2.png)
----------


### Digital Inputs
The digital inputs are connected to the Atmel Controller.  
The default firmware will debounce the inputs and send changes and a change counter cyclical to the Pi.
Please see here for a description of the firmware:  
[https://github.com/andino-systems/Andino-X2/tree/master/src/firmware](https://github.com/andino-systems/Andino-X2/tree/master/src/firmware)
  
The Inputs are calculated for ****24 Volt**** Input.  

![Andino X2 - digital input schematics](digital-input-schematics.png)

The Inputs can be driven by an external power supply (active mode) or with a "dry contact" with the internal voltage.   
For the  "dry contact" the internal jumper has to be closed. This will connect the "minus" contact with the ground of the board so this not a 100% isolated mode.   
![Andino X2 - digital input configuration](andino-X2-digital-input-configuration.png)
----------
### Relay Output
The Ralay are also controlled by the Atmel Controller. The default firmware can turn them on/off or pulse them.  
This will be done by the command:   
    REL1 1   
    REL1 0   
    REL2 1   
    REL2 0   
    RPU1 1500   
PRU1 1500 Pulses the relay 1 for 1500 ms.
Please see here for more details about the default firmware:   
[https://github.com/andino-systems/Andino-X2/tree/master/src/firmware](https://github.com/andino-systems/Andino-X2/tree/master/src/firmware)

----------
### Prepare the Debian
1. Release the ttyAMA0 from getty and asign it to GIO14, GPIO15
2. Add a driver for the SPI UART
3. Add a driver for the SPI Ethernet controller

#### Enable UART to GPIO

The UART on the GPIO is used for the 2G/GPRS Modem 

    sudo nano /boot/config.txt

add this at the end of the file..

    enable_uart=1   
    dtoverlay=pi3-disable-bt-overlay
    dtoverlay=pi3-miniuart-bt   

Stop getty on serial0:   

    sudo nano /boot/cmdline.txt 

Remove “**console=…**   
dwc_otg.lpm_enable=0 **console=serial0,115200 console=tty1** root=/dev/mmcblk0p2 rootfstype=ext4 .....

#### Add the Driver for the SPI UART

The first channel goes to the MPM Slot 1.
The second  channnel of the SPI UART is used to communicate with the Atmel Controller.

    cd ~/
    wget https://github.com/andino-systems/Andino-X2/raw/master/src/sc16is752-spi0-ce1.dtbo
    sudo cp sc16is752-spi0-ce1.dtbo /boot/overlays/
    sudo nano /boot/config.txt
Add or uncomment this  

    dtparam=spi=on
    dtoverlay=sc16is752-spi0-ce1

Then
    
    sudo reboot now
    ls /dev/ttySC*

ttySC0 – to MPM1   
ttySC1 – to Arduino   

Test the UART. (You can exit with CTRL-A then X)

    sudo nano minicom –setup
    # press Serial Port Setup

    +-----------------------------------------------------------------------+
    | A -    Serial Device      : /dev/ttySC1                               |
    | B - Lockfile Location     : /var/lock                                 |
    | C -   Callin Program      :                                           |
    | D -  Callout Program      :                                           |
    | E -    Bps/Par/Bits       : 38400 8N1                                 |
    | F - Hardware Flow Control : No                                        |
    | G - Software Flow Control : No                                        |
    |                                                                       |
    |    Change which setting?                                              |
    +-----------------------------------------------------------------------+
            | Screen and keyboard      |
            | Save setup as dfl        |
            | Save setup as..          |
            | Exit                     |
            | Exit from Minicom        |
            +--------------------------+

#### Add a driver for the SPI Ethernet controller

    sudo nano /boot/config.txt
add at the end:  
 
dtoverlay=enc28j60

    sudo reboot now
    ...
    ifconfig
    eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
    inet 192.168.178.74  netmask 255.255.255.0  broadcast 192.168.178.255
    inet6 fe80::50a1:6c17:239b:847c  prefixlen 64  scopeid 0x20<link>
    ether b8:27:eb:45:a3:bc  txqueuelen 1000  (Ethernet)
    RX packets 2077  bytes 186482 (182.1 KiB)
    RX errors 0  dropped 3  overruns 0  frame 0
    TX packets 1649  bytes 257186 (251.1 KiB)
    TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
    
    eth1: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
    ether c6:97:8d:3a:22:b6  txqueuelen 1000  (Ethernet)
    RX packets 0  bytes 0 (0.0 B)
    RX errors 0  dropped 0  overruns 0  frame 0
    TX packets 0  bytes 0 (0.0 B)
    TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
    device interrupt 191
    
#### Add the Driver for the RTC DS3231

Enable I2C and add the Module the RTC DS3231

	sudo nano /boot/config.txt
uncomment dtparam=i2c_arm=on and add the dtoverlay=i2c-rtc,ds3231

	dtparam=i2c_arm=on
	#dtparam=i2s=on
	#dtparam=spi=on
	dtoverlay=i2c-rtc,ds3231

Execute this

	sudo -s
	chmod +x /etc/rc.local 
	apt-get install -y i2c-tools
	apt-get purge -y fake-hwclock 
	apt-get remove fake-hwclock -y 
	dpkg --purge fake-hwclock 
	rm -f /etc/adjtime. 
	cp /usr/share/zoneinfo/Europe/Berlin /etc/localtime
	ln -s /home/pi/bin/ntp2hwclock.sh /etc/cron.hourly/ntp2hwclock
	sudo reboot now

After reboot test this RTC

	pi@raspberrypi:~ $ i2cdetect -y 1

	     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	00:          -- -- -- -- -- -- -- -- -- -- -- -- --
	10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
	70: -- -- -- -- -- -- -- --
	pi@raspberrypi:~ $

	hwchwclock -w
	hwclock -r

This Python script sets the NTP Time to the HWClock as long a NTP connection can established.
Place this script at /home/pi/bin/ntp2hwclock.sh (for example, see above)

	#!/bin/bash
	# Location of logfile
	LOGFILE="/usr/local/oeebox/etc/log/ntp.log"
	if [ ! -f $LOGFILE ]; then
	  touch $LOGFILE
	fi
	# Set the maximum allowed difference in seconds between Hw-Clock and Sys-Clock
	maxDiffSec="2"
	msgNoConnection="No connection to time-server"
	msgConnection="Connection to time-server"
	# Check for NTP connection
	if ( ntpq -p | grep -q "^*"  ); then
	        echo $msgConnection >> $LOGFILE
	        echo "---------------------------------"  >> $LOGFILE
	        secHwClock=$(sudo hwclock --debug | grep "^Hw clock time" | awk '{print $(NF-3)}')
	        echo "HwClock: $secHwClock sec" >> $LOGFILE
	        secSysClock=$(date +"%s")
	        echo "SysClock: $secSysClock sec" >> $LOGFILE
	        echo "---------------------------------" >> $LOGFILE
	        secDiff=$(($secHwClock-$secSysClock))
	        # Compute absolute value
	        if ( echo $secDiff | grep -q "-" ); then
	            secDiff=$(echo $secDiff | cut -d "-" -f 2)
	        fi
	        echo "Difference: $secDiff sec" >> $LOGFILE
	        msgDiff="HwClock difference: $secDiff sec"
	        if [ "$secDiff" -gt "$maxDiffSec" ] ; then
	                echo "---------------------------------" >> $LOGFILE
	                echo "The difference between Hw- and Sys-Clock is more than $maxDiffSec sec." >> $LOGFILE
	                echo "Hw-Clock will be updated" >> $LOGFILE
	                # Update hwclock from system clock
	                sudo hwclock -w
	                msgDiff="$msgDiff --> HW-Clock updated." >> $LOGFILE
	        fi
	        if !(awk '/./{line=$0} END{print line}' $LOGFILE | grep -q "$msgConnection") || [ "$secDiff" -gt "$maxDiffSec" ]; then
	                echo $(date)": "$msgConnection". "$msgDiff >> $LOGFILE
	        fi
	else
	        # No NTP connection
	        echo $msgNoConnection
	        if !(awk '/./{line=$0} END{print line}' $LOGFILE | grep -q "$msgNoConnection"); then
	                echo $(date)": $msgNoConnection" >> $LOGFILE
	        fi
	fi

----------
### Communication with the Atmel Controller

The UART communication with the Atmel Controller can be switched to either the Raspberry Pi or a external PC with a USB Adapter. With this the firmware development can be easily done with a PC.
The selection is done by two jumpers.
![Andino X2 - Arduino jumper location](andino-x2-connection-mode.jpg)

If the Jumpers are pluged in the communication is between the Pi and the Atmel.    
If the Jumpers are *not*  pluged in, the communication is between the Atmel and the PC.

#### Atmel Controller <-> Raspberry Pi

![Andino X2 - Raspberry with Atmel](andino-x2-raspberry-mode.jpg)

The device to communicate with the Atmel is /dev/ttySC1  
The Baudrate with the default firmware is 38400 Baud with 8/n/1 and no hardware flow control.
You can easily check this with minicom

    sudo aptget install minicom
    sudo minicom --setup
    
Use ctrl-a and then x to exit minicom. 
   
#### Atmel Controller <-> PC via USB

The Andino X2 allows to program the Atmel Controller from the Raspberry Pi with avrdude or from a PC via a USB Adapter.  
This Adapter is available at our shop or from ebay.  
![Andino X2 - Arduino-USB to Atmel](andino-x2-arduino-connector.jpg)
  
The Jumper at the Atmel connection Selector must not be pluged in.   
![Andino X2 - Arduino with Atmel](andino-x2-arduino-mode.jpg)

To program the Atmel Controller with the Arduino IDE just install the Mighty Core Boards.  
Please visit this site [https://github.com/MCUdude/MightyCore](https://github.com/MCUdude/MightyCore) and see the section "How to install"  
Use this settings inside the Arduino IDE:  
![Andino X2 - Arduino settings](andino-x2-arduino-ide-settings.png)  

The initial firmware installed on the X2 and the settings can be found here:    
[https://github.com/andino-systems/Andino-X2/tree/master/src/firmware](https://github.com/andino-systems/Andino-X2/tree/master/src/firmware)


----------
### Communication with Modem (optional)
![Andino X2 - GSM connector](k11-gsm-plug.png)

----------
### Drive the LED from Raspberry

The six LED in the Lid are RGB WS2812. There are driven by the GPIO 18.  
***Please note: Drive the LED with a maximim brightness of 50% (Value 128)!***     
You can drive the left three LED from the Raspberry and the three left from the Atmel.  
If you short cut the Jumper J1 you be able to controll all six LED from the Raspberry.
![Andino X2 - Arduino with Atmel](andino-x2-led.png)

HoTo install the software: 


    sudo apt-get update
    sudo apt-get install gcc make build-essential python-dev git scons swig
    git clone https://github.com/jgarff/rpi_ws281x
    cd rpi_ws281x/
    sudo scons
    cd python
    sudo python setup.py build
    sudo python setup.py install

Now set the configuration in the example file

    cd examples
    nano lowlevel.py

change the LED COUNT and the LED GPIO and the Brightness:


    # LED configuration.
    LED_CHANNEL = 0
    LED_COUNT   = 3      # How many LEDs to light.
    LED_FREQ_HZ = 800000 # Frequency of the LED signal.  Should be 800khz or 400khz.
    LED_DMA_NUM = 5      # DMA channel to use, can be 0-14.
    LED_GPIO    = 18	 # GPIO connected to the LED signal line.  Must support PWM!
    LED_BRIGHTNESS = 128 # Set to 0 for darkest and 255 for brightest
    LED_INVERT = 0  # Set to 1 to invert the LED signal, good if using NPN
    
Test

    sudo python lowlevel.py
	
You will find an example of how to drive the LED from the Atmel in the [Firmware section](https://github.com/andino-systems/Andino-X2/tree/master/src/firmware)

----------
### Firmware for the Atmel Controller
Please see here   
[https://github.com/andino-systems/Andino-X2/tree/master/src/firmware](https://github.com/andino-systems/Andino-X2/tree/master/src/firmware)

----------
### Setup the 2G EDGE Driver SIMCom800L

Open the Housing and insert the Micro SIM.
![Andino X2 - Insert SIM Card](andino-x2-gsm.png)

Remove PIN from SIM!

The Modem is connected to the internal UART of the Raspberry Pi.  
The Reset Line of the Modem is connected to GPIO 17.

#### Useful scripts
see at top to download.

    ./init.sh   	initialize the Ports to reset SIM800
    ./stop.sh   	Stop (hold in reset) the SIM800
    ./start.sh  	Start the SIM800 (release reset)
    ./restart.sh	Stop and Start again
    
    ./dial.sh   	Init the DialIn
    ./hangup.sh 	Shutdown PPP
    
    ./log.sh		Tail the log
    
#### Useful commands

	ifconfig
	ping 8.8.8.8
	sudo route add default dev ppp0
	sudo ifconfig eth0 down
	sudo ifconfig eth0 up

#### Prepare the Debian

    sudo nano /boot/cmdline.txt
    remove console...
    dwc_otg.lpm_enable=0   >>console=serial0,115200 console=tty1<<   root=/dev/mmcblk0
    
    sudo nano /boot/config.txt
    Add to the end:
    
    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    enable_uart=1
    dtoverlay=pi3-disable-bt-overlay
    dtoverlay=pi3-miniuart-bt
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    reboot
    
    
    sudo apt-get install ppp screen elinks minicom
    
    Test Modem: 
    sudo minicom --setup
    +-----------------------------------------------------------------------+
    | A -Serial Device  : /dev/ttyAMA0  |
    | B - Lockfile Location : /var/lock |
    | C -   Callin Program  :   |
    | D -  Callout Program  :   |
    | E -Bps/Par/Bits   : 115200 8N1|
    | F - Hardware Flow Control : No|
    | G - Software Flow Control : No|
    |   |
    |Change which setting?  |
    +-----------------------------------------------------------------------+
    
    minicom
    at
    OK
    ati
    SIM800 R14.18
    OK
    # Show Error as text
    AT+CMEE=2
    # SIM Ready?
    AT+CPIN?
    +CPIN: READY
    # Network available?
    AT+COPS?
    +COPS: 0,0,"D1"
    # Network quality?
    AT+CSQ
    +CSQ: 4,0
    # Set APN
    AT+CGDCONT=1,"IP","internet.telekom","",0,0


#### Setup ppp
"internet.telekom" is my APN and *must* be changed to the providers APN


    
    cd /etc/ppp/peers/
    sudo nano rnet 
    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    #imis/internet is the apn for idea connection
    connect "/usr/sbin/chat -v -f /etc/chatscripts/gprs -T internet.telekom"
    
    # For Raspberry Pi3 use /dev/ttyS0 as the communication port:
    /dev/ttyAMA0
    
    # Baudrate
    115200
    
    # Assumes that your IP address is allocated dynamically by the ISP.
    noipdefault
    
    # Try to get the name server addresses from the ISP.
    usepeerdns
    
    # Use this connection as the default route to the internet.
    defaultroute
    
    # Makes PPPD "dial again" when the connection is lost.
    persist
    
    # Do not ask the remote to authenticate.
    noauth
    
    # No hardware flow control on the serial link with GSM Modem
    nocrtscts
    
    # No modem control lines with GSM Modem
    local
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    
    sudo nano /etc/chatscripts/gprs
    
    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    # You can use this script unmodified to connect to cellular networks.
    # The APN is specified in the peers file as the argument of the -T command
    # line option of chat(8).
    
    # For details about the AT commands involved please consult the relevant
    # standard: 3GPP TS 27.007 - AT command set for User Equipment (UE).
    # (http://www.3gpp.org/ftp/Specs/html-info/27007.htm)
    
    ABORT   BUSY
    ABORT   VOICE
    ABORT   "NO CARRIER"
    ABORT   "NO DIALTONE"
    ABORT   "NO DIAL TONE"
    ABORT   "NO ANSWER"
    ABORT   "DELAYED"
    ABORT   "ERROR"
    
    # cease if the modem is not attached to the network yet
    ABORT   "+CGATT: 0"
    
    ""  AT
    TIMEOUT 12
    OK  ATH
    OK  ATE1
    
    # +CPIN provides the SIM card PIN
    #OK "AT+CPIN=1234"
    
    # +CFUN may allow to configure the handset to limit operations to
    # GPRS/EDGE/UMTS/etc to save power, but the arguments are not standard
    # except for 1 which means "full functionality".
    #OK AT+CFUN=1
    
    OK  AT+CGDCONT=1,"IP","\T","",0,0
    OK  ATD*99#
    TIMEOUT 22
    CONNECT ""
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#### Check the connection

    dial:
    
    sudo pon rnet
    
    check:
    tail -n 30 /var/log/syslog
    ifconfig
    
    set route:
    sudo route add default dev ppp0
    ping 8.8.8.8
    
    hang off:
    sudo poff rnet


#### Start while booting

    sudo nano /etc/network/interfaces
    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    # interfaces(5) file used by ifup(8) and ifdown(8)
    
    # Please note that this file is written to be used with dhcpcd
    # For static IP, consult /etc/dhcpcd.conf and 'man dhcpcd.conf'
    
    # Include files from /etc/network/interfaces.d:
    
    auto rnet
    iface rnet inet ppp
    provider rnet
      
    source-directory /etc/network/interfaces.d
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    sudo nano /etc/rc.local
    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ...
    ...
    
    printf "Restart SIM800L\n"
    sudo echo "21" > /sys/class/gpio/export
    sudo echo "out" > /sys/class/gpio/gpio21/direction
    sudo echo "1" > /sys/class/gpio/gpio21/value
    sudo echo "0" > /sys/class/gpio/gpio21/value
    sudo echo "1" > /sys/class/gpio/gpio21/value
    
    exit 0
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Useful Links   
[How to use Raspberry Pi with SIM900 GSM/GPRS add-on with bash](https://www.bashpi.org/?page_id=459 )  
[How to use internet using PPPD and SIM800 GSM addon?](https://raspberrypi.stackexchange.com/questions/44597/how-to-use-internet-using-pppd-and-sim800-gsm-addon )  

### Schematics

[Here are the Schematics of the Andino X2](schematic-andino-X2.pdf)

