# comelit-esp8266

Side-project del progetto [comelit-esphome](https://github.com/mansellrace/comelit-esphome).

Questo firmware alternativo, scritto su arduino-ide, si interfaccia al medesimo hardware del progetto madre.

![immagini/pcb_3d.png](https://github.com/mansellrace/comelit-esphome/blob/main/immagini/pcb_3d.png?raw=true)

## Caratteristiche:

- Consumo più basso dell'esp8266: viene disabilitato il wifi riducendo i consumi del microcontrollore, appena 0.11W contro 0.37W della versione con esphome.
- Pin in uscita (D2) che viene attivato per 1 secondo alla ricezione di una chiamata citofonica all'indirizzo preimpostato via firmware. Pin di uscita e tempistiche di attivazione sono facilmente modificabili.
- Pin in ingresso (D7), che quando viene portato a valore logico alto comanda l'apertura del portone principale.
- Pin in ingresso (D8), che quando viene portato a valore logico alto comanda l'apertura del portone secondario.

Il progetto può essere usato per costruire un semplice ripetitore di chiamata citofonica o per aggiungere punti di comando del portone principale.

Si può facilmente interfacciare ad altri sistemi domotici basati su qualsiasi tecnologia, ad esempio ad attuatori knx, shelly, homekit, nice, centrali di allarme, ricevitori/trasmettitori a 433MHz, etc etc. 

## Spiegazione del protocollo e dell'schema elettrico:

Disponibile spiegazione completa [alla pagina del progetto madre](https://github.com/mansellrace/comelit-esphome#hardware)

## Installazione:

- Installare arduino IDE
- Installare la compatibilità con ESP8266 (inserire la stringa http://arduino.esp8266.com/stable/package_esp8266com_index.json su File - preferenze - gestore di schede)
- Aprire il progetto e impostare il proprio codice utente (il numero che viene chiamato dall'esterno nei citofoni con tastiera numerica o il numero codificato tramite dip-switch dietro al posto interno)
- Collegare l'esp al pc ***disconnesso dalla scheda di interfaccia hardware***
- Compilare e installare sull'ESP8266

## Acquisto materiali e pcb

Ho la possibilità di fornire il pcb, i componenti, o anche l'intero hardware già saldato e collaudato.

Se sei interessato, contattami su mansellrace@gmail.com
