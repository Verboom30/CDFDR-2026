class StrategyPoint {
  int id;
  float x_mm, y_mm;
  String poiName = null;

  boolean useAlign = false;
  String compass = "";
  String orientation = "";
  int customAngle = 0;

  StrategyPoint(int id, float x_mm, float y_mm) {
    this.id = id;
    this.x_mm = x_mm;
    this.y_mm = y_mm;

    this.useAlign = false;
    this.compass = "A";
    this.orientation = "CUSTOM";
    this.customAngle = 0;
  }

  void draw(PGraphics pg, float scale) {
    float px = x_mm;
    float py = y_mm;

    if (this == selectedPoint) {
      pg.stroke(255, 0, 0);
      pg.strokeWeight(3.0 / mmToPx);
      pg.fill(255, 200, 200);
    } else {
      pg.fill(255, 0, 0);
      pg.stroke(0);
      pg.strokeWeight(1.0 / mmToPx);
    }

    pg.ellipse(px, py, 18.0 / mmToPx, 18.0 / mmToPx);

    pg.textFont(font);
    pg.textAlign(CENTER, CENTER);
    pg.textSize(22.0 / mmToPx);

    String label = "P" + id;
    float tx = px;
    float ty = py - 16.0 / mmToPx;

    // contour noir
    pg.fill(0);
    pg.text(label, tx - 1.0 / mmToPx, ty);
    pg.text(label, tx + 1.0 / mmToPx, ty);
    pg.text(label, tx, ty - 1.0 / mmToPx);
    pg.text(label, tx, ty + 1.0 / mmToPx);

    // violet
    pg.fill(180, 0, 255);
    pg.text(label, tx, ty);
  }

  boolean isHovered(float mouseX, float mouseY, float scale) {
    float px = x_mm * scale;
    float py = y_mm * scale;
    return dist(mouseX, mouseY, px, py) < 10;
  }
}
