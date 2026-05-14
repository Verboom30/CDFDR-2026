class StrategyPoint {

  int id;
  float x_mm;
  float y_mm;
  String poiName = null;

  // angle robot :
  // 0=haut, 90=droite, 180=bas, -90=gauche
  float angleDeg = 0;

  StrategyPoint(int id, float x_mm, float y_mm) {
    this.id = id;
    this.x_mm = x_mm;
    this.y_mm = y_mm;
    this.angleDeg = 0;
  }

  void draw(PGraphics pg, float scale) {
    float px = strategyX(this);
    float py = strategyY(this);

    if (this == selectedPoint) {
      drawRobotPreview(pg, px, py);
    }

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
    pg.pushMatrix();
    pg.translate(px, py);
    pg.scale(1, -1);

    pg.textFont(font);
    pg.textAlign(CENTER, CENTER);
    pg.textSize(22.0 / mmToPx);

    String txt = "P" + id;
    float tx = 0;
    float ty = -22.0 / mmToPx;

    pg.fill(0);
    pg.text(txt, tx - 1.0 / mmToPx, ty);
    pg.text(txt, tx + 1.0 / mmToPx, ty);
    pg.text(txt, tx, ty - 1.0 / mmToPx);
    pg.text(txt, tx, ty + 1.0 / mmToPx);

    pg.fill(180, 0, 255);
    pg.text(txt, tx, ty);

    pg.popMatrix();
  }

  void drawAngleArrow(PGraphics pg, float px, float py) {
    float a = radians(strategyAngle(this));
    float len = 40.0 / mmToPx;

    float x2 = px + sin(a) * len;
    float y2 = py + cos(a) * len;

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
    pg.rotate(robotHeadingToRenderRad(strategyAngle(this)));
    pg.noStroke();
    pg.triangle(
      0, 0,
      -10.0 / mmToPx, -4.0 / mmToPx,
      -10.0 / mmToPx,  4.0 / mmToPx
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

    pg.pushMatrix();
    pg.pushStyle();

    pg.translate(px, py);
    pg.rotate(robotImageToRenderRad(strategyAngle(this)));
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
}
