#include<ESP8266WiFi.h> 
#include<SoftwareSerial.h>
#include<PubSubClient.h>
#include <ArduinoJson.h>
 
WiFiClient wifiClient;    
PubSubClient mqttClient(wifiClient);
SoftwareSerial uart(D6,D5); //软件模拟串口RX=d6,TX=d5

const char* ssid="boombap";              //wifi名
const char* password="pppppppp";   //wifi密码
const char* mqttServer = "118.178.90.222";  //mqtt服务器

const String clientID = "ZigbeeProject";  //id设置，用MAC地址确保没同id
//const char* mqttUserName = "EndDevice1&a1iSI5TrX9Q";
//const char* mqttPassword = "5D3231A681CEF855672FC2199E53D47347A5A890";// MQTT服务端连接用户名密码

const int subQoS = 1;     // 客户端订阅主题时使用的QoS级别（截止2020-10-07，仅支持QoS = 1，不支持QoS = 2）
String topicPub1String = "/MyZigbeeProject/EndDevice1";   //发布内容的主题名字
String topicPub2String = "/MyZigbeeProject/EndDevice2";   //发布内容的主题名字
String topicPub3String = "/MyZigbeeProject/Router3";   //发布内容的主题名字
String topicPub4String = "/MyZigbeeProject/IndustryData";   //发布内容的主题名字
String topicSub1String = "/MyZigbeeProject/Control";   //订阅的主题名字

void setup() {
  Serial.begin(9600);
  uart.begin(9600);
  //Serial.listen();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);//默认关灯，成功连接服务器才开灯
  
  connectWifi(); //wifi连接函数

  mqttClient.setServer(mqttServer,1883);     //设置服务器，端口号
  mqttClient.setCallback(receiveCallback);  //回调函数，当收到订阅来的消息后，调用该函数

  connectMQTTServer();  //连接服务器，订阅主题

}

void loop() {
  int i = 0;
  String serialData, pubData;
  
  if(mqttClient.connected()) //确定是否成功连接服务器
  {
    digitalWrite(LED_BUILTIN, LOW);
    if(Serial.available()>0)
    {
      serialData = Serial.readString();
      uart.print(serialData);
      //pub1MQTTmsg(serialData);
      if(serialData.length() == 7)
      {
        
        pubData = ChangeToJson(serialData);
        switch(serialData[0])
        {
            case '1':pub1MQTTmsg(pubData);break;
            case '2':pub2MQTTmsg(pubData);break;
            case '3':pub3MQTTmsg(pubData);break;
            default:break;
        }
      }else if(serialData.length() == 11)
      {
          pubData = ChangeToJson2(serialData);
          pub4MQTTmsg(pubData);
      }
      delay(20);
    }
    
    mqttClient.loop();//保持心跳
  }else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    connectMQTTServer(); 
  }
  
}

String ChangeToJson(String str)
{
  String ret; 
  StaticJsonDocument<64> doc;

  doc["temperture"] = (str[1]-'0')*10 + (str[2]-'0');
  doc["humidity"] = (str[3]-'0')*10 + (str[4]-'0');
  doc["islight"] = (str[6]-'0');
  doc["isfire"] = (str[5]-'0');
  serializeJson(doc, ret);
  return ret;
}

String ChangeToJson2(String str)
{
  String ret; 
  StaticJsonDocument<96> doc;
  char temp[10];
  
  sprintf(temp,"%d.%d", (str[1]-'0')*10 + (str[2]-'0'), (str[3]-'0'));
  doc["temperture"] = temp;
  sprintf(temp,"%d.%d", (str[4]-'0')*10 + (str[5]-'0'), (str[6]-'0'));
  doc["humidity"] = temp;
  doc["co2"] = (str[7]-'0')*1000 + (str[8]-'0')*100 + (str[9]-'0')*10 + (str[10]-'0');
  serializeJson(doc, ret);
  return ret;
}

void connectWifi()
{
  WiFi.begin(ssid,password);      //连接wifi
  while(WiFi.status()!=WL_CONNECTED)  //确定成功连接wifi
  {
    delay(1000);
    uart.print("");
  }
  uart.println("");
  uart.println("WiFi connected");
  uart.println("");
}

void connectMQTTServer()
{
  String willString = "testwill";
  char willTopic[willString.length()+1];
  strcpy(willTopic,willString.c_str());

  //连接MQTT服务器(id,null,null,遗嘱主题，遗嘱质量，遗嘱保留，遗嘱信息，清除会话）
  if(mqttClient.connect(clientID.c_str(),NULL,NULL))
  {
    uart.println("MQTT Server Connected.");
    uart.print("Server Address: ");
    uart.println(mqttServer);
    uart.print("ClientId:");
    uart.println(clientID);

    //订阅指定主题
    subscribeTopic();
  }else
  {
    uart.print("MQTT Server Connect Failed. Client State:");
    uart.println(mqttClient.state());
    delay(3000);  
  }

}

void subscribeTopic()
{
  char subTopic1[topicSub1String.length()+1];  
  strcpy(subTopic1,topicSub1String.c_str());   //转成字符串
  
  if(mqttClient.subscribe(subTopic1,subQoS))  //订阅
  {
    uart.println("Subscribe Topic:");
    uart.println(subTopic1);
  }else {
    uart.print("Subscribe Fail...");
  }   
}

void receiveCallback(char* topic, byte* payload,unsigned int length) //topic为主题，payload为内容，length为内容长度
{
  uart.print("Message Received [");
  uart.print(topic);
  uart.print("] ");

  uart.println("");
  uart.print("Message Length(Bytes) ");
  uart.println(length);

  String sMessage = (char *)payload;        //将消息转换为String
  sMessage = sMessage.substring(0, length); //取合法长度 避免提取到旧消息
  uart.print("Message:");
  uart.println(sMessage);
  
  Serial.print(sMessage);
}

void pub1MQTTmsg(String messageString)
{
  char publishTopic[topicPub1String.length()+1];
  strcpy(publishTopic,topicPub1String.c_str());

  //发布的内容
  char publishMsg[messageString.length()+1];
  strcpy(publishMsg,messageString.c_str());

  if(mqttClient.publish(publishTopic,publishMsg))//发布
  {
    uart.print("Publish Topic:");uart.println(topicPub1String);
    uart.print("Publish message:");uart.println(publishMsg);    
  } else {
    uart.println("Message Publish Failed."); 
  }
}

void pub2MQTTmsg(String messageString)
{
  char publishTopic[topicPub2String.length()+1];
  strcpy(publishTopic,topicPub2String.c_str());

  //发布的内容
  char publishMsg[messageString.length()+1];
  strcpy(publishMsg,messageString.c_str());

  if(mqttClient.publish(publishTopic,publishMsg))//发布
  {
    uart.print("Publish Topic:");uart.println(topicPub2String);
    uart.print("Publish message:");uart.println(publishMsg);    
  } else {
    uart.println("Message Publish Failed."); 
  }
}

void pub3MQTTmsg(String messageString)
{
  char publishTopic[topicPub3String.length()+1];
  strcpy(publishTopic,topicPub3String.c_str());

  //发布的内容
  char publishMsg[messageString.length()+1];
  strcpy(publishMsg,messageString.c_str());

  if(mqttClient.publish(publishTopic,publishMsg))//发布
  {
    uart.print("Publish Topic:");uart.println(topicPub3String);
    uart.print("Publish message:");uart.println(publishMsg);    
  } else {
    uart.println("Message Publish Failed."); 
  }
}

void pub4MQTTmsg(String messageString)
{
  char publishTopic[topicPub4String.length()+1];
  strcpy(publishTopic,topicPub4String.c_str());

  //发布的内容
  char publishMsg[messageString.length()+1];
  strcpy(publishMsg,messageString.c_str());

  if(mqttClient.publish(publishTopic,publishMsg))//发布
  {
    uart.print("Publish Topic:");uart.println(topicPub4String);
    uart.print("Publish message:");uart.println(publishMsg);    
  } else {
    uart.println("Message Publish Failed."); 
  }
}
