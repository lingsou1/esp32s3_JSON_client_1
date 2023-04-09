/*
接线说明:无

程序说明:烧录进源地的板子,实现了发送请求信息(包含JSON数据)
        这是客户端的程序,实现了向服务器发送包含JSON数据的请求消息
        本实例用于演示esp32S3的json数据通讯。
        操作测试本程序需要使用两台esp32s3开发板。其中一台为服务器端，一台为客户端。
        本程序为客户端程序，功能如下：
        程序功能:
        1:向服务器发送相关的包含有JSON数据的请求信息
        2:可实现部分发送

        本例中客户端需要发送的JSON数据:
        {
          "info": {
            "name": "lingsou",
            "url": "www.bilibili.com",
            "email": "haoze20212021@outlook.com"
          },
          "digital_pin": {
            "digitPin": "digitPinValue",
            "BOOT" : "bootValue"
          },
          "analog_pin": {
            "analogPin": "analogPinValue",
            "capPin": "capPinValue"
          
          }
        }



注意事项:


函数示例:无

作者:灵首

时间:2023_4_5

*/
#include <Arduino.h>
#include <WiFiMulti.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi

//这是需要获取数据的引脚 
#define BOOT 0    //这是按键引脚
#define analogPin 15  //这是模拟引脚
#define capPin 12   //这是电容式触摸引脚
#define digitalPin 8 //这是数字引脚

static int bootValue;
static int analogPinValue;
static int capPinValue;
static int digitalPinValue;


/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}



/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU1029","12345678");
  wifi_multi.addAP("haoze1029","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/*
# brief  在客户端构建一个包含JSON数据的字符串,放入HTTP请求中去
# param   int type :需要构建的JSON数据的内容 1~4是可选参数
# retval  会返回一个JSON数据构成的字符串
*/
String buildData(int type){
  //获取引脚数值
  bootValue = digitalRead(BOOT);
  analogPinValue = analogRead(analogPin);
  capPinValue = analogRead(capPin);
  digitalPinValue = digitalRead(digitalPin);

  if(type == 1){
    //构建动态内存
    StaticJsonDocument<384> doc;

    //建立一个JSON对象
    JsonObject info = doc.createNestedObject("info");
    info["name"] = "lingsou";
    info["url"] = "www.bilibili.com";
    info["email"] = "haoze20212021@outlook.com";

    JsonObject digital_pin = doc.createNestedObject("digital_pin");
    digital_pin["digitalPin"] = String(digitalPinValue);
    digital_pin["BOOT"] = String(bootValue);

    JsonObject analog_pin = doc.createNestedObject("analog_pin");
    analog_pin["analogPin"] = String(analogPinValue);
    analog_pin["capPin"] = String(capPinValue);

    //将构建的内容JSON数据化,并返回
    String output;
    serializeJson(doc, output);

    return output;
  }
  else if(type == 2){
    StaticJsonDocument<128> doc;
    JsonObject info = doc.createNestedObject("info");
    info["name"] = "lingsou";
    info["url"] = "www.bilibili.com";
    info["email"] = "haoze20212021@outlook.com";

    String output;
    serializeJson(doc, output);

    return output;
  }
  else if(type == 3){
    StaticJsonDocument<128> doc;

    JsonObject digital_pin = doc.createNestedObject("digital_pin");
    digital_pin["digitalPin"] = String(digitalPinValue);
    digital_pin["BOOT"] = String(bootValue);

    String output;
    serializeJson(doc, output);

    return output;
  }
  else if(type == 4){
    StaticJsonDocument<128> doc;

    JsonObject analog_pin = doc.createNestedObject("analog_pin");
    analog_pin["analogPin"] = String(analogPinValue);
    analog_pin["capPin"] = String(capPinValue);

    String output;
    serializeJson(doc, output);

    return output;
  }

}



/*
# brief 通过WiFiClient库向指定网址建立连接并发送构建的JSON数据,通过串口输出观察发送的数据
# param  const char* host:需要建立连接的网站的网址
# param   const int httpPort:对应的端口号
# param   int jsonType :这是部分发送数据时填入的参数,不同参数会发送不同的JSON数据,实现节约网络资源的目的(1~4是可选的参数)
# retval  无,但是会通过串口打印一些内容
*/
void wifiClientRequest(const char* host,const int httpPort,int jsonType){
  WiFiClient client;

  //构建需要向服务器发送的JSON数据
  String sendJsonData =  buildData(jsonType);

  //格式很重要 String("GET ") 这个中有一个空格,应该是不能省的,省略会导致HTTP请求发送不出去,很关键的
  String httpRequest =  String("GET /")  + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n\r\n" + sendJsonData;

  //输出连接的网址
  Serial.print("connecting to :");
  Serial.print(host);
  Serial.print("\n");


  //连接网络服务器
  if(client.connect(host,httpPort)){
    //成功后输出success
    Serial.print("success\n");

    //向服务器发送HTTP请求
    client.print(httpRequest);    

    //串口输出HTTP请求信息
    Serial.print("sending request:");   
    Serial.print(httpRequest);
    Serial.print("\n");

    //获取并显示服务器响应状态行
    //只能用单引号
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response is :");
    Serial.print(status_response);
    Serial.print("\n");
  }
  else{
    Serial.print("connect failed!!!\n");
  }

  //结束连接
  client.stop();

  // 并且通过串口输出断开连接信息
  Serial.print("Disconnected from "); 
  Serial.println(host); 
  Serial.print("\n");  
}



void setup() {
  //连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //IO口的设置
  pinMode(BOOT,OUTPUT);
  pinMode(analogPin,OUTPUT);
  pinMode(capPin,OUTPUT);
  pinMode(digitalPin,OUTPUT);

  //wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
}



void loop(){
  //反复发送HTTP请求
  wifiClientRequest("192.168.0.123",80,1);
  delay(1200);
}
