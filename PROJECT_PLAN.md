# SCADA Mimic Maker — Project Management Plan

## Phase 1: Backend Core (C & SQLite)
- [ ] **Task 1: Static File Server** — Build `backend/server.c` to handle HTTP GET requests and serve files from `public/`.
  - *Done Criteria:* Navigating to `http://localhost:8080` loads `public/main.html`.
  - *Next Step:* Move to Task 2.
- [ ] **Task 2: Database Setup** — Link SQLite inside `backend/server.c` to create/open `projects.db` on startup.
  - *Done Criteria:* Server creates `projects.db` table schema without errors.
  - *Next Step:* Move to Task 3.
- [ ] **Task 3: Core JSON API** — Add HTTP endpoints for `GET /api/projects/:id` and `PUT /api/projects/:id`.
  - *Done Criteria:* Sending a test JSON payload via `curl` stores and retrieves data from SQLite.
  - *Next Step:* Move to Phase 2.

## Phase 2: IDE Canvas Frontend
- [ ] **Task 4: UI Shell Structure** — Build the layout in `public/main.html` (Toolbar, Palette, Canvas, Properties Panel).
  - *Done Criteria:* Browser renders side-by-side workspace panels.
  - *Next Step:* Move to Task 5.
- [ ] **Task 5: Basic Object Placement** — Implement drag-and-drop/clicking from palette to canvas for rectangles, circles, and labels.
  - *Done Criteria:* Geometric forms and labels render, select, move, and resize on canvas.
  - *Next Step:* Move to Task 6.
- [ ] **Task 6: Properties Panel Binding** — Link selection events so object parameters load into inputs and update live.
  - *Done Criteria:* Changing an input value in the sidebar immediately alters the selected canvas object.
  - *Next Step:* Move to Phase 3.

## Phase 3: Integration & Persistence
- [ ] **Task 7: API Sync & Save Flow** — Wire the IDE "Save" button to serialize canvas state into JSON and PUT to backend.
  - *Done Criteria:* Refreshing the browser reloads exact object positions from SQLite.
  - *Next Step:* Move to Task 8.
- [ ] **Task 8: Basic Application Export** — Implement `/api/projects/:id/export` in C to output standalone HTML/JS bundles.
  - *Done Criteria:* Opening the exported directory renders a static preview without IDE controls.
  - *Next Step:* Move to Phase 4.

## Phase 4: Component Expansion
- [ ] **Task 9: Interactive & Complex Components** — Add indicators, buttons, valves, and pumps to palette with OPC-UA tag configuration fields.
  - *Done Criteria:* Complex SCADA elements drop onto canvas and save tag mappings to database.
