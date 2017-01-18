<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  <head>
    <title>MJPG-streamer</title>
    <meta http-equiv="content-type" content="text/html; charset=iso-8859-1" />
    <link rel="stylesheet" href="style.css" type="text/css" />
	
<script type="text/javascript">

	function style_display_on(){
    	if(window.ActiveXObject) { // IE
        	return "block";
    	} else if (window.XMLHttpRequest) { // Mozilla, Safari,...
       	 return "table-row";
    	}
	}

	var http_request = false;
	var image_width = "512px";
	var image_height = "384px";

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
	

	var audioOn=0;
	var audioFormat=0;
	function PLAllData(str)
	{
		var all_str = new Array();
		all_str = str.split("\r");
		//alert(all_str.length);

		for (var i=0; i<all_str.length; i++) {
			var fields_str = new Array();
			fields_str = all_str[i].split(":");
			//alert(all_str[i]);
			if(fields_str.length == 2)
			{
/*
				if(fields_str[0] == "resolution")
				{
					if(fields_str[1] == "1080P")
					{
						image_width = "1920px";
						image_height = "1080px";
					}
					if(fields_str[1] == "2MP")
					{
						image_width = "1600px";
						image_height = "1200px";
					}
					if(fields_str[1] == "720P")
					{
						image_width = "1280px";
						image_height = "720px";
					}
					if(fields_str[1] == "VGA"){
						image_width = "640px";
						image_height = "480px";
					}
					else if(fields_str[1] == "HVGA")
					{
						image_width = "480px";
						image_height = "320px";
					} 
					else if(fields_str[1] == "QVGA")
					{
						image_width = "320px";
						image_height = "240px";
					} 
					else if(fields_str[1] == "QQVGA")
					{
						image_width = "160px";
						image_height = "120px";
					}
*/
				}
				else if(fields_str[0] == "audioCompression")
				{
					if(fields_str[1] == "0")
					{
						audioFormat = 0;
					}
					else if(fields_str[1] == "1")
					{
						audioFormat = 1;
					}
					else {
						audioFormat = 0;
					}
				}
				else if(fields_str[0] == "audioEnable")
				{
					if(fields_str[1] == "YES")
					{
						audioOn = 1;
					}
					else{
						audioOn = 0;
					}
				}
				else
				{
					alert("PARSE ERROR!");
				}
		
			}
			//if(audioOn == 1)
			//	playAudio();
		}
	}
	var player="";
	function playAudio()
	{
		//var player = document.UrawPlay;
		//alert(audioFormat);

		if(player==""){
			/*
			var _app = navigator.appName; 
			alert(_app);
			if (_app == 'Netscape') {  
				var src="<embed code='UrawPlay.class', name='UrawPlay', width='1', height='1', type='application/x-java-applet;version=1.5.0'>";  
				document.getElementById("applet").innerHTML = src;
				alert(src);
			}  
			else if (_app == 'Microsoft Internet Explorer') {  
			    var src="<OBJECT , classid='clsid:8AD9C840-044E-11D1-B3E9-00805F499D93', width='1', height='1', name='UrawPlay', code='UrawPlay.class', codebase='.', type='application/x-java-applet'>";
				document.getElementById("applet").innerHTML = src;
			}
			*/
			var src="<APPLET codebase='.' name='UrawPlay' code='UrawPlay.class' width='800' height='0'>Your browser is completely ignoring the [APPLET] tag!</APPLET>";
			document.getElementById("applet").innerHTML = src;

			player=document.UrawPlay;
		}
        player.play(audioFormat);
	}
	
	function stopAudio()
	{
		//var player = document.UrawPlay;
		if(player!="")
        	player.stop();
	}

	function pageInit()
	{
		makeRequest("/goform/getVideoSettings?action=realtime&resolution=", "n/a", Handler);
		makeRequest("/goform/getAudioSettings?audioCompression=&audioEnable=","n/a", Handler);
		
//		if(audioOn == 1)
	//	{
		//	playAudio();
		//}
	}
	
	


</script>

  </head>
  <body onload="pageInit();">
    <div id="content">
      <img src="/goform/video" width=<% getVideoWidth(); %>px height=<% getVideoHeight(); %>px />  
    </div>
	<br />
	<br />
	AUDIO:
	<button onclick="playAudio();">on</button>
	<button onclick="stopAudio();">off</button>
	<div id="applet">
	</div>
  </body>
</html>
