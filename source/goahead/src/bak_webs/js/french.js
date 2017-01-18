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
["Erreur :( Impossible de créer une session XMLHTTP !"],
["Impossible de se connecter à la webcam !"],
["Des informations sont requises."],
["Annuler"],
["Continuer"],
["Ouverture de session ... Veuillez patienter."],
["Création de compte... Veuillez patienter."],
["Enregistrement de la webcam... Veuillez patienter."],
["Finalisation ... Veuillez patienter."],
["Connexion à votre réseau sans fil en cours. <br>Ceci peut prendre quelques minutes."],
["Ouverture de la page de configuration internet ... Veuillez patienter."],
["Assurez-vous d'avoir correctement renseigné votre email. Il sera utilisé pour récupérer les informations de votre compte."]
];

var language1=[
//CamPreview.asp 
["Configuration de la vidéo de la webcam"],
["Vidéo de la webcam"],
["Félicitations, vous êtes maintenant"],
["connecté à votre webcam."],
["Connecter la webcam à Internet"]
];

var language2=[
// aplist.asp
["Configuration réseau"],
["Connecter à Internet"],
["Choisir un réseau Wi-Fi..."],
["Pour vous connecter par câble Ethernet,"],
["cliquez sur le bouton « Utiliser Ethernet » ci-dessous."],
["Connexion au réseau... Veuillez patienter."],
["Utiliser Ethernet"],
["Veuillez saisir le mot de passe du réseau<br> &nbsp;&nbsp;&nbsp;for "],
//["Impossible de se connecter à votre routeur wifi ! Vérifiez votre mot de passe et réessayez ! Code d'erreur :"],
["Le mot de passe Wi-Fi saisi est incorrect. Veuillez réessayer. "],
//["Impossible d'obtenir l'adresse IP de votre routeur wifi ! Vérifiez votre mot de passe et réessayez ! Code d'erreur :"],
["La caméra ne peut pas établir la connexion. Veuillez vérifier votre routeur et réessayer. "],
["Le mot de passe de votre réseau doit contenir au moins 8 caractères."],
["Caractères du mot de passe interdits."],
["Veuillez saisir 5 ou 13 caractères ASCII pour la clé WEP."],
["Caractères interdits dans la clé WEP."],
[" ou saisissez 10 ou 26 caractères de la clé hex WEP."],
["Format invalide de clé WEP."],
["La connexion aux réseaux connus sera"],
["automatique. Si aucun réseau connu n'est"],
["disponible,il vous faudra en sélectionner un auquel vous connecter."],
[""],
["Votre webcam capte un faible signal Wi-Fi en se connectant à votre réseau sans fil. Un faible signal WiFi donnera une mauvaise performance vidéo tels des trames saccadées et des images dégradées. Veuillez rapprocher la webcam de votre routeur sans fil ou du point d'accès pour avoir un signal plus puissant et une meilleure performance de webcam."],
["Mot de passe"],
["Recherche de réseau sans fil... Veuillez patienter."],
["Saisissez les données du réseau"],
["Autre réseau"],
["Nom"],
["Sécurité"],
["Sécurité/coder"],
["Aucun"],
["Veuillez saisir le nom de votre réseau"],
["Veuillez vérifier que votre câble Ethernet est bien branché et réessayez !"],
["Rafraîchir"],
["Veuillez vérifier que vous êtes connecté à votre webcam."],
["Ouvrir"],
["Partager"],
//["Impossible de se connecter à internet ! Veuillez vérifier votre connexion à internet et réessayez ! Code d'erreur :"],
["La caméra ne peut pas accéder à Internet. Votre connexion Internet fonctionne-t-elle ? "],
["Veuillez brancher le câble Ethernet de votre webcam."],
["Afficher le mot de passe"],
["Impossible de se connecter à votre routeur wifi ! Veuillez vérifier votre réseau et réessayez !"],
["Le serveur de la caméra n'est pas disponible pour l'instant. "],
[""]
];

var language3=[
//CreateAccount.asp
["Créer configuration de compte"],
["Se connecter au serveur"],
["Créer votre compte"],
["Nom d’utilisateur"],
["Mot de passe"],
["Confirmer"],
["Courriel"],
["Si vous possédez déjà un compte actif, veuillez cliquer sur le bouton « Déjà inscrit » ci-dessous."],
["Déjà inscrit"],
["Seuls les lettres (a-z A-Z), les chiffres (0 à 9), le trait de soulignement (_), le point (.), le tiret (-) et l'arobase @ sont autorisés pour le nom d'utilisateur."],
["Le trait de soulignement (_), le point (.), le tiret (-) et l'arobase @ ne doivent pas être le premier caractère du nom d'utilisateur."],
["Le mot de passe doit comporter au moins 6 caractères."],
["Les mots de passe saisis ne correspondent pas. Veuillez réessayer."],
["Adresse email invalide."],
["Prénom"],
["Nom"],
["Voici les paramètres de compte qui ont été saisis. Vérifiez leur exactitude avant de continuer."],
// ["Vous devrez valider cette adresse e-mail plus tard au moyen de l'e-mail que nous vous avons envoyé."],
["Assurez-vous de consultez vos e-mails dans les prochaines 24 heures. Vous ne pourrez pas accéder à votre compte avant d'avoir cliqué sur le lien de validation que nous vous avons envoyé."],
["Créer votre compte Smart Wi-Fi Linksys"],
["Mot de passe : Faible"],
["Mot de passe : Puissant"],
["Essayez d'utiliser une combinaison d'au moins 7 caractères majuscules ou minuscules, chiffres et symboles afin de maximiser la fiabilité de votre mot de passe."],
["Créez un compte"],
["Les e-mails sont différents."],
["Le mot de passe doit être composé de 6 à 128 caractères, dont au moins un chiffre et une lettre."]
];

var language4=[
///login.asp 
["Configuration de connexion"],
["Se connecter au serveur"],
["Saisissez vos identifiants"],
["Nom d’utilisateur"],
["Mot de passe"],
["Connexion"]
];

var language5=[
//CamRegister.asp
["Configuration d'enregistrement de webcam"],
["Se connecter au serveur"],
["Informations de la webcam"],
["Nom de la webcam"],
["Description"],
["Terminer"]
];

var language6=[
["Désactiver la configuration WiFi"],
["Veuillez mettre l'interrupteur en position BASSE pour terminer le processus de configuration."],
["Attention : votre webcam ne s'initialisera pas tant que cela ne sera pas fait !"],
["Veuillez mettre l'interrupteur de \'configuration WiFi\' sur OFF pour continuer."],
[""],
[""],
[""],
[""],
["Veuillez mettre l'interrupteur en position BASSE pour terminer le processus de configuration."]
];

var language7=[
["autres..."],
["vous sélectionnez .."],
["une erreur se produit."],
["Voulez-vous vraiment vous connecter à un réseau non sécurisé ?"],
["OK"]
];

var language8=[
["Adresse e-mail ou mot de passe incorrect(e). Veuillez réessayer."],
["Le compte Linksys Smart Wi-Fi a été désactivé pour {username}. "],
["Votre compte n'a pas été validé. Consultez votre boîte de réception pour le message de validation."],
["Aucun compte Linksys Smart Wi-Fi n'existe pour {username}. "], 
["Une erreur inattendue est survenue lors de la communication avec les services Linksys Smart Wi-Fi. Réessayez plus tard lorsque vous disposez d'une connexion Internet."],
["Un compte Linksys Smart Wi-Fi existe déjà pour {username}. "], 
["Votre compte a été verrouillé après l'échec de tentatives de connexion répétées. "],
["Impossible de connecter la caméra au serveur. Assurez-vous que votre réseau est connecté à Internet, et réessayez."],
["Impossible de connecter la caméra au serveur. Assurez-vous que votre réseau est connecté à Internet, et réessayez."]
];