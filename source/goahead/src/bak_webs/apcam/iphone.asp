
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>

<title>Service</title>

<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />

<script type="text/javascript" src="www.seedonk.com/seedonk/iphone/main.js"></script>

<script type="text/javascript">

	function style_display_on(){
    	if(window.ActiveXObject) { // IE
        	return "block";
    	} else if (window.XMLHttpRequest) { // Mozilla, Safari,...
       	 return "table-row";
    	}
	}

	var http_request = false;
	var image_width = "320px";
	var image_height = "480px";

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
		//alert(all_str.length);

		for (var i=0; i<all_str.length; i++) {
			var fields_str = new Array();
			fields_str = all_str[i].split(":");
			if(fields_str.length == 2)
			{
				if(fields_str[0] == "resolution"){
					if(fields_str[1] == "VGA"){
						image_width = "640px";
						image_height = "480px";
					} else if(fields_str[1] == "QVGA"){
						image_width = "320px";
						image_height = "240px";
					}else if(fields_str[1] == "HVGA"){
						image_width = "480px";
						image_height = "320px";
					}
				}else{
					alert("PARSE ERROR!");
				}
		
			}

		}
	}

	function pageInit()
	{
		makeRequest("/goform/getVideoSettings?resolution=", "n/a", Handler);
	}


</script>


<link type="text/css" rel="stylesheet" href="www.seedonk.com/seedonk/iphone/istyle.css"/>

</head>
<body onload="pageInit();"  onorientationchange="updateOrientation()">

<table width="100%">
<tr>
<td>
	<div id="header" >
		<img src="/goform/video" height=240 width=320 />
		<!--  <img src="Creek.jpg" width="100%" /></div>  --->
	</div>
</td>
</tr>
<tr>
	<table>
		<tr>
		<td ><input type="button"  value="Video Setup" name="video" id="video" onclick=window.location.assign("for-iphone/video.asp") /></td>
		<td ><input type="button"  value="Audio Setup" name="audio" id="audio" onclick=window.location.assign("for-iphone/audio.asp") /></td>
		<td ><input type="button"  value="Camera Setup" name="camera" id="camera" onclick=window.location.assign("for-iphone/camera.asp") /></td>
		
		</td>
		</tr>
	</table>

	</tr>
	</table>
</body>
</html>

