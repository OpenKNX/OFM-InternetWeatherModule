# Konzepte Wetter Neu

Statt wenigen großen Kanälen mir einer umfangreichen (aber unvollständigen) Standardauswahl an Werten soll es nun viele kleine Kanäle geben für die möglichst jeder Werte-Typ ausgewählt werden kann.

Features:
* Es soll z.B. auch möglich sein zusätzlich Pollen-Daten abzufragen die über Open-Meteo oder einen anderen Dienst verfügbar sind.
* Es sollen Aggregationen von Werten nutzbar sein
    * Am besten daher mindestens 3 KOs je Kanal für Alternativ:
        * Eine einfache Nutzung von Min/Avg/Max (nicht bei allen Zeitebenen)
        * 3 Zeitpunkte (nicht bei allen Zeitebenen)
        * 3 verschiedne Werte zum selben Zeitpunkt

## ETS-UI

Die Definition eines Ausgabe soll mehrstufig erfolgen:

### 1. Zeitliche Dimension

Angelehnt an Open-Meteo. Andere Dienste haben z.B. keine  Minuten

* Jetzt
* Tag
* Stunde
* 15 Minuten

### 3. Zeitpunkt/Zeitraum

Für "jetzt" keine Auswahl (-> 3 Werte), sonst:

* Ein Zeitpunkt (mit unterschiedlichen Werten) -> 1 Zeitauswahl + 3 Werte
* Unterschiedliche Zeitpunkte (des selben Wertes) -> 3 Zeitauswahlen + 1 Wert
* Zeitintervall (für Aggregation eines Wertes) -> 2 Zeitauswahl für Beginn/Ende + 1 Wert

### 4. Zeitauswahl (0-3 mal)

Für "jetzt" keine Auswahl, sonst:

* Aktuelles Intervall -> erlaubt anschließend bis zu 3 unterschiedeliche Werte
* -m ... +n in Zukunft
* Vergangenheit
* (nur für Stunde/15Minute) "heute um ..."
* (nur für Tag) nachts/morgens/mittags/abends


### 5. Wert (1-3 mal)

Abhängig von Verfügbarkeit für in 1. gewählte Zeit, z.B, "gefühlt Temperatur".
U.u. sollte die Auswahl auf auf zwei Ebenen aufgeteilt werden um eine bessere Übersicht zu erhalten.

[Werte-Auswahl](werte-open-meteo.md)



### 6. Aggregation (nur für Zeitintervall)

3 Parameter aus:
* Min (Standard für 1.)
* Mittelwert (Standard für 2.)
* Max (Standard für 3.)
* Spannbreite
* Summe
* Median?
* Option ggf. Sumnme bis?



### Beispiel:

1. Stunde
2. gefühlte Temp
3. Aggregation
4. aktuelle Stunde bis Tagesende
5. Maximum

### Anforderungen zur Umsetzbarkeit

* Maximale Temperatur der nächsten 5 Stunden
* Maximale Temperatur bis zum Tagesende
* Maximale Minimaltemperatur der nächsten 7 Tage


## Abfrageermittlung

* Der abgefragte Umfang soll minimiert werden
  * Zunächst minimale Anzahl von Requests
* Requests müssen disjunkt nach Ort erfolgen. D.h.: separat für Geräteort, Ort1, Ort2
* Requests müssen disjunkt nach Radiation direction inkl. ohne erfolgen
* Zeitfenster müssen minimiert gesetzt werden:
  * \[past_days, forecast_days\]
  * \[past_hours, forecast_hours\]
  * \[past_minutely_15; forecast_minutely_15\]

````
Request[location 1..3][]
for c in enabled_channels

````