void action(GyverPortal& p) {
  if (p.form("/connect")) {      // кнопка нажата
    p.copyStr("lg", data.ssid);  // Копируем данные из поля lg в переменную data.ssid
    p.copyStr("ps", data.pass); // Копируем данные из поля ps в переменную data.pass
    if(useLocalAddress){
      p.copyStr("la", data.localAddress); // Копируем данные из поля la в переменную data.localAddress
    };
    if(mqttEnable){
      p.copyStr("mqtts", data.mqttserver);  // копируем сервер mqtt
      p.copyStr("mqttp", data.mqttport);  // копируем сервер mqtt
      p.copyStr("mqttlog", data.mqttlogin);  // копируем сервер mqtt
      p.copyStr("mqttpass", data.mqttpassword);  // копируем сервер mqtt
    }
    EEPROM.put(0, data);              // сохраняем
    if (EEPROM.commit()) {  //Записываем
      DEBUGLN("EEPROM successfully committed"); //Запись прошла успешно
    } else {
      DEBUGLN("ERROR! EEPROM commit failed"); //Запись не прошла
    }
    delay(100);
    DEBUGLN("SYSTEM RESTART");
    ESP.restart(); // Перезагрузка микроконтроллера
  }

  //Если изменились настройки
  if (ui.click()) {
    if (ui.clickInt("mode", data.operatingMode)) {
      operatingMode = data.operatingMode;
      DEBUG("Type: ");
      DEBUGLN(data.operatingMode);
    }
    
    if (ui.clickInt("onHumi", data.humiOn)) {
      DEBUG("onHumi: ");
      DEBUGLN(data.humiOn);
    }

    if (ui.clickInt("offHumi", data.humiOff)) {
      DEBUG("offHumi: ");
      DEBUGLN(data.humiOff);
    }

    if (ui.clickInt("hysHumi", data.humiHyst)) {
      DEBUG("hysHumi: ");
      DEBUGLN(data.humiHyst);
    }

    if (ui.clickInt("onTemp", data.tempOn)) {
      DEBUG("onTemp: ");
      DEBUGLN(data.tempOn);
    }

    if (ui.clickInt("offTemp", data.tempOff)) {
      DEBUG("offTemp: ");
      DEBUGLN(data.tempOff);
    }

    if (ui.clickInt("hysTemp", data.tempHyst)) {
      DEBUG("hysTemp: ");
      DEBUGLN(data.tempHyst);
    }

    if (ui.clickBool("useMQTT", data.mqttEnable)) {
      mqttEnable = data.mqttEnable;      
      DEBUG("useMQTT: ");
      DEBUGLN(data.mqttEnable);
    }

    if (ui.clickBool("useLocAdd", data.useLocalAddress)) {
      useLocalAddress = data.useLocalAddress;      
      DEBUG("useLocalAddress: ");
      DEBUGLN(data.useLocalAddress);
    }

    if (ui.clickBool("manMode", manualMode)) {
      DEBUG("Manual Mode = : ");
      DEBUGLN(manualMode);
    }

    if (ui.clickBool("OutInvert", data.invertOut)) {
      invertOut = data.invertOut;
      DEBUG("Invert Out = ");
      DEBUGLN(invertOut);
    }

    if (ui.clickBool("switchFan", statusFan)) {
      DEBUG("statusFan: ");
      DEBUGLN(statusFan);        
    }
    
    if (ui.click("btnSaveSettings")){
      DEBUGLN("Save Settings");
        EEPROM.put(0, data);              // сохраняем
        if (EEPROM.commit()) {
          DEBUGLN("EEPROM successfully committed");
        } else {
          DEBUGLN("ERROR! EEPROM commit failed");
        }     
    }
  }
  // Если было обновление
  if (ui.update()) {
    if (ui.update("fan")) ui.answer(statusFan);
    ui.updateInt("tempValue", currentTemperature);
    ui.updateInt("humiValue", currentHumidity);
  }
}