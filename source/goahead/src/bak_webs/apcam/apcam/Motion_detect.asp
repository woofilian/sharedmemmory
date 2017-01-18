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
		//alert(fields_str);
		if(fields_str.length == 2)
		{
			//alert(all_str[i]);
			if(fields_str[0] == "motionEnable"){
				var array = document.getElementsByName("motionEnable");
				//alert(array.length);
				var j;
				for(j=0; j<array.length; j++){
					//alert(array[j].value);
					if(array[j].value == fields_str[1]){
						array[j].checked = true;
					}
				}
				disable();
				//alert("second");
			}else if(fields_str[0] == "sensitivity"){
				//alert(fields_str[0]);
				document.getElementById("sensitivity").value = fields_str[1];
			}//else if(fields_str[0] == "zonemask"){
				//alert(fields_str[0]);
				//document.getElementById("zonemask").value = fields_str[1];
			//}
		}
	}
}

function pageInit()
{
	makeRequest("/goform/getmotiondetectSettings?motionEnable=&sensitivity=", "n/a", Handler);
}

function disable()
{
	if(motion_form.motionEnable[0].checked == true)
	{
		document.getElementById("sensitivity").disabled = false;
//		document.getElementById("zonemask").disabled = false;
	}
	else if(motion_form.motionEnable[1].checked == true)
	{
		document.getElementById("sensitivity").disabled = true;
//		document.getElementById("zonemask").disabled = true;
	}
}

</script>

</head>
<body onLoad="pageInit()">
<table class="body"><tr><td width="529">
<h1 id="ftpTitle_text">Ap Motion Detect </h1>
<hr />
<form method="get" name ="motion_form" action="/goform/updatemotiondetectSettings">

<!-- ==================  Audio Enable  ================== -->
<table border="1" cellpadding="2" cellspacing="1" width="100%">
<tbody>
<tr>
  <td class="title" colspan="3" id="motion_detect_text">Enable Motion Detect </td>
</tr>

<tr>
	<td width="51%" height="25" align="left" class="head">&nbsp;Enable Motion Detect:&nbsp;</td>
    <td width="23%" align="center" ><input type="radio" name="motionEnable" value="YES" onClick = "disable();">YES</td>
    <td width="26%" align="center" ><input type="radio" name="motionEnable" value="NO" onClick = "disable();">NO</td>
</tr>

</tbody>
</table>


<br />
<!-- ==================  motion Setting  ================== -->
<table  border="1" cellpadding="2" cellspacing="1" width="100%" >
<tbody>

<tr>
  <td class="title" colspan="3" id="motion_setting">motion detect Setting </td>
</tr>

<tr>
      <td height="25"  align="left" class="head">&nbsp;Sensitivity:&nbsp;</td>
      <td  colspan="3" align="left" >&nbsp;
	  	<!-- <input type = "text" name = "sensitivity" id = "sensitivity" /> -->
		
		<select name="sensitivity" id = "sensitivity">
        <option value="0"  >0</option>
        <option value="1"  >1</option>
        <option value="2"  >2</option>
        <option value="3"  >3</option>
        <option value="4"  >4</option>
        <option value="5"  >5</option>
		</select>
		
		</td>
</tr>

<!--
<tr>
<td  align="left" height="25" class="head">&nbsp;Detect Zone:&nbsp;</td>
<td  align="left" >&nbsp;
<input type = "text" name = "zonemask" id = "zonemask" />
</td>
<td align="left" ><span class="content">[0xfffff]</span></td>
</tr>
-->

</tbody>
</table>

<br />

<table border="0" cellpadding="2" cellspacing="1" width="540">
  <tbody><tr align="center">

    <td>
      <input style="width: 120px;" value="Apply" id="motionApply" type="submit"> &nbsp; &nbsp;
      <input style="width: 120px;" value="Cancel" id="motionCancel" type="reset" onClick="window.location.reload()" >
    </td>
  </tr>
</tbody></table>

<br />
</form>

</td></tr></table>
</body></html>
