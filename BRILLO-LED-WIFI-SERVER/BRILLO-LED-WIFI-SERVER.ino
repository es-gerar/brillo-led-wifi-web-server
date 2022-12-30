

/* https://www.GERAR.es

Programa para controlar un LED, Servo o Salida PWM con servidor web para ESP8266 y ESP32

*/


#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


const char* ssid = "Tigre Banana";
const char* password = "nlrdpdlna!NLRDPDLNA!";

const int led_pin = LED_BUILTIN;
String slider_value = "0";

const int frequency = 5000;
const int led_channel = 0;
const int resolution = 8;

const char* input_parameter = "value";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Brightness Control Web Server</title>
    <style>
      html {font-family: Times New Roman; display: inline-block; text-align: center;}
      h2 {font-size: 2.3rem;}
      p {font-size: 1.9rem;}
      body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
      .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #38c0ff  ;
        outline: none; -webkit-transition: .2s; transition: opacity .2s;}
      .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background:#01070a; cursor: pointer;}
      .slider::-moz-range-thumb { width: 35px; height: 35px; background: #01070a; cursor: pointer; } 
    </style>
  </head>
  <body>
    <h2>GERAR ESP32 WEB SERVER <br>Control de Brillo, Servo, Etc.</h2>

    <p id="demo"></p>
    <p><span id="textslider_value">%SLIDERVALUE%</span></p>
    <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="255" value="%SLIDERVALUE%" step="1" class="slider"></p>
    <script>

    function updateSliderPWM(element) {
      var slider_value = document.getElementById("pwmSlider").value;
      document.getElementById("textslider_value").innerHTML = slider_value;
      console.log(slider_value);
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/slider?value="+slider_value, true);
      xhr.send();
    }

    setInterval(myTimer, 50);

    function myTimer() {
      const d = new Date();
      /* Imprime la hora*/
      document.getElementById("demo").innerHTML = d.toLocaleTimeString();

      /*Coge el valor del Slider*/
      var slider_value = document.getElementById("pwmSlider").value;
      document.getElementById("textslider_value").innerHTML = slider_value;

      /*Carga la variable en memoria con la última medición del Slider*/
      var lastSliderValue = localStorage.getItem("brillo");

      /*Si la última medición es diferente, guardará, notificará y enviará mensaje*/
      if (slider_value!= lastSliderValue){
        localStorage.setItem("brillo", slider_value);
        console.log("*");

        /*ENVIA MENSAJE*/
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/slider?value="+slider_value, true);
        xhr.send();

      }

      /*console.log(slider_value);*/

    }

  </script>

  </body>
</html>
)rawliteral";

String processor(const String& var){
  if (var == "SLIDERVALUE"){
    return slider_value;
  }
  return String();
}

void setup(){
  Serial.begin(115200);

  //Manejo de PWM en ESP32 ////////////////////////////
  ledcSetup(led_channel, frequency, resolution);
  ledcAttachPin(led_pin, led_channel);
  ledcWrite(led_channel, slider_value.toInt());

//////////////////////////////// CONEXIÓN RED WI-FI //
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println(WiFi.localIP());

  // WEBSERVER REQUESTS //////////////////////////////////////
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      slider_value = message;
      int valorDelSlider=slider_value.toInt();
      int valorDelSliderAlReves=map (valorDelSlider,0,255,255,0);
      ledcWrite(led_channel, valorDelSliderAlReves);
    }
    else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });
  
  server.begin();
}
  
void loop() {
  
}
