<html><head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Camera Setup</title>

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
	var auto_exposure = false;

	for (var i=0; i<all_str.length; i++) {
		var fields_str = new Array();
		fields_str = all_str[i].split(":");
		//alert(all_str[i]);
		if(fields_str.length == 2)
		{
			//alert(fields_str[0]);
			if (fields_str[0] == "contrast"){
				document.getElementById("contrast").value = fields_str[1];
			}else if(fields_str[0] == "brightness"){
				document.getElementById("brightness").value = fields_str[1];
			}else if(fields_str[0] == "saturation"){
				document.getElementById("saturation").value = fields_str[1];
            }else if (fields_str[0] == "ncontrast"){
				document.getElementById("ncontrast").value = fields_str[1];
			}else if(fields_str[0] == "nbrightness"){
				document.getElementById("nbrightness").value = fields_str[1];
			}else if(fields_str[0] == "nsaturation"){
				document.getElementById("nsaturation").value = fields_str[1];
			}else if(fields_str[0] == "rotation"){
				 document.getElementById("rotation").value = fields_str[1];
			}else if(fields_str[0] == "flicker"){
				document.getElementById("flicker").value = fields_str[1];
			}
			else if(fields_str[0] == "ir"){
				document.getElementById("ir").value = fields_str[1];
			}
			/*
			else if(fields_str[0] == "exposure"){
				var array = getElementsbyName("exposure");
				var i;
				for(i=0; i<array.length; i++){
					if(array[i].value == fields_str[1]){
						array[i].checked = true;
					}
				}
				if(fields_str[1] == "0"){
					auto_exposure = true;
					document.getElementById("shutter_speed").disabled = true;
				}
			}else if(fields_str[0] == "shutter_speed"){
				if(!auto_exposure){
					document.getElementById("shutter_speed").value = fields_str[1];
				}
			}
			*/
		}

	}
}

function pageInit()
{
	makeRequest("/goform/getVideoSettings?contrast=&brightness=&saturation=&ncontrast=&nbrightness=&nsaturation=&rotation=&flicker=&ir=", "n/a", Handler);
}

function disable(){ 
	if(camera_form.exposure[0].checked==true){//Auto
		document.camera_form.shutter_speed.disabled=true;
	}
	else if(camera_form.exposure[1].checked==true){//Manual
		document.camera_form.shutter_speed.disabled=false;
	}
}

function isdigit(s,str,up,down){
				var r,re; 
				re = /\d\d*/i; 
				r = s.match(re); 
				if(r==s){ 
					if(up==down && up=='x'){
						return 1;
					}
					if(eval(s)<=up && eval(s)>=down){
						return 1;
					}
				} 
				alert(str+' must be a integer,['+down+'-'+up+']'); 
				return 0; 
		}

function CheckValue()
{
	if(isdigit(document.camera_form.brightness.value,"Brightness",100,0)==0){
		return false;
	}
	if(isdigit(document.camera_form.contrast.value,"Contrast",100,0)==0){
		return false;
	}
	if(isdigit(document.camera_form.saturation.value,"Saturation",100,0)==0){
		return false;
	}
    if(isdigit(document.camera_form.brightness.value,"NBrightness",100,0)==0){
		return false;
	}
	if(isdigit(document.camera_form.contrast.value,"NContrast",100,0)==0){
		return false;
	}
	if(isdigit(document.camera_form.saturation.value,"NSaturation",100,0)==0){
		return false;
	}
	document.camera_form.submit();
	return true;
}

</script>
</head>
<body onLoad="pageInit();" >
<table class="body"><tbody><tr><td>

<h1 id="securityTitle">Image Setup</h1>
<hr />

<form method="get" name="camera_form" action="/goform/updateVideoSettings" onSubmit="return CheckValue()">

<!-- ---------------------  Video Properties  --------------------- -->
<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody>
<tr>
  <td class="title" colspan="3" id="imagepage">Image Properties</td>
</tr>
  <tr>
    <td class="head" id="brightnessid">Brightness:</td>
    <td align="left" width="50"><input type="text" name="brightness" id="brightness" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';} "/></td> 
	
	<td align="left" ><span class="content">[0~100]</span></td>
  </tr>
  <tr>
  	<td class="head" id="contrastid">Contrast: </td>
	<td align="left" width="50"><input type="text" name="contrast" id="contrast" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';}" /></td> 
	<td align="left" ><span class="content">[0~100]</span></td>
	
  </tr>
  <tr>
  	<td class="head" id="saturationid">Saturation: </td>
	<td align="left" width="50"><input type="text" name="saturation" id="saturation" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';} "/></td> 
	
	<td align="left" ><span class="content">[0~100]</span></td>
  </tr>
   <tr>
  	<td class="head" id="nbrightnessid">NBrightness: </td>
	<td align="left" width="50"><input type="text" name="nbrightness" id="nbrightness" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';} "/></td> 
	
	<td align="left" ><span class="content">[0~100]</span></td>
  </tr>
   <tr>
  	<td class="head" id="ncontrastid">NContrast: </td>
	<td align="left" width="50"><input type="text" name="ncontrast" id="ncontrast" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';} "/></td> 
	
	<td align="left" ><span class="content">[0~100]</span></td>
  </tr>
   <tr>
  	<td class="head" id="nsaturationid">NSaturation: </td>
	<td align="left" width="50"><input type="text" name="nsaturation" id="nsaturation" size="5" value="7" onChange="if(/\D/.test(this.value)){alert( '只能输入数字 ');this.value= '';} "/></td> 
	
	<td align="left" ><span class="content">[0~100]</span></td>
  </tr>
  
</tbody></table>

<br />

<!--------------------------- Flickerless ------------------->
<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tbody>
  <tr>
    <td class="title" colspan="2">Flickerless</td>
  </tr>
  <tr> 
    <td width="267" class="head" id="flickerid">Flickerless Mode</td>
    <td width="252">
      <select name="flicker" id="flicker" size="1">
			<option value="50HZ" selected>50HZ </option>
			<option value="60HZ">60HZ </option>
      </select>
    </td>
  </tr>
</tbody></table>

<br/>

<!------------------------Flip Mode ----------------------------->
<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" >
<tbody>

<tr>
	<td class="title" colspan="2">Flip</td>
</tr>
<tr> 
    <td width="267" class="head" id="flickerid">Flip Mode</td>
    <td width="252">
      <select name="rotation" id="rotation" size="1">
			<option value="NORMAL" selected >NORMAL</option>
			<option value="VFLIP">VFLIP </option>
			<option value="MIRROR">MIRROR </option>
			<option value="MIRROR-VFLIP">MIRROR-VFLIP </option>
      </select>
    </td>
</tr>
</tbody></table>

<br/>

<!------------------------IR Mode ----------------------------->
<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" >
<tbody>

<tr>
	<td class="title" colspan="2">IR</td>
</tr>
<tr> 
    <td width="267" class="head" id="irid">IR Mode</td>
    <td width="252">
      <select name="ir" id="ir" size="1">
			<option value="AUTO" selected >AUTO </option>
			<option value="ON">ON </option>
			<option value="OFF">OFF </option>
      </select>
    </td>
</tr>
</tbody></table>

<br/>


<!------------------------------Exposure Control -------------->
<!--
<table id="div_exp" name="div_exp" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tbody><tr> 
    <td class="title" colspan="4" id="exposureid">Exposure Control</td>
  </tr>
  <tr>
  	<td width="268" align="left" class="head"><input  name="exposure" type="radio" value="0"  onClick="disable();"/>Auto:</td>
  </tr>
   <tr>
	<td align="left" class="head"><input  name="exposure" type="radio" value="1"  onClick="disable();" />Manual:</td>
  </tr>
  <tr>
  	<td align="left" class="head">Shutter Speed: </td>
	<td width="251"><span >&nbsp;<span >
	  <select id="shutter_speed" name="shutter_speed">
        <option value="4" >1/50s</option>
        <option value="5" >1/60s</option>
	    <option value="6" >1/100s</option>
        <option value="7" >1/150s</option>
        <option value="8" >1/200s</option>
        <option value="9" >1/250s</option>
        <option value="10" >1/350s</option>
        <option value="11" >1/500s</option>
        <option value="12" >1/1000s</option>
        <option value="13" >1/2000s</option>
        <option value="14" >1/4000s</option>
        <option value="15" >1/8000s</option>
        <option value="16" >1/16000s</option>
      </select>
	</span></span> </td> 
  </tr>
</tbody></table>

<br />

--->

<!----------------------- LED mode--------------------->
<!---

<table id="div_wep" name="div_wep" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" >
<tbody>
<tr>
	<td class="title"> IR </td>
</tr>

<tr>
	<td  class="head"><input name="led" id="led" type="checkbox" value="0" checked/>
	<span >Enable LED Indicator </span>
	</td>
</tr>
</tbody> </table>

<br />
---->


<table border="0" cellpadding="2" cellspacing="1" width="540">
  <tbody><tr align="center">

    <td>
      <input style="width: 120px;" value="Apply" id="cameraApply" type="submit" > &nbsp; &nbsp;
      <input style="width: 120px;" value="Cancel" id="cameraCancel" type="reset" onClick="window.location.reload()" >
    </td>
  </tr>
</tbody></table>
</form>

</td></tr></tbody></table>
</body></html>
 
