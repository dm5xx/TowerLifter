// cc by DM5XX @ GNU GPLv3
// LLAP! 

#include <Ethernet.h>


#define Relay1 2
#define Relay2 3
#define Hal1 6
#define Hal2 7

#define StateDown 1
#define StateMiddle 2
#define StateUp 3

#define GoUP 3
#define StopMoving 2
#define GoDown 1


//#define DEBUG

IPAddress ip(192, 168, 1, 155);         // The IP Address you want to use...
String co ="31.31.231.42";          // Url where the .js and .css files are located!
          // Url where the .js and .css files are located!
byte mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAC};

// the dns server ip
IPAddress dnServer(192, 168, 1, 40);
// the router's gateway address:
IPAddress gateway(192, 168, 1, 40);
// the subnet:
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(80);
char requestString[100];

bool isHal1Closed = false;
bool isHal2Closed = false;
byte currentState = 0;
byte TowerCommand = 0;
byte oldCommand = 0;

int switchingDelay = 2000;

void getCurrentPosition()
{
    #if defined DEBUG
      Serial.println("---getCurrentPosition Call");
    #endif

    isHal1Closed = !digitalRead(Hal1);
    isHal2Closed = !digitalRead(Hal2);

    if(isHal1Closed && isHal2Closed)
    {
      #if defined DEBUG
        Serial.println("CurrentPosition is UP");
      #endif
      currentState = StateUp;
      return;
    }
    if(!isHal1Closed && !isHal2Closed)
    {
      #if defined DEBUG
        Serial.println("CurrentPosition is DOWN");
      #endif
      currentState = StateDown;
      return;
    }
    if(isHal1Closed && !isHal2Closed)
    {
      #if defined DEBUG
        Serial.println("CurrentPosition is Middle");
      #endif
      currentState = StateMiddle;
      return;
    }
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);  
  pinMode(Hal1, INPUT);
  pinMode(Hal2, INPUT);

  getCurrentPosition();

  Serial.begin(115200);

  Ethernet.begin(mac, ip, dnServer, gateway, subnet);  
  delay(500);
  server.begin();


  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

// the loop function runs over and over again forever
void loop() {

  moveTower();
  Webserver();
  #if defined DEBUG
    delay(2000);
  #endif
}



void Webserver(){
  EthernetClient client = server.available();
  if (client) {
    int charIndex = 0;

    while (client.connected()) {

      if (client.available()) {
        char c = client.read();

        #if defined DEBUG
          Serial.write(c);
        #endif

        if (charIndex < 100) {
          requestString[charIndex] = c;
          charIndex++;
        }

        if (requestString[0] == 'G' && c == '\n') {
          char delimiter[] = " ";
          char *ptr;           
          char box[3][20];

          ptr = strtok(requestString, delimiter);
          
          for(int i=0;((ptr != NULL) && (i<3));i++) {
             strcpy(box[i], ptr);             
             ptr = strtok(NULL, delimiter);
          }
         
          unsigned long params[3];

          boolean cmdSet = strstr(box[1], "/Set");
          boolean cmdGet = strstr(box[1], "/Get"); // see if its a get request

          if(cmdSet)
          {

            char limiter[] = "/";
            char *pr;           
            char paramsBox[4][6];
  
            pr = strtok(box[1], limiter);
            
            for(int i=0;((pr != NULL) && (i<4));i++) {
               strcpy(paramsBox[i], pr);             
               pr = strtok(NULL, limiter);
            }
  
            params[0] = strtoul(paramsBox[1], NULL, 10);
            params[1] = strtoul(paramsBox[2], NULL, 10);
            params[2] = strtoul(paramsBox[3], NULL, 10);
            
            #if defined DEBUG    
              Serial.print("Param0: ");
              Serial.println(params[0]);
              Serial.print("Param1: ");
              Serial.println(params[1]);
              Serial.print("Param2: ");
              Serial.println(params[2]);
            #endif
          }
        
          client.println("HTTP/1.1 200 OK");
          client.println("Access-Control-Allow-Origin: *");
          
          if(cmdSet>0)
          {
            if(params[0] == GoDown)
            {
              #if defined DEBUG
                Serial.println("GoDown detected");
              #endif
              TowerCommand = GoDown;
            }
            
            if(params[0] == GoUP)
            {
              #if defined DEBUG
                Serial.println("GoUP detected");
              #endif
              TowerCommand = GoUP;
            }
            
            if(params[0] == StopMoving)
            {
              #if defined DEBUG
                Serial.println("STOP detected");
              #endif
              TowerCommand = StopMoving;
            }           
            moveTower();
          }
          else
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println(getStatus()); 

          break;          
        }
      }
    }
    delay(1);
    client.stop();
  }
}

String getStatus(){
  String result;
  result = "{\"V\":["+String(currentState)+","+isHal1Closed+","+isHal2Closed+","+String(TowerCommand)+","+String(oldCommand)+"]}";
  return result;
}

void moveUp()
{
  #if defined DEBUG
    Serial.println("Fahre Tower aus");
  #endif
  stop();
  delay(switchingDelay);
  digitalWrite(Relay1, LOW);
  digitalWrite(Relay2, HIGH);
}

void moveDown()
{
  #if defined DEBUG
    Serial.println("Fahre Tower ein");
  #endif
  stop();
  delay(switchingDelay);
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, LOW);
}

void stop()
{
  #if defined DEBUG
    Serial.println("Ich stoppe!!!");
  #endif
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
}


void moveTower()
{
  getCurrentPosition();
  
  #if defined DEBUG
    Serial.println("------------movingTower Call");
    Serial.println("olCommmand: " + oldCommand);
    Serial.println("Towercommand: " + TowerCommand);
  #endif

  if(oldCommand==0 && TowerCommand == 0)
  {
    #if defined DEBUG
      Serial.println("oldCommand==0 && TowerCommand == 0 nothing to do!");
    #endif
    return;
  }
  else
  {
    if( (TowerCommand == GoUP && currentState == StateUp) ||
        (TowerCommand == GoDown && currentState == StateDown) ||
        TowerCommand == StopMoving)
    {
        #if defined DEBUG
          Serial.println("Endstatus erreicht oder Stop gedrückt");
        #endif
        stop();
        TowerCommand = 0;
        oldCommand = 0;
        return;
    }
    else
    {
      if(TowerCommand == oldCommand)
      {
        #if defined DEBUG
          Serial.println("oldCommand = TowerCommand - einfach weitermachen was auch immer");
        #endif
        return;
      }
      else
      {
        if(TowerCommand == 2)
        {
          #if defined DEBUG
            Serial.println("Das Stop dürfte nicht erreicht werden");
          #endif
          stop();
        }
        if(TowerCommand == 3)
        {
          moveUp();
        }
        if(TowerCommand == 1)
        {
          moveDown();
        }
        
        oldCommand = TowerCommand;
      }
    }
  }
}