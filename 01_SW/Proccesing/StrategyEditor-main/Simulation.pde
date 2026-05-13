public static PVector robotPos;

public static int currentSegment = 0;
public static float t = 0.0;
public static boolean isSimulating = false;

public static float robotSpeed_mm_per_sec = 300.0;
public static float frameRateSim = 60.0;

// angle robot en degrés robot : 0 haut, 90 droite, 180 bas, -90 gauche
public static float robotAngleDeg = 0.0;
public static float rotationSpeedDeg = 8.0;
public static boolean rotatingAtPoint = false;
public static float angleToleranceDeg = 2.0;

// compatibilité éventuelle
public static float robotAngle = 0.0;

void updateSimulation() {
  if (!isSimulating || StrategyEditor.points.size() < 2) return;

  if (currentSegment >= StrategyEditor.points.size() - 1) {
    isSimulating = false;
    println("[SIM] End of path");
    return;
  }

  if (rotatingAtPoint) {
    float targetAngle;

    if (differentialRobotMode) {
      if (currentSegment < StrategyEditor.points.size() - 1) {
        StrategyPoint p1 = StrategyEditor.points.get(currentSegment);
        StrategyPoint p2 = StrategyEditor.points.get(currentSegment + 1);
        targetAngle = headingTo(p1.x_mm, p1.y_mm, p2.x_mm, p2.y_mm);
      } else {
        targetAngle = robotAngleDeg;
      }
    } else {
      StrategyPoint p = StrategyEditor.points.get(currentSegment);
      targetAngle = p.angleDeg;
    }

    robotAngleDeg = approachAngle(robotAngleDeg, targetAngle, rotationSpeedDeg);
    robotAngle = robotAngleDeg;

    if (abs(shortestAngleDiffDeg(robotAngleDeg, targetAngle)) < angleToleranceDeg) {
      robotAngleDeg = targetAngle;
      robotAngle = robotAngleDeg;
      rotatingAtPoint = false;
      t = 0.0;
    }

    return;
  }

  StrategyPoint p1 = StrategyEditor.points.get(currentSegment);
  StrategyPoint p2 = StrategyEditor.points.get(currentSegment + 1);

  float dist_mm = dist(p1.x_mm, p1.y_mm, p2.x_mm, p2.y_mm);
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

  if (robotPos == null) {
    robotPos = new PVector(x, y);
  } else {
    robotPos.set(x, y);
  }

  t += deltaT;

  if (t >= 1.0) {
    robotPos.set(p2.x_mm, p2.y_mm);
    currentSegment++;
    t = 0.0;

    if (currentSegment < StrategyEditor.points.size()) {
      float targetAngle;

      if (differentialRobotMode) {
        if (currentSegment < StrategyEditor.points.size() - 1) {
          StrategyPoint a = StrategyEditor.points.get(currentSegment);
          StrategyPoint b = StrategyEditor.points.get(currentSegment + 1);
          targetAngle = headingTo(a.x_mm, a.y_mm, b.x_mm, b.y_mm);
        } else {
          targetAngle = robotAngleDeg;
        }
      } else {
        StrategyPoint arrivedPoint = StrategyEditor.points.get(currentSegment);
        targetAngle = arrivedPoint.angleDeg;
      }

      if (abs(shortestAngleDiffDeg(robotAngleDeg, targetAngle)) > angleToleranceDeg) {
        rotatingAtPoint = true;
      } else {
        robotAngleDeg = targetAngle;
        robotAngle = robotAngleDeg;
        rotatingAtPoint = false;
      }
    }
  }
}

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

  float px = robotPos.x * scale;
  float py = robotPos.y * scale;

  pg.pushMatrix();
  pg.pushStyle();

  pg.translate(px, py);
  pg.rotate(robotImageToRenderRad(robotAngleDeg));
  pg.imageMode(CENTER);

  pg.fill(255, 0, 0, 70);
  pg.stroke(255, 0, 0);
  pg.strokeWeight(2.0 / mmToPx);
  pg.ellipse(0, 0, hitbox_mm * scale, hitbox_mm * scale);

  pg.image(imgToDraw, 0, 0, width_mm * scale, height_mm * scale);

  pg.popStyle();
  pg.popMatrix();
}

float shortestAngleDiffDeg(float a, float b) {
  float diff = b - a;
  while (diff > 180) diff -= 360;
  while (diff <= -180) diff += 360;
  return diff;
}

float approachAngle(float current, float target, float maxStepDeg) {
  float diff = shortestAngleDiffDeg(current, target);

  if (abs(diff) <= maxStepDeg) return target;
  return current + (diff > 0 ? maxStepDeg : -maxStepDeg);
}

float easeInOut(float t) {
  return t * t * (3.0 - 2.0 * t);
}
