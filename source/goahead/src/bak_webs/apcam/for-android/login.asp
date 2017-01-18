<!DOCTYPE html PUBLIC “-//W3C//DTD XHTML 1.0 Transitional//EN” “http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd“>
<html>
<head>
<title>Login Setup</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<link rel="stylesheet" href="../jqtouch/jqtouch.css" type="text/css">
<link rel="stylesheet" href="../apple/theme.css" type="text/css">
<script src="../jqtouch/jquery-1.4.2.min.js" type="text/javascript" charset="utf-8"></script>
<script src="../jqtouch/jqtouch.js" type="application/x-javascript" charset="utf-8"></script>
<script type="text/javascript" charset="utf-8" src="/js/<%getLGString();%>.js?randomId=<%getUniqueString(1);%>"></script>
<script type="text/javascript" charset="utf-8" src="/js/entire.js?randomId=<%getUniqueString(2);%>"></script>
<script language="JavaScript" type="text/javascript">
	//alert(languageid + " and " + (language1[0].length -1));
var oemtype = "<%getOEMPartnerType(); %>";	
</script>
</head>
<body onLoad="LOGINinitValue();">
	<div id="jqt">
		<div class="current">
			<div class="toolbar"><script>document.write(language4[1])</script></div><br>
			<div id="page">
				<div class="head">&nbsp;&nbsp;&nbsp;<script>document .write(language4[2])</script></div>
				<ul class="rounded">
					<div style="background-color:#FFFFFF; margin-left:6px; margin-right:6px;margin-top:10px;margin-bottom:10px;" >
						<div onClick="document.getElementById('owner_name').focus();" class="owner_name" id="box_name">
							<div style ="float:left;height:28px;margin-top:5px" id="name_width"><script>document.write(language4[3])</script></div>
							<div style ="float:right;" id="edit_name"><input type="text" name="owner_name" id="owner_name" class="ownerName" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
						<div onClick="document.getElementById('owner_password').focus();" class="owner_password">
							<div style ="float:left;height:28px;margin-top:10px" id="password_width"><script>document.write(language4[4])</script></div>
							<div style ="float:right;" id="edit_password"><input type="password" name="owner_password" id="owner_password" class="ownerPass" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
					</div>
				</ul>
				<div id = "display" class="login_info" style="visibility:hidden">
				Login with your SingNet email ID <br>
				(eg yourname@singnet.com.sg) and password. 
				<br><br>
				Please check your SingNet Service letter for your <br>password if you have never changed your email <br>
				password. Otherwise, please call our hotline 1688.
				</div>
				<br><br><br><br><br>	
				<div id="loginbutton">
					<ul class="individual"><script>document.write(" <li><a href='javascript:loginNextStep();'>" + language4[5] + "</a></li>");</script></ul>
				</div>
			</div>
			<center>
				<div align = "center" id = "gif" style="visibility:hidden">
					<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
					<div id = "tips">
						<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language[5]);</script></FONT>
					</div>
				</div>
			</center>
			
			<center>
				<div id = "information">
				</div>
			</center>
			<div id = "button"></div>
		</div>
	</div>		
</body>
</html>
