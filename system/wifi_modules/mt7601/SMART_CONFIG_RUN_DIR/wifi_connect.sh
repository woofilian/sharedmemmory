#!/bin/sh

iwpriv wlan0 set NetworkType=Infra

# MODE : OPEN
# ----------------------------------------------------------------
if [[ "$1" = "0" ]]; then
    iwpriv wlan0 set AuthMode=OPEN
    iwpriv wlan0 set EncrypType=NONE
    iwpriv wlan0 set SSID=$$_SSID_$$

# MODE : WEP
# ----------------------------------------------------------------
elif [[ "$1" = "1" ]]; then

    iwpriv wlan0 set AuthMode=SHARED
    iwpriv wlan0 set EncrypType=WEP
    iwpriv wlan0 set SSID=$$_SSID_$$
    iwpriv wlan0 set Key1=$$_PSK_$$
    iwpriv wlan0 set DefaultKeyID=1

# MODE : WPAPSK_TKIP
# ----------------------------------------------------------------
elif [[ "$1" = "2" && "$2" = "2" ]]; then

    iwpriv wlan0 set AuthMode=WPAPSK
    iwpriv wlan0 set EncrypType=TKIP
    iwpriv wlan0 set SSID=$$_SSID_$$
    iwpriv wlan0 set WPAPSK=$$_PSK_$$

# MODE : WPAPSK_AES
# ----------------------------------------------------------------
elif [[ "$1" = "2" && "$2" = "3" ]]; then

    iwpriv wlan0 set AuthMode=WPAPSK
    iwpriv wlan0 set EncrypType=AES
    iwpriv wlan0 set SSID=$$_SSID_$$
    iwpriv wlan0 set WPAPSK=$$_PSK_$$

# MODE : WPA2PSK_TKIP
# ----------------------------------------------------------------
elif [[ "$1" = "3" && "$2" = "2" ]]; then

    iwpriv wlan0 set AuthMode=WPA2PSK
    iwpriv wlan0 set EncrypType=TKIP
    iwpriv wlan0 set SSID=$$_SSID_$$
    iwpriv wlan0 set WPAPSK=$$_PSK_$$

# MODE : WPA2PSK_AES
# ----------------------------------------------------------------
elif [[ "$1" = "3" && "$2" = "3" ]]; then

    iwpriv wlan0 set AuthMode=WPA2PSK
    iwpriv wlan0 set EncrypType=AES
    iwpriv wlan0 set SSID=$$_SSID_$$
    iwpriv wlan0 set WPAPSK=$$_PSK_$$

fi

iwpriv wlan0 radio_on
iwpriv wlan0 connStatus

