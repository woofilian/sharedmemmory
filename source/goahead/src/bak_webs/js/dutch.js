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
["Fout : ( Niet in staat om een XMLHTTP extensie te creëren! "],
["Niet in staat om verbinding te maken met de camera!"],
["Informatie vereist."],
["Annuleren"],
["Doorgaan"],
["Inloggen ... wachten a.u.b."],
["Account aanmaken ... wachten a.u.b."],
["Registreren camera ... wachten a.u.b."],
["Naar eindpagina gaan ... wachten a.u.b."],
["Verbinding maken met uw draadloze netwerk. <br>Dit kan een aantal minuten duren."],
["Ga naar Internet Instellingen pagina ... wachten a.u.b."],
["Zorg ervoor dat uw e-mail adres correct is ingevuld. Het zal gebruikt worden om uw account informatie door te sturen."]
];

var language1=[
//CamPreview.asp 
["Cameraweergave Instellingen "],
["Cameraweergave  "],
["Gefeliciteerd, u bent succesvol"],
["verbonden met uw camera"],
["Verbind camera met het internet"]
];

var language2=[
// aplist.asp
["Netwerkinstellingen"],
["Verbinding maken met het internet"],
["Kies een wifi-netwerk…"],
["Als u verbinding wilt maken via de Ethernet-kabel"],
["dient u op de onderstaande 'Gebruik Ethernet' knop te klikken."],
["Verbinding maken met het netwerk ... wachten a.u.b."],
["Gebruik Ethernet"],
["Voer a.u.b. het netwerkwachtwoord in <br> &nbsp;&nbsp;&nbsp;voor "],
["Niet in staat om verbinding te maken met uw wifi-router! Controleer uw wachtwoord en probeer opnieuw!"],
["Niet in staat om IP-adres te verkrijgen vanaf uw wifi-router! Controleer uw wachtwoord en probeer opnieuw!"],
["Uw netwerkwachtwoord dient uit minimaal 8 tekens te bestaan."],
["Ongeldige tekens in het wachtwoord."],
["Voer a.u.b. 5 of 13 tekens in voor de WEP ASCII sleutel"],
["Ongeldige tekens in de WEP-sleutel."],
["of voer 10 of 26 tekens in voor de WEP hex sleutel."],
["Ongeldig WEP-sleutel formaat."],
["Bekende netwerken worden automatisch "],
["toegevoegd. Indien er geen bekende netwerken"],
["beschikbaar zijn, dient u er zelf één te selecteren."],
[""],
["Uw camera ontvangt een zwak wifi-signaal tijdens het verbinden met uw draadloze netwerk. Een zwak wifi-signaal veroorzaakt een slechte videoprestatie, zoals een verlaagde framerate of een verslechtering van het beeld. Plaats uw camera dichter in de buurt van uw draadloze router of acces point om een sterker signaal en een betere cameraprestatie te verkrijgen."],
["Wachtwoord"],
["Draadloos netwerk scannen ... wachten a.u.b."],
["Netwerkinformatie invoeren"],
["Ander Netwerk"],
["Naam"],
["Beveiliging"],
["Beveiliging/Coderen"],
["Geen"],
["Voer a.u.b. uw netwerknaam in"],
["Controleer of uw Ethernet-kabel correct is verbonden en probeer opnieuw! Foutcode:"],
["Verversen"],
["Controleer of u verbinding heeft gemaakt met uw camera."],
["Open"],
["Delen"],
["Niet in staat om verbinding te maken met het internet! Controleer uw internetverbinding en probeer opnieuw!"],
["Sluit de Ethernet-kabel voor uw camera aan.  "],
["Toon wachtwoord"],
["Niet in staat om verbinding te maken met uw wifi-router! Controleer uw netwerk en probeer opnieuw!"],
["Niet in staat om verbinding te maken met het internet! Controleer uw internetverbinding en probeer opnieuw!"],
[""]
];

var language3=[
//CreateAccount.asp
["Account aanmaken Instellingen"],
["Verbinding maken met de Server"],
["Maak uw account aan"],
["Gebruikersnaam"],
["Wachtwoord"],
["Bevestigen"],
["E-mail"],
["Als u al een account heeft, dient u op de 'Gebruik Bestaand Account' knop te klikken. "],
["Gebruik Bestaand Account"],
["Uitsluitend letters (a-z A-Z), nummers (0-9), underscores (_), punten (.), streepjes (-) en @ zijn toegestaan voor de Gebruikersnaam."],
["Gebruik geen underscore (_), punt (.), streepje (-) of @ als eerste teken van de Gebruikersnaam."],
["Het wachtwoord dient uit minstens 6 tekens te bestaan."],
["De ingevoerde wachtwoorden komen niet overeen. Probeer opnieuw."],
["Ongeldig e-mailadres."],
["First Name"],
["Last Name"],
[""],
[""],
[""],
[""],
[""],
[""],
[""],
[""],
[""]
];

var language4=[
///login.asp 
["Login Instellingen"],
["Verbinding maken met de Server"],
["Meld u aan met uw Account "],
["Gebruikersnaam"],
["Wachtwoord"],
["Aanmelden"]
];

var language5=[
//CamRegister.asp
["Cameraregistratie Instellingen"],
["Verbinding maken met de Server"],
["Camera Informatie"],
["Camera Naam"],
["Beschrijving"],
["Voltooien"]
];

var language6=[
["Wifi-instellingen uitschakelen"],
["Breng de schakelaar in de BENEDEN positie om het instellingenproces te voltooien."],
["Waarschuwing: uw camera zal niet initialiseren totdat dit is gebeurd! "],
["Breng de \"Wifi-instellingen\" schakelaar in de UIT positie om verder te gaan. "],
[""],
[""],
[""],
[""],
["Breng de schakelaar in de BENEDEN positie om het instellingenproces te voltooien."]
];

var language7=[
["andere.."],
["u selecteert .."],
["Er is een fout opgetreden."],
["Wenst u nog steeds verbinding te maken met een onbeveiligd netwerk?"],
["goed"]
];

var language8=[
["Invalid email address or password. Please try again."],
["The Linksys Smart Wi-Fi Account for {username} has been disabled. "], 
["Your login has not been validated. Please check your email for the validation message."],
["A Linksys Smart Wi-Fi account for {username} does not exist. "], 
["There was an unexpected error communicating with the Linksys Smart Wi-Fi services. Try again later when you have an Internet connection."],
["A Linksys Smart Wi-Fi account for {username} already exists. "], 
["Your account was locked after repeated login attempts failed."],
["Connection can not be established. Network connection is not normal. Server settings are not accurate or the server is down."],
["Connection times out and can not be established.\nNetwork connection is not normal,\nserver settings are not accurate, e.g. invalid IP, \nor the server is down."]
];
