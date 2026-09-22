# SCADA Mimic Maker IDE — Comprehensive Functional Description (v2.0)

---

## Executive Summary

**SCADA Mimic Maker** is a **browser-based integrated development environment (IDE)** for visual design, configuration, and deployment of SCADA mimic screens. The system comprises a **local backend server** (Python/Flask) and a **frontend web application** (JavaScript/Canvas) that enables engineers to drag-and-drop components, configure properties, save projects, and export standalone SCADA applications that run on field devices.

The system explicitly separates IDE functionality (editing screens) from runtime functionality (generated apps communicating with PLCs). This FD covers the complete IDE workflow from project creation through export.

---

## User Personas & Workflows

### Primary Users

| Persona | Role | Goals | Workflow |
|---------|------|-------|----------|
| **Plant Engineer** | Designs SCADA screens | Create reusable, professional mimic visualizations; reduce manual coding | 1. Open/create project<br>2. Drag components onto canvas<br>3. Configure properties (tags, thresholds, colors)<br>4. Preview screen behavior<br>5. Save project<br>6. Export as runtime app<br>7. Deploy to field device |
| **Field Technician** | Operates deployed SCADA app | Monitor process state; interact with equipment via buttons/sliders | Runs exported app; views live data from PLC; clicks buttons to trigger actions |
| **Project Manager** | Manages project lifecycle | Track multiple SCADA projects; organize teams; manage versions | Lists projects; opens; renames; backs up; coordinates with engineers |

### User Needs & Outcomes

- **Engineer:** "I need to create a pump control screen in 30 minutes, not code it from scratch." → Outcome: Visual editor reduces design time by 70%.
- **Technician:** "I need a reliable display that shows real-time pump status and lets me start/stop it." → Outcome: Exported app runs without latency; buttons execute within 500ms.
- **Manager:** "I need to know what projects exist and prevent accidental data loss." → Outcome: Project listing, rename, export backup; deletion requires confirmation.

---

## System Architecture

### Components & Responsibilities

```
┌─────────────────────────────────────────────────────────────┐
│                    Browser (Frontend)                        │
│  ┌──────────────┬──────────────┬──────────────┐              │
│  │   Toolbar &  │  Component   │   Design     │              │
│  │  Navigation  │  Palette     │   Canvas     │              │
│  └──────────────┴──────────────┴──────────────┘              │
│  ┌──────────────────────────────────────────────┐            │
│  │       Properties Panel & Inspector            │            │
│  └──────────────────────────────────────────────┘            │
│  ┌──────────────────────────────────────────────┐            │
│  │    Preview Mode (read-only simulation)       │            │
│  └──────────────────────────────────────────────┘            │
└─────────────────────────────────────────────────────────────┘
                         ↕ (HTTP JSON)
┌─────────────────────────────────────────────────────────────┐
│                    Local Backend Server                      │
│  ┌──────────────────────────────────────────────┐            │
│  │  API Layer (REST endpoints)                   │            │
│  │  • Projects CRUD                              │            │
│  │  • Screens CRUD                               │            │
│  │  • Export orchestration                       │            │
│  └──────────────────────────────────────────────┘            │
│  ┌──────────────────────────────────────────────┐            │
│  │  Validation & Business Logic                  │            │
│  │  • JSON schema validation                     │            │
│  │  • Component property rules                   │            │
│  │  • Concurrent edit conflict resolution        │            │
│  └──────────────────────────────────────────────┘            │
│  ┌──────────────────────────────────────────────┐            │
│  │  Persistence Layer (SQLite)                   │            │
│  │  • projects, screens, components tables       │            │
│  └──────────────────────────────────────────────┘            │
│  ┌──────────────────────────────────────────────┐            │
│  │  Export Engine                                │            │
│  │  • Generates Python runtime app               │            │
│  │  • Packages dependencies                      │            │
│  │  • Creates distributable ZIP                  │            │
│  └──────────────────────────────────────────────┘            │
└─────────────────────────────────────────────────────────────┘
                         ↕ (SQLite file)
┌─────────────────────────────────────────────────────────────┐
│              Local File System (/projects/*.db)              │
└─────────────────────────────────────────────────────────────┘
```

---

## Feature Specification

### 1. Project Management

#### Operations

| Operation | Input | Output | Success Criteria |
|-----------|-------|--------|------------------|
| **Create Project** | Project name (1-100 chars) | Project ID + metadata | Project listed in /api/projects; no duplicate names allowed |
| **List Projects** | None | Array of {id, name, created_at, modified_at, num_screens} | Returns ≤100 projects; sorted by modified_at (descending) |
| **Open Project** | Project ID | Full project structure (metadata + all screens + components) | Load completes in <2s; all screens and component data intact |
| **Update Metadata** | Project ID + {name, description} | Updated metadata | New name visible in list; old name no longer accessible |
| **Delete Project** | Project ID | Confirmation + deletion | Project removed from database; no recovery possible; user warned |
| **Export Project** | Project ID | ZIP file (standalone SCADA app) | ZIP contains Python runtime, config.json, templates; runs without IDE |

#### Data Model: Projects Table

```sql
CREATE TABLE projects (
  id TEXT PRIMARY KEY,           -- UUID (e.g., "proj-abc123def456")
  name TEXT NOT NULL UNIQUE,     -- Project name (1-100 chars, no special chars except _-)
  description TEXT,              -- Optional description (max 500 chars)
  created_at TIMESTAMP NOT NULL, -- ISO 8601 (UTC)
  modified_at TIMESTAMP NOT NULL,-- Updated on any screen/component change
  num_screens INTEGER DEFAULT 0  -- Denormalized for quick listing
);
```

---

### 2. Screen Design & Editing

#### Operations

| Operation | Input | Output | Success Criteria |
|-----------|-------|--------|------------------|
| **Create Screen** | Project ID + {name, canvas_width, canvas_height, background_color} | Screen ID + empty canvas | Screen listed under project; dimensions match specified values |
| **Load Screen** | Screen ID | Screen data (name, dimensions, all components with properties) | Load in <1s; all component positions, types, properties accurate |
| **Save Screen** | Screen ID + updated components array | Confirmation + modified_at timestamp | Changes persisted; previous version overwritten; no merge logic |
| **Delete Screen** | Screen ID | Confirmation | Screen removed; no recovery; project num_screens decremented |
| **Rename Screen** | Screen ID + new name | Updated metadata | Name changed; components unaffected |

#### Data Model: Screens Table

```sql
CREATE TABLE screens (
  id TEXT PRIMARY KEY,              -- UUID
  project_id TEXT NOT NULL,         -- FK → projects.id
  name TEXT NOT NULL,               -- Screen name (1-100 chars)
  canvas_width INTEGER NOT NULL,    -- Pixels (min 640, max 2560)
  canvas_height INTEGER NOT NULL,   -- Pixels (min 480, max 1920)
  background_color TEXT DEFAULT '#FFFFFF', -- Hex color
  created_at TIMESTAMP NOT NULL,
  modified_at TIMESTAMP NOT NULL,
  locked_by_user TEXT,              -- User ID if screen is being edited (NULL = unlocked)
  locked_at TIMESTAMP,              -- Timestamp of lock
  FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE
);
```

---

### 3. Component Palette & Canvas

#### Supported Components

| Category | Components | Attributes | Validation |
|----------|-----------|------------|-----------|
| **Geometric** | Rectangle, Circle, Triangle, Polygon | x, y, width, height, fill_color, stroke_color, stroke_width, rotation | Dimensions ≥ 5px; colors valid hex; rotation 0-360° |
| **Information** | Label, Text Box | x, y, width, height, text, font_size, font_color, alignment, border | Font size 8-72pt; text ≤500 chars; alignment in {left, center, right} |
| **Simple Interactive** | Boolean Indicator, Multi-State Indicator, Boolean Button, Toggle Button, Slider, Bar Indicator, Numeric Display, Text Input, Numeric Input | x, y, width, height, + component-specific properties (e.g., min_value, max_value for slider) | Boolean button must have on_state_color & off_state_color; slider min < max; tags must be valid OPC-UA names |
| **Complex Components** | Digital Valve, Analog Valve, Motor (Soft-Start), VFD Motor, Gauge, Pump, Fan | x, y, width, height, status_tag, command_tag, alarms, thresholds | Tags must match PLC address format; status/command tags are required for interactive components |

#### Component Properties Structure

Each component has:
```json
{
  "id": "comp-xyz789",
  "type": "boolean_indicator",          // Component type
  "x": 100,                             // Pixel position
  "y": 50,
  "width": 40,
  "height": 40,
  "visible": true,                      // Show/hide
  "locked": false,                      // Prevent accidental moves
  "z_index": 5,                         // Layer order
  "properties": {
    "status_tag": "PLC1.pump.running",  // OPC-UA tag reference
    "on_color": "#00FF00",
    "off_color": "#FF0000",
    "label": "Pump Running"
  }
}
```

#### Data Model: Components Table

```sql
CREATE TABLE components (
  id TEXT PRIMARY KEY,
  screen_id TEXT NOT NULL,
  component_type TEXT NOT NULL,  -- e.g., "boolean_indicator"
  properties JSON NOT NULL,      -- Full JSON properties object
  created_at TIMESTAMP NOT NULL,
  modified_at TIMESTAMP NOT NULL,
  FOREIGN KEY (screen_id) REFERENCES screens(id) ON DELETE CASCADE
);
```

---

### 4. Properties Panel & Editing

#### Behavior

- **On component selection:** Properties panel displays all editable fields for that component type.
- **On property change:** 
  - Immediate visual feedback on canvas (e.g., color change, size adjustment).
  - Validation error shown if property invalid (e.g., width < 5px, invalid tag format).
  - Unsaved indicator appears in toolbar.
- **Constraints:**
  - Cannot save screen if any component has validation errors.
  - Tag validation: OPC-UA format check (e.g., `Server.Node.Leaf`); warns if tag not in known PLC definition (if available).

#### Example Editing Workflow

1. Engineer clicks "Boolean Indicator" from palette → Cursor changes to crosshair.
2. Engineer drags on canvas → Rectangle appears.
3. Engineer releases → Component auto-selected; Properties panel opens.
4. Engineer types tag name → Validation runs; green checkmark if valid.
5. Engineer clicks color swatch → Color picker appears; selects green for "on" state.
6. Engineer hits "Save" → PUT request sent; screen updates in database.

---

### 5. Preview Mode

#### Purpose
**Read-only simulation** of screen behavior without exporting. Allows engineer to test interactions before deploying.

#### Behavior

- **Activation:** Click "Preview" button in toolbar.
- **State:** Components behave as if connected to PLC:
  - Boolean indicators toggle on click (simulated state).
  - Buttons execute "press" animation; can be configured to log actions.
  - Sliders respond to drag; numeric display updates.
  - Tags are **mocked** (no real PLC connection in IDE).
- **Exit:** Click "Exit Preview" or press Escape; canvas returns to edit mode.
- **Data persisted:** Preview does NOT modify saved screen; it's ephemeral.

#### Success Criteria

- Preview loads in <500ms.
- Interactions respond in <100ms.
- Engineer can toggle buttons, drag sliders, verify visual logic.
- No saved data is altered.

---

### 6. Save & Undo/Redo

#### Auto-Save & Unsaved Changes Tracking

| Scenario | Behavior | Outcome |
|----------|----------|---------|
| **Component added/modified** | "Unsaved changes" indicator shows; auto-save queued after 2s of inactivity | User sees asterisk (*) in screen tab; data not lost if browser crashes |
| **User clicks Save button** | POST/PUT request sent to server immediately | "Saved" confirmation shown; unsaved indicator clears |
| **Network unavailable** | Auto-save fails silently; data queued locally (IndexedDB) | Offline banner shown; user warned |
| **User navigates away with unsaved changes** | Browser prompt: "You have unsaved changes. Leave?" | User can cancel or confirm discard |

#### Undo/Redo

- **Undo:** Reverts last component edit (position, property change, add/delete) up to 50 actions back.
- **Redo:** Restores undone action.
- **Scope:** Per-screen; undo history lost on page refresh.
- **Success Criteria:** Undo/redo execute in <100ms; no data loss.

---

### 7. Export & Deployment

#### Export Process

| Step | Input | Action | Output |
|------|-------|--------|--------|
| 1. **Validate** | Entire project | Check all screens for incomplete components; verify all tags valid | Error list if validation fails; user must fix before export |
| 2. **Generate Runtime** | Screens + components | Create Python Flask app with generated templates and config | Python source code |
| 3. **Package Dependencies** | Python source | Bundle pyopc, Flask, templates into ZIP | Self-contained distributable |
| 4. **Create Config** | Project metadata | Write `config.json` with screen definitions, PLC connection info, tag mappings | config.json in ZIP root |
| 5. **Deliver** | Packaged files | Return ZIP file to user's browser; trigger download | `scada-mimic-ProjectName-v1.zip` |

#### Export Output Structure

```
scada-mimic-MyProject-v1.zip
├── app.py                      # Main Flask app
├── requirements.txt            # Python dependencies (pyopc, Flask, etc.)
├── config.json                 # Screen definitions, PLC connection config
├── screens/
│   ├── pump_control.html       # Generated HTML for each screen
│   ├── valve_status.html
│   └── ...
├── static/
│   ├── js/
│   │   └── runtime.js          # Client-side interactivity (buttons, sliders, updates)
│   ├── css/
│   │   └── styles.css          # Mimic styling
│   └── images/
├── templates/
│   └── base.html               # HTML base template
├── opc_client.py               # PLC communication module
├── README.md                   # Setup & deployment instructions
└── MANIFEST.txt                # File inventory

# config.json structure:
{
  "project_name": "My Project",
  "version": "1.0",
  "screens": [
    {
      "id": "screen-1",
      "name": "Pump Control",
      "components": [...]
    }
  ],
  "plc_connection": {
    "server_url": "opc.tcp://192.168.1.10:4840",
    "timeout_ms": 5000