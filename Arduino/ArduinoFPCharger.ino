#include <DFRobot_ID809.h>
/*
 * Script Général du projet Objet Co ...
 * Arduino
 */


// Code pour le lecteur d'empruntes
#define COLLECT_NUMBER 3  //Fingerprint sampling times, can be set to 2-3

/*Use software serial when using UNO or NANO */
#if ((defined ARDUINO_AVR_UNO) || (defined ARDUINO_AVR_NANO))
    #include <SoftwareSerial.h>
    SoftwareSerial Serial1(2, 3);  //RX, TX
    #define FPSerial Serial1
#else
    #define FPSerial Serial1
#endif

DFRobot_ID809 fingerprint;

#define PLACE "{\"status\": true, \"message\": \"Place finger on reader\", \"statusmsg\":\"fp_place_finger\"}"
const unsigned int BATTERY_CAPACITY = 2000;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // On attend la lecture série
  while(!Serial);

  
  // Emprunte
  FPSerial.begin(115200);
  fingerprint.begin(FPSerial);
  
  while(fingerprint.isConnected() == false){
    Serial.println(F("{\"status\": false, \"message\": \"Unable to start communication with the fingerprint sensor\", statusmsg:\"fp_init\"}"));
    delay(1000);
  }

  /*
  while(!lipo.begin()) {
    Serial.println(F("{\"status\": false, \"message\": \"Unable to start communication with the battery sensor\", statusmsg:\"bat_init\"}"));
    delay(1000);
  }
  */
  
  
  
}

/*
 * Changement couleur empreinte digitale
 */
void changeColorFP(String value) {
  DFRobot_ID809::eLEDColor_t color = fingerprint.eLEDWhite;
    DFRobot_ID809::eLEDMode_t mode = fingerprint.eKeepsOn;
    
    if (value.equals("green")) {
      color = fingerprint.eLEDGreen;
    } else if (value.equals("red")) {
      color = fingerprint.eLEDRed;
    } else if (value.equals("yellow")) {
      color = fingerprint.eLEDYellow;
    } else if (value.equals("blue")) {
      color = fingerprint.eLEDBlue;
    } else if (value.equals("cyan")) {
      color = fingerprint.eLEDCyan;
    } else if (value.equals("magenta")) {
      color = fingerprint.eLEDMagenta;
    } else if (value.equals("white")) {
      color = fingerprint.eLEDWhite;
    } else if (value.equals("off")) {
      color = fingerprint.eLEDWhite;
      mode = fingerprint.eNormalClose;
    }

    
    fingerprint.ctrlLED(mode,color, 0);
    Serial.println(F("{\"status\": true, \"message\": \"[FP] Set color OK\", \"statusmsg\":\"ok\"}"));
}

/*
  * Procédure : 
  * - LED Jaune pour demander présentation du doigt
  * - Scan ...
  * - Vert pour ok, Rouge pour faux
*/
    
int scanFP() {
      
  fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDYellow, 0);
      if ((fingerprint.collectionFingerprint(/*timeout=*/0)) != ERR_ID809) {
        fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDGreen, 0);
        while(fingerprint.detectFinger());
        return fingerprint.search();
      } else {
        fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDRed, 5);
        Serial.print(F("{\"status\": false, \"message\": \"[FP] Capturing error : "));
        Serial.print(fingerprint.getErrorDescription());
        Serial.println(F("\", \"statusmsg\":\"fp_error\"}"));
        return -1;
      }
}

int regFP() {
  int id;
  if((id = fingerprint.getEmptyID()) == ERR_ID809){
      Serial.print(F("{\"status\": false, \"message\": \"[FP] Get id error : "));
      Serial.print(fingerprint.getErrorDescription());
      Serial.println(F("\", \"statusmsg\":\"fp_error\"}"));
      return -1;
  }

  
  int i = 0;
  while (i < COLLECT_NUMBER) {
    fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDYellow, 0);
    Serial.println(PLACE);
    if((fingerprint.collectionFingerprint(10)) != ERR_ID809){
      fingerprint.ctrlLED(fingerprint.eFastBlink,fingerprint.eLEDGreen,3);
      Serial.println(F("{\"status\": true, \"message\": \"OK. Release finger of reader\", \"statusmsg\":\"fp_release_finger_ok\"}"));
      i++;
    } else {
      fingerprint.ctrlLED(fingerprint.eFastBlink,fingerprint.eLEDRed,3);
      Serial.println(F("{\"status\": true, \"message\": \"OK. Release finger of reader\", \"statusmsg\":\"fp_release_finger_ok\"}"));
    }

    while(fingerprint.detectFinger());
  }

  if(fingerprint.storeFingerprint(id) != ERR_ID809){
    fingerprint.ctrlLED(fingerprint.eKeepsOn,fingerprint.eLEDGreen,0);
   return id; 
  } else {
    Serial.print(F("{\"status\": false, \"message\": \"[FP] Save error : "));
      Serial.print(fingerprint.getErrorDescription());
      Serial.println(F("\", \"statusmsg\":\"fp_error\"}"));
      return -1;
  }
  
  
}

void deleteFP(String id) {
  fingerprint.delFingerprint(id.toInt());
  fingerprint.ctrlLED(fingerprint.eFastBlink,fingerprint.eLEDYellow,3);
  Serial.println(F("{\"status\": true, \"message\": \"FingerPrint deleted\", \"statusmsg\":\"fp_delete_ok\"}"));
}

int getNbEmpreintes() {
  return fingerprint.getEnrollCount();
}


/*
 * Gestion d'une requête de type "fingerprint"
 * action : nom de l'action
 * value : valeur à appliquer (si la requête en nécessite une)
 */
void handleFP(String action, String value) {
  String found1 = F("{\"status\": true, \"message\": \"[FP] FP Found\", \"data\":");
  String found2 = F(", \"statusmsg\":\"ok\"}");
   // Changement de couleur
  if (action.equals("color")) {
    changeColorFP(value);
    return;
  } else if (action.equals("register")) {
    Serial.println(PLACE);
    int rep = 0;
    if (rep != 0) {
         Serial.print(found1);
         Serial.print(rep);
         Serial.println(found2);  
    } else if (rep == 0){
      rep = regFP();
      if (rep != -1) {
        Serial.print(found1);
       Serial.print(rep);
       Serial.println(found2); 
      } else {
        return;
      }
    } else {
      return;
    }
    
    
    return; 
  } else if (action.equals("scan")) {
    Serial.println(PLACE);
      int rep = scanFP();
      if (rep != -1 && rep != 0) {
        Serial.print(F("{\"status\": true, \"message\": \"[FP] FP Found\", \"data\":"));
         Serial.print(rep);
         Serial.println(F(", \"statusmsg\":\"ok\"}"));
      } else if (rep ==0) {
        fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDRed, 5);
        Serial.println(F("{\"status\": false, \"message\": \"[FP] FP Not Found\", \"statusmsg\":\"fp_not_found\"}"));
       } else {
        return;
      }
      return;
    } else if (action.equals("delete")) {
      deleteFP(value);
      return;
   } else if (action.equals("get_nb")) {
      Serial.print(F("{\"status\": true, \"message\": \"[FP] OK\", \"data\":"));
      Serial.print(getNbEmpreintes());
      Serial.println(F(", \"statusmsg\":\"ok\"}"));
   }
  else {
    Serial.print(F("{\"status\": false, \"message\": \"[FP] Action Undefined : "));
    Serial.print(action);
    Serial.println(F("\", \"statusmsg\":\"fp_act_und\"}"));
  }
  
}

void handleBat(String action, String value) {
  if (action.equals("get_percent")) {
        Serial.print(F("{\"status\": true, \"message\": \"[BAT] Bat Chg\", \"data\":"));
         Serial.print(50);
         Serial.println(F(", \"statusmsg\":\"ok\"}"));
  } else {
    Serial.print(F("{\"status\": false, \"message\": \"[BAT] Action Undefined : "));
    Serial.print(action);
    Serial.println(F("\", \"statusmsg\":\"bat_act_und\"}"));
  }
}


 String antiHeadeache(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void handleText(String data) {

  
  
  
  
  String type = antiHeadeache(data, '"', 3);
  String action = antiHeadeache(data, '"', 7);
  String value = antiHeadeache(data, '"', 11);

  if (type.equals("fingerprint")) {
    //Serial.println("handling");
    handleFP(action,value);
  } else if (type.equals("battery")) {
    handleBat(action,value);
  }else {
    Serial.println(F("{\"status\": false, \"message\": \"Action Undefined\", \"statusmsg\":\"act_und\"}"));
  }


}






String incoming = "";

void loop() {
  // put your main code here, to run repeatedly:
 if (Serial.available() > 0) {
    // read the incoming:
    incoming = Serial.readString();
    // say what you got:
    incoming.replace("\n","");
    
    handleText(incoming);

    Serial.flush(); 
 }  
  
}
