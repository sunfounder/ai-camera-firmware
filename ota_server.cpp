#include "ota_server.hpp"

WebServer server(80);
char version[10] = "";

const char* serverIndex ="<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>ESP32-Cam OTA</title><style>html{ width: 100%; height: 100%; margin: 0;} body{ width: 100%; height: 100%; margin: 0; background-color: rgb(236 240 245); display: flex; justify-content: center; align-items: center;} .card{ width: 90%; max-width: 400px; height: 260px; background-color: white; position: fixed; border-radius: 10px; display: flex; justify-content: space-around; align-items: center; flex-direction: column; box-shadow: 4px 4px 20px 4px rgba(30, 30, 50, .1);} .title{ font-weight: bold; font-size: 20px; margin: 10px 0;} .ota-form{ width: 90%; display: flex; flex-direction: column; justify-content: center; align-items: center;} .select{ width: 100%; height: 40px; background-color: #f9fbfb; border: 1px solid #ced1d1; border-radius: 4px; display: flex; justify-content: center; align-items: center; margin: 10px 0;} .upload-button input[type='submit']{ opacity: 0; width: 100%; height: 100%;} .upload-button{ width: 100%; height: 40px; background-color: #019cda; color: #fafafa; border-radius: 4px; display: flex; justify-content: center; align-items: center; margin: 10px 0; position: relative;} .upload-label{ position: absolute;} .hint-box{ width: 100%; color: #000; display: flex; justify-content: space-between;} .progess-box{ width: 100%; color: #000; display: flex; justify-content: space-between;} .upload-label{ width: 100%; text-align: center;} .dark{ color: #000;} .light{ color: #fafafa;} .progress-container{ width: 100%; height: 16px; position: relative; border-radius: 4px; background-color: #f9fbfb; border: 1px solid #ced1d1; margin: 10px 0;} #progress-bar{ width: 0%; height: 100%; background-color: #019cda; top: 0; left: 0; border-radius: 4px; display: flex; align-items: center;} </style><script>let file=null; function handleSelect(){ const elem=document.createElement('input'); elem.accept='.bin'; elem.type='file'; elem.addEventListener('change', ()=>{ if (elem.files.length==1){ file=elem.files[0]; let fileName=file.name; select=document.querySelector('.hint'); select.innerText=fileName;}}); elem.click();} function updateProgressBar(progress){ let progressBar=document.getElementById('progress-bar'); let progressText=document.getElementById('progress-text'); progress=Math.min(100, Math.max(0, progress)); progress=Math.round(progress); progressBar.style.width=progress + '%'; progressText.innerText=progress + '%';} function updateVersion(){ let xhr=new XMLHttpRequest(); xhr.open('GET', '/version'); xhr.send(); xhr.onreadystatechange=function (){ if (xhr.readyState==4 && xhr.status==200){ let version=xhr.responseText; let versionElem=document.querySelector('#version'); versionElem.innerText='Current version: ' + version;}}} function handleUpdate(){ let formData=new FormData(); formData.append('update', file); let xhr=new XMLHttpRequest(); xhr.open('POST', '/update'); xhr.upload.addEventListener('progress', (e)=>{ if (e.lengthComputable){ let progress=e.loaded / e.total * 100; updateProgressBar(progress); if (progress==100){ updateSuccess();}}}); xhr.send(formData); let updateButton=document.querySelector('.upload-button'); updateButton.innerText='Updating...'; updateButton.style.backgroundColor='#dfa814';} function updateSuccess(){ let updateButton=document.querySelector('.upload-button'); updateButton.innerText='Success'; updateButton.style.backgroundColor='#01da5f';} window.onload=function (){ updateVersion();}</script></head><body><div class='card'><div class='title'>SunFounder ESP32-Cam OTA</div><div id='version'>Current version: </div><form class='ota-form'><div class='select' onclick='handleSelect()'>选择文件</div><div class='hint-box'><div class='hint'>Select bin file.</div><div id='progress-text'></div></div><div class='progress-container'><div id='progress-bar'></div></div><div class='upload-button' onclick='handleUpdate()'>Update</div></form></div></body></html>";

void otaBegin(const char* _version) {
  strcpy(version, _version);
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/version", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", version);
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

void otaLoop() {
  server.handleClient();
}