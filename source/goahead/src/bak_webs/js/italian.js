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
["Fallito :( Impossibile creare un'istanza XMLHTTP!"],
["Impossibile connettersi alla camera!"],
["Informazione necessaria."],
["Cancella"],
["Continua"],
["Connessione in corso ... Attendere prego"],
["Creazione account in corso ... Attendere prego"],
["Registrazione camera in corso ... attendere prego"],
["Indirizzamento alla pagina finale in corso ... Attendere prego"],
["Connessione alla rete wireless, <br> questa operazione potrebbe impiegare qualche minuto. <br>Connessione alla rete……Attendere Prego."],
["Indirizzamento alla pagina delle impostazioni in corso... Attendere prego"],
["Controllare che l'indirizzo mail inserito sia corretto. Questo potrò essere utilizzato per il recupero delle informazioni dell'account."]
];

var language1=[
//CamPreview.asp 
["Impostazioni Anteprima Camera"],
["Anteprima Camera"],
["Congratulazioni, operazione conclusa con successo"],
["Collegato alla camera"],
["Connettere la camera a internet"]
];

var language2=[
// aplist.asp
["Impostazioni di rete"],
["Connettere a internet"],
["Scegliere una rete Wi-Fi ..."],
["Se desideri collegare +Cam tramite il cavo ethernet,"],
["clicca sul pulsante 'Usare Ethernet' sottostante."],
["Connessione alla rete in corso ... Attendere prego"],
["Usare Ethernet"],
["Inserire la password di rete<br> &nbsp;&nbsp;&nbsp; "],
["Impossibile connettersi al router wifi! Si prega di controllare la password e di riprovare!"],
["Impossibile acquisire indirizzo ip dal router wifi! Si prega di controllare la password e riprovare!"],
["La password di rete deve contenere almeno 8 caratteri"],
["La password contiene caratteri non validi"],
["Si prega di inserire 5 o 13 caratteri per la chiave WEP ASCII "],
["La chiave WEP contiene caratteri non validi."],
["Inserisci 10 o 26 caratteri della chiave esadecimale WEP "],
["Formato chiave WEP non valido"],
["Il dispositivo si collegherà automaticamente alla reti conosciute."],
["Se non ci sono reti conosciute,"],
["selezionarne una per continuare."],
[""],
["Il segnale Wi-Fi della telecamera è debole. Un segnale Wi-Fi scarso può pregiudicare le prestazioni video della telecamera, ad esempio perdita di frame rate e deterioramento delle immagini. Si prega di posizionare la telecamera più vicina al vostro router wireless o access point per ottenere segnale e prestazioni migliori."],
["Password"],
["Scansione reti wireless in corso ... Si prega di attendere"],
["Inserire le informazioni della rete"],
["Altre reti"],
["Nome"],
["Sicurezza"],
["Sicurezza/Cifratura"],
["Nessuno"],
["Si prega di inserire il nome della rete"],
["Si prega di controllare che il cavo Ethernet sia inserito correttamente e riprovare!"],
["Aggiorna"],
["Si prega di verificare che la telecamera sia connessa."],
["Apri"],
["Condividi"],
["Impossibile connettersi a internet! Si prega di controllare la connessione a internet e riprovare!"],
["Si prega di inserire il cavo di rete nella telecamera"],
["Mostra password"],
["Impossibile connettersi al router wifi! Si prega di controllare le impostazioni della rete e riprovare!"],
["Si prega di controllare che il cavo Ethernet sia inserito correttamente e riprovare!"],
[""]
];


var language3=[
//CreateAccount.asp
["Impostazioni Creazione Account"],
["Connessione al server"],
["Crea un account"],
["Nome utente"],
["Password"],
["Conferma"],
["Email"],
["Se si dispone già di un account si prega di cliccare il pulsante 'Usa Esistente'."],
["Usa Esistente"],
["Solo lettere (a-z A-Z), numeri (0-9), underscore (_), punti (.), trattini (-) e @ sono consentiti nel Nome Utente"],
["Underscore (_), punti (.), trattini (-) e @ non sono permessi come primo carattere del Nome Utente"],
["La password deve essere lunga almeno 6 caratteri"],
["Le password inserite non corrispondono. Si prega di riprovare."],
["Indirizzo Email non valido."],
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
["Impostazioni Login"],
["Connettersi al server"],
["Inserire il proprio Account"],
["Nome utente"],
["Password"],
["Accedi"]
];

var language5=[
//CamRegister.asp
["Impostazioni Registrazione Telecamera"],
["Connettersi al server"],
["Informazioni telecamera"],
["Nome Telecamera"],
["Descrizione"],
["Fine"]
];

var language6=[
["Impostazioni spegnimento WiFi"],
["Si prega di impostare su OFF, il bottone sul retro della telecamera, per completare la processo di installazione"],
["Attenzione: La camera non sarà operativa fino a quando questo procedimento non sarà completato!"],
["Si prega di impostare su OFF, il bottone sul retro della telecamera, per continuare."],
[""],
[""],
[""],
[""],
["Si prega di impostare su OFF, il bottone sul retro della telecamera, per completare la processo di installazione"]
];

var language7=[
["Altri.."],
["hai selezionato .."],
["Si sono verificati alcuni errori."],
["Volete davvero connettervi a una rete non protetta?"],
["OK"]
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
