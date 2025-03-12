import processing.net.*;  // Networking library

Client myClient;
String serverIP = "192.168.0.177"; //TCD IP
//String serverIP = "192.168.1.161";
int serverPort = 2025;

// System states
int states[] = {0, 0, 0};

// Button states
boolean IsStartOn = false;
boolean IsStopOn = false;
boolean hasDecimal = false;

int pulseCountL = 0;
int pulseCountR = 0;
int avgPulseCount = 0;
String inputString = " ";
float distance = 0;
String input = " ";
float newvel = 0;
float oldvel = newvel;

float calculatedDistance = 0; 
float previousMillis = 0;
float currentMillis = 0;
int interval1 = 100;
float objectDistance = 20;

void setup() {
  size(600, 500);
  myClient = new Client(this, serverIP, serverPort); //creates connection
  background(100,170, 170);
  textSize(30);
  fill(0);
  text("CONTROL PANEL", 300 ,35);
 // drawStartButton();
 // drawStopButton();
  drawUpdateButton();
 // sayDistance();//  
} 

void draw() {
 background(100,170, 170);
  textSize(30);
  fill(0);
  text("CONTROL PANEL", 300 ,35);
  drawStartButton();
  drawStopButton();
  drawUpdateButton();
  handleTelemetry();
  sayDistance();
  updateVelocity(newvel);
  

  float currentMillis = millis();  // Get current time
  if (currentMillis - previousMillis >= interval1) { //as requested, updates control panel of distance travelled every 2 seconds.
    previousMillis = currentMillis;
  updateDistance(); }
}
 //runs constantly 


void drawStartButton() {
  // Button shadow
  fill(0, 0, 0, 30);
  noStroke(); //no border
  rect(120-3, 200-3, 105+6, 75+6); //creates rectangle of mentioned dimensions
  // Button body
  if (IsStartOn) {
    fill(60, 225, 300); //changes color
  } else {
    fill(60, 180, 230);
  }
  stroke(30, 120, 180); // Border color
  strokeWeight(2); // Border thickness
  rect(120, 200, 105, 75);

  // Button text
  fill(255);
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Start", 175, 200 + 75/2);
  
  
}

void drawStopButton() {
  fill(0, 0, 0, 30);
  noStroke();
  rect(370-3, 200-3, 105+6, 75+6);

  // Button body
  if (IsStopOn) {
    fill(200, 120, 120); // Darker red when pressed
  } else {
    fill(230, 140, 140); // Bright red for normal state
  }
  stroke(180, 100, 100); // Border color
  strokeWeight(2); // Border thickness
  rect(370, 200, 105, 75); // Rounded corners

  // Button text
  fill(255); // White text
  textSize(20);
  textAlign(CENTER, CENTER);
  text("Stop", 425, 200+ 75 /2);
}

void mousePressed() {
  // Check if "Start" button is clicked
  if (mouseX > 75 && mouseX < 275 && mouseY > 200 && mouseY < 275) {
    IsStartOn = true;
    sendCommand("start");
  }

  // Check if "Stop" button is clicked
  if (mouseX > 325 && mouseX < 525 && mouseY > 200 && mouseY < 275) {
    IsStopOn = true;
    sendCommand("stop");
  }
}

void mouseReleased() {
  IsStartOn = false;
  IsStopOn = false;
}

void sayDistance() {
  // Print distance
  fill(0);
  textSize(22);
  text("Distance Travelled: " + nf(calculatedDistance, 0, 2) + "m", 300, 95);

  // Print shadow
  fill(0, 0, 0, 30);
  noStroke();
  rect(114, 75, 376, 75);

  
 if (objectDistance > 10) {
   fill(0);
  textSize(22);
  text("Object Distance: " + nf(objectDistance, 0, 2) + "cm", 300, 130);}
  else {
    fill(255, 0 , 0);
    textSize(22);
    text("Object Distance less than 10cm: Stopping", 300, 130);
  }
}

void drawUpdateButton() {
  fill(0, 0, 0, 30);
  noStroke();
  rect(100, 325, 400, 125); 
  fill(30);
  text("Enter New Velocity:", 300, 355);
  fill(175);
  strokeWeight(4);
  rect(150, 375, 300, 50);
}

float calculateDistance() {
  avgPulseCount = (pulseCountL + pulseCountR)/2; //gets average as the buggy turns left/right and creates difference
  float circum = 7.3 * 3.14159; //7.3 = diameter
  calculatedDistance = avgPulseCount*circum/(8*100); //8 = pulses per revolution
  return calculatedDistance;
}

void keyPressed() {
if (key >= '0' && key <= '9') {
  input += key;
newvel = 0;
}
     else if (key == '.' && !hasDecimal) { // Allow only one decimal point
    input += key;
    hasDecimal = true;
   }
    else if (key == BACKSPACE && input.length() > 0) { // Remove last character
    if (input.charAt(input.length() - 1) == '.') {
      hasDecimal = false; // Reset decimal tracker if removed
    }
    input = input.substring(0, input.length() - 1);
    }
else if (key == ENTER && input.length() > 0 && !input.equals(".")) { // Convert to float
    newvel = float(input);
    println("You entered: " + newvel);
        hasDecimal = false;
        input = " ";
      //  sendCommand(newvel);
  }
}

void updateVelocity(float newvel) {
  if (oldvel != newvel && newvel != 0) {
  fill(175);
  strokeWeight(4);
  rect(150, 375, 300, 50); 
  fill(0);
text(newvel, 300, 400);
oldvel = newvel;
  }
}





void updateDistance() {
  if (myClient.active()) {
    sendCommand("UpdateDistance");
    calculateDistance();
}
}
 

void sendCommand(String command) {
  if (myClient.active()) {
    myClient.write(command + "\n");
    println("Sent: " + command);
  } else {
    println("Not connected!");
  }
}

// Continuously handle telemetry data from Arduino
void handleTelemetry() {
  if (myClient != null && myClient.available() > 0 && myClient.active()) {
    String data = myClient.readStringUntil('\n'); //reads data from aurdino
    if (data != null) {
      data = data.trim();
      println("Received Data: " + data);  //for debug

      String[] values = split(data, ',');
      if (values.length == 3) { // Expecting 2 values
        pulseCountR = int(values[0]);
        pulseCountL = int(values[1]); //inputs first value as right pulse count and second as left pulse count
        objectDistance = float(values[2]);
        // Update distance
        calculateDistance();
      }
    }
  }
} 
