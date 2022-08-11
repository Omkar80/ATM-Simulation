#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include<HTTPClient.h>
#include <UniversalTelegramBot.h>

char ssid[] = "ssid";     // your network SSID (name)
char password[] = "password"; // your network key

const char* serverName= "https://api.thingspeak.com/update";
String apiKey="apiKey";
WebServer server(80);

// Initialize Telegram BOT
#define BOTtoken "BOTtoken" // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int num2000= 5;
int num1000= 10;
int num500= 10;
int balance= 25000;

int loggedin=0;

int botRequestDelay = 500;
unsigned long lastTimeBotRan;
String chat_id;

int touch_inp(){
  int D0Status=touchRead(32);
  int D1Status=touchRead(33);
  int D2Status=digitalRead(25);
  int D3Status=digitalRead(26);
  int D4Status=touchRead(27);
  int D5Status=touchRead(14);
  int D6Status=touchRead(12);
  int D7Status=touchRead(13);
  int D8Status=touchRead(4);
  int D9Status=touchRead(15);
  
  if(D0Status<50){
    return 0;
  }
  if(D1Status<50){
    return 1;
  }
  if(D2Status==0){
    return 2;
  }
  if(D3Status==0){
    return 3;
  }
  if(D4Status<50){
    return 4;
  }
  if(D5Status<50){
    return 5;
  }
  if(D6Status<50){
    return 6;
  }
  if(D7Status<50){
    return 7;
  }
  if(D8Status<50){
    return 8;
  }
  if(D9Status<50){
    return 9;
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/login") {
        int otp=random(0,100);
        String otp_str=String(otp);
        if(otp<10){
          otp_str="0"+otp_str;
        }
        otp_str="OTP: "+otp_str;
        Serial.println(otp_str);
        bot.sendMessage(chat_id,otp_str,"Markdown");
        bot.sendMessage(chat_id,"Enter the OTP in ESP32","Markdown");
        delay(10000);
        int dig1=touch_inp();
        Serial.println(dig1);
        delay(5000);
        int dig2=touch_inp();
        Serial.println(dig2);
        int otp_enter=dig1*10+dig2;
        Serial.println(otp_enter);

        if(otp_enter==otp){
          bot.sendMessage(chat_id,"The OTP verification successful. You can proceed with your transaction.","Markdown");
          loggedin=1;
          digitalWrite(2,HIGH);
          delay(2000);
          digitalWrite(2,LOW);
        }
        else{
          bot.sendMessage(chat_id,"Sorry, OTP verification failed. Login again to proceed","Markdown");
          loggedin=0;
        }
    }

    if(text=="/withdraw"){
      if(loggedin==0){
        bot.sendMessage(chat_id,"You are not logged in. Please Login to Continue.","Markdown");
      }
      else{
         bot.sendMessage(chat_id,"Enter the amount to withdraw using ESP32","Markdown");
         bot.sendMessage(chat_id,"Available balance: "+ String(balance),"");
         bot.sendMessage(chat_id,"Enter number of digits","");
         delay(5000);
         int num_digits= touch_inp();
         Serial.print("Number of digits");
         Serial.println(num_digits);
         bot.sendMessage(chat_id,"Number of digits: "+String(num_digits),"Markdown");
         int n=num_digits;
         int amt=0;
         while(n>0){
          delay(5000);
          int dig=touch_inp();
          amt= amt + dig*pow(10,n-1);
          n=n-1;
          Serial.println(amt);
         }
         int amount=amt;
         bot.sendMessage(chat_id,"Amount to withdraw: "+String(amt),"Markdown");
         if(amount>balance){
          bot.sendMessage(chat_id,"Not Enough Balance","Markdown");
         }
         else{
           if(amount<500){
            bot.sendMessage(chat_id,"Minimum withdrawal amount is Rs.500","Markdown");
           }
           else{
            int withdraw2000=int(amount/2000);
            if(withdraw2000>num2000){
              withdraw2000=num2000;
            }
            amount=amount-withdraw2000*2000;
            int withdraw1000=int(amount/1000);
            if(withdraw1000>num1000){
              withdraw1000=num1000;
            }
            amount=amount-withdraw1000*1000;
            int withdraw500=int(amount/500);
            if(withdraw500>num500){
              withdraw500=num500;
            }
            amount=amount-withdraw500*500;
            if(amount!=0){
              bot.sendMessage(chat_id,"The entered amount cannot be withdrawn in 500,1000 and 2000 denominations","Markdown");
            }
            else{
              balance=balance-amt;
              num2000=withdraw2000-num2000;
              num1000=withdraw1000-num1000;
              num500=withdraw500-num500;
              String msg="Withdrawed Amount: "+ String(amt)+ "\n";
              msg+= "Number of 2000 Notes withdrawn: "+ String(withdraw2000)+ "\n";
              msg+= "Number of 1000 Notes withdrawn: "+ String(withdraw1000)+ "\n";
              msg+= "Number of 500 Notes withdrawn: "+ String(withdraw500) + "\n";
              msg+="\n";
              msg+="Balance Remaining: "+ String(balance);

              bot.sendMessage(chat_id,msg,"Markdown");
            }
          }
         }
      } 
    }

    if(text=="/balance"){
      if(loggedin==0){
        bot.sendMessage(chat_id,"You are not logged in. Please Login to Continue.","Markdown");
      }
      else{
        bot.sendMessage(chat_id,"Remaining Balance: "+String(balance),"Markdown");
      }
    }
    
    if (text == "/start") {
      String welcome = "ESP32 controller, " + from_name + ".\n";
      welcome += "/login: For Login\n";
      welcome += "/withdraw: For Withdrawal\n";
      welcome += "/balance: For Balance Check\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup() {
  Serial.begin(115200);

  client.setInsecure();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("HTTP server started");

  pinMode(32,INPUT);
  pinMode(33,INPUT);
  pinMode(25,INPUT_PULLUP);
  pinMode(26,INPUT_PULLUP);
  pinMode(27,INPUT);
  pinMode(14,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  pinMode(4,INPUT);
  pinMode(15,INPUT);
  pinMode(2,OUTPUT);
}

void loop() {

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      server.handleClient();
      if(WiFi.status()==WL_CONNECTED)
      {
        HTTPClient http;
        http.begin(serverName);
        String data1="api_key="+apiKey+"&field1="+String(num2000);
        int res1= http.POST(data1);
        String data2="api_key="+apiKey+"&field2="+String(num1000);
        int res2= http.POST(data2);
        String data3="api_key="+apiKey+"&field3="+String(num500);
        int res3= http.POST(data3);
        String data4="api_key="+apiKey+"&field4="+String(balance);
        int res4= http.POST(data4);
        http.end();
      }
    }

    lastTimeBotRan = millis();
  }
}
