public static PVector robotPos;

public static int currentSegment = 0;
public static float t = 0.0;
public static boolean isSimulating = false;

public static float robotWidth_mm = 400;
public static float robotHeight_mm = 400;
public static float robotHitbox_mm = 400;

public static float pamiWidth_mm = 120;
public static float pamiHeight_mm = 220;
public static float pamiHitbox_mm = 220;

public static float robotSpeed_mm_per_sec = 1500.0;
public static float frameRateSim = 60.0;

// orientation robot en radians
public static float robotAngle = 0.0;

// vitesse de rotation
public static float rotationSpeed = 0.2;

// phase rotation sur point
public static boolean rotatingAtPoint = false;

public static boolean differentialRobotMode = false;

PImage robotImg;
PImage pamiImg;


// ==========================================
// UPDATE SIMULATION
// ==========================================

void updateSimulation() {

  if (!isSimulating || StrategyEditor.points.size() < 2) return;

  // fin du trajet
  if (currentSegment >= StrategyEditor.points.size() - 1) {
    isSimulating = false;
    println("[SIM] End of path");
    return;
  }

  // ======================================
  // ROTATION SUR PLACE
  // ======================================
  if (rotatingAtPoint) {
  
    float targetAngle;
  
    if (differentialRobotMode) {
      // orientation vers le prochain point
      if (currentSegment < StrategyEditor.points.size() - 1) {
        StrategyPoint p1 = StrategyEditor.points.get(currentSegment);
        StrategyPoint p2 = StrategyEditor.points.get(currentSegment + 1);
  
        targetAngle = atan2(
          p2.y_mm - p1.y_mm,
          p2.x_mm - p1.x_mm
        );
      } else {
        targetAngle = robotAngle;
      }
    } else {
      // mode normal : angle défini sur le point
      StrategyPoint p = StrategyEditor.points.get(currentSegment);
      targetAngle = radians(p.angleDeg);
    }
  
    robotAngle = lerpAngle(
      robotAngle,
      targetAngle,
      rotationSpeed
    );
  
    if (abs(shortestAngleDiff(robotAngle, targetAngle)) < 0.02) {
      robotAngle = targetAngle;
      rotatingAtPoint = false;
      t = 0.0;
    }
  
    return;
  }

  // ======================================
  // DEPLACEMENT
  // ======================================

  StrategyPoint p1 = StrategyEditor.points.get(currentSegment);
  StrategyPoint p2 = StrategyEditor.points.get(currentSegment + 1);

  float dist_mm = dist(
    p1.x_mm, p1.y_mm,
    p2.x_mm, p2.y_mm
  );

  if (dist_mm < 1) {
    currentSegment++;
    return;
  }

  float duration_sec = dist_mm / robotSpeed_mm_per_sec;
  float totalFrames = max(1, duration_sec * frameRateSim);

  float deltaT = 1.0 / totalFrames;

  float tt = easeInOut(t);

  float x = lerp(p1.x_mm, p2.x_mm, tt);
  float y = lerp(p1.y_mm, p2.y_mm, tt);

  robotPos.set(x, y);

  t += deltaT;

  // arrivé au point
  if (t >= 1.0) {

    robotPos.set(p2.x_mm, p2.y_mm);

    currentSegment++;

    t = 0.0;

    // si point suivant existe => rotation
    if (currentSegment < StrategyEditor.points.size()) {
      rotatingAtPoint = true;
    }
  }
}



// ==========================================
// DRAW ROBOT
// ==========================================

void drawRobot(PGraphics pg, float scale) {

  if (!isSimulating || robotPos == null) return;

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

  float objWidth_px  = width_mm * scale;
  float objHeight_px = height_mm * scale;
  float hitbox_px    = hitbox_mm * scale;

  float px = robotPos.x * scale;
  float py = robotPos.y * scale;

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(px, py);

  // ajuste ici si nécessaire selon l'orientation native de tes PNG
  pg.rotate(robotAngle - HALF_PI);

  pg.imageMode(CENTER);

  pg.fill(255, 0, 0, 70);
  pg.stroke(255, 0, 0);
  pg.strokeWeight(2.0 / mmToPx);
  pg.ellipse(0, 0, hitbox_px, hitbox_px);

  pg.image(imgToDraw, 0, 0, objWidth_px, objHeight_px);

  pg.popStyle();
  pg.popMatrix();
}



// ==========================================
// OUTILS ANGLES
// ==========================================

float lerpAngle(float a, float b, float t) {

  float diff = shortestAngleDiff(a, b);

  return a + diff * t;
}

float shortestAngleDiff(float a, float b) {

  float diff = b - a;

  while (diff > PI) diff -= TWO_PI;
  while (diff < -PI) diff += TWO_PI;

  return diff;
}



// ==========================================
// EASING
// ==========================================

float easeInOut(float t) {
  return t * t * (3.0 - 2.0 * t);
}
