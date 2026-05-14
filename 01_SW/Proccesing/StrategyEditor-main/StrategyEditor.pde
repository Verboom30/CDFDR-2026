PGraphics terrainView;
StrategyEditorGUI gui;
PImage terrainImage;

PImage robotImg;
PImage pamiImg;

public static boolean usePAMI = false;
public static boolean differentialRobotMode = false;
public static boolean blueTeamMode = false;

// corrige uniquement l'orientation du PNG
public static float imageHeadingOffsetDeg = 90;

public static float robotWidth_mm = 390;
public static float robotHeight_mm =390;
public static float robotHitbox_mm = 450;

public static float pamiWidth_mm = 120;
public static float pamiHeight_mm = 220;
public static float pamiHitbox_mm = 220;

int nextPointId = 0;
public static ArrayList<StrategyPoint> points = new ArrayList<StrategyPoint>();

public static float mmToPx = 0.4;

public static final int TERRAIN_W_MM = 3000;
public static final int TERRAIN_H_MM = 2000;

int screenW;
int screenH;

PFont font;
public static StrategyPoint selectedPoint = null;
boolean isDragging = false;

ArrayList<POI> pois = new ArrayList<POI>();

CodePreviewWindow previewWindow = null;

// caméra
float viewOffsetX = 0;
float viewOffsetY = 0;
boolean isPanning = false;
float lastMouseX, lastMouseY;

// overlay
boolean showOverlay = false;

void settings() {
  screenW = round(TERRAIN_W_MM * mmToPx);
  screenH = round(TERRAIN_H_MM * mmToPx);
  size(screenW, screenH);
}

void setup() {
  surface.setTitle("StrategyEditor - Terrain View");
  surface.setLocation(100, 100);

  font = createFont("Arial Bold", 22);

  gui = new StrategyEditorGUI();
  gui.setMainApp(this);
  PApplet.runSketch(new String[] { "GUI" }, gui);

  previewWindow = new CodePreviewWindow("waiting");
  PApplet.runSketch(new String[] { "CodePreview" }, previewWindow);

  terrainView = createGraphics(screenW, screenH);

  terrainImage = loadImage("terrain.png");
  robotImg = loadImage("robot.png");
  pamiImg = loadImage("PAMI.png");

  loadOverlayResources();

  loadPOIs("poi.h");
  debugPrintPOIs();
}

void draw() {
  background(220);
  drawTerrain();

  if (isSimulating) {
    updateSimulation();
  }

  drawMouseCoordinates();
}

// =====================================================
// CONVERSIONS MONDE <-> ECRAN
// monde : origine bas-gauche
// écran : origine haut-gauche
// =====================================================

float worldToScreenX(float xMm) {
  return viewOffsetX + xMm * mmToPx;
}

float worldToScreenY(float yMm) {
  return viewOffsetY + (TERRAIN_H_MM - yMm) * mmToPx;
}

float screenToWorldX(float xScreen) {
  return (xScreen - viewOffsetX) / mmToPx;
}

float screenToWorldY(float yScreen) {
  return TERRAIN_H_MM - ((yScreen - viewOffsetY) / mmToPx);
}

// =====================================================
// ANGLES ROBOT
// 0 = haut, 90 = droite, 180 = bas, -90 = gauche
// =====================================================

float headingTo(float x1, float y1, float x2, float y2) {
  return degrees(atan2(x2 - x1, y2 - y1));
}

float robotHeadingToRenderRad(float angleDeg) {
  return radians(90 - angleDeg);
}

float robotImageToRenderRad(float angleDeg) {
  return radians(90 - (angleDeg + imageHeadingOffsetDeg));
}

float normalizeAngle180(float a) {
  while (a > 180) a -= 360;
  while (a <= -180) a += 360;
  return a;
}

// =====================================================
// TRANSFORMATION EQUIPE
// Points stockés en coordonnées JAUNE.
// En mode BLEU : X = 3000 - X, Y = Y, Theta = -Theta.
// =====================================================

float strategyX(StrategyPoint p) {
  return blueTeamMode ? TERRAIN_W_MM - p.x_mm : p.x_mm;
}

float strategyY(StrategyPoint p) {
  return p.y_mm;
}

float strategyAngle(StrategyPoint p) {
  return blueTeamMode ? normalizeAngle180(-p.angleDeg) : p.angleDeg;
}

float inverseStrategyX(float xMm) {
  return blueTeamMode ? TERRAIN_W_MM - xMm : xMm;
}

float inverseStrategyY(float yMm) {
  return yMm;
}

float inverseStrategyAngle(float angleDeg) {
  return blueTeamMode ? normalizeAngle180(-angleDeg) : normalizeAngle180(angleDeg);
}

// =====================================================

void drawTerrain() {
  terrainView.beginDraw();
  terrainView.background(255);

  // repère monde : bas-gauche, y vers le haut
  terrainView.pushMatrix();
  terrainView.translate(viewOffsetX, viewOffsetY + TERRAIN_H_MM * mmToPx);
  terrainView.scale(mmToPx, -mmToPx);

  if (terrainImage != null) {
    terrainView.pushMatrix();
    terrainView.scale(1, -1);
    terrainView.image(terrainImage, 0, -TERRAIN_H_MM, TERRAIN_W_MM, TERRAIN_H_MM);
    terrainView.popMatrix();
  } else {
    terrainView.fill(0);
    terrainView.text("Image terrain.png introuvable", 20, 20);
  }

  terrainView.noFill();
  terrainView.stroke(255, 0, 0);
  terrainView.strokeWeight(4.0 / mmToPx);
  terrainView.rect(0, 0, TERRAIN_W_MM, TERRAIN_H_MM);

  drawRobot(terrainView, 1.0);
  drawPOIs();
  drawPath();
  drawPoints();

  terrainView.popMatrix();

  if (showOverlay) {
    terrainView.filter(BLUR, 1);
    terrainView.fill(0, 0, 0, 100);
    terrainView.noStroke();
    terrainView.rect(0, 0, terrainView.width, terrainView.height);
  }

  drawOverlay(terrainView, mmToPx);

  terrainView.endDraw();

  image(terrainView, 0, 0);
}

void drawPoints() {
  for (StrategyPoint p : points) {
    p.draw(terrainView, 1.0);
  }
}

void drawPOIs() {
  for (POI p : pois) {
    p.draw(terrainView, 1.0);
  }
}

void drawPath() {
  if (points.size() < 2) return;

  terrainView.stroke(0, 100, 255);
  terrainView.strokeWeight(2.0 / mmToPx);

  for (int i = 0; i < points.size() - 1; i++) {
    StrategyPoint p1 = points.get(i);
    StrategyPoint p2 = points.get(i + 1);

    float x1 = strategyX(p1);
    float y1 = strategyY(p1);
    float x2 = strategyX(p2);
    float y2 = strategyY(p2);

    terrainView.line(x1, y1, x2, y2);
    drawArrow(terrainView, x1, y1, x2, y2);
  }
}

void drawArrow(PGraphics pg, float x1, float y1, float x2, float y2) {
  float mx = (x1 + x2) / 2.0;
  float my = (y1 + y2) / 2.0;

  float dx = x2 - x1;
  float dy = y2 - y1;
  float angle = atan2(dy, dx);

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(mx, my);
  pg.rotate(angle);
  pg.scale(1.0 / mmToPx);

  pg.fill(0, 100, 255);
  pg.noStroke();
  pg.triangle(-12, -6, -12, 6, 0, 0);

  pg.popStyle();
  pg.popMatrix();
}

// =====================================================
// SOURIS
// =====================================================

void mousePressed() {
  if (mouseButton == CENTER) {
    isPanning = true;
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    return;
  }

  StrategyPoint clicked = getPointUnderMouse();

  // SHIFT = réglage angle uniquement
  if (keyPressed && keyCode == SHIFT) {
    if (clicked != null) {
      selectedPoint = clicked;
      if (gui != null) gui.setSelectedPoint(selectedPoint);
    }
    return;
  }

  if (mouseButton == LEFT) {
    if (clicked != null) {
      selectedPoint = clicked;
      isDragging = true;
      if (gui != null) gui.setSelectedPoint(selectedPoint);
    } else if (gui == null || gui.isAddPointEnabled()) {
      int insertIndex = getSegmentIndexUnderMouse();

      float displayX_mm = round(constrain(screenToWorldX(mouseX), 0, TERRAIN_W_MM));
      float displayY_mm = round(constrain(screenToWorldY(mouseY), 0, TERRAIN_H_MM));
      float x_mm = round(constrain(inverseStrategyX(displayX_mm), 0, TERRAIN_W_MM));
      float y_mm = round(constrain(inverseStrategyY(displayY_mm), 0, TERRAIN_H_MM));

      POI snap = getNearbyPOI(x_mm, y_mm, 50);
      if (snap != null) {
        x_mm = snap.x;
        y_mm = snap.y;
      }

      StrategyPoint newPoint = new StrategyPoint(nextPointId++, x_mm, y_mm);
      if (snap != null) newPoint.poiName = snap.name;

      if (insertIndex != -1) {
        points.add(insertIndex + 1, newPoint);
        renumerotePoints();
      } else {
        points.add(newPoint);
      }

      selectedPoint = newPoint;
      if (gui != null) gui.setSelectedPoint(selectedPoint);
    }
  }

  if (mouseButton == RIGHT && clicked != null) {
    points.remove(clicked);
    renumerotePoints();
    if (clicked == selectedPoint) {
      selectedPoint = null;
      if (gui != null) gui.setSelectedPoint(null);
    }
  }
}

void mouseDragged() {
  if (isPanning) {
    viewOffsetX += mouseX - lastMouseX;
    viewOffsetY += mouseY - lastMouseY;
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    return;
  }

  // SHIFT + drag = angle robot
  if (selectedPoint != null && keyPressed && keyCode == SHIFT) {
    float worldMouseX = screenToWorldX(mouseX);
    float worldMouseY = screenToWorldY(mouseY);

    float displayPointX = strategyX(selectedPoint);
    float displayPointY = strategyY(selectedPoint);
    float displayAngle = headingTo(displayPointX, displayPointY, worldMouseX, worldMouseY);
    selectedPoint.angleDeg = inverseStrategyAngle(displayAngle);

    if (gui != null) gui.setSelectedPoint(selectedPoint);
    return;
  }

  if (isDragging && selectedPoint != null) {
    float displayX_mm = round(constrain(screenToWorldX(mouseX), 0, TERRAIN_W_MM));
    float displayY_mm = round(constrain(screenToWorldY(mouseY), 0, TERRAIN_H_MM));
    float x_mm = round(constrain(inverseStrategyX(displayX_mm), 0, TERRAIN_W_MM));
    float y_mm = round(constrain(inverseStrategyY(displayY_mm), 0, TERRAIN_H_MM));

    POI snap = getNearbyPOI(x_mm, y_mm, 50);
    if (snap != null) {
      x_mm = snap.x;
      y_mm = snap.y;
      selectedPoint.poiName = snap.name;
    } else {
      selectedPoint.poiName = null;
    }

    selectedPoint.x_mm = x_mm;
    selectedPoint.y_mm = y_mm;

    if (gui != null) gui.setSelectedPoint(selectedPoint);
  }
}

void mouseReleased() {
  isDragging = false;
  isPanning = false;
}

void mouseWheel(processing.event.MouseEvent event) {
  float e = event.getCount();

  float zoomFactor = 1.0 - e * 0.05;
  float newScale = constrain(mmToPx * zoomFactor, 0.2, 3.0);

  float worldX = screenToWorldX(mouseX);
  float worldY = screenToWorldY(mouseY);

  mmToPx = newScale;

  viewOffsetX = mouseX - worldX * mmToPx;
  viewOffsetY = mouseY - (TERRAIN_H_MM - worldY) * mmToPx;
}

// =====================================================

void renumerotePoints() {
  nextPointId = 0;
  for (StrategyPoint p : points) {
    p.id = nextPointId++;
  }
}

StrategyPoint insertPointAfter(StrategyPoint previousPoint) {
  if (previousPoint == null) return null;

  int index = points.indexOf(previousPoint);
  if (index == -1) return null;

  float x_mm;
  float y_mm;
  float angleDeg;

  if (index < points.size() - 1) {
    StrategyPoint nextPoint = points.get(index + 1);

    // Si on ajoute entre P2 et P3, le nouveau point est placé au milieu.
    x_mm = round((previousPoint.x_mm + nextPoint.x_mm) / 2.0);
    y_mm = round((previousPoint.y_mm + nextPoint.y_mm) / 2.0);
    angleDeg = headingTo(previousPoint.x_mm, previousPoint.y_mm, nextPoint.x_mm, nextPoint.y_mm);
  } else {
    // Si c'est le dernier point, on ajoute un point 100 mm devant lui.
    float a = radians(previousPoint.angleDeg);
    x_mm = round(constrain(previousPoint.x_mm + sin(a) * 100, 0, TERRAIN_W_MM));
    y_mm = round(constrain(previousPoint.y_mm + cos(a) * 100, 0, TERRAIN_H_MM));
    angleDeg = previousPoint.angleDeg;
  }

  StrategyPoint newPoint = new StrategyPoint(nextPointId++, x_mm, y_mm);
  newPoint.angleDeg = normalizeAngle180(angleDeg);

  POI snap = getNearbyPOI(x_mm, y_mm, 50);
  if (snap != null) {
    newPoint.x_mm = snap.x;
    newPoint.y_mm = snap.y;
    newPoint.poiName = snap.name;
  }

  points.add(index + 1, newPoint);
  renumerotePoints();

  selectedPoint = newPoint;
  return newPoint;
}

StrategyPoint getPointUnderMouse() {
  float mx = screenToWorldX(mouseX);
  float my = screenToWorldY(mouseY);

  for (StrategyPoint p : points) {
    if (dist(mx, my, strategyX(p), strategyY(p)) < 15 / mmToPx) {
      return p;
    }
  }
  return null;
}

int getSegmentIndexUnderMouse() {
  float threshold = 10;

  float mx = screenToWorldX(mouseX);
  float my = screenToWorldY(mouseY);

  for (int i = 0; i < points.size() - 1; i++) {
    StrategyPoint p1 = points.get(i);
    StrategyPoint p2 = points.get(i + 1);

    float d = distToSegment(mx, my, strategyX(p1), strategyY(p1), strategyX(p2), strategyY(p2));
    if (d < threshold / mmToPx) return i;
  }

  return -1;
}

float distToSegment(float px, float py, float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  if (dx == 0 && dy == 0) return dist(px, py, x1, y1);

  float t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
  t = constrain(t, 0, 1);

  float projX = x1 + t * dx;
  float projY = y1 + t * dy;

  return dist(px, py, projX, projY);
}

public String getDataPath(String filename) {
  return sketchPath("data/" + filename);
}

// =====================================================
// POI
// =====================================================

void loadPOIs(String filename) {
  String[] lines = loadStrings(filename);
  if (lines == null) return;

  for (String line : lines) {
    line = line.trim();

    if (line.startsWith("const Vec2")) {
      String[] parts = line.split("=");
      if (parts.length != 2) continue;

      String name = parts[0].replace("const Vec2", "").trim();
      String coord = parts[1].trim();

      coord = coord.replace("Vec2(", "")
        .replace(");", "")
        .split("//")[0]
        .trim();

      String[] coords = coord.split(",");

      if (coords.length == 2) {
        float x = float(trim(coords[0]));
        float y = float(trim(coords[1]));
        pois.add(new POI(name, x, y));
      }
    }
  }
}

void debugPrintPOIs() {
  for (POI p : pois) {
    println(p.name + " = (" + p.x + ", " + p.y + ")");
  }
}

POI getNearbyPOI(float x_mm, float y_mm, float tolerance_mm) {
  for (POI poi : pois) {
    if (dist(x_mm, y_mm, poi.x, poi.y) <= tolerance_mm) {
      return poi;
    }
  }
  return null;
}

// =====================================================
// HUD
// =====================================================

void drawMouseCoordinates() {
  float x_mm = round(constrain(screenToWorldX(mouseX), 0, TERRAIN_W_MM));
  float y_mm = round(constrain(screenToWorldY(mouseY), 0, TERRAIN_H_MM));

  fill(255);
  stroke(0);
  rect(10, height - 40, 260, 24);

  fill(0);
  textAlign(LEFT, CENTER);
  textFont(createFont("Arial", 12));
  text(
    "X: " + int(x_mm) + " mm   Y: " + int(y_mm) + " mm   Zoom: " + nf(mmToPx, 1, 2),
    15,
    height - 28
  );
}
