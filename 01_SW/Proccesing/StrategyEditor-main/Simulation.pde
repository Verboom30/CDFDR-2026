public static PVector robotPos;

public static int currentSegment = 0;
public static float t = 0.0;
public static boolean isSimulating = false;

public static float robotWidth_mm = 287;
public static float robotHeight_mm = 284;
public static float robotHitbox_mm = 400;

public static float robotSpeed_mm_per_sec = 1000.0;
public static float frameRateSim = 50.0;

// orientation robot en radians
public static float robotAngle = 0.0;

// vitesse de rotation
public static float rotationSpeed = 0.2;

// phase rotation sur point
public static boolean rotatingAtPoint = false;

PImage robotImg;


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

    StrategyPoint p = StrategyEditor.points.get(currentSegment);

    float targetAngle = radians(p.angleDeg);

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

  float robotWidth_px  = robotWidth_mm * scale;
  float robotHeight_px = robotHeight_mm * scale;
  float hitbox_px      = robotHitbox_mm * scale;

  float px = robotPos.x * scale;
  float py = robotPos.y * scale;

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(px, py);

  // si png pointe vers le haut :
  pg.rotate(robotAngle + HALF_PI);

  pg.imageMode(CENTER);

  // hitbox
  pg.fill(255, 0, 0, 70);
  pg.stroke(255, 0, 0);
  pg.strokeWeight(2.0 / mmToPx);
  pg.ellipse(0, 0, hitbox_px, hitbox_px);

  // robot
  pg.image(robotImg, 0, 0, robotWidth_px, robotHeight_px);

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
