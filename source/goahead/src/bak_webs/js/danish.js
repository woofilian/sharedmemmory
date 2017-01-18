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
["Fejl :( Kan ikke oprette en XMLHTTP-forekomst!"],
["Kan ikke oprette forbindelse til kamera!"],
["Information er nødvendig."],
["Annuller"],
["Fortsæt"],
["Logger ind ... vent venligst"],
["Opretter en konto ... vent venligst"],
["Registrerer kamera ... vent venligst"],
["Går til udførselssiden ... vent venligst"],
["Opretter forbindelse til dit trådløse netværk. <br>Dette kan taget et par minutter."],
["Går til siden Konfiguration af internet ... vent venligst"],
["Sørg for at du har indtastet din e-mail korrekt. Den vil blive brugt til at indhente kontoinformationer."]
];

var language1=[
//CamPreview.asp 
["Konfiguration af kamerafremviser"],
["Kamerafremviser"],
["Tillykke! Du har "],
["oprettet forbindelse til dit kamera"],
["Opret forbindelse fra kamera til internet"]
];

var language2=[
// aplist.asp
["Konfiguration af netværk"],
["Opret forbindelse til internettet"],
["Vælg et Wi-Fi-netværk..."],
["Hvis du vil oprette forbindelse via ethernet-kabel,"],
["skal du klikke på knappen 'Brug ethernet' nedenfor."],
["Opretter forbindelse til netværk ... vent venligst"],
["Brug ethernet"],
["Angiv netværkets adgangskode<br> &nbsp;&nbsp;&nbsp;for "],
//["Der kan ikke oprettes forbindelse til din WiFi-router! Kontrollér din adgangskode og forsøg igen! Fejlkode: "],
["Det kodeord til Wi-Fi som du har indtastet er forkert. Forsøg igen. "],
//["Kan ikke få IP-adresse fra din WiFi-router! Kontrollér din adgangskode og forsøg igen! Fejlkode: "],
["Kamera kan ikke oprette forbindelse. Kontrollér din router og forsøg igen. "],
["Din adgangskode til netværket skal indeholde mindst 8 tegn."],
["Ugyldige tegn i adgangskode."],
["Angiv 5 eller 13 tegn til WEP ASCII-nøgle"],
["Forkerte tegn i WEP-nøgle."],
[" eller angiv 10 eller 26 tegn til WEP-hexnøgle."],
["Ugyldig WEP-nøgleformat."],
["Kendte netværker vil blive tilføjet "],
["automatisk. Hvis ingen kendte netværker er "],
["tilgængelige, skal du vælge et."],
[""],
["Dit kamera er udsat for et svagt WiFi-signal, når der oprettes forbindelse via dit trådløse netværk. Svagt WiFi-signal vil give dårlig  videoydeevne, såsom afbrudte rammer og billedsløring. Anbring kameraet tættere på din trådløse router eller adgangspunkt for at få et stærkere signal og bedrer kameraydeevne."],
["Adgangskode"],
["Scanner trådløse netværk ... vent venligst"],
["Angiv netværksoplysninger"],
["Andet netværk"],
["Navn"],
["Sikkerhed"],
["Sikkerhed/Kryptering"],
["Ingen"],
["Angiv dit netværksnavn"],
["Kontrollér, at dit ethernet-kabel er forsvarligt tilsluttet og prøv igen!"],
["Opdater"],
["Kontrollér, som du har oprettet forbindelse til dit kamera."],
["Åbn"],
["Del"],
//["Kan ikke oprette forbindelse til internettet! Kontrollér din internetforbindelse og prøv igen! Fejlkode: "],
["Kamera kan ikke komme på internettet. Virker din internetforbindelse? "],
["Sæt ethernetkablet i til dit kamera."],
["Vis adgangskode"],
["Der kan ikke oprettes forbindelse til din WiFi-router! Kontrollér dit netværk og prøv igen!"],
["Kameraets server er ikke tilgængelig lige nu. "],
[""]
];

var language3=[
//CreateAccount.asp
["Konfigurér kontoindstillinger"],
["Opret forbindelse til serveren"],
["Opret din konto"],
["Brugernavn"],
["Adgangskode"],
["BEkræft"],
["E-mail"],
["Hvis du allerede har en konto, bedes du klikke på knappen 'Brug eksisterende' nedenfor."],
["Brug eksisterende"],
["Kun bogstaver (a-z A-Z), tal (0-9), understregninger (_), punktummer (.), bindestreg (-) og @ er tilladt til brugernavn."],
["Understregninger (_), punktummer (.), bindestreg (-) og @ må ikke være det første tegn i brugernavnet."],
["Adgangskode skal have en længde på mindst 6 tegn."],
["De angivne adgangskoder er ikke ens. Forsøg igen."],
["E-mail-adressen er ugyldig."],
["Fornavn"],
["Efternavn"],
["Her er de kontoindstillinger du har indtastet. Bekræft venligst at de er korrekte inden du fortsætter."],
//["Du vil senere skulle validere denne e-mailadresse via en e-mail som vi sender til dig."],
["Sørg for at tjekke din email inden for de næste 24 timer. Du vil først kunne få adgang til din konto, når du har trykket på det godkendelses-link, vi sender til dig."],
["Opret din Linksys Smart Wi-Fi konto."],
["Adgangskode: Svag"],
["Adgangskode: Stærk"],
["Prøv med en blanding af 7 eller flere store og små bogstaver, tal og symboler for at øge adgangskoden styrke."],
["Opret Konto"],
["Email-adresserne stemmer ikke."],
["Adgangskoden skal være 6 – 128 anslag og indeholde mindst ét tal og ét bogstav."]
];

var language4=[
///login.asp 
["Konfiguration af log ind"],
["Opret forbindelse til serveren"],
["Indtast din konto"],
["Brugernavn"],
["Adgangskode"],
["Log på"]
];

var language5=[
//CamRegister.asp
["Konfiguration af kameraregister"],
["Opret forbindelse til serveren"],
["Kameraoplysninger"],
["Kameranavn"],
["Beskrivelse"],
["Udfør"]
];

var language6=[
["Sluk for WiFi-konfiguration"],
["Sæt kontakten i NED-position for at fuldføre konfigurationsprocessen."],
["Advarsel: Dit kamera starter ikke, før dette er færdig!"],
["Sæt \"WiFi-konfiguration\"-kontakten til OFF for at fortsætte."],
[""],
[""],
[""],
[""],
["Sæt kontakten i NED-position for at fuldføre konfigurationsprocessen."]
];

var language7=[
["andet.."],
["du vælger .."],
["der skete en fejl."],
["Ønsker du stadig at oprette forbindelse til et ikke-sikkert netværk?"],
["OK"]
];

var language8=[
["Ugyldig email-adresse eller kodeord. Prøv igen."],
["Linksys Smart Wi-Fi-kontoen for {username} er deaktiveret. "], 
["Dit kodeord er ikke blevet godkendt. Tjek venligst din email for at se godkendelsesbeskeden."],
["Der eksisterer ingen Linksys Smart Wi-Fi-konto for {username}. "], 
["Der var en uventet kommunikationsfejl med Linksys Smart Wi-Fi-service. Prøv igen senere, når du har forbindelse til internettet."],
["Der eksisterer allerede en Linksys Smart Wi-Fi-konto for {username}. "], 
["Din konto er blevet låst efter gentagne fejlforsøg ved login. "],
["Kamera kan ikke forbinde til serveren. Sørg for at dit netværk er forbundet til internettet og prøv igen."],
["Kamera kan ikke forbinde til serveren. Sørg for at dit netværk er forbundet til internettet og prøv igen."]
];