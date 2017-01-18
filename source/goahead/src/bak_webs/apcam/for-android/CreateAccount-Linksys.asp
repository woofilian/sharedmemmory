<!DOCTYPE html PUBLIC “-//W3C//DTD XHTML 1.0 Transitional//EN” “http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd“>
<html>
<head>
<title>Create Account Setup</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<link rel="stylesheet" href="../jqtouch/jqtouch.css" type="text/css">
<link rel="stylesheet" href="../apple/theme.css?randomId=<%getUniqueString(3);%>" type="text/css">
<script src="../jqtouch/jquery-1.4.2.min.js" type="text/javascript" charset="utf-8"></script>
<script src="../jqtouch/jqtouch.js" type="application/x-javascript" charset="utf-8"></script>
<script type="text/javascript" charset="utf-8" src="/js/<%getLGString();%>.js?randomId=<%getUniqueString(1);%>"></script>
<script type="text/javascript" charset="utf-8" src="/js/entire.js?randomId=<%getUniqueString(2);%>"></script>
<script language="JavaScript" type="text/javascript">
	//alert(languageid + " and " + (language1[0].length -1));
var oemtype = "<%getOEMPartnerType(); %>";
	
</script>
</head>
<body  onLoad="ACOUNTinitValue();">
	<div id="jqt">
		<div class="current">
			<div class="toolbar"><script>document.write(language3[1])</script></div><br>
			<div id="page">
				<div class="head"><p style="padding-left:10px;font-size:16px"><script>document.write(language3[18])</p></script></div>
				<ul class="rounded">
					<div style="background-color:#FFFFFF; margin-left:6px; margin-right:6px;margin-top:5px;margin-bottom:5px;" >
						<div onClick=" document.getElementById('seedonk_firstname').focus();" class="seedonk_login" id="box_login">
							<div style ="float:left;height:25px;" id="firstname_width"><script>document.write(language3[14])</script></div>
							<div style ="float:right;" id="edit_firstname"><input type="text" name="seedonk_firstname" id="seedonk_firstname" class="loginInput" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
						<div onClick=" document.getElementById('seedonk_lastname').focus();" class="seedonk_passwd">
							<div style ="float:left;height:25px;margin-top:0px" id="lastname_width"><script>document.write(language3[15])</script></div>
							<div style ="float:right;" id="edit_lastname"><input type="text" name="seedonk_lastname" id="seedonk_lastname" class="passwdInput" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
						<div onClick=" document.getElementById('seedonk_login').focus();" class="seedonk_repasswd" >
							<div style ="float:left;height:25px;margin-top:2px" id="login_width"><script>document.write(language3[6])</script></div>
							<div style ="float:right;" id="edit_login"><input type="text" name="seedonk_login" id="seedonk_login" class="repasswdInput" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
					</div>
				</ul>

				<div id = "displaypwd" class="info" ><script>document.write(language3[24])</script></div>

				<ul class="rounded">
					<div style="background-color:#FFFFFF; margin-left:6px; margin-right:6px;margin-top:5px;margin-bottom:5px;" >
						<div onClick=" document.getElementById('seedonk_passwd').focus();" class="seedonk_email">
							<div style ="float:left;height:25px;margin-top:5px" id="passwd_width"><script>document.write(language3[4])</script></div>
							<div style ="float:right;" id = "edit_passwd"><input type="password" name="seedonk_passwd" id="seedonk_passwd" class="emailInput" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
					</div>
				</ul>
				
				<div id = "display" class="info" ><script>document.write(language3[7])</script></div>

				<div style="padding:27px;"></div>
				<div id ="accountbutton">
					<ul class="link">                               
						<a href='javascript:stepIntoLG();' style="TEXT-DECORATION:none">
							<li style="color:black; word-wrap: break-word; overflow:hidden;padding-right:10px">
								<script> 
									document.write(language3[8]);
								</script>                              
							</li>     
						</a>    
						<a href='javascript:stepIntoAccountInfo();' style="TEXT-DECORATION:none">
							<li style="color:black; word-wrap: break-word; overflow:hidden;padding-right:10px">
								<script> 
									document.write(language[4]);
								</script>                              
							</li>     
						</a>     
					</ul>	
					<!--
					<ul class="link">
						<script>
							document.write("<li><a href='javascript:stepIntoLG();'>" + language3[8] + "</a></li>");
							document.write("<li><a href='javascript:stepIntoAccountInfo();'>" + language[4] + "</a></li>"); 
						</script> 
					</ul>
					-->
				</div>
			</div>
			
			<div id="accountinfopage" style="visibility:hidden">

				<div class="accountinfo"><script>document.write(language3[16])</script></div>
				<div style="padding:20px;"></div>
				<div class="accountinfo" style="word-break : break-all; overflow:hidden;padding-right:10px"><script>document.write("<div id='accountinfo_firstname'></div>")</script></div>
				<div class="accountinfo" style="word-break : break-all; overflow:hidden;padding-right:10px"><script>document.write("<div id='accountinfo_lastname'></div>")</script></div>
				<div class="accountinfo" style="word-break : break-all; overflow:hidden;padding-right:10px"><script>document.write("<div id='accountinfo_email'></div>")</script></div>
				<div class="accountinfo" style="word-break : break-all; overflow:hidden;padding-right:10px"><script>document.write("<div id='accountinfo_password'></div>")</script></div>
				<div style="padding:20px;"></div>
				<div class="accountinfo"><script>document.write(language3[17])</script></div>
				
				<div id ="accountinfobutton">
					<ul class="link">
						<script>
							document.write("<li><a href='javascript:returnAccountPage();'>" + language[3] + "</a></li>");
							document.write("<li><a href='javascript:createAccountNextStep();'>" + language[4] + "</a></li>"); 
						</script> 
					</ul>
				</div>
			</div>
			
			<center>
				<div align = "center" id = "gif" style="visibility:hidden">
					<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
					<div id = "tips">
						<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language[6]);</script></FONT>
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
