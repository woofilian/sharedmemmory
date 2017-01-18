<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>

<!-- !!! Dont forget to modify   INIC mode treeapp.asp !!! -->

<head>
<title>DTree</title>
<meta http-equiv="content-type" content="text/html;charset=utf-8" />
<link rel="stylesheet" href="/dtree/dtree.css" type="text/css" />
<script type="text/javascript" src="/dtree/dtree.js"></script>
<script type="text/javascript" src="/lang/b28n.js"></script>
</head>

<body bgcolor=#FFFFFF onLoad="initValue()">
<script language="JavaScript">    
var isMenuLock = 0;
Butterlate.setTextDomain("main");

function initValue()
{
	var e = document.getElementById("openall");
	e.innerHTML = _("treeapp openall");
	e = document.getElementById("closeall");
	e.innerHTML = _("treeapp closeall");
}

function setLockMenu(flag){
	isMenuLock = flag;
}
function go(zz) {
	if(!isMenuLock)
		top.view.location=zz;
}
function refresh(){
	window.location.reload(false);
}
</script>

<p><a href="javascript: a.openAll();" id="openall">open</a> | <a href="javascript: a.closeAll();" id="closeall">close</a></p>

<script type="text/javascript">
var dpbsta = '<% getDpbSta(); %>';
var vpnen = '<% getVPNBuilt(); %>';
var meshb = '<% getMeshBuilt(); %>';
var wdsb = '<% getWDSBuilt(); %>';
var wscb = '<% getWSCBuilt(); %>';
var usbb = '<% getUSBBuilt(); %>';
var storageb = '<% getStorageBuilt(); %>';
var ftpb = '<% getFtpBuilt(); %>';
var smbb = '<% getSmbBuilt(); %>';
var mediab = '<% getMediaBuilt(); %>';
var webcamb = '<% getWebCamBuilt(); %>';
var printersrvb = '<% getPrinterSrvBuilt(); %>';
var itunesb = '<% getiTunesBuilt(); %>';
var syslogb = '<% getSysLogBuilt(); %>';
var swqos = '<% getSWQoSBuilt(); %>';
var ad = '<% isAntennaDiversityBuilt(); %>';
var wizardb = '<% getWizardBuilt(); %>';
var rvtb = '<% getRVTBuilt(); %>';

a = new dTree('a');
a.config.useStatusText=true;
a.config.useCookies=false;

//  nodeID, parent nodeID,  Name,  URL

if (navigator.appName.toUpperCase() == "NETSCAPE") 
{
	a.add(000,  -1, _("Camera Web Server"),                "javascript:go('/apcam/apcam/stream.asp');");
}
else
{
	a.add(000,  -1, _("Camera Web Server"),                "javascript:go('/apcam/apcam/activexPlay.asp');");
}	
	
if (wizardb == "1")
	a.add(100,   0, 'Wizard',               "javascript:go('/apcam/adm/wizard.asp');");

a.add(200, 0, _("APcam"),    				"javascript:a.oo(200);");
a.add(201, 200, _("Camera"),        		"javascript:go('/apcam/apcam/camera.asp');");
a.add(202, 200, _("video"),        			"javascript:go('/apcam/apcam/video.asp');");
a.add(203, 200, _("Audio"),        			"javascript:go('/apcam/apcam/audio.asp');")
a.add(206, 200, _("Motion Detect"), 		"javascript:go('/apcam/apcam/Motion_detect.asp');");	
a.add(207, 200, _("FTP Server"), 		 	"javascript:go('/apcam/apcam/ftp_setup.asp');")

a.add(900,   0, _("Administration"),        "javascript:a.oo(900);")
a.add(901, 900, _("Management"),            "javascript:go('/apcam/adm/management.asp');");
a.add(902, 900, _("Upload Firmware"),       "javascript:go('/apcam/adm/upload_firmware.asp');");
a.add(903, 900, _("Settings Management"),   "javascript:go('/apcam/adm/settings.asp');");
a.add(904, 900, _("Status"),                "javascript:go('/apcam/adm/status.asp');");

if(syslogb == '1')
{
	a.add(908, 900, _('System Log'),            'javascript:go(\'/apcam/adm/syslog.asp\');');
}


document.write(a);

</script>

<script  language="JavaScript"  type="text/javascript">
	//alert(navigator.appName);
	if (navigator.appName.toUpperCase() == "NETSCAPE") {
		//top.view.location = "/apcam/apcam/stream.asp";
		//window.document.getElementById("view").src = "/apcam/apcam/stream.asp";
		top.view.location="/apcam/apcam/stream.asp";
		/*
		document.write("<FRAME SRC='/apcam/apcam/stream.asp' NAME='view' SCROLLING='AUTO' MARGINWIDTH='0' TOPMARGIN='0' MARGINHEIGHT='0' FRAMEBORDER='NO'>");
		*/
	}
	else{
		//top.view.location = "/apcam/apcam/activexPlay.asp";
		//alert("ie");
		//window.document.getElementById("view").src = "/apcam/apcam/activexPlay.asp";
		top.view.location="/apcam/apcam/activexPlay.asp";
		/*
		document.write("<FRAME SRC='/apcam/apcam/activexPlay.asp' NAME='view' SCROLLING='AUTO' MARGINWIDTH='0' TOPMARGIN='0' MARGINHEIGHT='0' FRAMEBORDER='NO'>");
		*/
	}	
</script>

</body>

</html>
