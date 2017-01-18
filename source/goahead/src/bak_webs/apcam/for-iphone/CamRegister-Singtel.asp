<!DOCTYPE html PUBLIC “-//W3C//DTD XHTML 1.0 Transitional//EN” “http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd“>
<html>
<head>
<title>Camera Register Setup</title>
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

var imageNr = 0; // Serial number of current image
var finished = new Array(); // References to img objects which have finished downloading
var totimer; 
var oemtype = "<%getOEMPartnerType(); %>";
var descEnable = "<%getDescEnable(); %>";


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
  img.src = "/goform/snapshot?n="+imageNr++;
  if(imageNr == 10000)
  	imageNr = 0;
  var webcam = document.getElementById("img");
  totimer = setTimeout("loadImage()", 2000);
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
  setTimeout("createImageLayer()",100);
}

function pageinit()
{
	REGISTERinitValue(0);
	createImageLayer();
}



</script>
</head>
<body onLoad="pageinit();">
	<div id="jqt">
		<div class="current">
			<div class="toolbar"><script>document.write(language5[2])</script></div><br>
			<div id="page">
				<div class="head"><center><div id="img" style="width:300;height:164"><img src="/goform/snapshot" height=164 width=300 /></div></center></div>
				<ul class="rounded">
					<div style="background-color:#FFFFFF; margin-top:5px; margin-left:6px; margin-right:6px;margin-top:10px;margin-bottom:10px;" >
						<div onClick="document.getElementById('cameraname').focus();" class="cameranameSingtel" id="box_cameraname">
							<div style ="float:left;height:25px;white-space:nowrap;" id="cameraname_width"><script>document.write(language5[3])</script></div>
							<div style ="float:right;" id="edit_cameraname"><input type="text" name="cameraname" id = "cameraname" class="cameranameInput" autocorrect="off" autocapitalize="off" autocomplete="off" /></div>
						</div>
					</div>
				</ul>
				<div id = "display" style="color:#FF0000;text-align: center;text-shadow: rgba(255,255,255,.8) 0 1px 0;"><script>document.write("")</script></div>
				<div id ="registerbutton">
					<ul class="link_finish"><script>document.write("<li><a href='javascript:camRegisterNextStep();'>" + language[4] + "</a></li>");</script></ul>
				</div>
			</div>
			<center>
				<div align = "center" id = "gif" style="visibility:hidden">
					<center><img align="center" src ="/graphics/ajax-loader.gif" /></center><br>
					<div id = "tips">
						<FONT SIZE="3" COLOR="#98a0ac">&nbsp;&nbsp;&nbsp;<script>document.write(language[7]);</script></FONT>
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
