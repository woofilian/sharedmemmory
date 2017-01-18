<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<title>AP Client Feature</title>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache" />
<META HTTP-EQUIV="Expires" CONTENT="-1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1; user-scalable=0;" />
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
	var ssid ="";
	var bssid ="";
	var channel ="";
	var mode ="";
	var enc ="";
	var key ="";
	var keytype ="";
	var wpapsk ="";
	var iswep = false;
	var channel_num = "auto";
	var wlanNetArray = new Array();
	var oldhead ="";
	var old2head ="";
	var othernet = false;
	var oldnetname;
	var oldnetpassword;
	var ap_count;
	var num =0;
	var interval = 2000;
	var stoptimer;
	var inittimer;
	var isstop = false;
	var unsetup = true;
	var firstfocus = true;
	var isshowpw = false;
	var selectedrow =0;
	var response;
	var percent;
	var security;
	var currow;
	
	var currentbssid="<%getCfgZero(1, 'ApCliBssid'); %>";
	var currentencrytype="<%getCfgZero(1, 'ApCliEncrypType'); %>";
	var currentmode="<%getCfgZero(1, 'ApCliAuthMode'); %>";
	var currentkey="<%getCfgZero(1, 'ApCliKey1Str'); %>";
	var currentwpapsk="<%getCfgZero(1, 'ApCliWPAPSK'); %>";
    var hasrj45="<% getRJ45State(); %>";
    var usewps="0";
	var oemtype = "<%getOEMPartnerType(); %>";

	
	//alert(languageid + " and " + (language1[0].length -1));
</script>
</head>
<body onLoad="initValue();">
	<div id="jqt">
		<div class="current">
			<div id ="bar">
				<div class="toolbar"><script>document.write(language2[1])</script></div>
			</div>
			<div id ="page">
				<div style ="height:20px; padding-top:10px;padding-left:10px; font-size:16px;">
					<script>document.write(language2[2])</script>                               
				</div>                                                                       
				<div style="height:20px;text-align:right; padding-right:10px;">              
					<button class="refresh" onclick="javascript:refreshInitValue()">  
					<script>document.write(language2[31]);</script></button>           
				</div>
				<ul class="rounded">
					<div style="background-color:#FFFFFF; margin-left:5px; margin-right:5px;margin-top:5px;margin-bottom:5px;" >
						 <div id = "divSite" ></div> 
					</div>
				</ul>
				<div class="wordUpper">
					<script>document.write(language2[16])</script><br>
					<script>document.write(language2[17])</script><br>
					<script>document.write(language2[18])</script><br>
					<script>document.write(language2[19])</script><br>
				</div><br>
<script>
if(hasrj45 == "1")
{
				document.write("<div class='wordLower'>");
				document.write(language2[3]);
				document.write("<br>&nbsp;&nbsp;");
				document.write(language2[4]);
				document.write("</FONT>	</div>");
}
</script>
<!--
				<div class="wordLower"> 
					<script>document.write(language2[3])</script><br>&nbsp;&nbsp;
					<script>document.write(language2[4])</script></FONT>
				</div>
-->
                </div><br>
			<center>
				<div align = "center" id = "gif">
					<div id = "tips">
						<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language2[22]);</script></FONT>
					</div>
					<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
				</div>
			</center>
			
			<center>
				<div id = "information">
				</div>
			</center>
			<div id = "button"></div>
			<div id = "wpsbutton"></div>
		</div>
	</div>
</body>
</html>

