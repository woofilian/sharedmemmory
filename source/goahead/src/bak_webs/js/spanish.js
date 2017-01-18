// revision history:
// 2012-08-07
// support english, simple chinese, traditional chinese, italian, russian, Portuguese
//
// 2012-09-06
// distinguish unsecured AP
//
// 2012-11-12
//
// 2012-12-03 add language type: germany, french, spanish, netherlands

// 2012-12-10 change language order into english, simple chinese, traditional chinese, italian, russian, Portuguese
// German,  Spanish,  Dutch,  French.
// JavaScript Document
var language=[
//for public
["Error :( No se ha podido crear un caso XMLHTTP."],
["No se ha podido conectar con la cámara."],
["Se necesita información."],
["Cancelar"],
["Continuar"],
["Se está iniciando una sesión... espere"],
["Se está creando una cuenta... espere"],
["Se está registrando la cámara... espere"],
["Se está yendo a la página final... espere"],
["Se está conectando a la red inalámbrica. <br>Esto puede tardar unos pocos minutos."],
["Se está yendo a la página de configuración de Internet... espere"],
["Asegúrese de que ha introducido correctamente la dirección de correo electrónico. Esta se utilizará para recuperar información de la cuenta."]
];

var language1=[
//CamPreview.asp 
["Configuración de visualización previa de la cámara"],
["Visualización previa de la cámara"],
["Enhorabuena, se ha conectado "],
["correctamente con la cámara"],
["Conectar la cámara a Internet"]
];

var language2=[
// aplist.asp
["Configuración de red"],
["Conectar a Internet"],
["Elija una red Wi-Fi..."],
["Si desea conectar a través de un cable de Ethernet"],
["haga clic sobre el botón 'Utilizar Ethernet' a continuación."],
["Se está conectando a la red... espere"],
["Utilizar Ethernet"],
["Introduzca la contraseña de la red<br> &nbsp;&nbsp;&nbsp;para "],
//["No ha sido posible conectar a su enrutador wifi. Compruebe la contraseña e inténtelo de nuevo. Código de error: "],
["La contraseña Wi-Fi que ha introducido es incorrecta. Vuelva a intentarlo de nuevo. "],
//["No ha sido posible conseguir la dirección de ip de su enrutador wifi. Compruebe la contraseña e inténtelo de nuevo. Código de error: "],
["La cámara no puede establecer la conexión. Por favor compruebe su router e inténtelo de nuevo. "],
["La contraseña para la red debe contener al menos 8 caracteres."],
["Caracteres no válidos en la contraseña."],
["Introduzca 5 o 13 caracteres para la clave WEPASCII"],
["Caracteres no válidos en la clave WEP."],
[" o introduzca los 10 o 26 caracteres de la clave WEP hexagonal."],
["Formato de clave WED no válido."],
["Se conectará a redes desconocidas "],
["automáticamente. Si no hay redes conocidas "],
["disponibles, deberá seleccionar una a la que conectarse."],
[""],
["Su cámara está recibiendo una señal débil de Wi-Fi cuando se conecta a la red inalámbrica. Una señal Wi-Fi débil provocará un rendimiento bajo del vídeo, como deterioro de la imagen o pérdida de fotogramas. Coloque la cámara más cerca del enrutador inalámbrico o del punto de acceso para conseguir una señal más fuerte y n mejor rendimiento de la cámara."],
["Contraseña"],
["Se está escaneando la red inalámbrico... espere"],
["Introducir la información de la red"],
["Otras redes"],
["Nombre"],
["Seguridad"],
["Seguridad/Cifrado"],
["Ninguno"],
["Introduzca el nombre de red"],
["Verifique que el cable Ethernet está conectado correctamente e inténtelo de nuevo."],
["Actualizar"],
["Compruebe si se ha conectado a la cámara."],
["Abrir"],
["Compartir"],
//["No se puede conectar a Internet. Compruebe la conexión a Internet e inténtelo de nuevo. Código de error: "],
["La cámara no puede conectarse a internet. ¿Funciona su conexión a internet correctamente? "],
["Conecte el cable de Ethernet a su cámara."],
["Mostrar contraseña"],
["No ha sido posible conectar a su enrutador wifi. Compruebe la red e inténtelo de nuevo."],
["El servidor de la cámara no está disponible en este momento."],
[""]
];

var language3=[
//CreateAccount.asp
["Crear configuración de cuenta"],
["Conectar al servidor"],
["Crear una cuenta"],
["Nombre de usuario"],
["Contraseña"],
["Confirmar"],
["Correo electrónico"],
["Si ya tiene una cuenta, haga clic sobre el botón \"Utilizar existente\" a continuación."],
["Utilizar existente"],
["En el nombre de usuario solo se permiten letras (de la a a la z y de la A a la Z), números (del 0 al 9), guiones bajos (_), puntos (.), guiones (-) y @."],
["Guiones bajos (_), puntos (.), guiones (-) y @ no podrán ser el primer carácter del nombre de usuario."],
["La contraseña debe contener al menos 6 caracteres."],
["Las contraseñas introducidas no coinciden. Inténtelo de nuevo."],
["Dirección de correo electrónico no válida."],
["Nombre"],
["Apellido"],
["Aquí puede consultar los datos de cuenta que ha introducido. Por favor verifique que todo es correcto antes de continuar."],
// ["Tendrá que validar esta dirección de correo electrónico con un correo que le enviaremos posteriormente."],
["Asegúrese de consultar su correo electrónico en las próximas 24 horas. No podrá acceder a su cuenta hasta que haya hecho clic en el enlace de validación que le hayamos enviado."],
["Cree su cuenta de Linksys Smart Wi-Fi."],
["Contraseña: Poco segura"],
["Contraseña: Buena"],
["Inténtelo usando una combinación de 7 o más caracteres con mayúsculas o minúsculas, números y símbolos para maximizar la seguridad de la contraseña."],
["Crear cuenta"],
["Los correos electrónicos no coinciden."],
["La contraseña debe tener entre 6 y 128 caracteres y albergar al menos un número y una letra."]
];

var language4=[
///login.asp 
["Configuración de inicio de sesión."],
["Conectar al servidor"],
["Introducir una cuenta"],
["Nombre de usuario"],
["Contraseña"],
["Iniciar sesión"]
];

var language5=[
//CamRegister.asp
["Configuración de registro de la cámara"],
["Conectar al servidor"],
["Información de la cámara"],
["Nombre de la cámara"],
["Descripción"],
["Finalizar"]
];

var language6=[
["Desactivar la configuración de Wi-Fi"],
["Coloque el interruptor en la posición de ABAJO para completar el proceso de configuración."],
["Advertencia: la cámara no se inicializará hasta que haya hecho esto."],
["Coloque el interruptor de \"Configuración de WiFi\" en la posición de APAGADO (OFF) para continuar."],
[""],
[""],
[""],
[""],
["Coloque el interruptor en la posición de ABAJO para completar el proceso de configuración."]
];

var language7=[
["otros..."],
["ha seleccionado..."],
["se ha producido algún error."],
["¿Todavía desea conectar a una red no segura?"],
["bueno"]
];

var language8=[
["Correo electrónico o contraseña invalidos Vuelva a intentarlo."],
["La cuenta de Linksys Smart Wi-Fi  de {username} ha sido desactivada. "],
["No se ha validado su inicio de sesión. Por favor consulte su correo para el mensaje de validación."],
["No existe una cuenta de Linksys Smart Wi-Fi para {username}. "], 
["Se produjo un error inesperado de comunicación con los servicios de Linksys Smart Wi-Fi. Inténtelo de nuevo más tarde cuando tenga conexión a internet."],
["Ya existe una cuenta de Linksys Smart Wi-Fi para {username}. "], 
["Tras repetidos intentos fallidos para iniciar la sesión se ha bloqueado su cuenta. "],
["La cámara no puede conectar con el servidor. Asegúrese de que su red está conectada a internet e inténtelo de nuevo."],
["La cámara no puede conectar con el servidor. Asegúrese de que su red está conectada a internet e inténtelo de nuevo."]
];