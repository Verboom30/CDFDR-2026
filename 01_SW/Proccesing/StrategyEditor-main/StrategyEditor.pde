PGraphics terrainView;
StrategyEditorGUI gui;
PImage terrainImage;

// mode affichage robot
public static boolean usePAMI = false;

int nextPointId = 0;

public static ArrayList<StrategyPoint> points =
  new ArrayList<StrategyPoint>();

public static float mmToPx = 0.8;

final int TERRAIN_W_MM = 3000;
final int TERRAIN_H_MM = 2000;

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

float lastMouseX;
float lastMouseY;


// =====================================================
// SETTINGS
// =====================================================

void settings() {

  screenW = round(TERRAIN_W_MM * mmToPx);
  screenH = round(TERRAIN_H_MM * mmToPx);

  size(screenW, screenH);
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  surface.setTitle("StrategyEditor - Terrain View");
  surface.setLocation(100, 100);

  font = createFont("Arial Bold", 22);

  gui = new StrategyEditorGUI();
  gui.setMainApp(this);
  PApplet.runSketch(
    new String[] { "GUI" },
    gui
  );

  previewWindow =
    new CodePreviewWindow("waiting");

  PApplet.runSketch(
    new String[] { "CodePreview" },
    previewWindow
  );

  terrainView =
    createGraphics(screenW, screenH);

  terrainImage = loadImage("terrain.png");

  robotImg = loadImage("robot.png");
  pamiImg  = loadImage("PAMI.png");

  loadOverlayResources();

  loadPOIs("poi.h");
  debugPrintPOIs();
}


// =====================================================
// DRAW
// =====================================================

void draw() {

  background(220);

  drawTerrain();

  if (isSimulating)
    updateSimulation();

  drawMouseCoordinates();
}


// =====================================================
// CONVERSIONS
// =====================================================

float screenToMmX(float xScreen) {
  return (xScreen - viewOffsetX) / mmToPx;
}

float screenToMmY(float yScreen) {
  return (yScreen - viewOffsetY) / mmToPx;
}

float mmToScreenX(float xMm) {
  return xMm * mmToPx + viewOffsetX;
}

float mmToScreenY(float yMm) {
  return yMm * mmToPx + viewOffsetY;
}


// =====================================================
// DRAW TERRAIN
// =====================================================

void drawTerrain() {

  terrainView.beginDraw();

  terrainView.background(255);

  terrainView.pushMatrix();

  terrainView.translate(
    viewOffsetX,
    viewOffsetY
  );

  terrainView.scale(mmToPx);

  if (terrainImage != null) {

    terrainView.image(
      terrainImage,
      0,
      0,
      TERRAIN_W_MM,
      TERRAIN_H_MM
    );

  } else {

    terrainView.fill(0);
    terrainView.text(
      "Image terrain.png introuvable",
      20,
      20
    );
  }

  // contour rouge table
  terrainView.noFill();
  terrainView.stroke(255, 0, 0);
  terrainView.strokeWeight(4.0 / mmToPx);
  terrainView.rect(
    0,
    0,
    TERRAIN_W_MM,
    TERRAIN_H_MM
  );

  drawRobot(terrainView, 1.0);

  drawPOIs();
  drawPath();
  drawPoints();

  if (showOverlay) {

    terrainView.filter(BLUR, 1);

    terrainView.fill(0, 0, 0, 100);
    terrainView.noStroke();

    terrainView.rect(
      0,
      0,
      TERRAIN_W_MM,
      TERRAIN_H_MM
    );
  }

  drawOverlay(terrainView, 1.0);

  terrainView.popMatrix();

  terrainView.endDraw();

  image(terrainView, 0, 0);
}


// =====================================================
// DRAW OBJECTS
// =====================================================

void drawPoints() {
  for (StrategyPoint p : points)
    p.draw(terrainView, 1.0);
}

void drawPOIs() {
  for (POI p : pois)
    p.draw(terrainView, 1.0);
}


// =====================================================
// PATH
// =====================================================

void drawPath() {

  if (points.size() < 2) return;

  terrainView.stroke(0, 100, 255);
  terrainView.strokeWeight(2.0 / mmToPx);

  for (int i = 0; i < points.size() - 1; i++) {

    StrategyPoint p1 = points.get(i);
    StrategyPoint p2 = points.get(i + 1);

    terrainView.line(
      p1.x_mm,
      p1.y_mm,
      p2.x_mm,
      p2.y_mm
    );

    drawArrow(
      terrainView,
      p1.x_mm,
      p1.y_mm,
      p2.x_mm,
      p2.y_mm
    );
  }
}

void drawArrow(
  PGraphics pg,
  float x1,
  float y1,
  float x2,
  float y2
) {
  float mx = (x1 + x2) / 2.0;
  float my = (y1 + y2) / 2.0;

  float angle = atan2(y2 - y1, x2 - x1);

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(mx, my);
  pg.rotate(angle);
  pg.scale(1.0 / mmToPx);

  pg.fill(0, 100, 255);
  pg.noStroke();

  pg.triangle(
    -12, -6,
    -12,  6,
     0,   0
  );

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

  StrategyPoint clicked =
    getPointUnderMouse();

  // SHIFT = mode rotation
  if (keyPressed &&
    keyCode == SHIFT) {

    if (clicked != null) {
      selectedPoint = clicked;

      if (gui != null)
        gui.setSelectedPoint(
          selectedPoint
        );
    }

    return;
  }

  if (mouseButton == LEFT) {

    if (clicked != null) {

      selectedPoint = clicked;
      isDragging = true;

      if (gui != null)
        gui.setSelectedPoint(
          selectedPoint
        );

    } else if (
      gui == null ||
      gui.isAddPointEnabled()
      ) {

      int insertIndex =
        getSegmentIndexUnderMouse();

      float x_mm =
        round(screenToMmX(mouseX));

      float y_mm =
        round(screenToMmY(mouseY));

      POI snap =
        getNearbyPOI(
        x_mm,
        y_mm,
        50
        );

      if (snap != null) {
        x_mm = snap.x;
        y_mm = snap.y;
      }

      StrategyPoint newPoint =
        new StrategyPoint(
        nextPointId++,
        x_mm,
        y_mm
        );

      if (snap != null)
        newPoint.poiName =
          snap.name;

      if (insertIndex != -1) {

        points.add(
          insertIndex + 1,
          newPoint
        );

        renumerotePoints();

      } else {
        points.add(newPoint);
      }
    }
  }

  if (
    mouseButton == RIGHT &&
    clicked != null
    ) {

    points.remove(clicked);
    renumerotePoints();
  }
}


void mouseDragged() {

  // pan caméra
  if (isPanning) {

    viewOffsetX +=
      mouseX - lastMouseX;

    viewOffsetY +=
      mouseY - lastMouseY;

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    return;
  }

  // rotation point
  if (
    selectedPoint != null &&
    keyPressed &&
    keyCode == SHIFT
    ) {

    float px =
      mmToScreenX(
      selectedPoint.x_mm
      );

    float py =
      mmToScreenY(
      selectedPoint.y_mm
      );

    float a =
      atan2(
      mouseY - py,
      mouseX - px
      );

    selectedPoint.angleDeg =
      degrees(a);

    if (selectedPoint.angleDeg < 0)
      selectedPoint.angleDeg += 360;

    if (gui != null)
      gui.setSelectedPoint(
        selectedPoint
      );

    return;
  }

  // drag point
  if (
    isDragging &&
    selectedPoint != null
    ) {

    float x_mm =
      round(
      constrain(
      screenToMmX(mouseX),
      0,
      TERRAIN_W_MM
      )
      );

    float y_mm =
      round(
      constrain(
      screenToMmY(mouseY),
      0,
      TERRAIN_H_MM
      )
      );

    POI snap =
      getNearbyPOI(
      x_mm,
      y_mm,
      50
      );

    if (snap != null) {

      x_mm = snap.x;
      y_mm = snap.y;

      selectedPoint.poiName =
        snap.name;

    } else {
      selectedPoint.poiName =
        null;
    }

    selectedPoint.x_mm = x_mm;
    selectedPoint.y_mm = y_mm;

    if (gui != null)
      gui.setSelectedPoint(
        selectedPoint
      );
  }
}


void mouseReleased() {
  isDragging = false;
  isPanning = false;
}


void mouseWheel(
  processing.event.MouseEvent event
) {

  float e =
    event.getCount();

  float zoomFactor =
    1.0 - e * 0.05;

  float newScale =
    constrain(
    mmToPx * zoomFactor,
    0.2,
    3.0
    );

  float worldX =
    screenToMmX(mouseX);

  float worldY =
    screenToMmY(mouseY);

  mmToPx = newScale;

  viewOffsetX =
    mouseX - worldX * mmToPx;

  viewOffsetY =
    mouseY - worldY * mmToPx;
}


// =====================================================
// UTILS
// =====================================================

void renumerotePoints() {

  nextPointId = 0;

  for (StrategyPoint p : points)
    p.id = nextPointId++;
}

StrategyPoint getPointUnderMouse() {

  for (StrategyPoint p : points) {

    if (p.isHovered(
      mouseX - viewOffsetX,
      mouseY - viewOffsetY,
      mmToPx
      )) {
      return p;
    }
  }

  return null;
}


int getSegmentIndexUnderMouse() {

  float threshold = 10;

  float mx =
    mouseX - viewOffsetX;

  float my =
    mouseY - viewOffsetY;

  for (int i = 0; i < points.size() - 1; i++) {

    StrategyPoint p1 = points.get(i);
    StrategyPoint p2 = points.get(i + 1);

    float d =
      distToSegment(
      mx,
      my,
      p1.x_mm * mmToPx,
      p1.y_mm * mmToPx,
      p2.x_mm * mmToPx,
      p2.y_mm * mmToPx
      );

    if (d < threshold)
      return i;
  }

  return -1;
}


float distToSegment(
  float px,
  float py,
  float x1,
  float y1,
  float x2,
  float y2
) {

  float dx = x2 - x1;
  float dy = y2 - y1;

  if (dx == 0 && dy == 0)
    return dist(px, py, x1, y1);

  float t =
    ((px - x1) * dx +
    (py - y1) * dy) /
    (dx * dx + dy * dy);

  t = constrain(t, 0, 1);

  float projX =
    x1 + t * dx;

  float projY =
    y1 + t * dy;

  return dist(
    px,
    py,
    projX,
    projY
  );
}


public String getDataPath(
  String filename
) {
  return sketchPath(
    "data/" + filename
  );
}


// =====================================================
// POI
// =====================================================

void loadPOIs(String filename) {

  String[] lines =
    loadStrings(filename);

  if (lines == null) return;

  for (String line : lines) {

    line = line.trim();

    if (line.startsWith(
      "const Vec2"
      )) {

      String[] parts =
        line.split("=");

      if (parts.length != 2)
        continue;

      String name =
        parts[0]
        .replace(
        "const Vec2",
        ""
        )
        .trim();

      String coord =
        parts[1]
        .trim();

      coord =
        coord.replace(
        "Vec2(",
        ""
        )
        .replace(
        ");",
        ""
        )
        .split("//")[0]
        .trim();

      String[] coords =
        coord.split(",");

      if (coords.length == 2) {

        float x =
          float(
          trim(coords[0])
          );

        float y =
          float(
          trim(coords[1])
          );

        pois.add(
          new POI(
          name,
          x,
          y
          )
          );
      }
    }
  }
}


void debugPrintPOIs() {

  for (POI p : pois) {
    println(
      p.name +
      " = (" +
      p.x +
      "," +
      p.y +
      ")"
      );
  }
}


POI getNearbyPOI(
  float x_mm,
  float y_mm,
  float tolerance_mm
) {

  for (POI poi : pois) {

    if (
      dist(
      x_mm,
      y_mm,
      poi.x,
      poi.y
      ) <= tolerance_mm
      ) {
      return poi;
    }
  }

  return null;
}


// =====================================================
// HUD
// =====================================================

void drawMouseCoordinates() {

  float x_mm =
    round(
    screenToMmX(mouseX)
    );

  float y_mm =
    round(
    screenToMmY(mouseY)
    );

  fill(255);
  stroke(0);

  rect(
    10,
    height - 40,
    250,
    24
  );

  fill(0);

  textAlign(LEFT, CENTER);

  textFont(
    createFont(
    "Arial",
    12
    )
    );

  text(
    "X: " + int(x_mm) +
    " mm   Y: " +
    int(y_mm) +
    " mm   Zoom: " +
    nf(mmToPx, 1, 2),

    15,
    height - 28
    );
}
