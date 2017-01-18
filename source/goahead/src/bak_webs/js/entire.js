
/*       -----      aplist property      ----    */
var http_request = false;
var oldsrc ="";
var old2src ="";

var inalertmode;
var old3src =""; //for alert or confirm.
var oldtips ="";
var oldbutton ="";
var confirmcallback;
var needshowgif;
var needreloadvalues = false;


/*       -----      @End      ----    */

/*       -----------   Not important code ----------     */

function style_display_on(){
	if (window.ActiveXObject){ // IE
		return "block";
	}
	else if (window.XMLHttpRequest){ // Mozilla, Safari,...
		return "table-row";
	}
}

function max(a, b)
{
	if(a>b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

/*      -----------    @End   ---------- */

/*     -------         The same code  -----------   */

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
		alertpage(language[0]);
		return false;
	}
	http_request.onreadystatechange = handler;
	http_request.open('POST', url, true);
	http_request.send(content);
}


function isEmpty(str) {
  if ((str==null) || (str.length==0)) return true
  return false 
}

/*        ----        @End       -----------*/



/*----------------          CamPreview  js code                    -------------------------*/
//0, adnroid, 1 safari
function CAMPreviewinitValue(broswerid){
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
	if(oemtype == "4" || oemtype == "5")
	{
		document.getElementById("singtelcampreview").style.visibility = "visible";
	}
	else
	{
		document.getElementById("singtelcampreview").style.display = "none";
		document.getElementById("singtelcampreview").disabled = true;
	}
		
	if(broswerid == 1)
	{
		previewvideotimer = setTimeout("CAMPreviewLoadVideo()", 2000);
	}
}

function CAMPreviewLoadVideo()
{
	document.getElementById("videostreamid").src = "";
	document.getElementById("videostreamid").src = "/goform/video";
	previewvideotimer = setTimeout("CAMPreviewLoadVideo()", 2000);
}

function stepIntoSL()
{
	clearTimeout(previewvideotimer);
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
	window.location.assign("aplist.asp");
}


/*-----------                 @End                                       ------------------*/



/*-----------                 VerifyMode js code                             --------------------*/
function VERIFYinitValue(){
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
}

function VERIFYUnload()
{
	makeRequest("/goform/apcamMode?action=clear", "n/a", VERIFYUnloadCallback);
}

function VERIFYUnloadCallback()
{
	if (http_request.readyState == 4) 
	{
		if (http_request.status == 200) 
		{
		}
	}
}

function verifyModeNextStep(){
	makeRequest("/goform/apcamMode", "n/a", verifyModeHandlerNextStep);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}


function verifyModeHandlerNextStep(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("UnsetupMode") != null){
				var appendStr = result.substr(result.indexOf("secret")+7);	
				window.location.assign("finish.asp?"+appendStr);
			}
			else{
				// display the web again
				document.getElementById("page").innerHTML = oldsrc;
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
				
				if(oemtype == "4")
				{// singtel pz
					alertpage("Please press the Wi-Fi Setup button on the back of the camera once to complete the setup process")
				}
				else if(oemtype == "3")
				{// normal ptz, outdoor
					alertpage(language6[8]);
				}
				else if(oemtype != "0")
				{// non-bthome
					alertpage(language6[1]);
				}
				else
				{// bthome
					alertpageforverifypage(language6[5]);
					document.getElementById("verifymodetitleid").innerHTML = language6[7];
				}
			}
		} else {
			alertpage(language[1]);
			if(inalertmode)
			{
				old3src = oldsrc;
				needshowgif = false;
			}
			else
			{
				document.getElementById("page").innerHTML = oldsrc;
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		}
	}
}

/*-----------                 @End                                       ------------------*/

/*-----------                 CamRegister js code                             --------------------*/

//0, adnroid, 1 safari
function REGISTERinitValue(broswerid){
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
	
	if(oemtype == "4" || oemtype == "5")
	{
		var maxwidth = document.getElementById("cameraname_width").offsetWidth;
		
		var editlen = document.getElementById("box_cameraname").offsetWidth  - maxwidth - 20;
		document.getElementById("cameraname_width").style.width  = maxwidth +"px";
		document.getElementById("edit_cameraname").style.width  = editlen+"px";
	}
	else
	{
		if(descEnable == "0")
		{
			var box_all= document.getElementById("box_all");                        
			var box_cameradesc=document.getElementById("box_cameradesc");                   
			box_all.removeChild(box_cameradesc); 

			var box_cameraname = document.getElementById("box_cameraname");         
			box_cameraname.style.borderBottom = "0px";                              
			box_cameraname.style.height="30px";

			var maxwidth = document.getElementById("cameraname_width").offsetWidth;
			var editlen = document.getElementById("box_cameraname").offsetWidth  - maxwidth - 20;
			document.getElementById("cameraname_width").style.width  = maxwidth +"px";
			document.getElementById("edit_cameraname").style.width  = editlen+"px";
		}
		else
		{
			var maxwidth = max(document.getElementById("cameraname_width").offsetWidth , document.getElementById("cameradesc_width").offsetWidth );
			
			var editlen = document.getElementById("box_cameraname").offsetWidth  - maxwidth - 20;
			document.getElementById("cameraname_width").style.width  = maxwidth +"px";
			document.getElementById("edit_cameraname").style.width  = editlen+"px";
			document.getElementById("cameradesc_width").offsetWidth = maxwidth;
			document.getElementById("edit_cameradesc").style.width  = editlen+"px";
		}
	}
	
	if(broswerid == 1)
		document.getElementById("videostreamid").src = "/goform/video";
}



function camRegisterNextStep(){
	var camera_name, camera_desc;
	var postdata = "";
	if(!camRegisterCheckValue())
		return;
	camera_name = document.getElementById("cameraname").value;
	postdata += "cameraname=" + encodeURIComponent(camera_name) + "&";
	//if(oemtype != "4")
	if(descEnable == "1")
	{
		camera_desc = document.getElementById("cameradesc").value;
		postdata += "cameradesc=" + encodeURIComponent(camera_desc) + "&";
	}
	
	makeRequest("/goform/camRegister", postdata, camRegisterHandlerNextStep);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}


function camRegisterHandlerNextStep(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("success") != null){
				if(oemtype=="3" || oemtype=="4")
				{
					/* jlu, directly go to finish.asp*/
					var appendStr = result.substr(result.indexOf("secret")+7);	
					window.location.assign("finish.asp?closeap=1&"+appendStr);
				}
				else
				{
					window.location.assign("VerifyMode.asp");
				}
			}
			else{
				document.getElementById("page").innerHTML = oldsrc;
				if(isNaN(result))
				{
					document.getElementById("display").innerHTML=result;
				}
				else
				{
					document.getElementById("display").innerHTML = ErrorMessageParse(parseInt(result));
				}
				document.getElementById("display").innerHTML = http_request.responseText;
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		} else {
			alertpage(language[1]);/* Unable to connect to camera! */
			if(inalertmode)
			{
				old3src = oldsrc;
				needshowgif = false;
			}
			else
			{
				document.getElementById("page").innerHTML = oldsrc;
				document.getElementById("display").innerHTML = "";
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		}
	}
}


function camRegisterCheckValue() {
  var msg = "";
  var targetObj = null;
  var cameraname = document.getElementById("cameraname");
  if (isEmpty(cameraname.value)) {
    msg = language5[3] + msg;
    targetObj = cameraname;
  }
  if (targetObj != null) {
    msg = msg + " " + language[2];
   document.getElementById("display").innerHTML = msg;	
   return false;
  }
  return true;
}


/*-----------                 @End                                       ------------------*/


/*-----------                 login js code                             --------------------*/

function LOGINinitValue(){
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
	
	if(oemtype == "4" || oemtype == "5")
		document.getElementById("display").style.visibility = "visible";
	
	var maxwidth = max(document.getElementById("name_width").offsetWidth , document.getElementById("password_width").offsetWidth );
	
	var editlen = document.getElementById("box_name").offsetWidth  - maxwidth - 20;
	document.getElementById("name_width").style.width  = maxwidth +"px";
	document.getElementById("edit_name").style.width  = editlen+"px";
	document.getElementById("password_width").offsetWidth = maxwidth;
	document.getElementById("edit_password").style.width  = editlen+"px";
}


var g_seedonk_login="";
function ErrorMessageParse(errorIndex)
{
	var errString = language8[errorIndex] + "";
	if(errorIndex==1 || errorIndex==3 || errorIndex==5)
	{
		errString=errString.replace(/{username}/,g_seedonk_login);
	}

	return errString;
}


function loginNextStep(){	
	document.getElementById("display").innerHTML = "";
	var owner_name, owner_password;
	var postdata = "";
	if(!loginCheckValue())
		return;
	g_seedonk_login=document.getElementById("owner_name").value;
	owner_name = document.getElementById("owner_name").value;
	postdata += "owner_name=" + encodeURIComponent(owner_name) + "&";
	owner_password = document.getElementById("owner_password").value;
	postdata += "owner_password=" + encodeURIComponent(owner_password) + "&";
	
	makeRequest("/goform/login", postdata, loginHandlerNextStep);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}


function loginHandlerNextStep(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("success") != null){
				if(oemtype == "4" || oemtype == "5")
					window.location.assign("CamRegister-Singtel.asp");
				else
					window.location.assign("CamRegister.asp");
			}else{
				document.getElementById("page").innerHTML = oldsrc;
				if(result.match("Invalid Sign-In Name and/or Password.") != null)
					document.getElementById("display").innerHTML = "Incorrect log-in details, please try again.";
				else if(isNaN(result))
				{
					document.getElementById("display").innerHTML=result;
				}
				else
				{
					document.getElementById("display").innerHTML = ErrorMessageParse(parseInt(result));
				}
				document.getElementById("display").style.visibility = "visible";
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		} else {
			alertpage(language[1]);
			if(inalertmode)
			{
				old3src = oldsrc;
				needshowgif = false;
			}
			else
			{
				document.getElementById("page").innerHTML = oldsrc;
				document.getElementById("display").innerHTML = "";
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		}
	}
} 


function loginCheckValue() {
  var msg = "";
  var targetObj = null;
  var owner_password = document.getElementById("owner_password");
  var owner_name = document.getElementById("owner_name");
  document.getElementById("display").style.color = "#ff0000";
  if (isEmpty(owner_password.value)) {
	if(isEmpty(msg))
	{
		msg = language4[4];
	}
	else
	{
  		msg = language4[4] + "," + msg;
	}
    targetObj = owner_password;
  }
  if (isEmpty(owner_name.value)) {
	if(isEmpty(msg))
	{
		msg = language4[3];
	}
	else
	{
  		msg = language4[3] + "," + msg;
	}
    targetObj = owner_name;
  }
  
  var targetObj1 = null;
  var emailPat=/^(.+)@(.+)([.][a-zA-Z0-9]{2,})$/;
  if (oemtype == "1" && !emailPat.test(owner_name.value)){
    targetObj1 = owner_name;
  }
  
  if(targetObj != null) { //required fields
    msg = msg + " " + language[2];
    document.getElementById("display").innerHTML = msg;	
	document.getElementById("display").style.visibility = "visible";
    return false;
  }
  else if(targetObj1 != null)
  {
	document.getElementById("display").innerHTML  = language3[13];
	document.getElementById("display").style.visibility = "visible";
	return false;
  }
  return true;
}


/*-----------                 @End                                       ------------------*/









/*-----------                 CreateAccount js code                             --------------------*/
function  ACOUNTinitValue(){
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
	
	var passwordtips = document.getElementById("passwordtips");
	if(passwordtips != null)
  	{
		document.getElementById("passwordtips").style.visibility = "hidden";
		//document.getElementById("passwordtips").style.display = "none";
		prevheight = document.getElementById("passwordtips").style.height;
		document.getElementById("passwordtips").style.height = "0px";
		document.getElementById("passwordtips").disabled = true;
  	}
	
	var maxwidth = max(document.getElementById("login_width").offsetWidth , document.getElementById("passwd_width").offsetWidth );

	if(oemtype == "0" || oemtype == "1")
	{
		maxwidth = max(maxwidth, document.getElementById("firstname_width").offsetWidth);
		maxwidth = max(maxwidth, document.getElementById("lastname_width").offsetWidth);
	}
	
	if(oemtype != "1")
	{
		maxwidth = max(maxwidth, document.getElementById("repasswd_width").offsetWidth);
		maxwidth = max(maxwidth, document.getElementById("email_width").offsetWidth);
	}
	
	if(oemtype == "0")
	{
		maxwidth = max(maxwidth, document.getElementById("reemail_width").offsetWidth);
	}
	
	var editlen = document.getElementById("box_login").offsetWidth  - maxwidth - 20;
	document.getElementById("login_width").offsetWidth  = maxwidth +"px";
	document.getElementById("edit_login").style.width  = editlen+"px";
	document.getElementById("passwd_width").offsetWidth = maxwidth +"px";
	document.getElementById("edit_passwd").style.width  = editlen+"px";

	if(oemtype == "0" || oemtype == "1")
	{
		document.getElementById("firstname_width").offsetWidth = maxwidth;
		document.getElementById("edit_firstname").style.width  = editlen+"px";
		document.getElementById("lastname_width").offsetWidth = maxwidth;
		document.getElementById("edit_lastname").style.width  = editlen+"px";
	}
	
	if(oemtype != "1")
	{
		document.getElementById("repasswd_width").offsetWidth = maxwidth;
		document.getElementById("edit_repasswd").style.width  = editlen+"px";
		document.getElementById("email_width").offsetWidth = maxwidth;
		document.getElementById("edit_email").style.width  = editlen+"px";
	}
	
	if(oemtype == "0")
	{
		document.getElementById("reemail_width").offsetWidth = maxwidth;
		document.getElementById("edit_reemail").style.width  = editlen+"px";
	}
}

function createAccountNextStep(){	
	document.getElementById("display").innerHTML = "";
	var seedonk_login, seedonk_passwd, seedonk_email, seedonk_firstname, seedonk_lastname;
	var postdata ="";
	
	if(oemtype != "1")
	{
/*
		if(!createAccountCheckValueLinksys())
			return;
	}
	else
	{
*/
		if(!createAccountCheckValue())
			return;
	}
	g_seedonk_login = document.getElementById("seedonk_login").value;
	seedonk_login = document.getElementById("seedonk_login").value;
	postdata += "seedonk_login=" + encodeURIComponent(seedonk_login) + "&";
	seedonk_passwd = document.getElementById("seedonk_passwd").value;
	postdata += "seedonk_passwd=" + encodeURIComponent(seedonk_passwd) + "&";

	if(oemtype == "0" || oemtype == "1")
	{
		seedonk_firstname = document.getElementById("seedonk_firstname").value;
		postdata += "seedonk_firstname=" + encodeURIComponent(seedonk_firstname) + "&";
		seedonk_lastname = document.getElementById("seedonk_lastname").value;
		postdata += "seedonk_lastname=" + encodeURIComponent(seedonk_lastname) + "&";
	}
	
	if(oemtype != "1")
	{
		seedonk_email = document.getElementById("seedonk_email").value;
		postdata += "seedonk_email=" + encodeURIComponent(seedonk_email) + "&";
	}

	makeRequest("/goform/createAccount", postdata, createAccountHandlerNextStep);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
	
	if(oemtype == "1")
	{
		document.getElementById("page").style.visibility = "visible";
		document.getElementById("page").style.display = style_display_on();
		document.getElementById("page").disabled = false;
		
		document.getElementById("accountinfopage").style.visibility = "hidden";
		document.getElementById("accountinfopage").style.display = "none";
		document.getElementById("accountinfopage").disabled = true;
	}
}

function createAccountHandlerNextStep(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("success") != null){
				window.location.assign("CamRegister.asp");
			}else{
				document.getElementById("page").innerHTML = oldsrc;
				if(isNaN(result))
				{
					document.getElementById("display").innerHTML = result;
				}
				else
				{
					document.getElementById("display").innerHTML = ErrorMessageParse(parseInt(result));// language8[parseInt(result)];//http_request.responseText;		
				}
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		} else {
			alertpage(language[1]);
			if(inalertmode)
			{
				old3src = oldsrc;
				needshowgif = false;
			}
			else
			{
				document.getElementById("page").innerHTML = oldsrc;
				document.getElementById("display").innerHTML = "";			
				document.getElementById("gif").style.visibility = "hidden";
				document.getElementById("gif").style.display = "none";
				document.getElementById("gif").disabled = true;
			}
		}
	}
}

function initpwfield()
{
	passwordstate = 0;
}

function inputPassword(e)
{	
	var tmppasswordstate;

	//var pwvalue = document.getElementById("seedonk_passwd").value;
	var pwvalue = e.target.value;
	var len = pwvalue.length;
	
	var pwcharlen = false;
	var pwcharwithnum = false;
	var pwcharwithalpha = false;
	var pwcharwithspecial = false;
	for(var i = 0; i < len; i++) {
		if(pwvalue.charAt(i) >= '0' && pwvalue.charAt(i) <= '9')
		{
			pwcharwithnum = true;
		}
		else if((pwvalue.charAt(i) >= 'a' && pwvalue.charAt(i) <= 'z') || (pwvalue.charAt(i) >= 'A' && pwvalue.charAt(i) <= 'Z'))
		{
			pwcharwithalpha = true;
		}
		else 
		{
			pwcharwithspecial = true;
		}
	}
	
	if(len > 6)
		pwcharlen = true;
	
	if(pwcharwithnum && pwcharwithalpha && pwcharwithspecial && pwcharlen)
	{
		tmppasswordstate = 2;
	}
	else
	{
		tmppasswordstate = 1;
	}
	
	if(tmppasswordstate != passwordstate)
	{
		document.getElementById("passwordtips").style.visibility = "visible";
		//document.getElementById("passwordtips").style.display = style_display_on();
		document.getElementById("passwordtips").style.height = prevheight;
		document.getElementById("passwordtips").disabled = false;
	
		if(tmppasswordstate == 1)
		{
			document.getElementById("display").style.color = "#ff0000";
			document.getElementById("display").innerHTML  = language3[19];// weak
		}
		else
		{
			document.getElementById("display").style.color = "#22b14c";
			document.getElementById("display").innerHTML  = language3[20];// strong
		}
		passwordstate = tmppasswordstate;
	}
}




function createAccountCheckValue() {
  var msg = "";
  var targetObj = null;
  var targetObj2 = null;
  var targetObj3 = null;
  var seedonk_firstname = document.getElementById("seedonk_firstname");
  var seedonk_lastname = document.getElementById("seedonk_lastname");
  var seedonk_email = document.getElementById("seedonk_email");
  var seedonk_reemail = document.getElementById("seedonk_reemail");
  var seedonk_repasswd = document.getElementById("seedonk_repasswd");
  var seedonk_passwd = document.getElementById("seedonk_passwd");
  var seedonk_login = document.getElementById("seedonk_login");
  var passwordtips = document.getElementById("passwordtips");
  document.getElementById("display").style.color = "#ff0000";
  
  if(passwordtips != null)
  {
	document.getElementById("passwordtips").style.visibility = "hidden";
	//document.getElementById("passwordtips").style.display = "none";
	document.getElementById("passwordtips").style.height = "0px";
	document.getElementById("passwordtips").disabled = true;
  }
	
  if (seedonk_firstname != null && isEmpty(seedonk_firstname.value)) {
	if(isEmpty(msg))
	{
		msg = language3[14];
	}
	else
	{
  		msg = language3[14] + "," + msg;
	}
    targetObj = seedonk_firstname;
  }
  if (seedonk_lastname != null && isEmpty(seedonk_lastname.value)) {
	if(isEmpty(msg))
	{
		msg = language3[15];
	}
	else
	{
  		msg = language3[15] + "," + msg;
	}
    targetObj = seedonk_lastname;
  }
  
   if (seedonk_reemail != null && isEmpty(seedonk_reemail.value)) {
	if(isEmpty(msg))
	{
		msg = language3[5] + " " + language3[6];
	}
	else
	{
  		msg = language3[5] + " " + language3[6] + "," + msg;
	}
    targetObj = seedonk_reemail;
  }
  
  if (isEmpty(seedonk_email.value)) {
	if(isEmpty(msg))
	{
		msg = language3[6];
	}
	else
	{
  		msg = language3[6] + "," + msg;
	}
    targetObj = seedonk_email;
  }
  if (isEmpty(seedonk_repasswd.value)) {
	if(isEmpty(msg))
	{
		msg = language3[5] + " " + language3[4];
	}
	else
	{		   
    	msg = language3[5] + " " + language3[4] + "," + msg;
	}
	targetObj = seedonk_repasswd;
  }
  if (isEmpty(seedonk_passwd.value)) {
	if(isEmpty(msg))
	{
		msg = language3[4];
	}
	else
	{	
    	msg = language3[4] + "," + msg;
	}
	targetObj = seedonk_passwd;
  }  
  if (isEmpty(seedonk_login.value)) {
	if(isEmpty(msg))
	{
		msg = language3[3];
	}
	else
	{	
    	msg = language3[3] + "," + msg;
	}
	targetObj = seedonk_login;
  } 
  var loginname = seedonk_login.value;
  var loginnamePattern=/(^[a-zA-Z0-9_.\-@]+$)/;    //only letters (a-zA-Z), numbers (0-9), underscores (_), periods (.), dash (-) and @ 				//are allowed.
  if (!loginnamePattern.test(loginname)) { 
    targetObj2 = seedonk_login;
  }
  var loginnamePattern2=/(^[_.\-@]{1}[a-zA-Z0-9_.\-@]*$)/;   //underscores (_), periods (.), dash (-) and @ should not be the first 					 //character of Sign-In Name
  if (loginnamePattern2.test(loginname)) { 
    targetObj3 = seedonk_login;
  }  
  var targetObj4 = null;
  var targetObj5 = null;

  
  if (seedonk_passwd.value.length <6) {
    targetObj4 = seedonk_passwd;
  }
  else if(oemtype == "1")/* for linksys, >6, 1 num, 1cha*/
  {
	var len = seedonk_passwd.value.length;
	var num = false;
	var cha = false;
	for(var i = 0; i < len; i++)
	{
		if(seedonk_passwd.value.charAt(i) >= '0' && seedonk_passwd.value.charAt(i) <= '9')
		{
			num = true;
		}
		else if((seedonk_passwd.value.charAt(i) >= 'a' && seedonk_passwd.value.charAt(i) <= 'z') || (seedonk_passwd.value.charAt(i) >= 'A' && seedonk_passwd.value.charAt(i) <= 'Z'))
		{
			cha = true;
		}
	}
	if(num == false || cha == false)
	{
		targetObj4 = seedonk_passwd;
	}      
  }


  
  if (seedonk_passwd.value != seedonk_repasswd.value) {
    targetObj5 = seedonk_passwd;
  }
  var targetObj6 = null;
  var emailPat=/^(.+)@(.+)([.][a-zA-Z0-9]{2,})$/;
  if (!emailPat.test(seedonk_email.value)) {
    targetObj6 = seedonk_email;
  }
  var targetObj7 = null;
  if (seedonk_reemail != null && seedonk_email.value != seedonk_reemail.value) {
    targetObj7 = seedonk_reemail;
  }
  if (targetObj != null) { //required fields
    msg = msg + " " + language[2];        
    document.getElementById("display").innerHTML = msg;
  } else if (targetObj2 != null) {   //only letters (a-zA-Z), numbers (0-9), underscores (_), periods (.), dash (-) and @ are 	  		    		//allowed.
	document.getElementById("display").innerHTML = language3[9];
  } else if (targetObj3 != null) { //underscores (_), periods (.), dash (-) and @ should not be the first character of Sign-In Name
	 document.getElementById("display").innerHTML = language3[10];
  } else if (targetObj4 != null) { //password required length 
	if(oemtype == "1")
	document.getElementById("display").innerHTML = language3[24];
	else
  	document.getElementById("display").innerHTML = language3[11];
  } else if (targetObj5 != null) { //password nomatch              
    document.getElementById("display").innerHTML  = language3[12];
  } else if (targetObj6 != null) { //email invalid           
    document.getElementById("display").innerHTML  = language3[13];
  } else if (targetObj7 != null){ //different email
  	 document.getElementById("display").innerHTML  = language3[23];//
  } else {
  	return true;
  } 
  return false;
}


function createAccountCheckValueLinksys() {
  var msg = "";
  var targetObj = null;
  var seedonk_firstname = document.getElementById("seedonk_firstname");
  var seedonk_lastname = document.getElementById("seedonk_lastname");
  var seedonk_passwd = document.getElementById("seedonk_passwd");
  var seedonk_login = document.getElementById("seedonk_login");
  document.getElementById("display").style.color = "#ff0000";
  if (isEmpty(seedonk_firstname.value)) {
	if(isEmpty(msg))
	{
		msg = language3[14];
	}
	else
	{
  		msg = language3[14] + "," + msg;
	}
    targetObj = seedonk_firstname;
  }
  if (isEmpty(seedonk_lastname.value)) {
	if(isEmpty(msg))
	{
		msg = language3[15];
	}
	else
	{
  		msg = language3[15] + "," + msg;
	}
    targetObj = seedonk_lastname;
  }
  if (isEmpty(seedonk_passwd.value)) {
	if(isEmpty(msg))
	{
		msg = language3[4];
	}
	else
	{	
    	msg = language3[4] + "," + msg;
	}
	targetObj = seedonk_passwd;
  }  
  if (isEmpty(seedonk_login.value)) {
	if(isEmpty(msg))
	{
		msg = language3[6];
	}
	else
	{	
    	msg = language3[6] + "," + msg;
	}
	targetObj = seedonk_login;
  } 
  
  var targetObj4 = null;
  if (seedonk_passwd.value.length <6 || seedonk_passwd.value.length > 128) {
    targetObj4 = seedonk_passwd;
  }
  else
  {  
	var len = seedonk_passwd.value.length;
	var num = false;
	var cha = false;
	for(var i = 0; i < len; i++)
	{
		if(seedonk_passwd.value.charAt(i) >= '0' && seedonk_passwd.value.charAt(i) <= '9')
		{
			num = true;
		}
		else if((seedonk_passwd.value.charAt(i) >= 'a' && seedonk_passwd.value.charAt(i) <= 'z') || (seedonk_passwd.value.charAt(i) >= 'A' && seedonk_passwd.value.charAt(i) <= 'Z'))
		{
			cha = true;
		}
	}
	if(num == false || cha == false)
	{
		targetObj4 = seedonk_passwd;
	}      
  }

  var targetObj6 = null;
  var emailPat=/^(.+)@(.+)([.][a-zA-Z0-9]{2,})$/;
  if (!emailPat.test(seedonk_login.value)) {
    targetObj6 = seedonk_login;
  }
  
  if (targetObj != null) { //required fields
    msg = msg + " " + language[2];        
    document.getElementById("display").innerHTML = msg;
  } else if (targetObj4 != null) { //password required length              
    document.getElementById("display").innerHTML = language3[24];
  } else if (targetObj6 != null) { //email invalid           
    document.getElementById("display").innerHTML  = language3[13];
  } else {
  	return true;
  } 
  return false;
}

function stepIntoLG()
{
	var src ="";
	src +="<FONT SIZE='3' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;"+ language[8] + "</FONT>";
	document.getElementById("tips").innerHTML = src;
	document.getElementById("page").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
	if(oemtype == "1")
		window.location.assign("login-Linksys.asp");
	else
		window.location.assign("login.asp");
}

function stepIntoAccountInfo()/* linksys */
{
	if(!createAccountCheckValueLinksys())
		return;
		
	document.getElementById("accountinfo_firstname").innerHTML = language3[14] + ":" + document.getElementById("seedonk_firstname").value;
	document.getElementById("accountinfo_lastname").innerHTML = language3[15] + ":" + document.getElementById("seedonk_lastname").value;
	document.getElementById("accountinfo_password").innerHTML = language3[4] + ":" + document.getElementById("seedonk_passwd").value;
	document.getElementById("accountinfo_email").innerHTML = language3[6] + ":" + document.getElementById("seedonk_login").value;
	document.getElementById("accountinfopage").style.visibility = "visible";
	document.getElementById("accountinfopage").style.display = style_display_on();
	document.getElementById("accountinfopage").disabled = false;
	
	document.getElementById("page").style.visibility = "hidden";
	document.getElementById("page").style.display = "none";
	document.getElementById("page").disabled = true;
}

function returnAccountPage()
{
	document.getElementById("page").style.visibility = "visible";
	document.getElementById("page").style.display = style_display_on();
	document.getElementById("page").disabled = false;
	
	document.getElementById("accountinfopage").style.visibility = "hidden";
	document.getElementById("accountinfopage").style.display = "none";
	document.getElementById("accountinfopage").disabled = true;
	
}
/*-----------                 @End                                       ------------------*/







/*-----------                 aplist js code                                       ------------------*/


function initValue(){	
    var src = "";
    src += "<div class='otherFirst'>"+language7[0]+"</div>";
    document.getElementById("divSite").innerHTML = src;	

    src ="";

    if(hasrj45=="1")
    {
        src +="<ul class='individual'>";
        src +="<li id='ethernet'><a href='javascript:useEthernet();'>" + language2[6] + "</a></li>";
        src +="</ul>";
    }

    document.getElementById("button").innerHTML = src; 	

    inittimer = setTimeout("firstGoon()", 1000);
}

function refreshInitValue(){	
    var src = "";
    src += "<div class='otherFirst'>"+language7[0]+"</div>";
    document.getElementById("divSite").innerHTML = src;	
    src ="";

    if(hasrj45 == "1")
    {
        src +="<ul class='individual'>";
        src +="<li id='ethernet'><a href='javascript:useEthernet();'>" + language2[6] + "</a></li>";
        src +="</ul>";
    }

    document.getElementById("button").innerHTML = src; 	

    document.getElementById("gif").style.visibility = "visible";
    document.getElementById("gif").style.display = style_display_on();
    document.getElementById("gif").disabled = false;
    setTimeout("refreshGoon()", 0);
}

function firstGoon()
{
	//makeRequest("/goform/surveySites", "n/a", HandlerSurveySites);
	makeRequest("/goform/getApSitesInfo", "n/a", HandlerApInfo);
}

function refreshGoon()
{
	makeRequest("/goform/surveySites", "n/a", HandlerSurveySites);
	//makeRequest("/goform/getApSitesInfo", "n/a", HandlerApInfo);
}

function HandlerSurveySites()
{
	if (http_request.readyState == 4) {
		inittimer = setTimeout("loop1access()", 2500);
	}
}

function loop1access()
{
	makeRequest("/goform/succeedAccess", "n/a", HandlerAccess);
}

function HandlerAccess()
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("success") != null)
				makeRequest("/goform/getApSitesInfo", "n/a", HandlerApInfo);
			else
				inittimer = setTimeout("loop1access()", 100);
		} else {
			inittimer = setTimeout("loop1access()", 100);
		}
	}
}

function HandlerApInfo() {
	//alert("readyState:"+http_request.readyState+"status:"+http_request.status+"responseText:"+http_request.responseText);
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			ParseApData(http_request.responseText);
			
		} else {
			alertpage(language[1]);
		}
	}
}

function CheckHex(str){
	var len = str.length;
	for (var i = 0; i < str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
				(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
				(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')) {
			continue;
		}else
			return false;
	} 
	return true;
}

function CheckInjection(str){
	var len = str.length;
	for (var i = 0; i < str.length; i++) {
		//if (str.charAt(i) == ';' || str.charAt(i) == ',' ||
		if (str.charAt(i) == '\r' || str.charAt(i) == '\n') {
			return false;
		}else
			continue;
	}
	return true;
}

function CheckWep(){
	var mykey = document.getElementById("password");
	if (mykey.value.length == 5 || mykey.value.length == 13) {
		if (!CheckInjection(mykey.value)) {
			document.getElementById("display").innerHTML = language2[13];
			mykey.focus();
			return false;
		}
		keytype ="1";
	}else if(mykey.value.length == 10 || mykey.value.length == 26) {
		if (!CheckHex(mykey.value)) {
			document.getElementById("display").innerHTML = language2[15];
			mykey.focus();
			return false;
		}
		keytype ="0";
	}else {
		document.getElementById("display").innerHTML = language2[12] + "" + language2[14];
		mykey.focus();
		return false;
	}
	return true;
}

function CheckWpa(){
	var pskvar = document.getElementById("password");
	if (pskvar.value.length < 8) {
		document.getElementById("display").innerHTML = language2[10];
		pskvar.focus();
		return false;
	}if (!CheckInjection(pskvar.value)) {
		document.getElementById("display").innerHTML = language2[11];
		pskvar.focus();
		return false;
	}
	return true;
}

function ShowApList()
{
	var src = "";
 	var i;
	
	for (i = 0; i <= ap_count && wlanNetArray[i]; i++) {
		var sector1, sector2, sector3;
		if(currentbssid == wlanNetArray[i][2]){
			sector1 = "WiFiSelectedLow";
			sector2 = "WiFiSelectedHalf";
			sector3 = "WiFiSelectedFull";
		}else{
			sector1 = "WiFiUnselectedLow";
			sector2 = "WiFiUnselectedHalf";
			sector3 = "WiFiUnselectedFull";
		}
		src += "<div class='centerTable' onClick='setValues("+i+")' id='row"+i+"1' style='";
		if(currentbssid != wlanNetArray[i][2]){
			src += "padding-left:16px;";
		}
		src += "'>";
		src += "<div style='float:left;margin-top:2px;overflow:hidden;width:70%'>";
		if(currentbssid == wlanNetArray[i][2]){
			src += "<div style='float:left;'><img src ='/apcam/apple/img/checkmrk.png'/></div>";
		}
		src += "<div> <FONT SIZE='3' ";
		if(currentbssid == wlanNetArray[i][2]){
			src += "COLOR='#375386'>";
		}else{
			src += "COLOR='#0a0a0a'>";
		}
		src += "" + wlanNetArray[i][1]+"</FONT></div></div>";
		var percent = parseInt(wlanNetArray[i][4],10);
		if(percent<30){
			src += "<div style='float:right;padding-right:5px'> &nbsp; <img src='/apcam/apple/img/" +sector1 + ".png' width=20 height=15 align='center' valign='center'/></div>";
		}else if (percent<65){
			src += "<div style='float:right;padding-right:5px'> &nbsp; <img src='/apcam/apple/img/" +sector2 + ".png' width=20 height=15 align='center' valign='center'/></div>";
		}else{
			src += "<div style='float:right;padding-right:5px'> &nbsp; <img src='/apcam/apple/img/" +sector3 + ".png' width=20 height=15 align='center' valign='center'/></div>";
		}
		if(wlanNetArray[i][3] != "NONE"){
			src += "<div style='float:right;'> &nbsp; <img src='/apcam/apple/img/encrypted.png' width=13 heightn  =15 align='center' valign='center'/></div>";
		}
		src += "</div>";
  	}
  	src += "<div class='other' onClick='netinfo();' id='row"+i+"1'><div style='margin-top:2px;'>"+language7[0]+"</div></div>";
 	document.getElementById("divSite").innerHTML = src;
	
}

function ParseApData(ap_info){
  	ap_count = -1;
  	var ap = new Array();
  	var info = new Array();
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;	
 	ap= ap_info.split("\r\n");
 	for ( i = 0; i < ap.length; i++ ) {
    	info = ap[i].split(":");
    	if (info[0] == "CHANNEL") {
      		ap_count++;
      		if (!wlanNetArray[ap_count])
        		wlanNetArray[ap_count]=new Array();
     			wlanNetArray[ap_count][0] = info[1];
    	}
   		else if (info[0] == "SSID") {
      		wlanNetArray[ap_count][1] = ap[i].substr(5);
   	 	}
    	else if (info[0] == "MAC") {
	  		wlanNetArray[ap_count][2] = ap[i].substr(4);
    	}
    	else if (info[0] == "SECURITY") {
      		wlanNetArray[ap_count][3] = info[1];
    	}
    	else if (info[0] == "SIGNAL") {
      		wlanNetArray[ap_count][4] = info[1];
    	}
	else if (info[0] == "WPS"){
		wlanNetArray[ap_count][5] = info[1];
	}
  	}
	ShowApList();
 	page1button();
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
}

function netinfo(){
	var headsrc, pagesrc;
	firstfocus = true;
	isshowpw = false;
	ssid ="";
	bssid ="";
	channel ="";
	mode = "OPEN";
	enc = "NONE";
	key ="";
	keytype ="";
	wpapsk ="";
	othernet = true;
	iswep = false;
	oldnetpassword = "";
	oldnetname = "";
	oldhead = document.getElementById("bar").innerHTML;
	oldsrc = document.getElementById("page").innerHTML;
//head	
	headsrc = "";
	headsrc += "<div class='headUpper'>" + language2[23] + "</div>";
    headsrc += "<div class='headLower'>" + language2[24] + "</div><br>";
//first group	
	pagesrc = "";
	pagesrc += "<ul class='rounded' id='ultest'><div class='netfirst' id='nameinfo'>";
	pagesrc += "<div onClick='othernetname.focus();' class='nameStyle' id='nameinfotag'>" + language2[25];
	pagesrc += "<input type='text' name='othernetname' id='othernetname' class='nameInput' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
	pagesrc +="</div></ul>";
// an group	
	pagesrc +="<ul class='rounded'><div id ='netinfo' class='netinfo'>";	
	pagesrc += "<div onClick='SelectEncrypt()' class='encryptStyle'>" +language2[27];
	pagesrc += "<div name='othernetsec' id='othernetsec' style='float:right;padding-right:50px;font-size:16px;color:#98a0ac'>" +language2[28] + "</div></div>";
	pagesrc += "</div></ul><br><br><br>"; 
	pagesrc += "<div id = 'display' class='login'></div>";	
	document.getElementById("bar").innerHTML = headsrc;
	document.getElementById("page").innerHTML = pagesrc;

//fix the language shows not properly issue
var ultest_width = document.getElementById("ultest").offsetWidth;
var nameinfo_width = document.getElementById("nameinfo").offsetWidth;
var nameinfotag_width = document.getElementById("nameinfotag").offsetWidth;
var othernetname_width = document.getElementById("othernetname").offsetWidth;
//alert(ultest_width + "-" + nameinfo_width + "-" + nameinfotag_width + "-" + othernetname_width);
	page2button();	
}

function SelectEncrypt(){
	var headsrc, pagesrc;
	oldhead2 = document.getElementById("bar").innerHTML;
	oldsrc2 = document.getElementById("page").innerHTML;
	oldnetname = document.getElementById("othernetname").value;
	if( document.getElementById("password"))
		oldnetpassword = document.getElementById("password").value;
//Security head		
	headsrc = "";
	headsrc += "<div class='headUpper'>" + language2[23] + "</div>";
    headsrc += "<div class='headLower'>" + language2[26] + "</div><br>";
//Security page	
	pagesrc = "";
	pagesrc += "<ul class='rounded'><div class='securityinfo'>";				
	pagesrc += "<div class='securityLine' onClick='setSEvalue(0);'>&nbsp;&nbsp;&nbsp;None</div>";
	pagesrc += "<div class='securityLine' onClick='setSEvalue(1);'>&nbsp;&nbsp;&nbsp;WEP/OPEN</div>";
	pagesrc += "<div class='securityLine' onClick='setSEvalue(2);'>&nbsp;&nbsp;&nbsp;WEP/SHARED</div>";
	pagesrc += "<div class='securityLine' onClick='setSEvalue(3);'>&nbsp;&nbsp;&nbsp;WPAPSK/AES</div>";
	pagesrc += "<div class='securityLine' onClick='setSEvalue(4);'>&nbsp;&nbsp;&nbsp;WPAPSK/TKIP</div>";
	pagesrc += "<div class='securityLine' onClick='setSEvalue(5);'>&nbsp;&nbsp;&nbsp;WPA2PSK/AES</div>";						
	pagesrc += "<div class='security' onClick='setSEvalue(6);'>&nbsp;&nbsp;&nbsp;WPA2PSK/TKIP</div>";				
	pagesrc += "</div></ul><div style='padding:1px;height:25px; width:100%;'></div>";
	document.getElementById("button").innerHTML = ""; 
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
	document.getElementById("bar").innerHTML = headsrc;
	document.getElementById("page").innerHTML = pagesrc;	
}

var isResetSE=true;
function setSEvalue(i){
	var security = "";
	var src = "";
	iswep = false;
	switch(i){
		case 0:
			mode = "OPEN";
			enc = "NONE";
			security = "None";
			break;
		case 1:
			mode = "OPEN";
			enc = "WEP";
			security = "WEP/OPEN";
			iswep = true;
			break;
		case 2:
			mode = "SHARED";
			enc = "WEP";
			security = "WEP/SHARED"
			iswep = true;
			break;
		case 3:
			mode = "WPAPSK";
			enc = "AES";
			security = "WPAPSK/AES";
			break;
		case 4:
			mode = "WPAPSK";
			enc = "TKIP";
			security = "WPAPSK/TKIP";
			break;
		case 5:
			mode = "WPA2PSK";
			enc = "AES";
			security = "WPA2PSK/AES";
			break;
		case 6:
			mode = "WPA2PSK";
			enc = "TKIP";
			security = "WPA2PSK/TKIP";
			break;
		default:
			mode = "OPEN";
			enc = "NONE";
			security = "None";
			break;
	}  
	document.getElementById("bar").innerHTML = oldhead2;
	document.getElementById("page").innerHTML = oldsrc2;
	if(i != 0){
		src += "<div onClick='SelectEncrypt()' class='encryptSecurity'>" +language2[27];
		src += "<div name='othernetsec' id = 'othernetsec' style='float:right;padding-right:15px;font-size:16px;color:#98a0ac;'/>" +security + "</div></div>";
		if(firstfocus)
		{
			src += "<div class='security' id='networkpass'>" + language2[21];
		}
		else
		{
			src += "<div class='securitybottom' id='networkpass'>" + language2[21];
		}
		if(isshowpw)
		{
			src += "<input type='text' name='password' id = 'password' onfocus='otherfocusPW();' class='encryptPassword' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		}
		else
		{
			src += "<input type='password' name='password' id = 'password' onfocus='otherfocusPW();' class='encryptPassword' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		}
		if(firstfocus)
		{
			src += "<div id = 'pwshowdiv' ></div>";
		}
		else
		{
			src += "<div id = 'pwshowdiv' >"
			if(isshowpw)
			{
				src += "<div id='pwshow' onClick='otherwappwshow();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='checkbox' name='pwsbox' id='pwsbox' checked='true' onClick='otherwappwshow();'/>&nbsp;&nbsp;"+ language2[37] +"</div>";
			}
			else
			{
				src += "<div id='pwshow' onClick='otherwappwshow();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='checkbox' name='pwsbox' id='pwsbox' onClick='otherwappwshow();'/>&nbsp;&nbsp;"+ language2[37] +"</div>";
			}
			src += "</div>";
		}
		isResetSEvalue=true;
		document.getElementById("netinfo").innerHTML = src;
		document.getElementById("password").value = oldnetpassword;
	}else{
		src += "<div class='security' onClick='SelectEncrypt()'>" +language2[27];
		src += "<div name='othernetsec' id = 'othernetsec' style='float:right;padding-right:5px;font-size:16px;color:#98a0ac;'/>" +security + "</div></div>";
		document.getElementById("netinfo").innerHTML = src;
	}
	document.getElementById("othernetname").value = oldnetname;
	//document.getElementById("password").blur();
	//document.getElementById("password").focus();
	page2button();
}

function otherfocusPW(){
	var src = "";
	if(firstfocus)
	{
		document.getElementById("networkpass").style.borderBottom = "1px solid #CAD1D9";
		document.getElementById("password").value = "";
		
		src += "<div id='pwshow'  onClick='otherwappwshow();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='checkbox' name='pwsbox' id='pwsbox' onClick='otherwappwshow();'/>&nbsp;&nbsp;"+ language2[37] +"</div>";
		document.getElementById("pwshowdiv").innerHTML =src;
		
		firstfocus = false;
	}
	if(isResetSEvalue)
	{
		isResetSEvalue = false;
		document.getElementById("password").blur();
		document.getElementById("password").focus();
	}
}

function otherwappwshow()
{
	var pwinput = document.getElementById("networkpass");
	var pwdstr = document.getElementById("password").value;
	if(!document.getElementById("pwsbox").checked)
	{
		pwdstr.replace("'", "&#39;");
		pwinput.innerHTML=language2[21] + "<input type='password' name='password' id = 'password' class='encryptPassword' autocorrect='off' autocapitalize='off' autocomplete='off' value='" + pwdstr + "' />";
		isshowpw = false;
	}
	else
	{
		pwdstr.replace("'", "&#39;");
		pwinput.innerHTML=language2[21] + "<input type='text' name='password' id = 'password' class='encryptPassword' autocorrect='off' autocapitalize='off' autocomplete='off' value='" + pwdstr + "' />";
		isshowpw = true;
	}
	
	document.getElementById("password").blur();
	document.getElementById("password").focus();	
}

function Hilite(row){
	var yy = 'row' + row +1
    document.getElementById(yy).style.backgroundColor = "#daecff";
}

function unHilite(row){
    var yy = 'row' + row + 1
    document.getElementById(yy).style.backgroundColor = "#ffffff";
}

/*
    0x10----->doesn't connect to our camera.
    0x11----->doesn't get ip.
    0x12----->fails to resolve “server.seedonk.com”.
    0x13----->fails to ping ip of“server.seedonk.com".
*/

function verifyApcliMode(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var result = http_request.responseText;
			if(result.match("success") != null){
				if(oemtype == "1")
					window.location.assign("CreateAccount-Linksys.asp");
				else if(oemtype == "0")
					window.location.assign("CreateAccount.asp"); //bthome
				else if(oemtype == "4" || oemtype == "5") 
					window.location.assign("login.asp"); //singtel
				else
					window.location.assign("CreateAccount-Belkin.asp");
				window.clearInterval(stoptimer);
			}else{
				if(result.match(language7[2]) != null)
				{
					//response = language2[8] + "0x10";
					response = language2[8];
				}
				else if(result.match("fail to get ip") != null)
				{
					//response = language2[9] + "0x11";
					response = language2[9];
				}
				else if(result.match("fail to ping") != null)
				{
					//response = language2[35] + "0x12";
					response = language2[39];
				}
				else if(result.match("fail to dns") != null)
				{
					//response = language2[35] + "0x13";
					response = language2[35];
				}
				else
				{
					//response = language2[8] + "0x10";
					response = language2[8];
				}
				setTimeout("loop2access()", interval);
			}
		} else {
			response = language[1];
			setTimeout("loop2access()", interval);
		}
	}
}

function loop2access(){
	if(!isstop){	
		makeRequest("/goform/verifyApcliMode", "n/a", verifyApcliMode);
	}else{
		unsetup = false;
		currentbssid = "";
		needreloadvalues = true;
		if(enc == "NONE"){
			document.getElementById("page").innerHTML = old2src;
			if(othernet){
				page2button();
				document.getElementById("othernetname").value = oldnetname; 
			}else{
				ShowApList();
				page1button();
			}
			isstop = false;
			
			if(response.match(language3[4]))
			{
				//alertpage(language2[38] + "0x10");
				alertpage(language2[38]);
			}
			else
			{
				alertpage(response);
			}
		}else{
			document.getElementById("page").innerHTML = old2src;
			if(!firstfocus)
			{
				document.getElementById("pwsbox").checked = isshowpw;
			}
			page2button();
			isstop = false;
			if(enc == "WEP")
			{
				var modes = document.getElementsByName("moderadio");
				for(var i =0; i<2; i++)
				{
					if(modes[i].value == mode)
						modes[i].checked = true;
				}
			}
			document.getElementById("password").value = oldnetpassword;
			if(othernet){
				document.getElementById("othernetname").value = oldnetname; 
			}
			alertpage(response);
		}
	}
}

function reloadaplistpagevalues()
{
	if(enc == "NONE"){
		if(othernet)
			document.getElementById("othernetname").value = oldnetname; 
	}else{
		if(!firstfocus)
		{
			document.getElementById("pwsbox").checked = isshowpw;
		}
		
		if(enc == "WEP")
		{
			var modes = document.getElementsByName("moderadio");
			for(var i =0; i<2; i++)
			{
				if(modes[i].value == mode)
					modes[i].checked = true;
			}
		}
		document.getElementById("password").value = oldnetpassword;
		if(othernet){
			document.getElementById("othernetname").value = oldnetname; 
		}
	}
}

function handlerStop(){
	isstop = true;
	window.clearInterval(stoptimer);
}

function HandlerNextStep(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {		
			setTimeout("loop2access()", interval);
			stoptimer = self.setInterval("handlerStop()",1000*120);
		} else {	
			setTimeout("loop2access()", interval);
			stoptimer = self.setInterval("handlerStop()",1000*120);
		}
	}
}

function nextStep(){
	var postdata = "";
	var src = "";
	
	if(enc != "NONE") // when none, doesn't exist display element.
		document.getElementById("display").innerHTML = "";
		
	if(othernet){
		ssid = document.getElementById("othernetname").value;
		if(ssid.length == 0){
			document.getElementById("display").innerHTML = language2[29];
			document.getElementById("othernetname").focus();
			return;
		}
		for (var i = 0; i <= ap_count && wlanNetArray[i]; i++){
			if(wlanNetArray[i][1] == ssid){
				bssid = wlanNetArray[i][2];
				channel = wlanNetArray[i][0];
			}
		}
	}
	postdata += "apcli_ssid=" + encodeURIComponent(ssid) + "&";
	postdata += "apcli_bssid=" + bssid + "&";
	postdata += "apcli_channel=" + channel + "&";
	postdata += "apcli_enc=" + enc + "&";
	if(!iswep && enc != "NONE"){
		if(!CheckWpa())
			return;
		wpapsk = document.getElementById("password").value;
		oldnetpassword = wpapsk;
		postdata += "apcli_wpapsk=" + encodeURIComponent(wpapsk) + "&";
	}else if(iswep){
		if(!CheckWep())
			return;
		postdata += "apcli_default_key=1&";
		key = document.getElementById("password").value;
		oldnetpassword = key;
		postdata += "apcli_key1=" + encodeURIComponent(key) + "&";
		postdata += "apcli_key1type=" + keytype + "&";
	}
	postdata += "apcli_mode=" + mode + "&";	
	
	makeRequest("/goform/wirelessAplist", postdata, HandlerNextStep);
	old2src = document.getElementById("page").innerHTML;
	page3button();
	document.getElementById("page").innerHTML = src;
}

function setValues(row){
	currow = row;
	iswep = false;
	isshowpw = false;
	othernet = false;
	firstfocus = true;
	oldnetpassword = "";
	selectedrow = row;
	var src = "";
	ssid = wlanNetArray[row][1];
	bssid = wlanNetArray[row][2];
	channel = wlanNetArray[row][0];
	percent = parseInt(wlanNetArray[row][4],10);	
	security = new Array();
	security = wlanNetArray[row][3].split("/");
	oldsrc = document.getElementById("page").innerHTML;
	if(percent < 20){
/*
		var ok = confirm(language2[20]);
		if(!ok)
			return;
*/
		var tipsstr = language2[20] + "";
		if(oemtype == "4" || oemtype == "5")
		{
			var newstr = tipsstr.replace("WiFi", "Wi-Fi")
			tipsstr = newstr
		}
			
		confirmpage(tipsstr, setvalueslowpart);
		return;
	}
	
	if(security.length == 2){
		if(security[0] == "WPA1PSKWPA2PSK"){
			mode = "WPA2PSK";
		}else{
			mode = security[0];
		}if(security[1] == "TKIPAES"){
			enc = "AES";
		}else{
			enc = security[1];
		}
		if(oemtype != "0")
			src += "<br><div class='head'>&nbsp;&nbsp;&nbsp;"+language2[7]+ssid+"</div>";
		else
			src += "<br><div class='head' style='margin-left:10px;margin-right:10px'>"+language2[40]+"</div>";
		src += "<ul class='rounded'><div class='nextRounded' id = pwround>";
		src += "<div class='networkPass' id = 'networkpass' onClick='password.focus();'>"+ language2[21];
		src += "<input type='password' name='password' id = 'password' onfocus='focusPW();' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		src += "<div id = 'pwshowdiv' ></div>";
		src += "</div></ul>";
		src += "<div id = 'display'  class='login'></div>";
		src += "<div style='text-align:center;text-shadow: rgba(255,255,255,.8) 0 1px 0;color: rgb(76, 86, 108);padding: 53px;'></div>";
  		document.getElementById("page").innerHTML = src;
		page2button();
		
	}else if(wlanNetArray[row][3] == "NONE"){
		mode = "OPEN";
		enc = "NONE";
		if(percent >= 20){
/*
			var ok = confirm(language7[3])	
			if(!ok)
				return;
*/
			confirmpage(language7[3], nextStep);
			return;
		}
		else{
			nextStep();
		}
	}else if(wlanNetArray[row][3] == "WEP"){
		enc ="WEP";
		mode = "OPEN";
		if(oemtype != "0")
			src += "<br><div class='head'>&nbsp;&nbsp;&nbsp;"+language2[7]+ssid+"</div>";
		else
			src += "<br><div class='head' style='margin-left:10px;margin-right:10px'>"+language2[40]+"</div>";
		src += "<ul class='rounded'><div class='nextRounded'>";
		src += "<div class='networkPass1' id = 'networkpass' onClick='password.focus()'>"+ language2[21];
		src += "<input type='password' name='password' id = 'password' onfocus='focusPW();' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		src += "<div id = 'pwshowdiv' ></div>";
		src += "<div onClick='radiobutton.focus();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='radio' name='moderadio' id='moderadio' value='OPEN' onClick='wepmode(this.value)' checked='true' />&nbsp;&nbsp;"+ language2[33] +"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type='radio' name='moderadio' id='moderadio' value='SHARED' onClick='wepmode(this.value)'/>&nbsp;&nbsp;"+ language2[34]+"</div></div></ul>";
		src += "<div id = 'display'  class='radio'></div>";
		src += "<div style='text-align:center; text-shadow:rgba(255,255,255,.8) 0 1px 0;color:rgb(76, 86, 108); padding:52px;'></div>";
  		document.getElementById("page").innerHTML = src;
		page2button();
		iswep = true;	
		
		if(bssid == currentbssid)
		{
			var modes = document.getElementsByName("moderadio");
			for(var i =0; i<2; i++)
			{
				if(modes[i].value == currentmode)
				{
					modes[i].checked = true;
					mode = currentmode;
				}
			}
		}
	}	
	
	if(bssid == currentbssid)
	{	
		if(currentencrytype == "WEP")
		{
			document.getElementById("password").value = currentkey;
		}
		else if(currentencrytype == "TKIP" || currentencrytype == "AES")
		{
			document.getElementById("password").value = currentwpapsk;
		}
	}
}

function setvalueslowpart(){	
	var src = "";
	
	if(security.length == 2){
		if(security[0] == "WPA1PSKWPA2PSK"){
			mode = "WPA2PSK";
		}else{
			mode = security[0];
		}if(security[1] == "TKIPAES"){
			enc = "AES";
		}else{
			enc = security[1];
		}
		src += "<br><div class='head'>&nbsp;&nbsp;&nbsp;"+language2[7]+ssid+"</div>";
		src += "<ul class='rounded'><div class='nextRounded' id = pwround>";
		src += "<div class='networkPass' id = 'networkpass' onClick='password.focus();'>"+ language2[21];
		src += "<input type='password' name='password' id = 'password' onfocus='focusPW();' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		src += "<div id = 'pwshowdiv' ></div>";
		src += "</div></ul>";
		src += "<div id = 'display'  class='login'></div>";
		src += "<div style='text-align:center;text-shadow: rgba(255,255,255,.8) 0 1px 0;color: rgb(76, 86, 108);padding: 53px;'></div>";
  		document.getElementById("page").innerHTML = src;
		page2button();
		
	}else if(wlanNetArray[currow][3] == "NONE"){
		mode = "OPEN";
		enc = "NONE";
		if(percent >= 20){
/*
			var ok = confirm(language7[3])	
			if(!ok)
				return;
*/
			confirmpage(language7[3], nextStep);
		}
		else{
			nextStep();
		}
	}else if(wlanNetArray[currow][3] == "WEP"){
		enc ="WEP";
		mode = "OPEN";
		src += "<br><div class='head'>&nbsp;&nbsp;&nbsp;"+language2[7]+ssid+"</div>";
		src += "<ul class='rounded'><div class='nextRounded'>";
		src += "<div class='networkPass1' id = 'networkpass' onClick='password.focus()'>"+ language2[21];
		src += "<input type='password' name='password' id = 'password' onfocus='focusPW();' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' /></div>";
		src += "<div id = 'pwshowdiv' ></div>";
		src += "<div onClick='radiobutton.focus();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='radio' name='moderadio' id='moderadio' value='OPEN' onClick='wepmode(this.value)' checked='true' />&nbsp;&nbsp;"+ language2[33] +"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type='radio' name='moderadio' id='moderadio' value='SHARED' onClick='wepmode(this.value)'/>&nbsp;&nbsp;"+ language2[34]+"</div></div></ul>";
		src += "<div id = 'display'  class='radio'></div>";
		src += "<div style='text-align:center; text-shadow:rgba(255,255,255,.8) 0 1px 0;color:rgb(76, 86, 108); padding:52px;'></div>";
  		document.getElementById("page").innerHTML = src;
		page2button();
		iswep = true;	
		
		if(bssid == currentbssid)
		{
			var modes = document.getElementsByName("moderadio");
			for(var i =0; i<2; i++)
			{
				if(modes[i].value == currentmode)
				{
					modes[i].checked = true;
					mode = currentmode;
				}
			}
		}
	}	
	
	if(bssid == currentbssid)
	{	
		if(currentencrytype == "WEP")
		{
			document.getElementById("password").value = currentkey;
		}
		else if(currentencrytype == "TKIP" || currentencrytype == "AES")
		{
			document.getElementById("password").value = currentwpapsk;
		}
	}
}

function wepmode(wepmode)
{
	mode = wepmode;
}

function goback(){
	if(othernet){
		document.getElementById("bar").innerHTML = oldhead;
	}
	document.getElementById("page").innerHTML = oldsrc;
	if(!unsetup)
		ShowApList();
	page1button();
	iswep = false;
}

function focusPW(){
	var src = "";
	if(firstfocus)
	{
		if(enc == "WEP")
		{
			document.getElementById("password").value = "";
			document.getElementById("pwshowdiv").innerHTML = "<div id='pwshow' class='centerTable' onClick='swappwshow();'><input type='checkbox' name='pwsbox' id='pwsbox' onClick='swappwshow();'/>&nbsp;&nbsp;"+ language2[37] +"</div>";
		}
		else
		{
			document.getElementById("networkpass").style.borderBottom = "1px solid #CAD1D9";
			document.getElementById("password").value = "";
			
			src += "<div id='pwshow' onClick='swappwshow();' style='height:25px;border:0;-webkit-appearance:none; text-align:left; margin-top:10px; margin-bottom:8px; margin-left:5px; font-size:16px; font-weight: bold;color:#101010;'><input type='checkbox' name='pwsbox' id='pwsbox' onClick='swappwshow();'/>&nbsp;&nbsp;"+ language2[37] +"</div>";
			document.getElementById("pwshowdiv").innerHTML =src;
		}
		firstfocus = false;
		document.getElementById("password").focus();
	}
}

function swappwshow()
{
	var pwinput = document.getElementById("networkpass");
	var pwdstr = document.getElementById("password").value;
	
	if(document.getElementById("pwsbox").checked)
	{
		pwdstr = pwdstr.replace("'", "&#39;");	
		pwinput.innerHTML = language2[21] + "<input type='password' name='password' id = 'password' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' value='" + pwdstr + "' />";
		document.getElementById("pwsbox").checked = false;
		isshowpw = true;
	}
	else
	{
		pwdstr = pwdstr.replace("'", "&#39;");		
		pwinput.innerHTML = language2[21] + "<input type='text' name='password' id = 'password' class='networkPassInput' autocorrect='off' autocapitalize='off' autocomplete='off' value='" + pwdstr + "' />";
		document.getElementById("pwsbox").checked = true;
		isshowpw = true;
	}
	document.getElementById("password").focus();
}

function useEthernet(){
	window.clearInterval(inittimer);
	makeRequest("/goform/useEthernet", "n/a", HandlerUseEthernet);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	page4button();
}

function useWPS()
{
	usewps="1";
	makeRequest("/goform/wirelessAplist", "use_wps=1", HandlerNextStep);
	oldsrc = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	page5button();
}

function HandlerUseEthernet(){
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {		
			var result = http_request.responseText;
			if(result.match("success") != null){
				if(oemtype == "1")
					window.location.assign("CreateAccount-Linksys.asp");
				else if(oemtype == "0")
					window.location.assign("CreateAccount.asp"); //for bthome
				else if(oemtype == "4" || oemtype == "5")	
					window.location.assign("login.asp"); //for singtel
				else
					window.location.assign("CreateAccount-Belkin.asp");
			}else{
				document.getElementById("page").innerHTML = oldsrc;
				page1button();
				if(result.match("fail to get ip") != null)
				{
					//alertpage(language2[30] + "0x11");
					alertpage(language2[30]);
				}
				else if(result.match("fail to ping") != null)
				{
					//alertpage(language2[35] + "0x12");
					alertpage(language2[39]);
				}
				else if(result.match("fail to dns") != null)
				{
					//alertpage(language2[35] + "0x13");
					alertpage(language2[35]);
				}
				else if(result.match("no wire") != null)
				{
					alertpage(language2[36]);
				}
			}
		} else {	
			document.getElementById("page").innerHTML = oldsrc;
			page1button();
			alertpage(language[1]);
		}
	}
}

function page1button(){
	var src ="";
	
    if(hasrj45 == "1")
    {
	    src +="<ul class='individual'>";
		src +="<li id='ethernet'><a href='javascript:useEthernet();'>" + language2[6] + "</a></li>";
	    src +="</ul>";
    }

	document.getElementById("button").innerHTML = src; 

/*  //disable the wps function for A300
	src="";
	src +="<ul class='individual'>";
	src +="<li id='ethernet'><a href='javascript:useWPS();'>" + "use wps(pbc)" + "</a></li>";
        src +="</ul>";
	document.getElementById("wpsbutton").innerHTML = src; 
*/
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
}

function page2button(){
	var src ="";
	src += "<ul class='link'>";
	src += "<li id='backtoone'><a href='javascript:goback();'>" + language[3] + "</a></li>";
	src += "<li id='stillgo'><a href='javascript:nextStep();'>" + language[4] + "</a></li>";
	src += "</ul>";	
	document.getElementById("button").innerHTML = src; 
	document.getElementById("wpsbutton").innerHTML = "";
	document.getElementById("gif").style.visibility = "hidden";
	document.getElementById("gif").style.display = "none";
	document.getElementById("gif").disabled = true;
}

function page3button(){
	var src ="";
	src +="<FONT SIZE='3' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;"+ language[9] + "</FONT>";
	src +="<br><br>"
	src += "<FONT SIZE='3' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;"+ language2[5] + "</FONT>";
	document.getElementById("tips").innerHTML = src;
	document.getElementById("button").innerHTML = "";
	document.getElementById("wpsbutton").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}

function page4button(){
	var src ="";
	
	src += "<FONT SIZE='3' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;"+ language2[5] + "</FONT>";
	document.getElementById("tips").innerHTML = src;
	document.getElementById("button").innerHTML = "";
	document.getElementById("wpsbutton").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}

function page5button(){
	var src ="";
	src += "<FONT SIZE='4' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;" + "Instruction" + "</FONT> <br>";
	src += "<FONT SIZE='3' COLOR='#98a0ac'>&nbsp;&nbsp;&nbsp;" + "please press your<br>router's WPS button, <br>then wait for connecting..." + "</FONT>";
	document.getElementById("tips").innerHTML = src;
	document.getElementById("button").innerHTML = "";
	document.getElementById("wpsbutton").innerHTML = "";
	document.getElementById("gif").style.visibility = "visible";
	document.getElementById("gif").style.display = style_display_on();
	document.getElementById("gif").disabled = false;
}


function gobackfromalertpage(){
	document.getElementById("page").innerHTML = old3src;
	document.getElementById("information").innerHTML = "";
	document.getElementById("button").innerHTML = oldbutton; 
	
	if(needshowgif)
	{
		document.getElementById("gif").style.visibility = "visible";
		document.getElementById("gif").style.display = style_display_on();
		document.getElementById("gif").disabled = false;
	}
	inalertmode = false;
	
	if(needreloadvalues)
	{
		reloadaplistpagevalues();
		needreloadvalues = false;
	}
}

function gobackfromalertpageforverifypage(){
	document.getElementById("page").innerHTML = old3src;
	document.getElementById("information").innerHTML = "";
	document.getElementById("button").innerHTML = oldbutton; 
	document.getElementById("verifymodetitleid").innerHTML = language6[4];
	
	if(needshowgif)
	{
		document.getElementById("gif").style.visibility = "visible";
		document.getElementById("gif").style.display = style_display_on();
		document.getElementById("gif").disabled = false;
	}
	inalertmode = false;
	
	if(needreloadvalues)
	{
		reloadaplistpagevalues();
		needreloadvalues = false;
	}
}

function alertpageforverifypage(msg){
	inalertmode = true;
	old3src = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	
	//for msg
	var src ="";
	src += "<FONT SIZE='3' COLOR='#40566C'>&nbsp;&nbsp;&nbsp;"+ msg + "</FONT>";
	document.getElementById("information").innerHTML = src;
	
	needshowgif = false;
	if(document.getElementById("gif").style.visibility == "visible")
	{
		needshowgif = true;
		document.getElementById("gif").style.visibility = "hidden";
		document.getElementById("gif").style.display = "none";
		document.getElementById("gif").disabled = true;
	}
	
	//for button.
	src ="";
	src +="<ul class='individual'>";
	src +="<li id='ethernet'><a href='javascript:gobackfromalertpageforverifypage();'>" + language7[4] + "</a></li>";
	src +="</ul>";
	oldbutton = document.getElementById("button").innerHTML;
	document.getElementById("button").innerHTML = src; 
}

function alertpage(msg){
	inalertmode = true;
	old3src = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	
	//for msg
	var src ="";
	src += "<FONT SIZE='3' COLOR='#40566C'>&nbsp;&nbsp;&nbsp;"+ msg + "</FONT>";
	document.getElementById("information").innerHTML = src;
	
	needshowgif = false;
	if(document.getElementById("gif").style.visibility == "visible")
	{
		needshowgif = true;
		document.getElementById("gif").style.visibility = "hidden";
		document.getElementById("gif").style.display = "none";
		document.getElementById("gif").disabled = true;
	}
	
	//for button.
	src ="";
	src +="<ul class='individual'>";
	src +="<li id='ethernet'><a href='javascript:gobackfromalertpage();'>" + language7[4] + "</a></li>";
	src +="</ul>";
	oldbutton = document.getElementById("button").innerHTML;
	document.getElementById("button").innerHTML = src; 
}

function gobackfromconfirmpage(okorno){
	document.getElementById("information").innerHTML = "";
	if(okorno)
	{

		confirmcallback();
	}
	else
	{
		document.getElementById("page").innerHTML = old3src;
		document.getElementById("button").innerHTML = oldbutton; 
	}
}

function confirmpage(msg, callback){
	confirmcallback = callback;
	
	old3src = document.getElementById("page").innerHTML;
	document.getElementById("page").innerHTML = "";
	
	//for msg
	var src ="";
	src += "<FONT SIZE='3' COLOR='#40566C'>&nbsp;&nbsp;&nbsp;"+ msg + "</FONT>";
	document.getElementById("information").innerHTML = src;
	
	//for button.
	src ="";
	src += "<ul class='link'>";
	src += "<li id='backtoone'><a href='javascript:gobackfromconfirmpage(false);'>" + language[3] + "</a></li>";
	src += "<li id='stillgo'><a href='javascript:gobackfromconfirmpage(true);'>" + language7[4] + "</a></li>";
	src += "</ul>";	
	oldbutton = document.getElementById("button").innerHTML;
	document.getElementById("button").innerHTML = src; 
}




/*-----------                 @End                                       ------------------*/
