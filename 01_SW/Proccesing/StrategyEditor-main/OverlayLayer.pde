boolean showOverlay = true;

PImage robotOverlayImg;

float overlayRobotSize_mm = 400;  // ou la taille que tu veux pour le rendu visuel



void drawOverlay(PGraphics pg, float scale) {
  if (!showOverlay) return;

  pg.pushStyle();

  // Filtre bleu transparent sur tout le terrain
  pg.fill(0, 0, 255, 40);
  pg.noStroke();
  pg.rect(0, 0, pg.width, pg.height);

  // Texte directions
  pg.fill(255);
  pg.textAlign(CENTER, CENTER);
  pg.textSize(40);
  pg.textFont(createFont("Arial-Bold", 40));

  // NORTH (haut, centré)
  pg.text("NORTH", pg.width / 2, 40);

  // SOUTH (bas, centré)
  pg.text("SOUTH", pg.width / 2, pg.height - 40);

  // WEST (gauche, centré verticalement)
  pg.pushMatrix();
  pg.translate(40, pg.height / 2);
  pg.rotate(-HALF_PI);
  pg.text("WEST", 0, 0);
  pg.popMatrix();

  // EAST (droite, centré verticalement)
  pg.pushMatrix();
  pg.translate(pg.width - 40, pg.height / 2);
  pg.rotate(HALF_PI);
  pg.text("EAST", 0, 0);
  pg.popMatrix();


  // Lignes des axes depuis (0,0)
  pg.stroke(255);
  pg.strokeWeight(5);
  pg.line(0, 0, 100, 0);   // Axe X (vers la droite)
  pg.line(0, 0, 0, 100);   // Axe Y (vers le bas)

  // Triangles directionnels au bout des lignes

  pg.noStroke();
  pg.fill(255);

  // Triangle pour X (à 100, 0) → pointant vers la droite
  pg.triangle(100, -10, 100, 10, 120, 0);
  pg.textFont(createFont("Arial-Bold", 14));
  pg.textAlign(LEFT, CENTER);
  pg.text("X", 115, 15);

  // Triangle pour Y (à 0, 100) → pointant vers le bas
  pg.triangle(-10, 100, 10, 100, 0, 120);
  pg.textAlign(CENTER, TOP);
  pg.text("Y", 15, 115);


  drawAngleRosace(pg, scale);


  pg.popStyle();
}

void drawAngleRosace(PGraphics pg, float scale) {
  float centerX = pg.width / 2;
  float centerY = pg.height / 2;
  float radius = 140;
  float lineLength = 20;
  float labelOffset = 30;
  int labelSize = 14;

  String[] labels = { "A", "AB", "B", "BC", "C", "CA" };
  int[] angles =    {   0, 60, 120, 180, 240, 300 };

  float labelRadius = radius * 0.75;  // plus proche du centre

  pg.stroke(255);
  pg.strokeWeight(2);
  pg.noFill();

  // Cercle central
  pg.strokeWeight(3);
  pg.ellipse(centerX, centerY, radius * 2, radius * 2);

  // Dessin de l'image du robot au centre
  if (robotOverlayImg != null) {
    float robotSize_px = overlayRobotSize_mm * StrategyEditor.mmToPx;

    pg.pushMatrix();
    pg.translate(centerX, centerY);
    pg.rotate(HALF_PI);  // rotation de 90° vers la droite

    pg.imageMode(CENTER);
    pg.tint(255); // transparence légère
    pg.image(robotOverlayImg, 0, 0, robotSize_px, robotSize_px);
    pg.noTint();
    pg.popMatrix();
  }



  // Traits d'angle (tous les 30°)
  for (int a = 0; a < 360; a += 30) {
    float angle = radians(-a);

    float x1 = centerX + cos(angle) * (radius - lineLength);
    float y1 = centerY + sin(angle) * (radius - lineLength);
    float x2 = centerX + cos(angle) * radius;
    float y2 = centerY + sin(angle) * radius;
    pg.line(x1, y1, x2, y2);

    String label = str(a) + "°";
    float lx = centerX + cos(angle) * (radius + labelOffset);
    float ly = centerY + sin(angle) * (radius + labelOffset);
    pg.textAlign(CENTER, CENTER);
    pg.textFont(createFont("Arial-Bold", labelSize));
    pg.fill(255);
    pg.text(label, lx, ly);
  }


  pg.textFont(createFont("Arial-Bold", 16));
  pg.textAlign(CENTER, CENTER);
  pg.fill(255);

  for (int i = 0; i < labels.length; i++) {
    float a = radians(-angles[i]);  // même correction que pour les ticks
    float lx = centerX + cos(a) * labelRadius;
    float ly = centerY + sin(a) * labelRadius;
    pg.text(labels[i], lx, ly);
  }
}

void loadOverlayResources() {
  robotOverlayImg = loadImage("robot.png");
}
