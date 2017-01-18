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
["Fel :( Kan inte skapa någon XMLHTTP-instans!"],
["Kan inte ansluta till kameran!"],
["Information krävs."],
["Avbryt"],
["Fortsätt"],
["Loggar in ... vänta"],
["Skapar konto ... vänta"],
["Registrerar kameran ... vänta"],
["Ska slutföra sidan ... vänta"],
["Ansluter till det trådlösa nätverket. <br>Det kan ta några minuter."],
["Går till sidan Inställningar för internet ... vänta"],
["Se till att du anger rätt e-postadress. Den används till att hämta kontoinformation."]
];

var language1=[
//CamPreview.asp 
["Inställningar för förgranskning av kamera"],
["Förgranskning av kamera"],
["Grattis, du har anslutit "],
["till kameran"],
["Anslut kameran till internet"]
];

var language2=[
// aplist.asp
["Nätverksinställningar"],
["Anslut till internet"],
["Välj WiFi-nätverk ..."],
["Om du vill ansluta via Ethernet-kabel"],
["klickar du på knappen 'Använd Ethernet' nedan."],
["Ansluter till nätverk ... vänta"],
["Använd Ethernet"],
["Ange nätverkslösenord<br> &nbsp;&nbsp;&nbsp;för "],
//["Kan inte ansluta till wifi-routern! Kontrollera lösenordet och försök igen! Felkod: "],
["Du har angett fel Wi-Fi-lösenord. Försök igen. "],
//["Kan inte hämta ip-adress från wifi-routern! Kontrollera lösenordet och försök igen! Felkod: "],
["Kameran kan inte upprätta en anslutning. Kontrollera routern och försök igen. "],
["Nätverkslösenordet ska innehålla minst 8 tecken."],
["Lösenordet innehåller ogiltiga tecken."],
["Ange 5 eller 13 tecken för WEP ASCII-nyckel"],
["WEP-nyckeln innehåller ogiltiga tecken."],
[" eller ange 10 eller 26 tecken för WEP hex-nyckel."],
["Ogiltigt WEP-nyckelformat."],
["Kända nätverk ansluts "],
["automatiskt. Om det inte finns några kända nätverk "],
["får du välja vilket du vill ansluta till."],
[""],
["Kameran får en svag WiFi-signal när den ansluter till det trådlösa nätverket. En svag WiFi-signal ger dålig videokvalitet som förlorade bildrutor och försämrade bilder. Placera kameran närmare den trådlösa routern eller accesspunkten för att få en starkare signal och högre videokvalitet."],
["Lösenord"],
["Skannar det trådlösa nätverket ... vänta"],
["Ange nätverksinformation"],
["Annat nätverk"],
["Namn"],
["Säkerhet"],
["Säkerhet/Kryptera"],
["Ingen"],
["Ange nätverksnamn"],
["Kontrollera att Ethernet-kabeln är korrekt ansluten och försök igen!"],
["Uppdatera"],
["Kontrollera om du är ansluten till kameran."],
["Öppna"],
["Dela"],
//["Kan inte ansluta till internet! Kontrollera anslutningen till internet och försök igen! Felkod:"],
["Kameran kan inte ansluta till internet. Fungerar din internetanslutning? "],
["Koppla in Ethernet-kabeln i kameran."],
["Visa lösenord"],
["Kan inte ansluta till wifi-routern! Kontrollera nätverket och försök igen!"],
["Kameraservern är inte tillgänglig just nu. "],
[""]
];

var language3=[
//CreateAccount.asp
["Inställningar för att skapa konto"],
["Anslut till server"],
["Skapa ett konto"],
["Användarnamn"],
["Lösenord"],
["Bekräfta"],
["Mejl"],
["Om du redan har ett konto klickar du på knappen 'Använd befintligt' nedan."],
["Använd befintligt"],
["Du kan använda endast bokstäver (a-z A-Z), siffror (0-9), understreck (_), punkt (.), bindestreck (-) och snabel-a (@) i användarnamnet."],
["Det första tecknet i användarnamnet ska inte vara understreck (_), punkt (.), bindestreck (-) eller snabel-a (@) ."],
["Lösenordet måste vara minst 6 tecken."],
["Lösenorden matchar inte. Försök igen."],
["Ogiltig mejladress."],
["Förnamn"],
["Efternamn"],
["Här är de kontoinställningar du angett. Kontrollera att de stämmer innan du fortsätter."],
// ["Du måste bekräfta denna e-postadress senare via ett mejl vi skickar till dig."],
["Se till att hålla koll på e-posten under det kommande dygnet. Efter detta kommer du inte åt ditt konto förrän du har klickat på bekräftelselänken vi skickat till dig."],
["Skapa ett Linksys Smart Wi-Fi-konto."],
["Lösenord: Svagt"],
["Lösenord: Starkt"],
["Försök att använda en blandning mellan 7 eller fler små och stora bokstäver, siffror och symboler för att skapa ett riktigt säkert lösenord."],
["Skapa konto"],
["E-postadresserna stämmer inte överens."],
["Lösenord måste vara 6–128 tecken långa och innehålla minst en siffra och minst en bokstav."]
];

var language4=[
///login.asp 
["Inställningar för inloggning"],
["Anslut till server"],
["Ange ditt konto"],
["Användarnamn"],
["Lösenord"],
["Logga in"]
];

var language5=[
//CamRegister.asp
["Inställningar för registrering av kamera"],
["Anslut till server"],
["Kamerainformation"],
["Namn på kameran"],
["Beskrivning"],
["Slutför"]
];

var language6=[
["Stäng av WiFi-inställningarna"],
["Sätt omkopplaren i läge NED för att slutföra installationsprocessen."],
["Varning: kameran initieras inte förrän detta gjorts!"],
["Sätt omkopplaren \"WiFi-inställningar\" på OFF för att fortsätta."],
[""],
[""],
[""],
[""],
["Sätt omkopplaren i läge NED för att slutföra installationsprocessen."]
];

var language7=[
["annat ..."],
["du väljer ..."],
["något fel inträffar."],
["Vill du ändå ansluta till ett osäkert nätverk?"],
["OK"]
];

var language8=[
["Ogiltig e-postadress eller lösenord. Försök igen."],
["Linksys Smart Wi-Fi-kontot har avaktiverats för {username}. "], 
["Din inloggning har inte bekräftats. Kontrollera om du har ett bekräftelsemejl i e-posten."],
["Det finns inget Linksys Smart Wi-Fi-konto för {username}. "], 
["Ett oväntat fel uppstod vid kommunikation med Linksys Smart Wi-Fi-tjänsterna. Försök igen senare när du har en internetanslutning."],
["Det finns redan ett Linksys Smart Wi-Fi-konto för {username}. "], 
["Ditt konto har låsts efter att upprepade inloggningsförsök misslyckats."],
["Kameran kan inte ansluta till servern. Kontrollera att du är ansluten till internet och försök igen."],
["Kameran kan inte ansluta till servern. Kontrollera att du är ansluten till internet och försök igen."]
];