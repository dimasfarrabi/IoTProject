 /*
  * File  : DFRobot_SIM7000_MQTT.ino
  * Power : SIM7000 needs 7-12V DC power supply
  * Brief : This example verify MQTT send verification
  *         With initialization completed, we connect to iot.dfrobot.com.cn
  *         Then send data to a topic
  *         Thus we finished the MQTT send verification
  * Note  : If you use Mega please connect PIN8 PIN10 and set PIN_RX = 10
  *         If you select another IOT, please confirm that the IOT is in the whitelist of your NB card
  *         The maximum length of data transmitted each time is 50
  */

#include <Wire.h>
#include <DFRobot_SIM7000.h>

//Login website (http://iot.dfrobot.com.cn) to register an account ,fill the following information based on your account
#define serverIP        "103.163.139.48"
#define IOT_CLIENT      ""
#define IOT_USERNAME    ""
#define IOT_KEY         ""
#define IOT_TOPIC       "data"

#define PIN_TX          11
#define PIN_RX          10

SoftwareSerial          mySerial(PIN_RX,PIN_TX);
DFRobot_SIM7000         sim7000;

void setup(){
    int signalStrength;
    Serial.begin(115200);
    while(!Serial);
    sim7000.begin(mySerial);
    Serial.println("Turn ON SIM7000......");
    if(sim7000.turnON()){                                        //Turn ON SIM7000
        Serial.println("Turn ON !");
    }
    Serial.println("Set baud rate......");
    while(1){
        if(sim7000.setBaudRate(19200)){                          //Set SIM7000 baud rate from 115200 to 19200 reduce the baud rate to avoid distortion
            Serial.println("Set baud rate:19200");
            break;
        }else{
            Serial.println("Faile to set baud rate");
            delay(1000);
        }
    }

    Serial.println("Check SIM card......");
    if(sim7000.checkSIMStatus()){                                //Check SIM card
        Serial.println("SIM card READY");
    }else{
        Serial.println("SIM card ERROR, Check if you have insert SIM card and restart SIM7000");
        while(1);
    }


    Serial.println("Set net mode......");
    while(1){
        if(sim7000.setNetMode(NB)){                              //Set net mod NB-IOT
            Serial.println("Set NB-IOT mode");
            break;
        }else{
            Serial.println("Fail to set mode");
            delay(1000);
        }
    }

    Serial.println("Get signal quality......");
    delay(500);
    signalStrength=sim7000.checkSignalQuality();                 //Check signal quality from (0-30)
    Serial.print("signalStrength =");
    Serial.println(signalStrength);
    delay(500);

    Serial.println("Attaching service......");
    while(1){
        if(sim7000.attacthService()){                            //Open the connection
            Serial.println("Attach service");
            break;
        }else{
            Serial.println("Fail to Attach service");
            delay(1000);
        }
    }
}

void loop(){
    String  sendData = "Water_Level:8,Rain_Gauge:9";
    Serial.print("Connect to :");
    Serial.println(serverIP);
    if(sim7000.openNetwork(TCP,serverIP,1883)){                  //Connect to server
        Serial.println("Connected !");
    }else{
        Serial.println("Failed to connect");
        return;
    }
    delay(3000);

    Serial.print(sendData);
    Serial.println(" ......");
    if(sim7000.mqttPublish(IOT_TOPIC,sendData)){                 //Send data to topic
        Serial.println("Send OK");
    }else{
        Serial.println("Failed to send");
        return;
    }
    delay(200);

    Serial.println("Close connection......");
    if(sim7000.closeNetwork()){                                  //Close connection
        Serial.println("Close connection !");
    }else{
        Serial.println("Fail to close connection !");
        return;
    }
    delay(2000);
}
