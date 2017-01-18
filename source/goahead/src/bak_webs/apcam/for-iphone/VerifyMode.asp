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
<link rel="stylesheet" href="../apple/theme.css" type="text/css">
<script src="../jqtouch/jquery-1.4.2.min.js" type="text/javascript" charset="utf-8"></script>
<script src="../jqtouch/jqtouch.js" type="application/x-javascript" charset="utf-8"></script>
<script type="text/javascript" charset="utf-8" src="/js/<%getLGString();%>.js?randomId=<%getUniqueString(1);%>"></script>
<script type="text/javascript" charset="utf-8" src="/js/entire.js?randomId=<%getUniqueString(2);%>"></script>

<script language="JavaScript" type="text/javascript">
	<% setServerSetupSwitchState() ; %>
	var bShowAlert=0;
	var oemtype = "<%getOEMPartnerType(); %>";
	
	//alert(languageid + " and " + (language1[0].length -1));
	
</script>

</head>



<body onLoad="VERIFYinitValue();" onUnload="VERIFYUnload();">
	<div id="jqt">
		<div class="current">
			<div id = "verifymodetitleid" style="font-weight: bold;text-align:center;font-size:18px;color:#FFFFFF;-webkit-box-sizing: border-box;border-bottom: 1px solid #2d3642;padding: 10px;height: 45px;background-color:#748aa7;position: relative;">
			    <script>
				if(oemtype != "0")
				{
					var tipsstr = language6[0] + ""
					if(oemtype == "4" || oemtype == "5")
					{
						var newstr = tipsstr.replace("WiFi", "Wi-Fi")
						tipsstr = newstr
					}
						
					document.write(tipsstr)
				}
				else
					document.write(language6[4])
				</script>
			</div><br>
			
			<div id = "page">
				<ul style="font-size: 14px;text-align: left center;text-shadow: rgba(255,255,255,.8) 0 1px 0;color: rgb(76, 86, 108);">
					<div style="width: 100%;font-size: 14px;text-align: left center;text-shadow: rgba(255,255,255,.8) 0 1px 0;color: rgb(76, 86, 108);">
					
						<div style="padding-right:20px;padding-left:20px;padding-top:20px;">
<!-- //Please set the "WiFi Setup" switch to OFF to complete setup process-->
						<script>
						if(oemtype == "3")
							document.write(language6[8])
						else if(oemtype == "4")
							document.write("Please press the Wi-Fi Setup button on the back of the camera once to complete the setup process")
						else if(oemtype != "0")
							document.write(language6[1])
						else
							document.write(language6[5])
						</script>
						</div>
						<center><div>
						<img width="220" height="180" src="/apcam/apple/img/<%getWifiSetupOffPngName();%>.png?randomId=<%getUniqueString(3);%>" />
						</div></center>
<!-- //Warning: your camera will not initialize until this is done!-->

						<div style="padding-right:20px;padding-left:20px;padding-bottom:10px; color:#ff0000">
						<script>
						if(oemtype != "0")
							document.write(language6[2])
						else
							document.write(language6[6])
						</script>
						</div>

					</div>
				</ul>

<!--//<div style="padding:62px;"></div>-->
				<div id ="verifymodebutton" >
					<ul class="link_finish"><li><a href="javascript:verifyModeNextStep();" >
					<script>document.write(language[4])
					</script></a></li></ul>
				</div>
				
			</div>
			
			<center>
				<div align = "center" id = "gif" style="visibility:hidden">
					<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
					<div id = "tips">
<!-- //Going to finish page ... please wait -->
						<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language[8]);</script></FONT>
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