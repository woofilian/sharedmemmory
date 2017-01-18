<html><head>
<title>Audio Setup</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<style type="text/css">
#wps_progress_bar {
width:250px;
height:15;
margin: 0 auto;
border: 1px solid gray;
}
</style>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script language="JavaScript" type="text/javascript">


function style_display_on(){
    if(window.ActiveXObject) { // IE
        return "block";
    } else if (window.XMLHttpRequest) { // Mozilla, Safari,...
        return "table-row";
    }
}

var http_request = false;

function makeRequest(url, content, handler) {
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
	http_request.onreadystatechange = handler;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function Handler() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			//document.write(http_request.responseText);
			//alert(http_request.responseText);
			PLAllData(http_request.responseText);
			
		} else {
			alert('There was a problem with the request.');
		}
	}
}

function PLAllData(str)
{
	var all_str = new Array();
	all_str = str.split("\r");

	
	for (var i=0; i<all_str.length; i++) {
		var fields_str = new Array();
		fields_str = all_str[i].split(":");
		if(fields_str.length == 2)
		{
			//alert(fields_str);
			if (fields_str[0] == "audioEnable"){
				var array = document.getElementsByName("audioEnable");
				var j;
				for(j=0; j<array.length; j++){
					if(array[j].value == fields_str[1]){
						array[j].checked = true;
					}
				}
				disable();
			}else if(fields_str[0] == "audioVolume"){
				document.getElementById("audioVolume").value = fields_str[1];
			}else if(fields_str[0] == "audioSpeakerVolume"){
				document.getElementById("audioSpeakerVolume").value = fields_str[1];
			}else if(fields_str[0] == "audioCompression"){
				document.getElementById("audioCompression").value = fields_str[1];
			}
		}

	}
}

function pageInit()
{
	makeRequest("/goform/getAudioSettings?audioEnable=&audioCompression=&audioVolume=&audioSpeakerVolume=", "n/a", Handler);
}

function disable()
{
	if(audio_form.audioEnable[0].checked == true)
	{
		document.getElementById("audioCompression").disabled = false;
		document.getElementById("audioVolume").disabled = false;
		document.getElementById("audioSpeakerVolume").disabled = false;
	}
	else if(audio_form.audioEnable[1].checked == true)
	{
		document.getElementById("audioCompression").disabled = true;
		document.getElementById("audioVolume").disabled = true;
		document.getElementById("audioSpeakerVolume").disabled = true;
	}
}

</script>

</head>
<body onLoad="pageInit()">
<table class="body"><tr><td width="529">
<h1 id="wpsTitle_text">Ap Camera Audio </h1>
<hr />
<form method="get" name ="audio_form" action="/goform/updateAudioSettings">

<!-- ==================  Audio Enable  ================== -->
<table border="1" cellpadding="2" cellspacing="1" width="100%">
<tbody>
<tr>
  <td class="title" colspan="3" id="audioConfig_text">Enable Audio </td>
</tr>

<tr>
	<td width="51%" height="25" align="left" class="head">&nbsp;Enable Audio:&nbsp;</td>
    <td width="23%" align="center" ><input type="radio" name="audioEnable" value="YES" checked onClick = "disable();">YES</td>
    <td width="26%" align="center" ><input type="radio" name="audioEnable" value="NO" onClick = "disable();">NO</td>
</tr>

</tbody>
</table>


<br />
<!-- ==================  Audio Setting  ================== -->
<table  border="1" cellpadding="2" cellspacing="1" width="100%" >
<tbody>

<tr>
  <td class="title" colspan="3" id="audio_setting">Audio Setting </td>
</tr>

<tr>
      <td width="51%" height="25"  align="left" class="head">&nbsp;Codec:&nbsp;</td>
      <td width="49%" colspan="2" align="left" >&nbsp;
		<select name="audioCompression" id = "audioCompression">
		<option value="0" >PCM</option>
		<option value="1" >G.711</option>
		<option value="2" >G.726</option>
		<option value="3" >OPUS</option>
		</select></td>
</tr>

<tr>
<td  align="left" height="25" class="head">&nbsp;Speaker Volume:&nbsp;</td>
<td align="left" colspan="3">&nbsp;
         <select name="audioVolume" id = "audioVolume">
         <option value="0" >0</option>
         <option value="20" >20</option>
         <option value="40" >40</option>
         <option value="60" >60</option>
         <option value="80" >80</option>
         <option value="100">100</option>
         </select>
</td>	
</tr>

<tr>
<td  align="left" height="25" class="head">&nbsp;MicroPhone Volume:&nbsp;</td>
<td align="left" colspan="3">&nbsp;
         <select name="audioSpeakerVolume" id = "audioSpeakerVolume">
         <option value="0" >0</option>
         <option value="20" >20</option>
         <option value="40" >40</option>
         <option value="60" >60</option>
         <option value="80" >80</option>
         <option value="100">100</option>
         </select>
</td>	
</tr>
</tbody>
</table>



<table border="0" cellpadding="2" cellspacing="1" width="540">
  <tbody><tr align="center">

    <td>
      <input style="width: 120px;" value="Apply" id="audioApply" type="submit"> &nbsp; &nbsp;
      <input style="width: 120px;" value="Cancel" id="audioCancel" type="reset" onClick="window.location.reload()" >
    </td>
  </tr>
</tbody></table>

<br />
</form>

</td></tr></table>
</body></html>
