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
["Failure :( Cannot create an XMLHTTP instance!"],
["Unable to connect to camera!"],
["Information is required."],
["Cancel"],
["Continue"],
//number 5
["Logging in ... please wait"],
["Creating account ... please wait"],
["Registering camera ... please wait"],
["Going to finish page ... please wait"],
// padding
["Connecting to your wireless network. <br>This may take a few minutes."],
["Going to Setup Internet page ... please wait"],
["Please make sure that you enter your email correctly. It will be used for retrieving account information."]
];

var language1=[
//CamPreview.asp 
["Camera Preview Setup"],
["Camera Preview"],
["Congratulations, you have successfully "],
["connected to your camera"],
["Connect Camera to Internet"]
];

var language2=[
// aplist.asp
["Network Setup"],
["Connect to Your Internet"],
["Choose a Wi-Fi Network..."],
["If you would like to connect via Ethernet cable"],
["please click on the 'Use Ethernet' button below."],
["Connecting to network ... please wait"],
["Use Ethernet"],

// no.7  line up
["Please enter the network password<br> &nbsp;&nbsp;&nbsp;for "],
//["Unable to connect to your wifi router! Please check your password and try again! Error Code: "],
["The Wi-Fi password you entered is incorrect. Please try again."],
//["Unable to get ip address from your wifi router! Please check your password and try again! Error Code: "],
["Camera cannot establish connection. Please check your router and try again."],
["Your network password should contain at least 8 characters."],
["Invalid characters in password."],
["Please input 5 or 13 characters for WEP ASCII key"],
["Invalid characters in WEP key."],
[" or input 10 or 26 characters of WEP hex key."],
["Invalid WEP key format."],
// the comment.
// no.16
["Known networks will be joined "],
["automatically. If no known networks are "],
["available, you will need to select one to join."],
[""],
//for weak wireless 
// no.20
["Your camera is experiencing a weak Wi-Fi signal when connecting to your wireless network. Weak WiFi signal will cause poor video performance such as dropped frames and Image deterioration. Please place the camera closer to your wireless router or access point to get a stronger signal and better camera performance."],
["Password"],
["Scanning wireless network ... please wait"],
//for network information
// no.23
["Enter Network Information"],
["Other Network"],
["Name"],
["Security"],
["Security/Encrypt"],
["None"],
["Please input your network name"],
//above 29
// no.30
//["Please verify your Ethernet cable is connected properly and try again! Error Code: "],
["Please verify your Ethernet cable is connected properly and try again!"],
["Refresh"],
["Please check whether you have connected to your camera."],
["Open"],
["Share"],
//["Can not connect to internet! Please check your internet connection and try again! Error Code: "],
["Camera cannot get on the Internet. Is your Internet connection working?"],
["Please plug in Ethernet cable for your camera."],
["Show Password"],
["Unable to connect to your wifi router! Please check your network and try again!"],
["Camera's server is not available at this time."],
// no.40
["Please enter your wireless network password. This is normally located on your router"]
];


var language3=[
//CreateAccount.asp
["Create Account Setup"],
["Connect to Our Server"],
["Create Your New Account"],
["Username"],
["Password"],
["Confirm"],
["Email"],
["If you already have an existing account, please click on the 'Use Existing' button below."],
["Use Existing"],
["Only letters (a-z A-Z), numbers (0-9), underscores (_), periods (.), dash (-) and @ are allowed for Username."],
// 10
["Underscores (_), periods (.), dash (-) and @ should not be the first character of Username."],
["Password must be at least 6 characters in length."], 
["The passwords entered do not match. Please try again."],
["Invalid email address."],
["First Name"],
["Last Name"],
["Here are the account settings you have entered. Please verify they are correct before proceeding."],
// ["You will need to validate this email address later through an email we sent you."],  // SEEDONK-1235, change to follows
// ["If you do not access the camera in the next 24 hours you will need to validate your account by clicking on the link in the email we sent you."],
["Be sure to check your email in the next 24 hours. You will not be able to access your account after that until you click the validation link we sent you."],
["Create your Linksys Smart Wi-Fi Account"],
["Password:Weak"],
// 20
["Password:Strong"],
["Try using a mixture of 7 or more upper or lower case characters, numbers and symbols to maximise your password strength."],
["Create Account"],
//above 23
["The emails do not match."],
["Password must be 6-128 characters long, and contain at least one number and one letter."]// for linksys
];

var language4=[
///login.asp 
["Login Setup"],
["Connect to Our Server"],
["Sign In to Your Account"],
["Username"],
["Password"],
["Sign in"]
];

var language5=[
//CamRegister.asp
["Camera Register Setup"],
["Connect to Server"],
["Name Your Camera"],
["Camera Name"],
["Description"],
["Finish"]
];

var language6=[
["One Last Step"],
// double quote
["Please set the switch to DOWN position to complete the setup process."],
// pop up Please set the 'WiFi Setup' switch to OFF to continue.
["Warning: Setup will not succeed until this step is done!"],
// not use 
["Please set the \"WiFi Setup\" switch to OFF to continue."],
//add for BTHome, only english
["Almost there..."],
["Please move the switch to CAMERA position to complete the setup process"],
["Please note: your set-up will not be complete until you do this"],
["Unable to Complete Setup"],
//ptz
["Please press the Wifi button on the back of the camera once, to complete the setup process."]
];

var language7=[
["other.."],
["you select .."],
["some error happen."],
["Do you still want to connect to an unsecured network?"],
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
["Connection times out and can not be established. Network connection is not normal. Server settings are not accurate. E.g. invalid IP or the server is down."]
];
