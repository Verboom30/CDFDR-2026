import controlP5.*;
import java.io.File;

ControlP5 cp5;

StrategyPoint selected = null;

Textlabel labelInfo;

Textfield fieldX;
Textfield fieldY;
Textfield fieldAngle;

boolean updatingFromGUI = false;

Toggle toggleAddEnabled;
boolean addPointEnabled = true;

Toggle toggleShowOverlay;
Toggle toggleDifferentialRobot;
Toggle togglePAMI;

int lastAutoSave = 0;
int autoSaveInterval = 30 * 1000;


public class StrategyEditorGUI extends PApplet {

  StrategyEditor mainApp;

  public void settings() {
    size(400, 800);
  }

  public void setup() {

    surface.setTitle("StrategyEditor - GUI");
    surface.setLocation(1320, 100);

    cp5 = new ControlP5(this);

    labelInfo = cp5.addTextlabel("labelInfo")
      .setText("No point selected")
      .setPosition(20, 20)
      .setSize(360, 100)
      .setColorValue(color(0, 102, 153));

    fieldX = cp5.addTextfield("x_mm")
      .setPosition(20, 100)
      .setSize(100, 30)
      .setAutoClear(false)
      .setLabel("X (mm)")
      .setText("0");
    fieldX.getCaptionLabel().setColor(color(0, 102, 153));

    fieldY = cp5.addTextfield("y_mm")
      .setPosition(140, 100)
      .setSize(100, 30)
      .setAutoClear(false)
      .setLabel("Y (mm)")
      .setText("0");
    fieldY.getCaptionLabel().setColor(color(0, 102, 153));

    fieldAngle = cp5.addTextfield("angleDeg")
      .setPosition(260, 100)
      .setSize(100, 30)
      .setAutoClear(false)
      .setLabel("Angle °")
      .setText("0");
    fieldAngle.getCaptionLabel().setColor(color(0, 102, 153));

    cp5.addTextlabel("helpAngle")
      .setText("SHIFT + drag sur point = orientation")
      .setPosition(20, 145)
      .setColorValue(color(0, 102, 153));

    toggleAddEnabled = cp5.addToggle("addPointEnabled")
      .setPosition(20, 190)
      .setSize(20, 20)
      .setLabel("Add points enabled")
      .setValue(true);
    toggleAddEnabled.getCaptionLabel().setColor(color(0, 102, 153));

    cp5.addButton("selectPrevPoint")
      .setLabel("Previous point")
      .setPosition(20, 240)
      .setSize(120, 30);

    cp5.addButton("selectNextPoint")
      .setLabel("Next point")
      .setPosition(160, 240)
      .setSize(120, 30);

    cp5.addButton("deleteSelectedPoint")
      .setLabel("Delete selected")
      .setPosition(20, 280)
      .setSize(260, 30);

    cp5.addButton("saveStrategy")
      .setLabel("Save strategy")
      .setPosition(20, 340)
      .setSize(120, 30);

    cp5.addButton("loadStrategy")
      .setLabel("Load strategy")
      .setPosition(160, 340)
      .setSize(120, 30);

    cp5.addButton("resetStrategy")
      .setLabel("Reset strategy")
      .setPosition(20, 390)
      .setSize(120, 30);

    cp5.addButton("reloadTempStrategy")
      .setLabel("Reload temp strategy")
      .setPosition(20, 430)
      .setSize(260, 30);

    cp5.addButton("startSimulation")
      .setLabel("Start simulation")
      .setPosition(20, 500)
      .setSize(120, 30);

    toggleDifferentialRobot = cp5.addToggle("differentialRobotMode")
      .setPosition(160, 500)
      .setSize(20, 20)
      .setValue(false)
      .setLabel("Robot différentiel");
    toggleDifferentialRobot.getCaptionLabel().setColor(color(0, 102, 153));

    togglePAMI = cp5.addToggle("usePAMI")
      .setPosition(160, 540)
      .setSize(20, 20)
      .setValue(false)
      .setLabel("PAMI");
    togglePAMI.getCaptionLabel().setColor(color(0, 102, 153));

    toggleShowOverlay = cp5.addToggle("showOverlay")
      .setPosition(20, 590)
      .setSize(20, 20)
      .setValue(false)
      .setLabel("Show table overlay");
    toggleShowOverlay.getCaptionLabel().setColor(color(0, 102, 153));

    reloadTempStrategy();
  }

  public void draw() {
    background(220);

    if (millis() - lastAutoSave > autoSaveInterval) {
      savePointsToFile("strategy_temp.json");
      lastAutoSave = millis();
      println("[GUI] Auto-save");
    }
  }

  public void updateLabelInfo() {
    if (selected == null) return;

    labelInfo.setText(
      "Point P" + selected.id +
      "\nX: " + nf(selected.x_mm, 0, 0) + " mm" +
      "\nY: " + nf(selected.y_mm, 0, 0) + " mm" +
      "\nPOI: " + selected.poiName +
      "\nAngle: " + nf(selected.angleDeg, 0, 1) + "°"
    );
  }

  public void setSelectedPoint(StrategyPoint p) {
    selected = p;

    if (selected != null) {
      updatingFromGUI = true;

      updateLabelInfo();

      fieldX.setText(nf(selected.x_mm, 0, 0));
      fieldY.setText(nf(selected.y_mm, 0, 0));
      fieldAngle.setText(nf(selected.angleDeg, 0, 1));

      updatingFromGUI = false;
    } else {
      labelInfo.setText("No point selected");
      fieldX.setText("");
      fieldY.setText("");
      fieldAngle.setText("");
    }
  }

  public void controlEvent(ControlEvent e) {
    if (updatingFromGUI) return;

    if (e.getName().equals("addPointEnabled")) {
      addPointEnabled = e.getValue() == 1;
      return;
    }

    if (e.getName().equals("differentialRobotMode")) {
      StrategyEditor.differentialRobotMode = e.getValue() == 1;
      println("[GUI] Robot différentiel: " + StrategyEditor.differentialRobotMode);
      return;
    }

    if (e.getName().equals("usePAMI")) {
      StrategyEditor.usePAMI = e.getValue() == 1;
      println("[GUI] PAMI: " + StrategyEditor.usePAMI);
      return;
    }

    if (selected == null) return;

    switch(e.getName()) {

    case "x_mm":
      try {
        selected.x_mm = constrain(
          Float.parseFloat(e.getStringValue()),
          0, 3000
        );
      }
      catch(Exception ex) {
      }
      break;

    case "y_mm":
      try {
        selected.y_mm = constrain(
          Float.parseFloat(e.getStringValue()),
          0, 2000
        );
      }
      catch(Exception ex) {
      }
      break;

    case "angleDeg":
      try {
        float a = Float.parseFloat(e.getStringValue());

        while (a < 0) a += 360;
        while (a >= 360) a -= 360;

        selected.angleDeg = a;
      }
      catch(Exception ex) {
      }
      break;
    }

    updateLabelInfo();
  }

  public boolean isAddPointEnabled() {
    return addPointEnabled;
  }

  public void setMainApp(StrategyEditor app) {
    this.mainApp = app;
  }

  public void saveStrategy() {
    savePointsToFile("strategy_temp.json");
    selectOutput("Save strategy to...", "saveStrategyToFile");
  }

  public void saveStrategyToFile(File selection) {
    if (selection == null) return;

    String path = selection.getAbsolutePath();

    if (!path.toLowerCase().endsWith(".json")) {
      path += ".json";
    }

    savePointsToFile(path);
  }

  public void savePointsToFile(String path) {
    JSONObject data = exportPointsToJSON();

    if (path.startsWith("/") || path.contains(":")) {
      saveJSONObject(data, path);
    } else {
      saveJSONObject(data, mainApp.getDataPath(path));
    }
  }

  public JSONObject exportPointsToJSON() {
    JSONObject data = new JSONObject();
    JSONArray list = new JSONArray();

    for (StrategyPoint p : StrategyEditor.points) {
      JSONObject entry = new JSONObject();

      entry.setInt("id", p.id);
      entry.setFloat("x_mm", p.x_mm);
      entry.setFloat("y_mm", p.y_mm);
      entry.setFloat("angleDeg", p.angleDeg);

      if (p.poiName != null) {
        entry.setString("poi", p.poiName);
      }

      list.append(entry);
    }

    data.setJSONArray("strategy", list);

    return data;
  }

  public void loadStrategy() {
    selectInput("Select strategy...", "loadStrategyFromFile");
  }

  public void reloadTempStrategy() {
    File f = new File(mainApp.getDataPath("strategy_temp.json"));
    loadStrategyFromFile(f);
  }

  public void loadStrategyFromFile(File selection) {
    if (selection == null) return;

    JSONObject data = loadJSONObject(selection.getAbsolutePath());
    JSONArray list = data.getJSONArray("strategy");

    StrategyEditor.points.clear();

    for (int i = 0; i < list.size(); i++) {
      JSONObject entry = list.getJSONObject(i);

      StrategyPoint p = new StrategyPoint(
        entry.getInt("id"),
        entry.getFloat("x_mm"),
        entry.getFloat("y_mm")
      );

      if (entry.hasKey("poi"))
        p.poiName = entry.getString("poi");

      if (entry.hasKey("angleDeg"))
        p.angleDeg = entry.getFloat("angleDeg");

      StrategyEditor.points.add(p);
    }

    mainApp.renumerotePoints();
  }

  public void resetStrategy() {
    int confirm =
      javax.swing.JOptionPane.showConfirmDialog(
      null,
      "Delete all points ?",
      "Confirm Reset",
      javax.swing.JOptionPane.YES_NO_OPTION
      );

    if (confirm == javax.swing.JOptionPane.YES_OPTION) {
      StrategyEditor.points.clear();
      mainApp.renumerotePoints();
      setSelectedPoint(null);
    }
  }

  public void deleteSelectedPoint() {
    if (selected == null) {
      println("[GUI] No selected point.");
      return;
    }

    StrategyEditor.points.remove(selected);

    mainApp.renumerotePoints();

    selected = null;
    StrategyEditor.selectedPoint = null;

    setSelectedPoint(null);

    println("[GUI] Selected point deleted.");
  }

  public void selectPrevPoint() {
    if (StrategyEditor.points.size() == 0) return;

    if (selected == null) {
      selected = StrategyEditor.points.get(0);
    } else {
      int i = StrategyEditor.points.indexOf(selected);
      if (i > 0)
        selected = StrategyEditor.points.get(i - 1);
    }

    setSelectedPoint(selected);
    StrategyEditor.selectedPoint = selected;
  }

  public void selectNextPoint() {
    if (StrategyEditor.points.size() == 0) return;

    if (selected == null) {
      selected = StrategyEditor.points.get(0);
    } else {
      int i = StrategyEditor.points.indexOf(selected);
      if (i < StrategyEditor.points.size() - 1)
        selected = StrategyEditor.points.get(i + 1);
    }

    setSelectedPoint(selected);
    StrategyEditor.selectedPoint = selected;
  }

  public void startSimulation() {
    if (StrategyEditor.points.size() < 2) {
      println("[GUI] Need at least 2 points.");
      return;
    }

    StrategyEditor.currentSegment = 0;
    StrategyEditor.t = 0.0;
    StrategyEditor.rotatingAtPoint = false;

    StrategyEditor.robotPos = new PVector(
      StrategyEditor.points.get(0).x_mm,
      StrategyEditor.points.get(0).y_mm
    );

    if (StrategyEditor.differentialRobotMode) {
      StrategyPoint p0 = StrategyEditor.points.get(0);
      StrategyPoint p1 = StrategyEditor.points.get(1);

      StrategyEditor.robotAngle = atan2(
        p1.y_mm - p0.y_mm,
        p1.x_mm - p0.x_mm
      );
    } else {
      StrategyEditor.robotAngle =
        radians(StrategyEditor.points.get(0).angleDeg);
    }

    StrategyEditor.isSimulating = true;

    println("[GUI] Simulation started.");
  }

  public void showOverlay(boolean val) {
    showOverlay = val;
  }
}
