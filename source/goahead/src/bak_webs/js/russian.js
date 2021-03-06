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
["Ошибка: (Невозможно создать XMLHTTP!)"],
["Не удается подключиться к камере"],
["Необходимо внести информацию"],
["Отменить"],
["Продолжить"],
["Вход выполняется… подождите"],
["Создание учетной записи…… подождите"],
["Регистрация камеры…… подождите"],
["Страница закрывается….подождите"],
["Подключение к вашей беспроводной сети<br>&nbsp;&nbsp;&nbsp; может занять несколько минут"],
["Настройка Интернет страницы……подождите"],
["Пожалуйста, проверьте, что адрес вашей электронной почты указан верно. Он будет использоваться для восстановления вашей учетной записи."]
];

var language1=[
//CamPreview.asp 
["Настройка предварительного просмотра камеры"],
["Предварительный просмотр с камеры"],
["Поздравляем, Вы успешно "],
["подключились к вашей камере"],
["Подключить камеру к Интернету"]
];

var language2=[
// aplist.asp
["Настройка Сети"],
["Подключение к Интернету"],
["Выбор Wi-Fi Сети….."],
["Если Вы хотите подключиться с помощью Ethernet, "],
["нажмите кнопку 'Использовать Ethernet'"],
["Соединение с сетью…………подождите"],
["Использовать Ethernet"],
["Введите сетевой пароль<br> &nbsp;&nbsp;&nbsp; для "],
["Не удается подключиться к Вашему Wi-Fi роутеру! Проверьте Ваш пароль и попробуйте еще раз!"],
["Не удается получить IP адрес от Вашего Wi-Fi роутера! Проверьте Ваш пароль и попробуйте еще раз!"],
["Ваш сетевой пароль должен содержать не менее 8 символов"],
["Недопустимые символы в пароле."],
["Введите 5 или 13 символов для WEP ASCII ключа"],
["Недопустимые символы в WEP hex ключе"],
["или введите 10 или 26 символов в WEP hex ключ"],
["Недопустимый формат WEP ключа"],
["Подключение к опознанным сетям происходит  "],
["автоматически. Если такие сети не обнаружены, "],
["выберите сеть для подключения."],
[""],
["При подключении к беспроводной сети Ваша камера получает слабый сигнал Wi-Fi. При слабом сигнале Wi-Fi качество видео снижается (например выпадение кадров и снижение качества изображения). Пожалуйста, установите камеру ближе к роутеру или точке доступа, чтобы получить более сильный сигнал и улучшить эффективность работы камеры."],
["Пароль"],
["Сканирование беспроводной сети….. Подождите"],
["Введите данные сети"],
["Другая Сеть"],
["Имя"],
["Безопасность"],
["Безопасность\Код"],
["Отсутствует"],
["Введите имя Вашей сети"],
["Пожалуйста, убедитесь, что сетевой кабель подключен правильно, и попробуйте еще раз!"],
["Обновить"],
["Проверьте, подключились ли Вы к камере"],
["Открыть"],
["Поделиться"],
["Не удается подключиться к Интернету!  Проверьте подключение к Интернету и попробуйте снова!"],
["Подключите кабель Ethernet к вашей камере."],
["Показать пароль"],
["Не удается подключиться к роутеру! Пожалуйста, проверьте Вашу сеть и попробуйте снова!"],
["Не удается подключиться к Интернету!  Проверьте подключение к Интернету и попробуйте снова!"],
[""]
];

var language3=[
//CreateAccount.asp
["Настройка создания учетной записи"],
["Подключение к серверу"],
["Создайте Вашу учетную запись"],
["Имя пользователя"],
["Пароль"],
["Подтвердить"],
["Электронная почта"],
["Если у Вас уже есть учетная запись, нажмите на кнопку ' Использовать Существующую '"],
["Использовать существующую"],
["Имя пользователя может содержать только буквы (а-я и А-Я), цифры (0-9), символы (_), (.),(-) и (@)"],
["Имя пользователя не может начинаться с цифры или символов (_), (.),(-) и (@)"],
["Пароль должен содержать не менее 6 символов"],
["Введенные пароли не совпадают. Попробуйте еще раз"],
["Неверный адрес электронной почты"],
["First Name"],
["Last Name"],
[""],
[""],
[""],
[""],
[""],
[""],
[""],
[""],
[""]
];

var language4=[
///login.asp 
["Настройки входа в систему"],
["Подключение к серверу"],
["Введите Вашу учетную запись"],
["Имя пользователя"],
["Пароль"],
["Войти"]
];

var language5=[
//CamRegister.asp
["Настройки регистрации камеры"],
["Соединение с сервером"],
["Информация о камере"],
["Имя камеры"],
["Описание"],
["Готово"]
];

var language6=[
["Отключить настройку Wi-Fi соединения"],
["Установите ; Wi-Fi Setup, перейдите в режим OFF (ВЫКЛ) для завершения установки"],
["Предупреждение: Ваша камера не начнет работать, если Вы не выполните данные действия!"],
["Для продолжения установите \"WiFi-Setup\" в положение OFF (ВЫКЛ)."],
[""],
[""],
[""],
[""],
["Установите ; Wi-Fi Setup, перейдите в режим OFF (ВЫКЛ) для завершения установки"]
];

var language7=[
["другой"],
["вы выбираете"],
["произошла ошибка"],
["Продолжить подключение к незащищенной сети?"],
["хорошо"]
];

var language8=[
["Invalid email address or password. Please try again."],
["The Linksys Smart Wi-Fi Account for {username} has been disabled. "], 
["Your login has not been validated. Please check your email for the validation message."],
["A Linksys Smart Wi-Fi account for {username} does not exist. "], 
["There was an unexpected error communicating with the Linksys Smart Wi-Fi services. Try again later when you have an Internet connection."],
["A Linksys Smart Wi-Fi account for {username} already exists. "], 
["Your account was locked after repeated login attempts failed."],
["Connection can not be established. Network connection is not normal. Server settings are not accurate or the server is down."],
["Connection times out and can not be established.\nNetwork connection is not normal,\nserver settings are not accurate, e.g. invalid IP, \nor the server is down."]
];
