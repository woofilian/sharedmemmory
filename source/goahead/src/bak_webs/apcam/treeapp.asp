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

a.add(900,   0, _("Administration"),        "javascript:a.oo(900);");
a.add(901, 900, _("Management"),            "javascript:go('/apcam/adm/management.asp');");
a.add(902, 900, _("Upload Firmware"),       "javascript:go('/apcam/adm/upload_firmware.asp');");

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
