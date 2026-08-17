# (2026-08-17) v0.5.2

* Unterstützung von optionaler Einbindung in OAM
  * Hinweis: Durch Setzen von `OPENKNX_INTERNETWEATHER_IGNORE` wird das Modul nicht in die Firmware integriert 
* Fix: Fehlender Version-Tag von Missing Version Definition
* Fix: Missing Changelog-Entry (Rollback zu Old Changelog-Format)

# v0.5.1

* Fix: Open-Meteo API hatte die komplette Basis-URL überschrieben
* Fix: KO Morning feels like temperature was filled with evening feels like temperature 


# (2026-02-17) v0.5.0 "Fix und KO-Benennung"

* Fix: Fehler im Speicherlayout behoben
* Feature: Gruppenobjekte umbenannt


# (2025-10-18) v0.4.2 "OpenKNX Update"

* Update: Aktualisierung zur Verwendung mit Common 1.5 und Producer 3.11.0


# (2025-07-25) v0.4.1 "Fix UVI Tageswerte"

* Fix #8: UV-Index heute und morgen war fehlerhaft (Im KO für heute wurden der Wert für morgen ausgegeben, das KO für morgen wurde nicht beschrieben)


# (2025-07-14) v0.4

***Update-Hinweis:*** Falls Open-Meteo als Wetterdienst verwendet wird, 
muss beim Update von früheren Versionen die Nutzung/Lizenz gewählt werden,
damit ein Abruf von Wetterdaten erfolgt.
 

* Erweiterung Open-Meteo
  * Unterstützung von API-Key und Server-URL
  * Dokumentation
* Änderung von Standard-Wetter-Dienst auf Open-Meteo


# Bis v0.3 (2024-08)

Versionen für interne Tests.

