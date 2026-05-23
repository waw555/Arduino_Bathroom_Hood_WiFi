// Создаем портал
void build() {
  GP.BUILD_BEGIN(GP_DARK);

  //Название страницы в браузере  
  GP.PAGE_TITLE("WiFi Вытяжка");

  // список имён компонентов на обновление
  GP.UPDATE("tempValue,humiValue,fan,switchFan,ModeOperating,ModeManual"); 
    //Проверка связи с ESP
  GP.ONLINE_CHECK(5000);
  //Перезагрузка страницы
  GP.RELOAD_CLICK("useMQTT,useLocAdd"); 
    
  if (enableAP){ //Если точка доступа включена
          // Форма ввода SSID и пароля в режиме точки доступа
          GP.FORM_BEGIN("/connect");
            M_GRID(
              M_BLOCK_TAB(
                "ПОДКЛЮЧЕНИЕ К WIFI",
                    GP.TEXT("lg", "Точка доступа", data.ssid, "", 20);
                    GP.BREAK();
                    GP.PASS_EYE("ps", "Пароль", data.pass, "", 20);
                    GP.BREAK();
                    GP.HR(GP_GRAY);
                    GP.BREAK();
                    M_BOX(GP.LABEL("Иcпользовать адрес"); GP.CHECK("useLocAdd", data.useLocalAddress););
                    if(useLocalAddress){
                      M_BOX(GP.SPAN("http://", GP_RIGHT); GP.TEXT("la", "Адресс", data.localAddress, "", 20);GP.SPAN(".local",GP_LEFT); );             
                    }
                    GP.BREAK();
                    GP.HR(GP_GRAY);
                    GP.BREAK();
                    M_BOX(GP.LABEL("Иcпользовать MQTT"); GP.CHECK("useMQTT", data.mqttEnable););
                    GP.BREAK();
                    if (mqttEnable){
                      M_BOX(GP.SPAN("Сервер MQTT",GP_LEFT); GP.TEXT("mqtts", "Сервер MQTT", data.mqttserver, "", 40); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Порт MQTT",GP_LEFT); GP.TEXT("mqttp", "Порт MQTT", data.mqttport, "", 6); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Логин MQTT",GP_LEFT); GP.TEXT("mqttlog", "Логин MQTT", data.mqttlogin, "", 20); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Пароль MQTT",GP_LEFT); GP.TEXT("mqttpass", "Пароль MQTT", data.mqttpassword, "", 20); );
                      GP.BREAK();
                    }
              );       
            );
            GP.SUBMIT("Сохранить");
          GP.FORM_END(); 
            //GP.BUTTON("btnSaveConnectSettings","Сохранить","",GP_GREEN,"",0,1);            
  }else{ //Если точка доступа отключена
        
        GP.UI_MENU("Вытяжка", GP_RED);   // начать меню

        // ссылки меню
        GP.UI_LINK("/", "Состояние");
        GP.UI_LINK("/manual", "Управление");
        GP.UI_LINK("/settings", "Настройки");
        GP.UI_LINK("/connect", "Подключение");
        GP.UI_LINK("/ota_update", "Обновление");

        // кастомный контент под меню
        GP.HR(GP_GRAY);
        GP.LABEL("Текущее время");
        GP.BREAK();
        GP.LABEL(ui.getSystemTime().encode()); 

        // начать основное окно
        GP.UI_BODY();

        // далее рандомный интерфейс для теста
        // позволяет "отключить" таблицу при ширине экрана меньше 700px
        GP.GRID_RESPONSIVE(700);  
        //ГЛАВНАЯ СТРАНИЦА
        if (ui.uri("/")) {
          
          M_GRID(
            M_BLOCK_TAB(
              "СОСТОЯНИЕ",        
              M_BOX(GP.LABEL("Температура: "); GP.LABEL_BLOCK("0", "tempValue"););
              GP.BREAK();
              M_BOX(GP.LABEL("Влажность: "); GP.LABEL_BLOCK("0", "humiValue"););
              GP.BREAK();
              M_BOX(GP.LABEL("Вентилятор: "); GP.LED_GREEN("fan"););
              GP.BREAK();
              M_BOX(GP.LABEL("Режим работы: ");
                if(manualMode){
                  GP.SPAN("Ручной", GP_RIGHT, "ModeManual", GP_GREEN);          
                }else{
                  GP.SPAN("Автоматический", GP_RIGHT, "ModeManual", GP_GREEN);           
                };
              );
              GP.BREAK();
              M_BOX(GP.LABEL("Режим измерения: ");
                if(operatingMode){
                  GP.SPAN("Температура", GP_RIGHT, "ModeOperating", GP_GREEN);          
                }else{
                  GP.SPAN("Влажность", GP_RIGHT, "ModeOperating", GP_GREEN);           
                };
              );
            );
          );
          
        } else if (ui.uri("/manual")) {
          
          M_GRID(
            M_BLOCK_TAB(
              "РЕЖИМ РАБОТЫ",
                M_BOX(
                GP_CENTER,
                  GP.SELECT("manMode", "Автоматический, Ручной",manualMode,0,0,1);  );
                GP.BREAK();
                if(manualMode){
                  M_BOX(GP.LABEL("ВЕНТИЛЯТОР"); GP.SWITCH("switchFan", statusFan););
                }
            );
          );
          
        } else if (ui.uri("/settings")) {
          
          M_GRID(
            M_BLOCK_TAB(
              "НАСТРОЙКИ",
              GP.LABEL("Режим измерения");
              GP.HR(GP_GRAY);
              GP.BREAK();        
              M_BOX(
                GP_CENTER,
                GP.SELECT("mode", "Влажность, Температура",data.operatingMode,0,0,1);  );
                GP.BREAK();
              if(!operatingMode){
                GP.LABEL("Влажность");
                GP.HR(GP_GRAY);
                GP.BREAK();
                M_BOX(GP.LABEL("Включить при "); GP.SPINNER("onHumi",data.humiOn,0,100););
                M_BOX(GP.LABEL("Выключить при "); GP.SPINNER("offHumi",data.humiOff,0,100););
                M_BOX(GP.LABEL("Гистерезис"); GP.SPINNER("hysHumi",data.humiHyst,1,20););
                GP.BREAK();
              }else{
                GP.LABEL("Температура");
                GP.HR(GP_GRAY);
                GP.BREAK();
                M_BOX(GP.LABEL("Включить при "); GP.SPINNER("onTemp",data.tempOn,-20,100););
                M_BOX(GP.LABEL("Выключить при "); GP.SPINNER("offTemp",data.tempOff,-20,100););
                M_BOX(GP.LABEL("Гистерезис"); GP.SPINNER("hysTemp",data.tempHyst,1,20););
              };
                GP.BREAK();
                GP.LABEL("Настройки выхода");
                GP.HR(GP_GRAY);
                GP.BREAK();
                M_BOX(GP.LABEL("Инвертировать выход"); GP.SWITCH("OutInvert", invertOut););
                GP.BREAK();
                GP.BUTTON("btnSaveSettings", "Сохранить");
            );
          );
          
        } else if (ui.uri("/connect")) {
          
          // Форма ввода SSID и пароля в режиме точки доступа
          GP.FORM_BEGIN("/connect");
            M_GRID(
              M_BLOCK_TAB(
                "ПОДКЛЮЧЕНИЕ К WIFI",
                    GP.TEXT("lg", "Точка доступа", data.ssid, "", 20);
                    GP.BREAK();
                    GP.PASS_EYE("ps", "Пароль", data.pass, "", 20);
                    GP.BREAK();
                    GP.HR(GP_GRAY);
                    GP.BREAK();
                    M_BOX(GP.LABEL("Иcпользовать адрес"); GP.CHECK("useLocAdd", data.useLocalAddress););
                    if(useLocalAddress){
                      M_BOX(GP.SPAN("http://"); GP.TEXT("la", "Адресс", data.localAddress, "", 20);GP.SPAN(".local"); );             
                    };
                    GP.BREAK();
                    GP.HR(GP_GRAY);
                    GP.BREAK();
                    M_BOX(GP.LABEL("Иcпользовать MQTT"); GP.CHECK("useMQTT", data.mqttEnable););
                    GP.BREAK();
                    if (mqttEnable){
                      M_BOX(GP.SPAN("Сервер MQTT",GP_LEFT); GP.TEXT("mqtts", "Сервер MQTT", data.mqttserver, "", 40); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Порт MQTT",GP_LEFT); GP.TEXT("mqttp", "Порт MQTT", data.mqttport, "", 6); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Логин MQTT",GP_LEFT); GP.TEXT("mqttlog", "Логин MQTT", data.mqttlogin, "", 20); );
                      GP.BREAK();
                      M_BOX(GP.SPAN("Пароль MQTT",GP_LEFT); GP.TEXT("mqttpass", "Пароль MQTT", data.mqttpassword, "", 20); );
                      GP.BREAK();
                    };
              );       
            );
            GP.SUBMIT("Сохранить");      
          GP.FORM_END();
        } else if (ui.uri("/ota_update")) {
          M_GRID(
            M_BLOCK_TAB(
              "ОБНОВЛЕНИЕ",
              GP.BUTTON_LINK("/", "Назад");
              GP.BREAK();
              GP.OTA_FIRMWARE("Обновить прошивку");
              GP.BREAK();
              GP.OTA_FILESYSTEM("Обновить файловую систему");
              GP.BREAK();
              GP.SEND("<div id='otaProgressWrap' style='display:none;margin-top:10px'>"
                      "<div style='margin-bottom:6px'>Прогресс обновления: <span id='otaProgressText'>0%</span></div>"
                      "<div style='width:100%;height:18px;border-radius:9px;background:#2a2d35;overflow:hidden'>"
                      "<div id='otaProgressBar' style='height:100%;width:0%;background:#2ecc71;transition:width .15s ease'></div>"
                      "</div></div>");
              GP.JS_BEGIN();
              GP.SEND("function gpBindOtaProgress(formId){"
                      "var form=getEl(formId);if(!form||form.dataset.gpBound)return;"
                      "form.dataset.gpBound='1';"
                      "form.addEventListener('submit',function(e){"
                        "e.preventDefault();"
                        "var fileInput=form.querySelector('input[type=file]');"
                        "if(!fileInput||!fileInput.files.length)return;"
                        "var data=new FormData(form);"
                        "var xhr=new XMLHttpRequest();"
                        "var wrap=getEl('otaProgressWrap');"
                        "var bar=getEl('otaProgressBar');"
                        "var txt=getEl('otaProgressText');"
                        "wrap.style.display='block';bar.style.width='0%';txt.textContent='0%';"
                        "xhr.upload.onprogress=function(ev){"
                          "if(ev.lengthComputable){"
                            "var p=Math.round((ev.loaded/ev.total)*100);"
                            "bar.style.width=p+'%';txt.textContent=p+'%';"
                          "}"
                        "};"
                        "xhr.onload=function(){location.reload();};"
                        "xhr.onerror=function(){alert('Ошибка загрузки файла');};"
                        "xhr.open('POST',form.action,true);xhr.send(data);"
                      "});"
                    "}"
                    "gpBindOtaProgress('firmware_form');"
                    "gpBindOtaProgress('filesystem_form');"
              );
              GP.JS_END();
            );
          );
        }
            GP.UI_END();    // Конец меню
    }
  GP.BUILD_END();
} //void build
