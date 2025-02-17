#include "settings_server.hpp"

WebServer server(80);
char version[10] = "";
int _apChannel = 1;

const char* serverIndex = "<!doctype html><html lang=en><head><meta charset=UTF-8><meta http-equiv=X-UA-Compatible content=\"IE=edge\"><meta name=viewport content=\"width=device-width,initial-scale=1\"><title>ESP32-Cam OTA</title><style>html{width:100%;height:100%;margin:0}body{width:100%;height:100%;margin:0;background-color:rgb(236 240 245);display:flex;justify-content:center;align-items:center}#version{margin-top:10px}.card{width:90%;max-width:400px;height:auto;background-color:#fff;position:fixed;border-radius:10px;display:flex;justify-content:space-around;align-items:center;flex-direction:column;box-shadow:4px 4px 20px 4px rgba(30,30,50,.1)}.otaSetting,.wifiSetting{width:90%}.otaTitle,.wifiTitle{font-size:20px}.title{font-weight:700;font-size:22px;margin:14px 0}.divider{width:90%;height:1px;background-color:#000;margin:10px}.ota-form{height:auto;display:flex;flex-direction:column;justify-content:center;align-items:center}.apChannelSelect{width:100%;display:flex;justify-content:space-between;align-items:center}.apChannelBox{height:40px;display:flex;justify-content:space-between;align-items:center}#selectMenu{width:54px;height:24px;background-color:#f9fbfb;border:1px solid #ced1d1;border-radius:4px;margin-right:30px}#selectMenu option{text-align:center}.confirmed{height:30px;background-color:#019cda;border-radius:4px;display:flex;justify-content:center;align-items:center;color:#fff;border:none;cursor:pointer}.select{width:100%;height:40px;background-color:#f9fbfb;border:1px solid #ced1d1;border-radius:4px;display:flex;justify-content:center;align-items:center;margin:10px 0}.upload-button input[type=submit]{opacity:0;width:100%;height:100%}.upload-button{width:100%;height:40px;background-color:#019cda;color:#fafafa;border-radius:4px;display:flex;justify-content:center;align-items:center;margin:10px 0;position:relative}.upload-label{position:absolute}.hint-box{width:100%;color:#000;display:flex;justify-content:space-between}.progess-box{width:100%;color:#000;display:flex;justify-content:space-between}.upload-label{width:100%;text-align:center}.dark{color:#000}.light{color:#fafafa}.progress-container{width:100%;height:16px;position:relative;border-radius:4px;background-color:#f9fbfb;border:1px solid #ced1d1;margin:10px 0}#progress-bar{width:0%;height:100%;background-color:#019cda;top:0;left:0;border-radius:4px;display:flex;align-items:center}</style><script>let file=null;function handleSelect(){const e=document.createElement(\"input\");e.accept=\".bin\",e.type=\"file\",e.addEventListener(\"change\",(()=>{if(1==e.files.length){file=e.files[0];let t=file.name;select=document.querySelector(\".hint\"),select.innerText=t}})),e.click()}function updateProgressBar(e){let t=document.getElementById(\"progress-bar\"),n=document.getElementById(\"progress-text\");e=Math.min(100,Math.max(0,e)),e=Math.round(e),t.style.width=e+\"%\",n.innerText=e+\"%\"}function updateSettings(){let e=new XMLHttpRequest;e.open(\"GET\",\"/settings\"),e.send(),e.onreadystatechange=function(){if(4==e.readyState&&200==e.status){let t=JSON.parse(e.responseText),n=document.querySelector(\"#version\");document.querySelector(\"#apChannel\");n.innerText=\"Current version: \"+t.version;document.getElementById(\"selectMenu\").value=t.apChannel}}}function handleUpdate(){let e=new FormData;e.append(\"update\",file);let t=new XMLHttpRequest;t.open(\"POST\",\"/update\"),t.upload.addEventListener(\"progress\",(e=>{if(e.lengthComputable){let t=e.loaded/e.total*100;updateProgressBar(t),100==t&&updateSuccess()}})),t.send(e);let n=document.querySelector(\".upload-button\");n.innerText=\"Updating...\",n.style.backgroundColor=\"#dfa814\"}function updateSuccess(){let e=document.querySelector(\".upload-button\");e.innerText=\"Success\",e.style.backgroundColor=\"#01da5f\"}function handleConfirm(){const e=document.getElementById(\"selectMenu\").value,t=new XMLHttpRequest;t.open(\"POST\",`/set-apChannel?apChannel=${e}`,!0),t.setRequestHeader(\"Content-Type\",\"application/json\"),t.send(JSON.stringify({apChannel:e})),t.onreadystatechange=function(){4===t.readyState&&200===t.status&&alert(\"AP channel set successfully. Reset the device to take effect.\")}}window.onload=function(){updateSettings()}</script></head><body><div class=card><div class=title>SunFounder ESP32-Cam</div><div class=wifiSetting><div class=wifiTitle>WiFi</div><div class=apChannelSelect><div id=apChannel>AP channel: </div><div class=apChannelBox><select id=selectMenu><option value=1>1</option><option value=2>2</option><option value=3>3</option><option value=4>4</option><option value=5>5</option><option value=6>6</option><option value=7>7</option><option value=8>8</option><option value=9>9</option><option value=10>10</option><option value=11>11</option></select><button type=button class=confirmed onclick=handleConfirm()>Confirm</button></div></div></div><div class=divider></div><div class=otaSetting><div class=otaTitle>OTA</div><div id=version>Current version:</div><form class=ota-form><div class=select onclick=handleSelect()>Choose File</div><div class=hint-box><div class=hint>Select bin file.</div><div id=progress-text></div></div><div class=progress-container><div id=progress-bar></div></div><div class=upload-button onclick=handleUpdate()>Update</div></form></div></div></body></html>";

void settingsBegin(const char* _version, int chn) {
  _apChannel = chn;
  strcpy(version, _version);
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/settings", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"version\":\"" + String(version) + "\",\"apChannel\":" + String(_apChannel) + "}");
  });
  server.on("/version", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", version);
  });
  server.on("/apChannel", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", String(_apChannel));
  });
  server.on("/set-apChannel", HTTP_POST, []() {
    if (server.hasArg("apChannel")) {
      _apChannel = server.arg("apChannel").toInt();
      Preferences pref;
      pref.begin("config");
      pref.putInt("apChannel", _apChannel);
      pref.end();
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", "OK");
    } else {
      server.sendHeader("Connection", "close");
      server.send(400, "text/plain", "Bad Request");
    }
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        delay(1000);
        ESP.restart();
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void settingsLoop() {
  server.handleClient();
}