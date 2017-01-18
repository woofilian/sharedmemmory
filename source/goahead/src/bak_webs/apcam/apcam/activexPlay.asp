<html><head>
<title>Video Stream Setup</title>



<%
	aspSmartEncParam(QUERY_STRING);
%>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<style type="text/css">
#wps_progress_bar {
width:250px;
height:15;
margin: 0 auto;
border: 1px solid gray;
}
</style>
<meta http-equiv="content-type" content="text/html; charset=utf-8">

<script>
	function pageInit()
	{
	    var stream;
	    stream="<% getVideoStream(); %>";
		src = "";
		src +="<OBJECT id=seedonkApcam classid=clsid:F1FD587C-197F-4CB7-89A3-8DBC8B654BBA ";
		src +="CODEBASE='/activex/testMFC.cab#version=1,1,0,2' ";
	    if(stream == "2")
	    {
    		src +="width=<% getVideoWidth(2); %> height=<% getVideoHeight(2); %> >"; 
    		src +="<PARAM NAME='ServerIP' VALUE='" + location.hostname + "'>";
    		src +="<PARAM NAME='VideoWidth' VALUE=<% getVideoWidth(2); %>>";
    		src +="<PARAM NAME='VideoHeight' VALUE=<% getVideoHeight(2); %>>";
    		src +="<PARAM NAME='AudioType' VALUE=<% getAudioType(2); %>>";
    		src +="<PARAM NAME='CodecType' VALUE=<% getVideoCodecType(2); %>>";
			src +="<PARAM NAME='StreamNo' VALUE=2>";
	    }
	    else
	    {
    		src +="width=<% getVideoWidth(1); %> height=<% getVideoHeight(1); %> >"; 
    		src +="<PARAM NAME='ServerIP' VALUE='" + location.hostname + "'>";
    		src +="<PARAM NAME='VideoWidth' VALUE=<% getVideoWidth(1); %>>";
    		src +="<PARAM NAME='VideoHeight' VALUE=<% getVideoHeight(1); %>>";
    		src +="<PARAM NAME='AudioType' VALUE=<% getAudioType(1); %>>";
    		src +="<PARAM NAME='CodecType' VALUE=<% getVideoCodecType(1); %>>";
			src +="<PARAM NAME='StreamNo' VALUE=1>";
		}
		src +="</OBJECT>";
		document.getElementById("activex").innerHTML = src;
		
		var codecNo = <% getVideoCodecType(); %>;
		if( codecNo == 1)
		{
			src = "";
			src = "<label>VideoStream: </label>";
			src += "<select name='videoStream' id = 'videoStream' onChange='showActiveX();'>";
			src += "<option value='1'  selected='selected'>Video-I</option>"
			src += "<option value='2' >Video-II</option>"
			src += "</select>";
			src += "<br />";
			document.getElementById("streamlabel").innerHTML = src;
		}
	}
	function showActiveX()
	{
		document.getElementById("activex").innerHTML = "";
		src = "";
		src +="<OBJECT id=seedonkApcam classid=clsid:F1FD587C-197F-4CB7-89A3-8DBC8B654BBA ";
		src +="CODEBASE='/activex/testMFC.cab#version=1,1,0,2' ";
		if(document.getElementById("VideoStream").value == "1")
		{
			src +="width=<% getVideoWidth(1); %> height=<% getVideoHeight(1); %> >"; 
			src +="<PARAM NAME='ServerIP' VALUE='" + location.hostname + "'>";
			src +="<PARAM NAME='VideoWidth' VALUE=<% getVideoWidth(1); %>>";
			src +="<PARAM NAME='VideoHeight' VALUE=<% getVideoHeight(1); %>>";
			src +="<PARAM NAME='AudioType' VALUE=<% getAudioType(); %>>";
			src +="<PARAM NAME='CodecType' VALUE=<% getVideoCodecType(1); %>>";
			src +="<PARAM NAME='StreamNo' VALUE=1>";
		}
		else
		{
			src +="width=<% getVideoWidth(2); %> height=<% getVideoHeight(2); %> >"; 
			src +="<PARAM NAME='ServerIP' VALUE='" + location.hostname + "'>";
			src +="<PARAM NAME='VideoWidth' VALUE=<% getVideoWidth(2); %>>";
			src +="<PARAM NAME='VideoHeight' VALUE=<% getVideoHeight(2); %>>";
			src +="<PARAM NAME='AudioType' VALUE=<% getAudioType(); %>>";
			src +="<PARAM NAME='CodecType' VALUE=<% getVideoCodecType(2); %>>";
			src +="<PARAM NAME='StreamNo' VALUE=2>";
		}
		src +="</OBJECT>";
		document.getElementById("activex").innerHTML = src;
	}	
	function playSpeakerAudio()
	{
		//seedonkApcam.stopPlaybackAudio();
    	seedonkApcam.playAudio();
	}
	function stopSpeakerAudio()
	{
        seedonkApcam.stopAudio();
	}
	function startPlaybackAudio()
	{
		//seedonkApcam.stopAudio();
    	seedonkApcam.startPlaybackAudio();
	}
	function stopPlaybackAudio()
	{
        seedonkApcam.stopPlaybackAudio();
	}
</script>

</HEAD>
<BODY onLoad = "pageInit();">
	<div id = "activex"></div>
	
	<div id = "streamlabel"></div>
	
	Speaker:&nbsp;&nbsp
	<button onClick="playSpeakerAudio();">on</button>&nbsp;&nbsp;&nbsp;
	<button onClick="stopSpeakerAudio();">off</button>
	<br>
	MicroPhone:
	<button onClick="startPlaybackAudio();">on</button>&nbsp;&nbsp;&nbsp;
	<button onClick="stopPlaybackAudio();">off</button>

</BODY>
</HTML>

