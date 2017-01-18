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
["Falha :( Não é possível criar uma instância XMLHTTP!"],
["Não é possível se conectar à câmera!"],
["Informações obrigatórias."],
["Cancelar"],
["Continuar"],
//number 5
["Conectando-se... aguarde"],
["Criando conta... aguarde"],
["Registrando a câmera... aguarde"],
["Indo para a página final... aguarde"],
// padding
["Conectando à sua rede sem fio. <br>Pode levar alguns minutos."],
["Indo para a página da internet de Configuração... aguarde"],
["Digite o seu e-mail corretamente. Ele será usado para recuperar as informações da conta."]
];

var language1=[
//CamPreview.asp 
["Configuração da demonstração da câmera"],
["Demonstração de câmera"],
["Parabéns, você conectou "],
["a sua câmera"],
["Conectar a câmera à internet"]
];

var language2=[
// aplist.asp
["Configuração de rede"],
["Conectar-se à Internet"],
["Escolha uma rede Wi-Fi..."],
["Se você quiser se conectar via cabo Ethernet"],
["clique no botão \"Usar Ethernet\" abaixo."],
["Conectando à rede... aguarde"],
["Usar Ethernet"],

// no.7  line up
["Digite a senha de rede<br> &nbsp;&nbsp;&nbsp;para "],
//["Não é possível se conectar ao roteador wifi! Verifique a sua senha e tente novamente. Código de erro: "],
["A senha de Wi-Fi digitada está incorreta Tente novamente."],
//["Não é possível obter o endereço ip do roteador wifi! Verifique a sua senha e tente novamente. Código de erro: "],
["A câmera não consegue estabelecer uma conexão. Verifique o seu roteador e tente novamente."],
["A senha de rede deve ter pelo menos 8 caracteres."],
["Caracteres inválidos na senha."],
["Insira 5 ou 13 caracteres para cada chave WEP ASCII."],
["Caracteres inválidos na chave WEP."],
[" ou insira 10 ou 26 caracteres na chave hexadecimal WEP."],
["Formato de chave WEP inválido."],
// the comment.
// no.16
["Redes conhecidas serão adicionadas "],
["automaticamente. Se nenhuma rede conhecida estiver "],
["disponível, selecione uma para adicionar."],
[""],
//for weak wireless 
// no.20
["Sua câmera está recebendo um sinal de W-Fi fraco quando se conecta à rede sem fio. Um sinal de Wi-Fi fraco causará desempenho de vídeo insatisfatório, tais como quadros removidos e deterioração de imagem. Coloque a câmera mais perto do roteador sem fio ou ponto de acesso para obter um sinal mais forte e um melhor desempenho da câmera."],
["Senha"],
["Analisando a rede sem fio... aguarde"],
//for network information
// no.23
["Insira as informações de rede"],
["Outra rede"],
["Nome"],
["Segurança"],
["Segurança/Criptografia"],
["Nenhum"],
["Digite o seu nome de rede"],
//above 29
// no.30
//["Verifique se o cabo Ethernet está conectado corretamente e tente novamente! Código de erro: "],
["Verifique se o cabo Ethernet está conectado corretamente e tente novamente!"],
["Atualizar"],
["Verifique se a câmera está conectada."],
["Abrir"],
["Compartilhar"],
//["Não é possível se conectar à internet! Verifique a sua conexão à internet e tente novamente! Código de erro: "],
["A câmera não consegue se conectar à internet. A sua internet está funcionando?"],
["Conecte o cabo Ethernet à câmera."],
["Exibir senha"],
["Não é possível se conectar ao roteador wifi! Verifique a sua rede e tente novamente!"],
["O servidor da câmera não está disponível no momento."],
// no.40
["Digite a sua senha de rede sem fio. Isso normalmente está no seu roteador."]
];


var language3=[
//CreateAccount.asp
["Criar configuração da conta"],
["Conectar ao servidor"],
["Criar sua conta"],
["Nome de usuário"],
["Senha"],
["Confirmar"],
["E-mail"],
["Se já tiver uma conta, clique no botão \"Usar existente\" abaixo."],
["Usar existente"],
["Somente letras (a-z e A-Z), números (0-9), sublinhados (_), pontos (.), traços (-) e @ são permitidos no nome de usuário."],
// 10
["Sublinhados (_), pontos (.), traços (-) e @ não devem ser o primeiro caractere do nome de usuário."],
["A senha deve ter pelo menos 6 caracteres."], 
["A senha inserida não coincide. Tente novamente."],
["Endereço de e-mail inválido"],
["Nome"],
["Sobrenome"],
["Aqui estão as configurações de conta que você inseriu. Verifique se estão corretas antes de continuar."],
// ["Esse e-mail deverá ser validado com um e-mail que enviamos a você."],  // SEEDONK-1235, change to follows
// ["Se você não acessar a câmera nas próximas 24 horas, será necessário validar a conta clicando no link no e-mail que enviamos a você."],
["Verifique o seu e-mail nas próximas 24 horas. Não será possível acessar a sua conta até que você clique no link de validação que enviamos."],
["Criar sua conta Linksys Smart Wi-Fi"],
["Senha: fraca"],
// 20
["Senha: forte"],
["Tente utilizar uma mistura de 7 ou mais caracteres com letras maiúsculas e minúsculas, números e símbolos para maximizar a força da senha."],
["Criar conta"],
//above 23
["Os e-mails não coincidem."],
["A senha deve ter de 6 a 128 caracteres e conter pelo menos uma letra e um número."]// for linksys
];

var language4=[
///login.asp 
["Configuração de login"],
["Conectar ao servidor"],
["Digite a sua conta"],
["Nome de usuário"],
["Senha"],
["Entrar"]
];

var language5=[
//CamRegister.asp
["Configuração de registro de câmera"],
["Conectar ao servidor"],
["Informações da câmera"],
["Nome de câmera"],
["Descrição"],
["Concluir"]
];

var language6=[
["Desligar o Wi-Fi"],
// double quote
["Coloque o botão na posição DOWN para completa o processo de configuração."],
// pop up Please set the 'Configuração de Wi-Fi' switch to OFF to continue.
["Aviso: a câmera não inicializará até que esteja pronta!"],
// not use 
["Defina o botão \"WiFi Setup\" para OFF para continuar."],
//add for BTHome, only english
["Quase lá..."],
["Mova o botão para a posição CAMERA para completar o processo de configuração."],
["Sua configuração não estará concluída até que você faça isso."],
["Não é possível completar a configuração"],
//ptz
["Pressione o botão Wi-Fi na parte traseira da câmera uma vez para completar o processo de configuração."]
];

var language7=[
["outro..."],
["sua seleção..."],
["ocorreu um erro."],
["Ainda quer conectar-se a uma rede insegura?"],
["OK"]
];


var language8=[
["Invalid email address or password. Please try again."],
["The Linksys Smart Wi-Fi Account for {username} has been disabled. "], 
["Your login has not been validated. Please check your email for the validation message."],
["A Linksys Smart Wi-Fi account for {username} does not exist. "], 
["There was an unexpected error communicating with the Linksys Smart Wi-Fi services. Try again later when you have an Internet connection."],
["A Linksys Smart Wi-Fi account for {username} already exists. "], 
["Your account was locked after repeated login attempts failed."],
["Não foi possível estabelecer uma conexão. A conexão de rede está instável. As configurações do servidor estão imprecisas ou o servidor está inoperante."],
["Não é possível conectar a câmera ao servidor. Verifique se a sua rede está conectada à internet e tente novamente."]
];

