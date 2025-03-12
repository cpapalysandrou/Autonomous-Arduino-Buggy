#include <WiFiS3.h>
//https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples/
//this explains the wifi code if anyone wants to have a better understanding

#define leftForward 10
#define leftBackward 6
#define rightForward 9
#define rightBackward 4
#define Rmotor 5
#define Lmotor 11
#define leftEncode 2
#define rightEncode 3 
const int leftEye = D7;
const int rightEye = D8;
bool leftEyePrevious = 0;
bool leftEyeCurrent = 0;
bool rightEyePrevious = 0;
bool rightEyeCurrent = 0; //initialises all pins
const int period = 1000;
const int echoPin = D12;
const int trigPin = D13;
float duration = 0;
float distanceCM = 0;
bool running = 0;
volatile int pulseCountR = 0;
volatile int pulseCountL = 0;
unsigned long previousMillis = 0;  // Stores the last time the function ran
const long interval1 = 3000;        // 2 seconds (2000 milliseconds)
const long interval3 = 30;

char name[] = "2E10_TC02";
char password[] = "LindaDoyle";

//char name[] = "eir47433547"; //Lukas wifi
//char password[] = "ugg6JxUvag";

//char name[] = "Thomas's iPhone";
//char password[] = "thomascoase";


WiFiServer server(2025);

void setup() {
  // Set motor pins as OUTPUT
  Serial.begin(9600);
  while (!Serial) {}
  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightBackward, OUTPUT);
  //laptop control buggy
  pinMode(leftEye, INPUT);
  pinMode(rightEye, INPUT);
 
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  WiFi.begin(name, password);
  while (WiFi.status() != WL_CONNECTED) { //waits until wifi connected
    Serial.print(" .... ");
    delay(500);
  }
  Serial.println("Connected to WiFi."); //says connected when on
  server.begin();
  Serial.print("IP Adress: ");
  Serial.println(WiFi.localIP());

  pinMode(leftEncode, INPUT_PULLUP);
  pinMode(rightEncode, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rightEncode), countPulsesR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(leftEncode), countPulsesL, CHANGE);
}

void loop() {
  handleclient(); //takes in info from processing
  if (running) { //ensures the processing connection is working before starting operation of buggy
    lineFollowingModule(); //operation of buggy
}
}



void moveForwardSlower() {
  analogWrite(Lmotor, 120);
  analogWrite(Rmotor, 120);
  digitalWrite(leftForward, HIGH);
  digitalWrite(leftBackward, LOW);
  digitalWrite(rightForward, HIGH);
  digitalWrite(rightBackward, LOW);
  //Serial.println("Moving Forward");
  //makes the buggy move forward full pelt
  teleTrigger();
}

void sendingTelemetry(WiFiClient client) {
   if (client) {
  client.println(
    String(pulseCountR) + "," + 
    String(pulseCountL) + "," +
    String(distanceCM)
  );
  }
}

void handleclient() {
  WiFiClient client = server.available(); //check if client connected
  if (client) { //client exists
    if (client.available()) { //data from proccessing
      String request = client.readStringUntil('\n'); //read data till newline to get everything
      request.trim(); //remove extra spaces sometimes needed
      Serial.println(request); //print request to the serial

      if (request == "start") { //if request is "start"
        running = 1; //set running to 1
        moveForwardSlower(); //move forward but loop handles it later
      } else if (request == "stop") { //if request is "stop"
        running = 0; //set running to 0
        stopBuggy(); //stop buggy
      }
      else if (request == "UpdateDistance") {
        teleTrigger();
      }
  }
    }
    

    client.flush(); //just for if start spammed etc
    

}

void countPulsesR() {
  pulseCountR++;
}

void countPulsesL() {
  pulseCountL++;
}

//each pulse represents 1/8 of a wheel spin, distance travelled is calculated in Processing
void turnLeftGentle() {
  analogWrite(Lmotor, 140 / 4);
  analogWrite(Rmotor, 140 * 1.1); //enable pins
  digitalWrite(leftForward, HIGH);
  digitalWrite(leftBackward, LOW);
  digitalWrite(rightForward, HIGH);
  digitalWrite(rightBackward, LOW);
  //Serial.println("Turning left gently");
}

void turnRightGentle() {
  analogWrite(Lmotor, 140 * 1.2);
  analogWrite(Rmotor, 140 / 4);
  digitalWrite(leftForward, HIGH);
  digitalWrite(leftBackward, LOW);
  digitalWrite(rightForward, HIGH);
  digitalWrite(rightBackward, LOW);
  //Serial.println("Turning right gently");
}

void stopBuggy() {
  analogWrite(Lmotor, 0);
  analogWrite(Rmotor, 0);
  //Serial.println("stopping");
  //everything set to low
  Serial.print("Stopping for obstacle at ");
    Serial.print(distanceCM);
    Serial.println("cm away");
}

int getChange() { //called constantly, to check if change in eyes has occured
  leftEyeCurrent = digitalRead(leftEye);
  rightEyeCurrent = digitalRead(rightEye);
  //finds status of both eyes
  if (rightEyeCurrent == rightEyePrevious && leftEyeCurrent == leftEyePrevious) {
    return 0;
  } else if (rightEyeCurrent != rightEyePrevious && leftEyeCurrent != leftEyePrevious) {
    return 0;
  } else if (leftEyeCurrent != leftEyePrevious) //&& rightEyeCurrent==rightEyePrevious)
  {
    return 1; // sends signal if lost on only left eye
  } else if (rightEyeCurrent != rightEyePrevious) //&& leftEyeCurrent==leftEyePrevious)
  {
    return 2; // sends signal if lost on only right eye
  }
}

void measureDistance() {
  digitalWrite(trigPin, 0);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, 0);
  duration = pulseIn(echoPin, HIGH);
  distanceCM = (duration * 0.034) / 2;
  delay(1);
}

void lineFollowingModule() {
  measureDistance();
  /* unsigned long currentMillis = millis();  // Get current time
  if (currentMillis - previousMillis >= interval1) { //as requested, updates control panel of distance travelled every 2 seconds.
    previousMillis = currentMillis;
    Serial.print("Distance Travelled: ");
    Serial.print(((pulseCountL + pulseCountR)/2)*7.3*3.14/(8*100));
    Serial.println("m");
  }*/
  if (getChange() == 0 && distanceCM > 10) { //||distanceCM>=10) calls getChange, which checks the eye within the fnctn
    //if both eyes see the same thing, the buggy will continue foward
    moveForwardSlower();
    // //moveForwardSlower();
  }
  if (distanceCM <= 10) { //if it notices that the distance is sub-10, stops
    stopBuggy();
    measureDistance(); //re-checks, if it finds distance is above 10, leaves while loop
  }
  while (getChange() != 0) { //enters while loop if the eyes are different
    if (getChange() == 1) { // if right eye wrong, turns gently right
      turnRightGentle();
      //turnRightSharp();
    }
    if (getChange() == 2) { //if left eye wrong, turns gently left
      turnLeftGentle();
    }
  }
}

void teleTrigger() {
  WiFiClient client = server.available(); //check if client connected
    Serial.println("Sending now");
 sendingTelemetry(client);
 delay(1);
}

// Push test

