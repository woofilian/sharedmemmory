// revision history:
// 2012-08-07
// support english, simple chinese, traditional chinese, italian, russian, Portuguese
//
// 2012-09-06
// distinguish unsecured AP
//
// 2012-11-12
//
// 2012-12-03 add language type: germany, french, spanish, netherlands

// 2012-12-10 change language order into english, simple chinese, traditional chinese, italian, russian, Portuguese
// German,  Spanish,  Dutch,  French.
// JavaScript Document
var language=[
//for public
["Fehler: (Kann keinen XMLHTTP-Vorgang erstellen!"],
["Kann nicht mit Kamera verbinden!"],
["Informationen erforderlich."],
["Abbrechen"],
["Fortsetzen"],
["Login läuft ... bitte warten"],
["Konto wird erstellt ... bitte warten"],
["Anmeldung Kamera läuft ... bitte warten"],
["Seite wird fertiggestellt ... bitte warten"],
["Verbindung mit Ihrem Drahtlos-Netzwerk wird hergestellt. <br>Das kann einige Minuten dauern."],
["Navigation zur Internet-Seite ... bitte warten"],
["Bitte achten Sie darauf, dass Sie Ihre E-Mail-Adresse korrekt eingeben. Sie wird benötigt, um Informationen zu Ihrem Konto abzurufen."]
];

var language1=[
//CamPreview.asp 
["Kameravorschau Einstellungen"],
["Kameravorschau"],
["Herzlichen Glückwunsch, Sie haben erfolgreich "],
["mit Ihrer Kamera verbunden"],
["Kamera mit Internet verbinden"]
];

var language2=[
// aplist.asp
["Netzwerkeinstellungen"],
["Mit Internet verbinden"],
["WLAN-Netzwerk wählen..."],
["Falls Sie sich über Ethernetkabel verbinden möchten"],
["klicken Sie bitte auf „Ethernet benutzen“ unten."],
["Verbindung mit Netzwerk wird hergestellt ... bitte warten"],
["Ethernet benutzen"],
["Bitte Netzwerk-Passwort eingeben<br> &nbsp;&nbsp;&nbsp;für "],
//["Verbindung mit Ihrem WLAN-Router kann nicht hergestellt werden! Bitte überprüfen Sie Ihr Passwort und versuchen Sie es noch einmal! Fehlercode: "],
["Das eingegebene WLAN-Kennwort ist nicht korrekt. Bitte versuchen Sie es erneut. "],
//["IP-Adresse von Ihrem WLAN-Router kann nicht erhalten werden! Bitte überprüfen Sie Ihr Passwort und versuchen Sie es noch einmal! Fehlercode: "],
["Die Kamera kann keine Verbindung herstellen. Bitte überprüfen Sie Ihren Router und versuchen Sie es erneut. "],
["Ihr Netzwerk-Passwort muss mindestens 8 Schriftzeichen enthalten."],
["Ungültige Schriftzeichen im Passwort."],
["Bitte geben Sie 5 oder 13 Schriftzeichen für den WEP ASCII Schlüssel ein"],
["Ungültige Schriftzeichen im WEP ASCII Schlüssel."],
[" oder geben Sie 10 oder 26 Schriftzeichen für den WEP Hex Schlüssel ein."],
["Ungültiges WEP-Schlüsselformat."],
["Mit bekannten Netzwerken wird jetzt automatisch verbunden. "],
["Stehen keine bekannten Netzwerke "],
["zur Verfügung, so müssen Sie eins zum Verbinden wählen."],
[""],
["Ihre Kamera erkennt ein schwaches WLAN-Signal, während die Verbindung mit Ihrem Drahtlos-Netzwerk hergestellt wird. Ein schwaches WLAN-Signal verursacht schlechte Videoleistung wie übersprungene Bildschirminhalte und Bildverschlechterung. Stellen Sie die Kamera dichter an Ihrem Drahtlos-Router oder Zugangspunkt auf, um ein stärkeres Signal und eine verbesserte Kameraleistung zu erhalten."],
["Passwort"],
["Drahtlos-Netzwerk wird gescannt ... bitte warten"],
["Netzwerk-Informationen eingeben"],
["Anderes Netzwerk"],
["Name"],
["Sicherheit"],
["Sicherheit/Verschlüsseln"],
["Keine"],
["Bitte geben Sie Ihren Netzwerknamen ein"],
//["Vergewissern Sie sich, dass Ihr Ethernetkabel angeschlossen ist und versuchen Sie es noch einmal! Fehlercode: "],
["Vergewissern Sie sich, dass Ihr Ethernetkabel angeschlossen ist und versuchen Sie es noch einmal!"],
["Erfrischen"],
["Überprüfen Sie bitte, ob Sie mit Ihrer Kamera verbunden sind."],
["Öffnen"],
["Freigeben"],
//["Kann nicht mit Internet verbinden! Bitte überprüfen Sie Ihre Internetverbindung und versuchen Sie es noch einmal! Fehlercode: "],
["Die Kamera kann keine Internetverbindung herstellen. Funktioniert Ihre Internetverbindung? "],
["Bitte Ethernetkabel für Ihre Kamera anschließen."],
["Passwort anzeigen"],
["Verbindung mit Ihrem WLAN-Router kann nicht hergestellt werden! Bitte überprüfen Sie Ihr Netzwerk und versuchen Sie es noch einmal!"],
["Dieser Kamera-Server ist zur Zeit leider nicht verfügbar. "],
[""]
];

var language3=[
//CreateAccount.asp
["Kontoeinstellungen erstellen"],
["Mit Server verbinden"],
["Erstellen Sie Ihr Konto"],
["Benutzername"],
["Passwort"],
["Bestätigen"],
["E-Mail"],
["Haben Sie bereits ein Konto, so klicken sie bitte auf „Bestehendes benutzen“."],
["Bestehendes benutzen"],
["Nur Buchstaben (a-z, A-Z), Ziffern (0-9), Unterstreichungen (_), Punkte (.), Strich (-) und @ sind als Benutzername zulässig."],
["Unterstreichungen (_), Punkte (.), Strich (-) und @ dürfen nicht das erste Schriftzeichen eines  Benutzernamens sein."],
["Das Passwort muss mindestens 6 Schriftzeichen haben."],
["Die eingegebenen Passworte stimmen nicht überein. Bitte versuchen Sie es noch einmal."],
["Ungültige E-Mail-Adresse."],
["Vorname"],
["Nachname"],
["Hier sind die Kontoeinstellungen, die Sie eingegeben haben. Bitte überprüfen Sie, ob sie korrekt sind, bevor Sie fortfahren."],
// ["Sie müssen diese E-Mail-Adresse später mit einer E-Mail, die wir Ihnen schicken, bestätigen. "],
["Rufen Sie Ihre E-Mails in den nächsten 24 Stunden ab. Sie können danach erst auf Ihr Konto zugreifen, wenn Sie auf den Validierungslink geklickt haben, den wir Ihnen geschickt haben"],
["Erstellen Sie Ihr Linksys Smart Wi-Fi-Konto."],
["Kennwort: Unsicher"],
["Kennwort: Sicher"],
["Versuchen Sie 7 oder mehr Groß- und Kleinbuchstaben, Ziffern und Sonderzeichen miteinander zu kombinieren, um die Kennwortsicherheit zu maximieren. "],
["Konto erstellen"],
["Die E-Mails stimmen nicht überein."],
["Das Kennwort muss 6 – 128 Zeichen lang sein und mindestens eine Ziffer und einen Buchstaben enthalten."]
];

var language4=[
///login.asp 
["Login-Einstellungen"],
["Mit Server verbinden"],
["Geben Sie Ihr Konto ein"],
["Benutzername"],
["Passwort"],
["Anmelden"]
];

var language5=[
//CamRegister.asp
["Kamera Anmeldeeinstellungen"],
["Mit Server verbinden"],
["Kamera Informationen"],
["Kamera Name"],
["Beschreibung"],
["Fertigstellen"]
];

var language6=[
["WLAN-Einstellungen ausschalten"],
["Bitte stellen Sie den Schalter nach UNTEN, um die Einstellungen zu beenden."],
["Warnung: Ihre Kamera wird vorher nicht initialisiert!"],
["Bitte stellen Sie den Schalter \„WLAN-Einstellungen\“ auf AUS, um fortzufahren."],
[""],
[""],
[""],
[""],
["Bitte stellen Sie den Schalter nach UNTEN, um die Einstellungen zu beenden."]
];

var language7=[
["sonstige.."],
["Sie wählen .."],
["Fehler eingetreten."],
["Möchten Sie sich weiterhin mit einem ungesicherten Netzwerk verbinden?"],
["OK"]
];

var language8=[
["Kennwort oder E-Mail-Adresse ist ungültig. Bitte versuchen Sie es erneut."],
["Das Linksys Smart Wi-Fi Konto für {username} wurde deaktiviert. "],
["Ihre Anmeldung wurde nicht validiert. Bitte überprüfen Sie Ihre E-Mails auf die Validierungsmeldung."],
["Ein Linksys Smart Wi-Fi-Konto für {username} ist nicht vorhanden. "], 
["Bei der Kommunikation mit den Linksys Smart Wi-Fi-Diensten ist ein unerwarteter Fehler aufgetreten. Versuchen Sie es später erneut, wenn Ihnen eine Internet-Verbindung zur Verfügung steht."],
["Ein Linksys Smart Wi-Fi-Konto für {username} ist bereits vorhanden. "], 
["Ihr Konto wurde nach mehreren fehlgeschlagenen Anmeldeversuchen gesperrt. "],
["Die Kamera kann keine Verbindung zum Server herstellen. Stellen Sie sicher, dass Ihr Netzwerk mit dem Internet verbunden ist und versuchen Sie es erneut."],
["Die Kamera kann keine Verbindung zum Server herstellen. Stellen Sie sicher, dass Ihr Netzwerk mit dem Internet verbunden ist und versuchen Sie es erneut."]
];