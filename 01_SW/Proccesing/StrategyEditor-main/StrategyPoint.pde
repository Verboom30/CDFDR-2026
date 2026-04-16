class StrategyPoint {

  int id;
  float x_mm;
  float y_mm;

  String poiName = null;

  // angle final sur ce point
  float angleDeg = 0;

  StrategyPoint(int id, float x_mm, float y_mm) {
    this.id = id;
    this.x_mm = x_mm;
    this.y_mm = y_mm;
    this.angleDeg = 0;
  }

  void draw(PGraphics pg, float scale) {

    float px = x_mm;
    float py = y_mm;

    // aperçu robot orienté sur le point sélectionné
    if (this == selectedPoint) {
      drawRobotPreview(pg, px, py);
    }

    // point sélectionné
    if (this == selectedPoint) {
      pg.stroke(255, 0, 0);
      pg.strokeWeight(3.0 / mmToPx);
      pg.fill(255, 180, 180);
    } else {
      pg.stroke(0);
      pg.strokeWeight(1.0 / mmToPx);
      pg.fill(255, 0, 0);
    }

    pg.ellipse(px, py, 18.0 / mmToPx, 18.0 / mmToPx);

    drawLabel(pg, px, py);
    drawAngleArrow(pg, px, py);
  }

  void drawLabel(PGraphics pg, float px, float py) {

    pg.textFont(font);
    pg.textAlign(CENTER, CENTER);
    pg.textSize(22.0 / mmToPx);

    String txt = "P" + id;

    float tx = px;
    float ty = py - 16.0 / mmToPx;

    pg.fill(0);
    pg.text(txt, tx - 1.0 / mmToPx, ty);
    pg.text(txt, tx + 1.0 / mmToPx, ty);
    pg.text(txt, tx, ty - 1.0 / mmToPx);
    pg.text(txt, tx, ty + 1.0 / mmToPx);

    pg.fill(180, 0, 255);
    pg.text(txt, tx, ty);
  }

  void drawAngleArrow(PGraphics pg, float px, float py) {

    float a = radians(angleDeg);

    float len = 40.0 / mmToPx;

    float x2 = px + cos(a) * len;
    float y2 = py + sin(a) * len;

    if (this == selectedPoint) {
      pg.stroke(0, 255, 255);
      pg.fill(0, 255, 255);
    } else {
      pg.stroke(255, 255, 0);
      pg.fill(255, 255, 0);
    }

    pg.strokeWeight(2.5 / mmToPx);

    pg.line(px, py, x2, y2);

    pg.pushMatrix();
    pg.translate(x2, y2);
    pg.rotate(a);
    pg.noStroke();
    pg.triangle(
      0, 0,
      -10.0 / mmToPx, -4.0 / mmToPx,
      -10.0 / mmToPx, 4.0 / mmToPx
    );
    pg.popMatrix();
  }

void drawRobotPreview(PGraphics pg, float px, float py) {

  PImage imgToDraw;
  float width_mm;
  float height_mm;
  float hitbox_mm;

  if (usePAMI) {
    imgToDraw = pamiImg;
    width_mm = pamiWidth_mm;
    height_mm = pamiHeight_mm;
    hitbox_mm = pamiHitbox_mm;
  } else {
    imgToDraw = robotImg;
    width_mm = robotWidth_mm;
    height_mm = robotHeight_mm;
    hitbox_mm = robotHitbox_mm;
  }

  if (imgToDraw == null) return;

  float a = radians(angleDeg);

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(px, py);

  // même convention que la simulation
  pg.rotate(a - HALF_PI);

  pg.imageMode(CENTER);

  pg.fill(0, 255, 255, 35);
  pg.stroke(0, 255, 255);
  pg.strokeWeight(2.0 / mmToPx);
  pg.ellipse(0, 0, hitbox_mm, hitbox_mm);

  pg.tint(255, 180);
  pg.image(imgToDraw, 0, 0, width_mm, height_mm);
  pg.noTint();

  pg.popStyle();
  pg.popMatrix();
}

  boolean isHovered(float mouseX, float mouseY, float scale) {
    float px = x_mm * scale;
    float py = y_mm * scale;
    return dist(mouseX, mouseY, px, py) < 12;
  }
}
