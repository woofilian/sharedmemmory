<html><head><title>burnin ALL</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<style type="text/css">
<!--
#loading {
       width: 250px;
       height: 200px;
       background-color: #3399ff;
       position: absolute;
       left: 50%;
       top: 50%;
       margin-top: -150px;
       margin-left: -250px;
       text-align: center;
}
-->
</style>

<script language="JavaScript" type="text/javascript">
document.write('<div id="loading" style="display: none;"><br><br><br>Burnin ALL <br><br> Please be patient and don\'t remove usb device if it presented...</div>');
Butterlate.setTextDomain("admin");

var firmware_path = "";

var secs
var timerID = null
var timerRunning = false
var timeout = 3
var delay = 1000

function style_display_on()
{
	if (window.ActiveXObject)
	{ // IE
		return "block";
	}
	else if (window.XMLHttpRequest)
	{ // Mozilla, Safari,...
		return "table-row";
	}
}

function InitializeTimer(){
    // Set the length of the timer, in seconds
    secs = timeout
    StopTheClock()
    StartTheTimer()
}

function StopTheClock(){
    if(timerRunning)
        clearTimeout(timerID)
    timerRunning = false
}

function StartTheTimer(){

    if (secs==0){
        StopTheClock()

        secs = timeout
        StartTheTimer()
    }else{
        self.status = secs
        secs = secs - 1
        timerRunning = true
        timerID = self.setTimeout("StartTheTimer()", delay)
    }
}

var http_request = false;
function makeRequest(url, content) {
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert('Giving up :( Cannot create an XMLHTTP instance');
        return false;
    }
    http_request.open('GET', url, true);
    http_request.send(content);
}

var _singleton = 0;
function uploadFirmwareCheck()
{
	if(_singleton)
		return false;
	
	if(document.UploadFirmware.filename.value == ""){
		alert("Firmware Upgrade: Please specify a file.");
		return false;
	}

	var sel_obj = document.getElementByIdx("fwType").value;
	var index = sel_obj.selectedIndex;        
	var sel_val = sel_obj.options[index].value; 
	var filename = document.UploadFirmware.filename.value;

	alert(sel_val+filename);

	StopTheClock();
    document.getElementById("loading").style.display="block";
	parent.menu.setLockMenu(1);
	_singleton = 1;
	return true;
}


function initTranslation()
{
	//var e = document.getElementById("uploadTitle");
	ede Firmware.innerHTML = _("upload title");

	//e = document.getElementById("uploadFW");
	//e.innerHTML = _("upload firmware");
	//e = document.getElementById("uploadFWLocation");
	//e.innerHTML = _("upload firmware location");
	//e = document.getElementById("uploadFWApply");
	//e.value = _("admin apply");
}

function pageInit(){
	initTranslation();

    document.UploadFirmware.UploadFirmwareSubmit.disabled = false;
//	document.UploadFirmware.filename.disabled = false;
	document.getElementById("loading").style.display="none";


}
</script></head><body onLoad="pageInit()">
<table class="body"><tbody><tr><td>
<h1 id="uploadTitle">Burnin ALL</h1>
<p><font id="uploadIntroduction1">Upgrade the SoC firmware to obtain new functionality. </font><font id="uploadIntroduction2" color="#ff0000">It takes about 1 minute to upload &amp; upgrade flash and be patient please. Caution! A corrupted image will hang up the system.</font> </p>

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
<tr>
    <td class="title" colspan="2" id="Release">Release Information</td>
</tr>
<tr>
  <td class="head" id="CurrentFW">Current Firmware Infomation</td>
  <td>
	<script type="text/javascript">
	var string='<% getASP(); %>';
	document.write(string);
  	</script>
  </td>
</tr>
</table>
<br>
<br>

<!-- ----------------- Upload firmware Settings ----------------- -->
<form method="post" name="UploadFirmware" action="/cgi-bin/upload_fs.cgi" enctype="multipart/form-data">
<table border="1" cellpadding="2" cellspacing="1" width="95%">
<tbody>
<tr>
  <td class="title" colspan="2" id="uploadFW">Update Image</td>
</tr>
<tr>
	<td class="head" id="uploadFWLocation">Location:</td>
	<td> <input name="filename" size="20" maxlength="256" type="file"> </td>
</tr>
</tbody>
</table>
<input value="Apply" id="uploadFWApply" name="UploadFirmwareSubmit" type="submit" onClick="return uploadFirmwareCheck();"> &nbsp;&nbsp;
</form>

  </tr>
</table>


<br>
</td></tr></tbody></table>
</body></html>
