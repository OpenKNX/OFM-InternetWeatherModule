# OFM-InternetWheaterModule

Dies ist ein Modul zur Integration von Internet Wetterdiensten.
Es wird das [OFM-Network](https://github.com/OpenKNX/OFM-Network) oder OFM-WLAN benötigt.
  
Features
- Aktuelle Werte
- Prognose Heute
- Prognose Morgen

KO's für
- Wetterbeschreibung
- Tagestemperatur
- Nachttemperatur
- Frühtemperatur
- Abendtemperatur
- Minimum Temperatur
- Maximum Temperatur
- Gefühlte Tagestemperatur
- Gefühlte Nachtemperatur
- Gefühlte Frühtemperatur
- Gefühlte Abendtemperatur
- Luftdruck
- Luftfeuchte
- Wind
- Windböen
- Windrichtung
- Regen
- Schnee
- UV-Index
- Wolken

# Wetterdienste

Derzeit wird nur OpenWheaterMap als Wetteranbieter unterstützt. 
Die SW-Architektur ist aber für weitere Dienste vorbereitet.
Pull Requests für andere Dienste sind willkommen!

## OpenWheaterMap

Für die Anfragen wird ein API Key von openwheatermap.com benötigt.
1000 Aufrufe pro Tag können gratis durchgeführt werden, jedoch muss auch dafür ein Account angelegt werden und die Subscription für das `One Call API 3.0` aktiviert werden. 
Bei der Subscription sollte das `Call per day limit` auf 1000 eingestellt werden, damit keine Kosten anfallen können.

![Subscription](doc/IW-Subscription.png)

Siehe https://openweathermap.org/price
