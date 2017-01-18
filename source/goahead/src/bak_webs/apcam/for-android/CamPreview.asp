<!DOCTYPE html PUBLIC “-//W3C//DTD XHTML 1.0 Transitional//EN” “http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd“>
<html>
<head>
<title>Camera Preview Setup</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />

<%
	setServerIpAndLanguageId(QUERY_STRING);
%>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<link rel="stylesheet" href="../jqtouch/jqtouch.css" type="text/css">
<link rel="stylesheet" href="../apple/theme.css" type="text/css">
<script src="../jqtouch/jquery-1.4.2.min.js" type="text/javascript" charset="utf-8"></script>
<script src="../jqtouch/jqtouch.js" type="application/x-javascript" charset="utf-8"></script>
<script type="text/javascript" charset="utf-8" src="/js/<%getLGString();%>.js?randomId=<%getUniqueString(1);%>"></script>
<script type="text/javascript" charset="utf-8" src="/js/entire.js?randomId=<%getUniqueString(2);%>"></script>
<script language="JavaScript" type="text/javascript">
<% realtimeParamAnalyze(1); %>
var setupmode = <% getCfgZero(1, "SetupState"); %>;
var previewvideotimer;
var oemtype = "<%getOEMPartnerType(); %>";
var imageNr = 0; // Serial number of current image
var finished = new Array(); // References to img objects which have finished downloading
var totimer; 
var urlQueryString;

function loadImage()
{
	createImageLayer();
}

function createImageLayer() {
  var img = new Image();
  img.style.position = "absolute";
  img.style.zIndex = -1;
  img.onload = imageOnload;
  img.width = 300;
  img.height = 164;
  var tmpurl=location.href;
  urlQueryString=tmpurl.substring(tmpurl.indexOf("?")+1)
  if(urlQueryString=="")
    img.src = "/goform/snapshot?n="+ (++imageNr);
  else
    img.src = "/goform/snapshot?n="+ (++imageNr)+"&"+urlQueryString;
  if(imageNr == 10000)
  	imageNr = 0;
  var webcam = document.getElementById("img");
  totimer = setTimeout("loadImage()", 1000);
  webcam.insertBefore(img, webcam.firstChild);
}

// Two layers are always present (except at the very beginning), to avoid flicker
function imageOnload() {
  clearTimeout(totimer);
  this.style.zIndex = imageNr; // Image finished, bring to front!
  while (1 < finished.length) {
    var del = finished.shift(); // Delete old image(s) from document
    del.parentNode.removeChild(del);
  }
  finished.push(this);
  //createImageLayer();
  setTimeout("createImageLayer()",100);
}

function onLoadInit(){
	createImageLayer();
	CAMPreviewinitValue(0);
	//if(typeof(informPageFinished) == "function")
	//SeedonkAndroid.informPageFinished();

	if(typeof(SeedonkAndroid) != "undefined")
	{
        	SeedonkAndroid.informPageFinished(window.location.href);
	}
}
	
</script>
</head>
<body onLoad="onLoadInit();">
 <div id="jqt">
	<div class="current">
		<div class="toolbar"><script>document.write(language1[1]);</script></div><br>
		<div id="page">
			<div class="head">
<!--
					&nbsp;&nbsp;<script>document.write(language1[2]);</script>
					&nbsp;&nbsp;<script>document.write(language1[3]);</script>
-->
			</div><br>
			<div class="preview"><center><div id ="img" style="width:300;height:164">
                <script>
                if(urlQueryString=="")
                    document.write("<img src='/goform/snapshot' height=164 width=300 />");  
                else
                    document.write("<img src='/goform/snapshot" + "?" + urlQueryString + "'" 
                        + "height=164 width=300 />");  
                </script>
			</div></center></div><br>
			<ul class="individual">                               
				<a href='javascript:stepIntoSL();' style="TEXT-DECORATION:none">
					<li style="color:black; word-wrap: break-word; overflow:hidden;padding-right:10px">
						<script> 
							if(oemtype == "4" || oemtype == "5")
								document.write(language[4]);
							else 
								document.write(language1[4]);
						</script>                              
					</li>                                                                           
				</a>                                                                            
			</ul>	
			
			<!--
			<ul class="individual">
				<script>
				if(oemtype == "4" || oemtype == "5")
					document.write("<li><a href='javascript:stepIntoSL();'>" + language[4] + "</a></li>");
				else
					document.write("<li><a href='javascript:stepIntoSL();'>" + language1[4] + "</a></li>");
				</script>
			</ul>
			-->
			
			<div class="wordLowerLeft" style="visibility:hidden" id="singtelcampreview">
			If you do not see an image, please ensure that your camera is switched on and try again.<br>
			For more information and further assistance on technical support, please visit:<br>
			<a href=http://www.singtel.com/homeLIVECam/FAQ>www.singtel.com/homeLIVECam/FAQ</a>
			</div>
			
		</div>
		
		<center>
			<div align = "center" id = "gif" style="visibility:hidden">
				<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
				<div id = "tips">
					<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language[10]);</script></FONT>
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
