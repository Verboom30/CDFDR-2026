ArrayList <PVector> points = new ArrayList <PVector> ();
 
import processing.serial.*;
Serial myPort;  // The serial port
int lf = 10;    // Linefeed in ASCII
String myString = null;
float x,y,angle,r;
float f = 0.5;
 
void setup() {
  // List all the available serial ports
  printArray(Serial.list());
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[0], 230400);
    
  size(2000,2000);
  points.add(new PVector(0,0));
}
 
void draw() {
  while (myPort.available() > 0){
    myString = myPort.readStringUntil(10);
    if (myString != null) {
      //print(myString);
      String[] q = splitTokens(myString, ";");
      if (q.length>1){
        angle=float(q[0]);  // Converts and prints float
        r = float(q[1]);  // Converts and prints float
        
        x = (f*sin(angle*(PI/180))*r + 1000);
        y = (-f*cos(angle*(PI/180))*r + 1000);
        stroke(255,0,0); // ellipse color: default white fill, red stroke
        strokeWeight(3);
        point(x,y);
        println(x,y);
      }
    }
        
  
  }
}
