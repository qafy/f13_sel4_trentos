entropy source 

können wir dummy verwenden in der software basierten variante oder sollen wir pseudo rng implementieren 


crypto 

wie genau soll verschlüsselt werden 
rsa asymmetrisch wird von srv.py verlangt aber das wird von os_crypto api nicht unterstützt

selber mit mbedtls machen?

oder anderen server schreiben der diffie helman + symmetrische verschlüsselung verwendet 


keystore api 

was sollen wir speichern? 
falls statisch festgelegt, sollen wir ihn auf die sd karte speichern und dann mit keystore api ansprechen, oder ramdisk erstellen und dort dann keys reinladen und dann wieder rausladen????

falls dynamisch, sollen wir protokoll schreiben, das beim start, schlüssel austauscht und dann speichert, server muss wieder angepasst dann werden

NOCH FRAGEN AUF ZULIP sollen wir schlüssel immer erst kurz vor verwendung laden oder einfach als globale variable speichern 

secure communication component

wie viele benutzer sollen unterstützt werden, genau so wie bei network stack 8 clienten oder reicht einer?

sollen wir das bestimmte methoden irgendwie wieder verpacken so wie das in den anderen komponenten gemacht wird?

tpm based solution 

was sollen wir bei os crypto anbieten? es wird nicht alles verwendet dh wir können auch nicht alles benchmarken 

soll tpm eine eigene komponente sein mit der man über rpc kommuniziert so wie crypto server oder soll man das versuchen so wie bei der crypto api hinzubekommen ohne komponenten 

Komponenten design, in der software basierten version, sind crypto und keystore intern, bei tpm extern, wäre es nicht sinnvoller das bei software auch extern zu machen, damit struktur gleich bleibt und man die komponenten einfach austauschen kann



anmerkungen 

nic_dummy hat evtl nen fehler 

dokumentation bei nic_dummy und entropy source sind falsch

beispiel bei filesystem api ist falsch hfs und hfile vertauscht + semikolon fehlt

