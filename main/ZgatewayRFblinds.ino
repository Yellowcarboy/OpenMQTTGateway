/*  
  OpenMQTTGateway  - ESP8266 or Arduino program for home automation 

   Act as a wifi or ethernet gateway between your 433mhz and a MQTT broker for RF Blinds 
   Send and receiving command by MQTT
 
  This gateway enables to:
 - receive MQTT data from a topic and send RF 433Mhz signal corresponding to the received MQTT data
 - publish MQTT data to a different topic related to received 433Mhz signal

*/
#include "User_config.h"

#ifdef ZgatewayRFBlind

#include <RFBlindComs.h> // library for controling Radio frequency switch
#    define RF_TRANSMITDISABLE_GPIO 16

RFBlindComs rfBlindComs = RFBlindComs();

void setupRFBlind() {
  //RF init parameters
  Log.notice(F("RFBlind_EMITTER_GPIO: %d " CR), RF_EMITTER_GPIO);
  Log.notice(F("RFBlind_RECEIVER_GPIO: %d " CR), RFBLIND_RECEIVER_GPIO);
  rfBlindComs.enableTransmit(RF_EMITTER_GPIO);
  rfBlindComs.setRepeatTransmit(RFBLIND_EMITTER_REPEAT);
  rfBlindComs.enableReceive(RFBLIND_RECEIVER_GPIO);
  Log.trace(F("ZgatewayRFBlind setup done" CR));
  pinMode(RF_TRANSMITDISABLE_GPIO, OUTPUT); 
  digitalWrite(RF_TRANSMITDISABLE_GPIO, LOW);
}

void RFBlindtoMQTT() {
  if (rfBlindComs.available()) {
    const int JSON_MSG_CALC_BUFFER = JSON_OBJECT_SIZE(4);
    StaticJsonDocument<JSON_MSG_CALC_BUFFER> jsonBuffer;
    JsonObject RFdata = jsonBuffer.to<JsonObject>();
    //Log.trace(F("Blind RF" CR));

    //DynamicJsonDocument doc(JSON_MSG_CALC_BUFFER);
    //doc["key"] = (char*)rfBlindComs.getReceivedValue();
    RFdata["value"] = (char*)rfBlindComs.getReceivedValue();
    rfBlindComs.resetAvailable();

    //char* MQTTvalue = RFdata.get<char*>("value");
    pub(subjectRFBlindtoMQTT, RFdata);
    //pub(subjectRFBlindtoMQTT, doc);
  
  }
}

#  ifdef jsonReceiving
void MQTTtoRFBlind(char* topicOri, JsonObject& RFdata) { // json object decoding
  if (cmpToMainTopic(topicOri, subjectMQTTtoRFBlind)) {
    digitalWrite(RF_TRANSMITDISABLE_GPIO, HIGH);
    delayMicroseconds(200);
    Log.trace(F("MQTTtoRFBlind json" CR));
    const char *data = RFdata["value"];
    Log.trace(F("RF MQTT data %s" CR),data);
    int valueRPT = RFdata["repeat"] | RFBLIND_EMITTER_REPEAT;
    rfBlindComs.setRepeatTransmit(valueRPT);
    rfBlindComs.send(data);
    Log.notice(F("MQTTtoRFBlind OK" CR));
    pub(subjectRFBlindtoMQTT, RFdata); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
    rfBlindComs.setRepeatTransmit(RFBLIND_EMITTER_REPEAT); // Restore the default value
    digitalWrite(RF_TRANSMITDISABLE_GPIO, LOW);
  }
}
#  endif
#endif
