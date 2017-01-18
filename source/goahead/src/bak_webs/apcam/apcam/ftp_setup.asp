<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>FTP Settings</title>

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

function pageInit()
{
	disableTrigger();
	disableDay();
	disableType();
}

function disableTrigger()
{
 	if(form_ftp.ftptriggerenable[0].checked==true){
		
	}
	else if(form_ftp.ftptriggerenable[1].checked==true){
	
	}
}

function disableDay()
{
	if(form_ftp.ftptriggerdays[0].checked==true){
		document.form_ftp.ftpsun.disabled=true;
		document.form_ftp.ftpmon.disabled=true;
		document.form_ftp.ftptue.disabled=true;
		document.form_ftp.ftpwed.disabled=true;
		document.form_ftp.ftpthu.disabled=true;
		document.form_ftp.ftpfri.disabled=true;
		document.form_ftp.ftpsat.disabled=true;
	}
	else if(form_ftp.ftptriggerdays[1].checked==true){
		document.form_ftp.ftpsun.disabled=false;
		document.form_ftp.ftpmon.disabled=false;
		document.form_ftp.ftptue.disabled=false;
		document.form_ftp.ftpwed.disabled=false;
		document.form_ftp.ftpthu.disabled=false;
		document.form_ftp.ftpfri.disabled=false;
		document.form_ftp.ftpsat.disabled=false;
}
}

function disableType()
{
	if(form_ftp.ftptriggertype[0].checked==true){
		document.form_ftp.ftpinterval.disabled=true;
	}
	else if(form_ftp.ftptriggertype[1].checked==true){
		document.form_ftp.ftpinterval.disabled=false;
	}
}

</script>

</head>
<body onLoad="pageInit()">
<table class="body"><tbody><tr><td>
<h1 id="ftpTitle">FTP Settings </h1>
<hr />
<form method=post name=form_ftp action="/goform/updateFtpServerSettings" ">
<table width="539" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
<tbody>
  <tr> 
    <td class="title" colspan="2" id="ftpSrvSet">FTP Server Setup</td>
  </tr>
  <tr> 
    <td width="189" class="head" id="ftpSrv">FTP Server Address</td>
    <td width="329">
	<input type=text name="ftpaddress" id="ftpaddress" value = "<% getCfgGeneral(1, "ftpaddress"); %>">	
	</td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvPort">FTP Port</td>
    <td>
      <input type=text name="ftpport" id="ftpport" size=5 maxlength=5 value = "<% if((getCfgGeneral(0, "ftpport")== "") write("21"); else getCfgGeneral(1,"ftpport");%>">   
	</td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvUserName">FTP User Name</td>
    <td>
      <input type=text name="ftpusername" id="ftpusername" value = "<% getCfgGeneral(1, "ftpusername"); %>">    
	</td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvPassword">FTP Password</td>
    <td>
      <input type=password name="ftppassword" id = "ftppassword" value = "<% getCfgGeneral(1, "ftppassword"); %>">
	</td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvUploadPath">FTP Upload Path</td>
    <td>
      <span>
        <input type=text name="ftpuploadpath" id = "ftpuploadpath" value = "<% getCfgGeneral(1, "ftpuploadpath"); %>">
	  	Relative PATH 
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
    <td width="23%" align="center" ><input type="radio" name="ftptriggerenable" value="1" onClick = "disableTrigger();" <% var te = getCfgZero(0, "ftptriggerenable"); if (te == "1") write("checked"); %>>YES</td>
    <td width="26%" align="center" ><input type="radio" name="ftptriggerenable" value="0" onClick = "disableTrigger();" <% if (te == "0") write("checked"); %>>NO</td>
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
	<input type="text" name="ftpstarttime"  size = 5 value ="<% getCfgGeneral(1, "ftpstarttime"); %>">
	<span>--</span>
	<input type="text" name="ftpendtime"  size = 5 value ="<% getCfgGeneral(1, "ftpendtime"); %>">
	<span>format:"xx:xx"</span>
	</td>
</tr>
	<tr>
	<td width="51%" height="25" align="left" class="head"><input type="radio" name="ftptriggerdays" value = "everyday"  onClick = "disableDay();" <% var days = getCfgZero(0, "ftptriggerdays"); if (days == "everyday") write("checked"); %>>always(days)</td>
	</tr>
	<tr>
	<td width="51%" height="25" align="left" class="head"><input type="radio" name="ftptriggerdays" value = "someday" onClick = "disableDay();" <% if (days == "someday") write("checked"); %>> only during time frame</td>
	</tr>
	<tr>
	<td colspan="2">
	&nbsp;&nbsp;&nbsp;
	<span><input type="checkbox" name ="ftpsun" value ="on" <% if (getCfgGeneral(0,"ftpsun") == "on") write("checked"); %>>Sun</span>
	<span><input type="checkbox" name ="ftpmon" value ="on" <% if (getCfgGeneral(0,"ftpmon") == "on") write("checked"); %>>Mon</span>
	<span><input type="checkbox" name ="ftptue" value ="on" <% if (getCfgGeneral(0,"ftptue") == "on") write("checked"); %>>Tue</span>
	<span><input type="checkbox" name ="ftpwed" value ="on" <% if (getCfgGeneral(0,"ftpwed") == "on") write("checked"); %>>Wed</span>
	<span><input type="checkbox" name ="ftpthu" value ="on" <% if (getCfgGeneral(0,"ftpthu") == "on") write("checked"); %>>Thu</span>
	<span><input type="checkbox" name = "ftpfri" value ="on" <% if (getCfgGeneral(0,"ftpfri") == "on") write("checked"); %>>Fri</span>
	<span><input type="checkbox" name = "ftpsat" value ="on" <% if (getCfgGeneral(0,"ftpsat") == "on") write("checked"); %>>Sat</span>
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
	<td width="51%" align="left" class="head"><input type="radio" name="ftptriggertype" value="motionevent" onClick = "disableType();"  <% var tt = getCfgZero(0, "ftptriggertype"); if (tt == "motionevent") write("checked"); %>>Event Trigger</td>
</tr>
<tr>
	<td  width="51%" align="left" class="head"><input type="radio" name="ftptriggertype" value = "schedule" onClick = "disableType();" <% if (tt == "schedule") write("checked"); %>>Schedule Trigger</td>
</tr>
<tr>
	<td width="51%" height="25"  align="left" class="head">Time Interval(Minute)</td>
	<td><input type="text" name="ftpinterval" id = "ftpinterval" value ="<% getCfgGeneral(1, "ftpinterval"); %>"></td>
</tr>

</tbody>
</table>

<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="ftpApply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Reset" id="ftpReset" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></tbody></table>
</body>
</html>

