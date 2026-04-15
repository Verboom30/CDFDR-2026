import controlP5.*;

ControlP5 cp5;

StrategyPoint selected = null;

Textlabel labelInfo;

Textfield fieldX, fieldY;
boolean updatingFromGUI = false;

Toggle toggleAddEnabled;
boolean addPointEnabled = true;

Toggle toggleShowOverlay;
Toggle toggleUseAlign;


int lastAutoSave = 0;
int autoSaveInterval = 30 * 1000; // xx seconds

public class StrategyEditorGUI extends PApplet {

  StrategyEditor mainApp;

  public void settings() {
    size(400, 800);
  }

  public void setup() {
    surface.setTitle("StrategyEditor - GUI");
    surface.setLocation(1320, 100);
    cp5 = new ControlP5(this);

    // Bloc 1 : Infos point sélectionné
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

    // Bloc 2 : Activation
    toggleAddEnabled = cp5.addToggle("addPointEnabled")
      .setPosition(20, 220)
      .setSize(20, 20)
      .setLabel("Add points enabled")
      .setValue(true);
    toggleAddEnabled.getCaptionLabel().setColor(color(0, 102, 153));

    // Bloc 3 : Navigation
    cp5.addButton("selectPrevPoint")
      .setLabel("Previous point")
      .setPosition(20, 270)
      .setSize(120, 30);
    cp5.addButton("selectNextPoint")
      .setLabel("Next point")
      .setPosition(160, 270)
      .setSize(120, 30);

    // Bloc 4 : Sauvegarde / chargement
    cp5.addButton("saveStrategy")
      .setLabel("Save strategy")
      .setPosition(20, 320)
      .setSize(120, 30);
    cp5.addButton("loadStrategy")
      .setLabel("Load strategy")
      .setPosition(160, 320)
      .setSize(120, 30);
    cp5.addButton("resetStrategy")
      .setLabel("Reset strategy")
      .setPosition(20, 370)
      .setSize(120, 30);
    cp5.addButton("reloadTempStrategy")
      .setLabel("Reload temp strategy")
      .setPosition(20, 410)
      .setSize(260, 30);

    // Bloc 5 : Simulation / overlay
    cp5.addButton("startSimulation")
      .setLabel("Start simulation")
      .setPosition(20, 470)
      .setSize(120, 30);

    toggleShowOverlay = cp5.addToggle("showOverlay")
      .setPosition(20, 520)
      .setSize(20, 20)
      .setValue(false)
      .setLabel("Show table overlay");
    toggleShowOverlay.getCaptionLabel().setColor(color(0, 102, 153));

    // Bloc de commande pour les orientations
    toggleUseAlign = cp5.addToggle("useAlign")
      .setPosition(20, 160)
      .setSize(20, 20)
      .setLabel("Enable align")
      .setValue(false);
    toggleUseAlign.getCaptionLabel().setColor(color(0, 102, 153));


    cp5.addScrollableList("compassZone")
      .setPosition(60, 160)
      .setSize(100, 100)
      .setBarHeight(20)
      .setItemHeight(20)
      .addItems(new String[] { "A", "AB", "B", "BC", "C", "CA" })
      .setValue(0);

    cp5.addScrollableList("orientationMode")
      .setPosition(180, 160)
      .setSize(100, 100)
      .setBarHeight(20)
      .setItemHeight(20)
      .addItems(new String[] { "CUSTOM", "NORTH", "EAST", "SOUTH", "WEST" })
      .setValue(0);

    cp5.addTextfield("customAngle")
      .setLabel("°")
      .setPosition(300, 160)
      .setSize(60, 20)
      .setAutoClear(false)
      .setText("0");


    // Chargement automatique au démarrage
    reloadTempStrategy();
  }


  public void draw() {
    background(220);

    if (millis() - lastAutoSave > autoSaveInterval) {
      println("auto-save triggered");
      savePointsToFile("strategy_temp.json");
      lastAutoSave = millis();
    }
  }
  
  public void updateLabelInfo(){
    labelInfo.setText(
        "Point P" + selected.id +
        "\nX: " + nf(selected.x_mm, 0, 0) + " mm" +
        "\nY: " + nf(selected.y_mm, 0, 0) + " mm" +
        "\nPOI: " + selected.poiName +
        "\nAlign: " + selected.useAlign +
        "\nCompass: " + selected.compass +
        "\norientation: " + selected.orientation +
        "\ncustomAngle: " + selected.customAngle
        );

  }


  public void setSelectedPoint(StrategyPoint p) {
    selected = p;

    if (selected != null) {
      updatingFromGUI = true;

      updateLabelInfo();

      fieldX.setText(nf(selected.x_mm, 0, 0));
      fieldY.setText(nf(selected.y_mm, 0, 0));

      // mettre à jour les composants d'alignement
      toggleUseAlign.setValue(selected.useAlign);
      cp5.get(ScrollableList.class, "compassZone").setStringValue(selected.compass);
      cp5.get(ScrollableList.class, "orientationMode").setStringValue(selected.orientation);
      cp5.get(Textfield.class, "customAngle").setText(str(selected.customAngle));

      updatingFromGUI = false;
    } else {
      labelInfo.setText("No point selected");
      fieldX.setText("");
      fieldY.setText("");
    }
  }

  public int compassIndexOf(String value) {
    String[] options = { "A", "AB", "B", "BC", "C", "CA" };
    for (int i = 0; i < options.length; i++) {
      if (options[i].equals(value)) return i;
    }
    return 0; // défaut si non trouvé
  }

  public int orientationIndexOf(String value) {
    String[] options = { "CUSTOM", "NORTH", "SOUTH", "WEST", "EAST" };
    for (int i = 0; i < options.length; i++) {
      if (options[i].equals(value)) return i;
    }
    return 0; // défaut si non trouvé
  }

  public void controlEvent(ControlEvent e) {
    if (updatingFromGUI) return;
    println("[DEBUG] controlEvent: " + e.getName());

    if (e.getName().equals("addPointEnabled")) {
      addPointEnabled = e.getValue() == 1;
      println("[GUI] Add point: " + (addPointEnabled ? "enabled" : "disabled"));
      toggleAddEnabled.setLabel(addPointEnabled ? "Add points enabled" : "Add points disabled");
      return;
    }

    if (selected != null) {
      switch (e.getName()) {
      case "x_mm":
        try {
          float newX = Float.parseFloat(e.getStringValue());
          selected.x_mm = constrain(newX, 0, 3000);
        }
        catch (Exception ex) {
          println("[GUI] Invalid X value");
        }
        break;

      case "y_mm":
        try {
          float newY = Float.parseFloat(e.getStringValue());
          selected.y_mm = constrain(newY, 0, 2000);
        }
        catch (Exception ex) {
          println("[GUI] Invalid Y value");
        }
        break;

      case "useAlign":
        selected.useAlign = (e.getValue() == 1);
        break;

      case "compassZone":
        int index = (int) e.getValue();  // getValue() retourne l’index sélectionné
        String[] optionsCompass = { "A", "AB", "B", "BC", "C", "CA" };
        if (index >= 0 && index < optionsCompass.length) {
          selected.compass = optionsCompass[index];
          println("[DEBUG] compass set to: " + selected.compass);
        }
        break;


      case "orientationMode":
        int i = (int) e.getValue();
        String[] orientations = { "CUSTOM", "NORTH", "EAST", "SOUTH", "WEST" };
        if (i >= 0 && i < orientations.length) {
          selected.orientation = orientations[i];
          println("[DEBUG] orientation set to: " + selected.orientation);
        }
        break;



      case "customAngle":
        try {
          selected.customAngle = constrain(int(e.getStringValue()), 0, 359);
        }
        catch (Exception ex) {
          println("[GUI] Invalid angle");
        }
        break;
      }
    }
    // Update le label d'info
    updateLabelInfo();
  }


  public boolean isAddPointEnabled() {
    return addPointEnabled;
  }


  public void saveStrategy() {
    // 1. Save temp file
    savePointsToFile("strategy_temp.json");

    // 2. Prompt for custom location
    selectOutput("Save strategy to...", "saveStrategyToFile");
  }

  public void saveStrategyToFile(File selection) {
    if (selection == null) {
      println("[GUI] Save cancelled.");
      return;
    }

    String path = selection.getAbsolutePath();
    if (!path.toLowerCase().endsWith(".json")) {
      path += ".json";
    }

    savePointsToFile(path);
    println("[GUI] Strategy saved to: " + path);
  }


  public void savePointsToFile(String path) {
    JSONObject data = exportPointsToJSON();

    // Si c'est un chemin absolu (ex: vient de selectOutput), on le garde tel quel
    if (path.startsWith("/") || path.contains(":")) {
      saveJSONObject(data, path);
    } else {
      // Sinon, on utilise le chemin relatif depuis le sketch
      saveJSONObject(data, mainApp.getDataPath(path));
    }
  }


  public void reloadTempStrategy() {
    println("[GUI] Reloading strategy_temp.json...");
    File f = new File(mainApp.getDataPath("strategy_temp.json"));
    loadStrategyFromFile(f);
  }



  public JSONObject exportPointsToJSON() {
    JSONObject data = new JSONObject();
    JSONArray list = new JSONArray();

    for (StrategyPoint p : StrategyEditor.points) {
      JSONObject entry = new JSONObject();
      entry.setInt("id", p.id);
      entry.setFloat("x_mm", p.x_mm);
      entry.setFloat("y_mm", p.y_mm);

      if (p.poiName != null) {
        entry.setString("poi", p.poiName);
      }

      // Ajout des paramètres d'alignement
      entry.setBoolean("useAlign", p.useAlign);
      entry.setString("compass", p.compass);
      entry.setString("orientation", p.orientation);
      entry.setInt("customAngle", p.customAngle);

      list.append(entry);
    }

    data.setJSONArray("strategy", list);
    return data;
  }


  public void setMainApp(StrategyEditor app) {
    this.mainApp = app;
  }

  public void loadStrategy() {
    selectInput("Select a strategy file to load...", "loadStrategyFromFile");
  }

  public void loadStrategyFromFile(File selection) {
    if (selection == null) {
      println("[GUI] Load cancelled.");
      return;
    }

    String path = selection.getAbsolutePath();

    JSONObject data = loadJSONObject(path);
    JSONArray list = data.getJSONArray("strategy");

    StrategyEditor.points.clear();

    for (int i = 0; i < list.size(); i++) {
      JSONObject entry = list.getJSONObject(i);
      int id = entry.getInt("id");
      float x = entry.getFloat("x_mm");
      float y = entry.getFloat("y_mm");

      StrategyPoint p = new StrategyPoint(id, x, y);

      if (entry.hasKey("poi")) p.poiName = entry.getString("poi");
      if (entry.hasKey("useAlign")) p.useAlign = entry.getBoolean("useAlign");
      if (entry.hasKey("compass")) p.compass = entry.getString("compass");
      if (entry.hasKey("orientation")) p.orientation = entry.getString("orientation");
      if (entry.hasKey("customAngle")) p.customAngle = entry.getInt("customAngle");

      StrategyEditor.points.add(p);
    }

    mainApp.renumerotePoints();
    println("[GUI] Loaded " + StrategyEditor.points.size() + " points from: " + path);
  }

  public void resetStrategy() {
    int confirm = javax.swing.JOptionPane.showConfirmDialog(null,
      "Are you sure you want to delete all points?",
      "Confirm Reset",
      javax.swing.JOptionPane.YES_NO_OPTION);

    if (confirm == javax.swing.JOptionPane.YES_OPTION) {
      StrategyEditor.points.clear();
      mainApp.renumerotePoints();
      setSelectedPoint(null);
      println("[GUI] Strategy reset.");
    } else {
      println("[GUI] Reset cancelled.");
    }
  }

  public void selectPrevPoint() {
    if (StrategyEditor.points.size() == 0) return;

    if (selected == null) {
      selected = StrategyEditor.points.get(0);
    } else {
      int index = StrategyEditor.points.indexOf(selected);
      if (index > 0) {
        selected = StrategyEditor.points.get(index - 1);
      }
    }

    setSelectedPoint(selected);
    StrategyEditor.selectedPoint = selected;
  }


  public void selectNextPoint() {
    if (StrategyEditor.points.size() == 0) return;
    if (selected == null) {
      selected = StrategyEditor.points.get(0);
    } else {
      int index = StrategyEditor.points.indexOf(selected);
      if (index < StrategyEditor.points.size() - 1) {
        selected = StrategyEditor.points.get(index + 1);
      }
    }
    setSelectedPoint(selected);
    StrategyEditor.selectedPoint = selected;
  }

  public void startSimulation() {
    if (StrategyEditor.points.size() >= 2) {
      StrategyEditor.currentSegment = 0;
      StrategyEditor.t = 0.0;
      StrategyEditor.robotPos = new PVector(
        StrategyEditor.points.get(0).x_mm,
        StrategyEditor.points.get(0).y_mm
        );
      StrategyEditor.isSimulating = true;
      println("[GUI] Simulation started.");
    } else {
      println("[GUI] Not enough points to simulate.");
    }
  }

  public void showOverlay(boolean val) {
    showOverlay = val;
  }
}
