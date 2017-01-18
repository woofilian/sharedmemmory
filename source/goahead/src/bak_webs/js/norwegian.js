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
["Mislykket: (Et XMLHTTP-tilfelle kunne ikke opprettes!)"],
["Kan ikke koble til kamera!"],
["Trenger informasjon."],
["Avbryt"],
["Fortsett"],
["Logger inn ... vent litt"],
["Oppretter brukerkonto ... vent litt"],
["Registrerer kamera ... vent litt"],
["Går til fullføringssiden ... vent litt"],
["Kobler til det trådløse nettverket. <br>Dette kan ta noen minutter."],
["Går til Internettilkoblingssiden... vent litt"],
["Sørg for at du har angitt e-postadressen riktig. Den brukes for å hente inn kontoopplysninger."]
];

var language1=[
//CamPreview.asp 
["Konfigurer kameraforhåndsvisning"],
["Kameraforhåndsvisning"],
["Gratulerer, du er nå "],
["koblet til kameraet"],
["Koble kameraet til Internett"]
];

var language2=[
// aplist.asp
["Nettverkskonfigurasjon"],
["Koble til Internett"],
["Velg et Wi-Fi-nettverk ..."],
["Hvis du vil koble til via Ethernet-kabel"],
["kan du klikke Bruk Ethernet-knappen nedenfor."],
["Kobler til nettverk ... vent litt"],
["Bruk Ethernet"],
["Skriv inn nettverkspassordet<br> &nbsp;&nbsp;&nbsp;for "],
//["Kan ikke koble til Wi-Fi-ruteren! Kontroller passordet og prøv igjen! Feilkode: "],
["Wi-Fi-passordet du oppga er feil. Prøv igjen. "],
//["Får ikke IP-adresse fra Wi-Fi-ruteren! Kontroller passordet og prøv igjen! Feilkode: "],
["Kamera kan ikke etablere tilkobling. Kontroller ruteren og prøv igjen. "],
["Nettverkspassordet skal inneholde minst 8 tegn."],
["Passordet inneholder ugyldige tegn."],
["Skriv 5 eller 13 tegn for WEP ASCII-nøkkel"],
["WEP-nøkkelen inneholder ugyldige tegn."],
[" eller skriv 10 eller 26 tegn for WEP Hex-nøkkel"],
["Ugyldig WEP-nøkkelformat."],
["Kjente nettverk blir tilkoblet "],
["automatisk. Hvis ingen nettverk er "],
["tilgjengelige, må du velge et som du vil kobles til."],
[""],
["Kameraet har svakt Wi-Fi-signal når det kobles til det trådløse nettverket. Svakt Wi-Fi-signal gir dårlig videoytelse, deriblant dropping av enkeltbilder og forringelse av bildet. Plasser kameraet nærere den trådløse ruteren eller tilgangspunktet for å få sterkere signal og bedre kameraytelse."],
["Passord"],
["Søker etter trådløst nettverk ... vent litt"],
["Skriv inn nettverksinformasjonen"],
["Annet nettverk"],
["Navn"],
["Sikkerhet"],
["Sikkerhet/kryptering"],
["Ingen"],
["Skriv inn nettverksnavnet"],
["Kontroller at Ethernet-kabelen er ordentlig tilkoblet og prøv igjen!"],
["Oppdater"],
["Undersøk om du har koblet til kameraet."],
["Åpne"],
["Share"],
//["Kan ikke koble til Internett! Undersøk Internett-forbindelsen og prøv igjen! Feilkode: "],
["Kameraet kan ikke komme på nett. Fungerer Internett-tilkoblingen? "],
["Koble til Ethernet-kabelen til kameraet."],
["Vis passord"],
["Kan ikke koble til Wi-Fi-ruteren! Kontroller nettverket og prøv igjen! Feilkode: "],
["Kameraets server er ikke tilgjengelig nå."],
[""]
];

var language3=[
//CreateAccount.asp
["Opprett brukerkontokonfigurasjon"],
["Koble til server"],
["Opprett brukerkonto"],
["Brukernavn"],
["Passord"],
["Bekreft"],
["E-post"],
["Hvis du allerede har en brukerkonto, klikker du Bruk eksisterende nedenfor."],
["Bruk eksisterende"],
["Bare bokstaver (a-z A-Z), tall (0-9), understrek (_), punktum (.), bindestrek (-) og @ kan brukes i brukernavnet."],
["Understrek (_), punktum (.), bindestrek (-) og @ skal ikke brukes som første tegn i brukernavnet."],
["Passordet må bestå av minst 6 tegn."],
["Passordene som ble skrevet er ikke like. Prøv igjen."],
["Ugyldig E-postadresse."],
["Fornavn"],
["Etternavn"],
["Her er kontoinnstillingene du har angitt. Bekreft at disse stemmer før du fortsetter."],
// ["Du må validere denne e-postadressen senere via en e-post vi sendte deg."],
["Sjekk e-posten innen de neste 24 timene. Du kan ikke få tilgang til kontoen etter dette før du har klikket på valideringskoblingen vi sendte deg."],
["Opprett Linksys Smart Wi-Fi-kontoen din."],
["Passord: Svakt"],
["Passord: Sterkt"],
["Prøv en kombinasjon av 7 eller flere store og små bokstaver, tall og symboler for å gjøre passordet sterkere."],
["Opprett konto"],
["E-postene samsvarer ikke."],
["Passordet må være 6 – 128 tegn langt, og bestå av minst ett tall og én bokstav."]
];

var language4=[
///login.asp 
["Innloggingsoppsett"],
["Koble til server"],
["Skriv inn brukerkonto"],
["Brukernavn"],
["Passord"],
["Logg inn"]
];

var language5=[
//CamRegister.asp
["Kameraregisteroppsett"],
["Koble til server"],
["Kamerainformasjon"],
["Kameranavn"],
["Beskrivelse"],
["Fullfør"]
];

var language6=[
["Slå av WiFi-konfigurasjon"],
["Sett bryteren i NED-stilling for å fullføre installasjonsprosessen."],
["Obs!: kameraet blir ikke initialisert før dette er gjort!"],
["Sett \"WiFi Setup\"-bryteren i AV-stilling for å fortsette."],
[""],
[""],
[""],
[""],
["Sett bryteren i NED-stilling for å fullføre installasjonsprosessen."]
];

var language7=[
["Annet ..."],
["Du er i ferd med å velge ..."],
["Det har oppstått en feil."],
["Vil du fremdeles koble til et usikret nettverk?"],
["OK"]
];

var language8=[
["Ugyldig e-postadresse eller passord. Prøv på nytt."],
["Linksys Smart Wi-Fi-kontoen for {username} er blitt deaktivert. "], 
["Påloggingen din er ikke validert. Sjekk e-posten din for valideringsmeldingen."],
["En Linksys Smart Wi-Fi-konto for {username} finnes ikke. "], 
["Det oppsto en uventet feil under kommunikasjon med Linksys Smart Wi-Fi-tjenestene. Prøv igjen senere når du har en Internett-tilkobling."],
["En Linksys Smart Wi-Fi-konto for {username} finnes allerede. "], 
["Kontoen din ble låst etter at gjentatte påloggingsforsøk mislyktes. "],
["Kamera kan ikke koble til serveren. Sørg for at nettverket er koblet til Internett og prøv igjen."],
["Kamera kan ikke koble til serveren. Sørg for at nettverket er koblet til Internett og prøv igjen."]
];