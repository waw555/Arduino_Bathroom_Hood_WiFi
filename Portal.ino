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
          GP.HTML("<a href=\"/\" style=\"display:inline-block;margin-bottom:12px;padding:8px 14px;background:#444;color:#fff;border-radius:6px;text-decoration:none;\">← Назад</a>");
          GP.OTA_FIRMWARE();
          GP.OTA_FILESYSTEM();
          GP.BREAK();
          GP.LABEL("Прогресс загрузки");
          GP.HTML("<progress id=\"otaProgress\" value=\"0\" max=\"100\" style=\"width:100%;display:none;\"></progress>");
          GP.HTML("<div id=\"otaProgressText\" style=\"display:none;margin-top:8px;\">0%</div>");
          GP.SCRIPT(R"(
            (function() {
              if (window.__otaProgressInit) return;
              window.__otaProgressInit = true;

              function initOtaProgress() {
                var forms = document.querySelectorAll('form');
                forms.forEach(function(form) {
                  if (form.__otaHooked) return;
                  if (!form.querySelector('input[type="file"]')) return;

                  form.__otaHooked = true;
                  form.addEventListener('submit', function(e) {
                    e.preventDefault();

                    var progress = document.getElementById('otaProgress');
                    var text = document.getElementById('otaProgressText');
                    if (!progress || !text) return;

                    progress.value = 0;
                    progress.style.display = 'block';
                    text.textContent = '0%';
                    text.style.display = 'block';

                    var xhr = new XMLHttpRequest();
                    xhr.open(form.method || 'POST', form.action, true);

                    xhr.upload.addEventListener('progress', function(event) {
                      if (!event.lengthComputable) return;
                      var percent = Math.round((event.loaded / event.total) * 100);
                      progress.value = percent;
                      text.textContent = percent + '%';
                    });

                    xhr.addEventListener('load', function() {
                      progress.value = 100;
                      text.textContent = '100%';
                      if (xhr.status >= 200 && xhr.status < 400) {
                        setTimeout(function() { location.reload(); }, 1500);
                      }
                    });

                    xhr.addEventListener('error', function() {
                      text.textContent = 'Ошибка загрузки';
                    });

                    xhr.send(new FormData(form));
                  });
                });
              }

              document.addEventListener('DOMContentLoaded', initOtaProgress);
              initOtaProgress();
            })();
          )");
        }
            GP.UI_END();    // Конец меню
    }
  GP.BUILD_END();
} //void build
