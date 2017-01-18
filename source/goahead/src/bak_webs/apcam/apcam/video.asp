<html><head>
<title>Video Stream Setup</title>

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

<script language="JavaScript" type="text/javascript">

function style_display_on(){
    if(window.ActiveXObject) { // IE
        return "block";
    } else if (window.XMLHttpRequest) { // Mozilla, Safari,...
        return "table-row";
    }
}

var http_request = false;

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
	var bakres;
	all_str = str.split("\r");
	//alert(all_str.length);

	for (var i=0; i<all_str.length; i++) {
		var fields_str = new Array();
		fields_str = all_str[i].split(":");
		//alert(fields_str.length);
		//alert(fields_str);
		if(fields_str.length == 2)
		{
			//alert(fields_str);
			//alert("11"+fields_str[0]+"111");
			if(fields_str[0] == "resolution"){
				//alert(fileds_str[0]);
				bakres=fields_str[1];
			}else if(fields_str[0] == "resolution2"){
				document.getElementById("resolution2").value = fields_str[1];
			}else if(fields_str[0] == "codec"){
				document.getElementById("codec").value = fields_str[1];
			}else if(fields_str[0] == "quality"){
				document.getElementById("quality").value = fields_str[1];
			}else if(fields_str[0] == "quality2"){
				document.getElementById("quality2").value = fields_str[1];
			}else if(fields_str[0] == "fps"){
				//alert(fields_str);
				document.getElementById("fps").value = fields_str[1];
			}else{
				alert("PARSE ERROR!");
			}
		
		}

	}
	onChangeCodec();
	document.getElementById("resolution").value = bakres;
}

function pageInit()
{
	makeRequest("/goform/getVideoSettings?resolution=&resolution2=&codec=&quality=&quality2=&fps=", "n/a", Handler);
}

function onChangeCodec()
{
    var src;
    var codectype;
    var i, total;
    var resbak=document.getElementById("resolution").value;
    var prodID=<% getProdID(); %>;

    document.getElementById( "resolution").innerHTML="";    
	codectype = document.getElementById("codec").value;
	if(codectype=="MJPEG")
	{
        //document.VSConfig.resolution.options[0] = new Option("2MP",     "2MP");
        document.VSConfig.resolution.options[0] = new Option("720P",    "720P");
        document.VSConfig.resolution.options[1] = new Option("VGA",     "VGA");
        document.VSConfig.resolution.options[2] = new Option("QVGA",    "QVGA");
        total=2;
    }
    else
    {
        if(prodID=="1a01")
        {
	        document.VSConfig.resolution.options[1] = new Option("VGA",     "VGA");
	        document.VSConfig.resolution.options[2] = new Option("QVGA",    "QVGA");
	        total=2;
        }
        else
        { 
	        document.VSConfig.resolution.options[0] = new Option("720P",     "720P");
	        document.VSConfig.resolution.options[1] = new Option("640x360",  "640x360");
	        document.VSConfig.resolution.options[2] = new Option("320x180",  "320x180");
	        total=2;
        }
    }

    

    for(i=0; i<total; i++)
    {
        if(document.VSConfig.resolution.options[i].value==resbak)
        {
            document.VSConfig.resolution.options[i].selected=true;
            break;
        }
    }

    if(i==total)
        document.VSConfig.resolution.options[0].selected=true;
}


</script>

</head>
<body onLoad="pageInit()">
<table class="body"><tbody><tr><td width="545" height="258">

  <h1 id="QoSTitleStr">Video Setup </h1>
  <hr/>
  
  
<form method="post" name ="VSConfig" action="/goform/updateVideoSettings">

<table id="div_video" name="div_video" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
<tbody>

<tr>
  <td class="title" colspan="2" id="VSConfig_text">video Stream </td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_1">&nbsp;Codec:&nbsp;</label></span></td>
<td align="left"><select id="codec" name="codec" style="width:100px" onChange="onChangeCodec()">
<option value="MJPEG"  >MJPEG</option>
<option value="H264"  >H264</option>
</select>
</td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_2" >&nbsp;Resolution:&nbsp;</label></span></td>
<td width="332" align="left">
<select id="resolution" name="resolution" style="width:100px" >
</select>
</td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_2" >&nbsp;Resolution2:&nbsp;</label></span></td>
<td width="332" align="left">
<select id="resolution2" name="resolution2" style="width:100px" >
<option value="VGA"  >VGA</option>
<option value="QVGA"  >QVGA</option>
</select>
</td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_4" >&nbsp;Quality:&nbsp;</label></span></td>
<td width="332" align="left">
<select id="quality" name="quality" style="width:100px" >
<option value="lowest"  >Lowest</option>
<option value="low"  >Low</option>
<option value="medium"  >Medium</option>
<option value="high"  >High</option>
<option value="highest"  >Highest</option>
</select>
</td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_4" >&nbsp;Quality2:&nbsp;</label></span></td>
<td width="332" align="left">
<select id="quality2" name="quality2" style="width:100px" >
<option value="lowest"  >Lowest</option>
<option value="low"  >Low</option>
<option value="medium"  >Medium</option>
<option value="high"  >High</option>
<option value="highest"  >Highest</option>
</select>
</td>
</tr>

<tr>
<td class="head"><span class="content"><label id="label_3" >&nbsp;Framerate:&nbsp;</label></span></td>
<td align="left">
<select id="fps" name="fps" style="width:100px"  >
<option value="30"  >30</option>
<option value="25"  >25</option>
<option value="20"  >20</option>
<option value="15"  >15</option>
<option value="10"  >10</option>
<option value="7.5"  >7.5</option>
<option value="5"  >5</option>
</select>
<span class="content">[1~30]</span>
</td>
</tr>


<tr>
<td class="head"><span class="content"><label id="label_3" >&nbsp;WDR:&nbsp;</label></span></td>
<td align="left">
<select id="wdr" name="wdr" style="width:100px"  >
<option value=""  ></option>
<option value="0"  >0</option>
<option value="1"  >1</option>
</select>
<span class="content">[0~1]</span>
</td>
</tr>



<tr>
<td class="head"><span class="content"><label id="label_3" >&nbsp;action:&nbsp;</label></span></td>
<td align="left">
<select id="action" name="action" style="width:100px"  >
<option value="non-realtime"  >non-realtime</option>
<option value="realtime"  >realtime</option>

 </select>
</td>
</tr>

</tbody></table>
<br/>

<table border="0" cellpadding="2" cellspacing="1" width="540">
  <tbody><tr align="center">

    <td>
      <input style="width: 120px;" value="Apply" id="videoApply"  type="submit"> &nbsp; &nbsp;
      <input style="width: 120px;" value="Cancel" id="videoCancel" type="reset" onClick="window.location.reload()" >
    </td>
  </tr>
</tbody></table>
</form>
</td>
</tr></tbody></table>
</body></html>
