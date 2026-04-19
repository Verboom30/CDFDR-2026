class POI {

  String name;
  float x, y;

  POI(String name, float x, float y) {
    this.name = name;
    this.x = x;
    this.y = y;
  }

  void draw(PGraphics pg, float scale) {
    float px = x;
    float py = y;

    pg.fill(255, 200, 0);
    pg.stroke(0);
    pg.strokeWeight(1.0 / mmToPx);
    pg.ellipse(px, py, 22.0 / mmToPx, 22.0 / mmToPx);

    pg.pushMatrix();
    pg.translate(px, py);
    pg.scale(1, -1);

    pg.textAlign(CENTER, CENTER);
    pg.textSize(20.0 / mmToPx);

    float tx = 0;
    float ty = -28.0 / mmToPx;

    pg.fill(0);
    pg.text(name, tx - 1.0 / mmToPx, ty);
    pg.text(name, tx + 1.0 / mmToPx, ty);
    pg.text(name, tx, ty - 1.0 / mmToPx);
    pg.text(name, tx, ty + 1.0 / mmToPx);

    pg.fill(0, 255, 0);
    pg.text(name, tx, ty);

    pg.popMatrix();
  }
}
