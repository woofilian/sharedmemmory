<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>SMTP Settings</title>

<style type="text/css">
#wps_progress_bar {
width:250px;
height:15;
margin: 0 auto;
border: 1px solid gray;
}
</style>

<script language="JavaScript" type="text/javascript">
function style_display_on(){
    if(window.ActiveXObject) { // IE
        return "block";
    } else if (window.XMLHttpRequest) { // Mozilla, Safari,...
        return "table-row";
    }
}

/***********************************************************************
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
			if (fields_str[0] == "ftpAddress"){
				document.getElementById("ftpAddress").value = fields_str[1];
			}else if(fields_str[0] == "ftpPort"){
				document.getElementById("ftpPort").value = fields_str[1];
			}else if(fields_str[0] == "ftpUsername"){
				document.getElementById("ftpUsername").value = fields_str[1]
			}else if(fields_str[0] == "ftpPassword"){
				document.getElementById("ftpPassword").value = fields_str[1];
			}else if(fields_str[0] == "ftpUploadpath"){
				document.getElementById("ftpUploadpath").value = fields_str[1];
			}
		}

	}
}
******************************************************************************/

function pageInit()
{
	disableTrigger();
	disableDay();
	disableType();
}

function disableTrigger()
{
 	if(form_smtp.smtptriggerenable[0].checked==true){
		
	}
	else if(form_smtp.smtptriggerenable[1].checked==true){
	
	}
}

function disableDay()
{
	if(form_smtp.smtptriggerdays[0].checked==true){
		document.form_smtp.smtpsun.disabled=true;
		document.form_smtp.smtpmon.disabled=true;
		document.form_smtp.smtptue.disabled=true;
		document.form_smtp.smtpwed.disabled=true;
		document.form_smtp.smtpthu.disabled=true;
		document.form_smtp.smtpfri.disabled=true;
		document.form_smtp.smtpsat.disabled=true;
	}
	else if(form_smtp.smtptriggerdays[1].checked==true){
		document.form_smtp.smtpsun.disabled=false;
		document.form_smtp.smtpmon.disabled=false;
		document.form_smtp.smtptue.disabled=false;
		document.form_smtp.smtpwed.disabled=false;
		document.form_smtp.smtpthu.disabled=false;
		document.form_smtp.smtpfri.disabled=false;
		document.form_smtp.smtpsat.disabled=false;
}
}

function disableType()
{
	if(form_smtp.smtptriggertype[0].checked==true){
		document.form_smtp.smtpinterval.disabled=true;
	}
	else if(form_smtp.smtptriggertype[1].checked==true){
		document.form_smtp.smtpinterval.disabled=false;
	}
}

</script>

</head>
<body onLoad="pageInit()">
<table class="body"><tbody><tr><td>
<h1 id="smtpTitle">SMTP Settings </h1>
<hr />
<form method=post name=form_smtp action="/goform/updateSmtpServerSettings" ">
<table width="539" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
<tbody>
  <tr> 
    <td class="title" colspan="2" id="smtprvSet">SMTP Server Setup</td>
  </tr>
  <tr> 
    <td width="189" class="head" id="smtpSrv">SMTP Server Address</td>
    <td width="329">
	<input type=text name="smtpaddress" id="smtpaddress" value = "<% getCfgGeneral(1, "smtpaddress"); %>">	
	</td>
  </tr>
  <tr>
    <td class="head" id="smtpSrvPort">SMTP Port</td>
    <td>
      <input type=text name="smtpport" id="smtpport" size=5 maxlength=5 value = "<% if((getCfgGeneral(0, "smtpport")== "") write("21"); else getCfgGeneral(1,"smtpport");%>">   
	</td>
  </tr>
  <tr>
    <td class="head" id="authEnable">SMTP Authentication</td>
    <td>
      <span><input type=radio name="smtpauthenable" id="smtpauthenable" value ="0" />Disable</span>
	  <span><input type=radio name="smtpauthenable" id="smtpauthenable" value ="1" />Enable</span>
	</td>
  </tr>
  <tr>
    <td class="head" id="smtpSrvUserName">User Name</td>
    <td>
      <input type=text name="smtpusername" id="smtpusername" value = "<% getCfgGeneral(1, "smtpusername"); %>">
	</td>
  </tr>
  <tr>
    <td class="head" id="smtpSrvPassword">Password</td>
    <td>
      <input type=text name="smtppassword" id = "smtppassword" value = "<% getCfgGeneral(1, "smtppassword"); %>"></td>
  </tr>
  <tr>
    <td class="head" id="smtpSrvFrom">E-mail From</td>
    <td>
      <input type=text name="smtpfrom" id = "smtpfrom" value = "<% getCfgGeneral(1, "smtpfrom"); %>"></td>
  </tr>
  <tr>
    <td class="head" id="smtpSrvTo">E-mail To</td>
    <td>
      <input type=text name="smtpto" id = "smtpto" value = "<% getCfgGeneral(1, "smtpto"); %>"></td>
  </tr>
  <tr>
    <td class="head" id="smtpSubject">subject</td>
    <td>
      <span>
        <input type=text name="smtpsubject" id = "smtpsubject" value = "<% getCfgGeneral(1, "smtpsubject"); %>">
	  </span>
      </td>
  </tr>
</tbody>  
</table>
<br/>


<!-- ==================  trigger Enable  ================== -->
<table width="539" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
<tbody>
<tr>
  <td class="title" colspan="3">Trigger Enable</td>
</tr>

<tr>
	<td width="51%" height="25" align="left" class="head">&nbsp;Enable:&nbsp;</td>
    <td width="23%" align="center" ><input type="radio" name="smtptriggerenable" value="1" onClick = "disableTrigger();" <% var te = getCfgZero(0, "smtptriggerenable"); if (te == "1") write("checked"); %>>YES</td>
    <td width="26%" align="center" ><input type="radio" name="smtptriggerenable" value="0" onClick = "disableTrigger();" <% if (te == "0") write("checked"); %>>NO</td>
</tr>

</tbody>
</table>
<br/>

<!-- ==================  time setting  ================== -->
<table width="539" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
<tbody>
<tr>
  <td class="title" colspan="3" >Time Setup</td>
</tr>

<tr>
	<td width="51%" height="25" align="left" class="head">&nbsp;Time Range&nbsp;</td>
    <td>
	<input type="text" name="smtpstarttime"  size = 5 value ="<% getCfgGeneral(1, "smtpstarttime"); %>">
	<span>--</span>
	<input type="text" name="smtpendtime"  size = 5 value ="<% getCfgGeneral(1, "smtpendtime"); %>">
	<span>format:"xx:xx"</span>
	</td>
</tr>
	<tr>
	<td width="51%" height="25" align="left" class="head"><input type="radio" name="smtptriggerdays" value = "everyday"  onClick = "disableDay();" <% var days = getCfgZero(0, "smtptriggerdays"); if (days == "everyday") write("checked"); %>>always(days)</td>
	</tr>
	<tr>
	<td width="51%" height="25" align="left" class="head"><input type="radio" name="smtptriggerdays" value = "someday" onClick = "disableDay();" <% if (days == "someday") write("checked"); %>> only during time frame</td>
	</tr>
	<tr>
	<td colspan="2">
	&nbsp;&nbsp;&nbsp;
	<span><input type="checkbox" name ="smtpsun" value ="on" <% if (getCfgGeneral(0,"smtpsun") == "on") write("checked"); %>>Sun</span>
	<span><input type="checkbox" name ="smtpmon" value ="on" <% if (getCfgGeneral(0,"smtpmon") == "on") write("checked"); %>>Mon</span>
	<span><input type="checkbox" name ="smtptue" value ="on" <% if (getCfgGeneral(0,"smtptue") == "on") write("checked"); %>>Tue</span>
	<span><input type="checkbox" name ="smtpwed" value ="on" <% if (getCfgGeneral(0,"smtpwed") == "on") write("checked"); %>>Wed</span>
	<span><input type="checkbox" name ="smtpthu" value ="on" <% if (getCfgGeneral(0,"smtpthu") == "on") write("checked"); %>>Thu</span>
	<span><input type="checkbox" name = "smtpfri" value ="on" <% if (getCfgGeneral(0,"smtpfri") == "on") write("checked"); %>>Fri</span>
	<span><input type="checkbox" name = "smtpsat" value ="on" <% if (getCfgGeneral(0,"smtpsat") == "on") write("checked"); %>>Sat</span>
	</td>
	</tr>
</tbody>
</table>

<br/>

<!-- ==================  Trigger setting  ================== -->
<table width="539" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
<tbody>
<tr>
  <td class="title" colspan="3" >Trigger Type</td>
</tr>

<tr>
	<td width="51%" align="left" class="head"><input type="radio" name="smtptriggertype" value="motionevent" onClick = "disableType();"  <% var tt = getCfgZero(0, "smtptriggertype"); if (tt == "event") write("checked"); %>>Event Trigger</td>
</tr>
<tr>
	<td  width="51%" align="left" class="head"><input type="radio" name="smtptriggertype" value = "schedule" onClick = "disableType();" <% if (tt == "schedule") write("checked"); %>>Schedule Trigger</td>
</tr>
<tr>
	<td width="51%" height="25"  align="left" class="head">Time Interval(Minute)</td>
	<td><input type="text" name="smtpinterval" id = "smtpinterval" value ="<% getCfgGeneral(1, "smtpinterval"); %>"></td>
</tr>

</tbody>
</table>

<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="smtpApply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Reset" id="smtpReset" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></tbody></table>
</body>
</html>

