ArrayList <PVector> points = new ArrayList <PVector> ();
ArrayList <PVector> coord_robot = new ArrayList <PVector> ();
PImage Table; 
import processing.serial.*;
Serial myPort;
int lf = 10;
String myString = null;
float x, y, angle;
int size_x = 1500;
int size_y = 900;
int midd_x = size_x / 2;
int midd_y = size_y / 2;
float scale = 4;
float scale_robot = 2;
PVector p1, p2, p3, p4;

void setup() {
  myPort = new Serial(this, Serial.list()[0], 230400);
  
  size(1500, 900);
  Table = loadImage("playmat_2025_FINAL.png");
  points.add(new PVector(0, 0));
  coord_robot.add(new PVector(0, 0, 0));
  
  points.add(new PVector(midd_x - 150 * scale, midd_y + 100 * scale));
  points.add(new PVector(midd_x - 150 * scale, midd_y - 100 * scale));
  points.add(new PVector(midd_x + 150 * scale, midd_y - 100 * scale));
  points.add(new PVector(midd_x + 150 * scale, midd_y + 100 * scale));
  p1 = points.get(1);
  p2 = points.get(2);
  p3 = points.get(3);
  p4 = points.get(4);
}

void draw() {
  background(255); // Efface tout
  image(Table, p2.x, p2.y, 300 * scale, 200 * scale);
  draw_table(); // Redessine le cadre + coordonnées

  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(10);
    if (myString != null) {
      String[] q = splitTokens(myString, ";");
      if (q.length > 2) {
        x = float(q[0]);
        y = float(q[1]);
        angle = float(q[2]);

        coord_robot.clear();
        coord_robot.add(new PVector(p1.x + x * scale / 10, p1.y - y * scale / 10, angle));
        PVector coord = coord_robot.get(0);

        // Affichage du robot carré orienté
        pushMatrix();
        translate(coord.x, coord.y);
        rotate(radians(coord.z)); 

        float robotSize = 10 * scale * scale_robot;

        // Carré
        stroke(0, 255, 0);
        strokeWeight(2);
        fill(255, 255, 255);
        rectMode(CENTER);
        rect(0, 0, robotSize, robotSize);

        // Flèche vers l’avant
        fill(255, 0, 0);
        noStroke();
        float arrowSize = robotSize * 0.2;
        triangle(
          0, -robotSize / 2 - arrowSize / 2,
          -arrowSize / 2, -robotSize / 2,
          arrowSize / 2, -robotSize / 2
        );
        popMatrix();

        println(x, y, angle);
      }
    }
  }
}

void draw_table() {
  // Cadre
  stroke(0);
  strokeWeight(3);
  line(p1.x, p1.y, p2.x, p2.y);
  line(p2.x, p2.y, p3.x, p3.y);
  line(p3.x, p3.y, p4.x, p4.y);
  line(p4.x, p4.y, p1.x, p1.y);

  // Graduation X
  for (int i = 0; i <= 30; i++) {
    stroke(255, 0, 0);
    strokeWeight(1.5);
    fill(0);
    textSize(3 * scale);
    float px = p1.x + i * 10 * scale;
    text(i * 100, px - 5, p1.y + 15);
    ellipse(px, p1.y, 2, 20);
  }

  // Graduation Y
  for (int i = 0; i <= 20; i++) {
    stroke(255, 0, 0);
    strokeWeight(1.5);
    fill(0);
    textSize(3 * scale);
    float py = p2.y + i * 10 * scale;
    text(2000 - i * 100, p2.x - 45, py + 5);
    ellipse(p2.x, py, 20, 2);
  }

  // Point origine
  stroke(0, 255, 0);
  strokeWeight(10);
  point(p1.x, p1.y);
}
