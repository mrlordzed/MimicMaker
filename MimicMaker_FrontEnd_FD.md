# SCADA Mimic Maker Frontend — Functional Description

The frontend is a browser-based visual editor for designing SCADA mimic screens. It manages user interactions, canvas rendering, component placement, and API synchronization with the backend server.

## Core Workspace Elements
* **Toolbar & Navigation:** Project and screen management, undo/redo, saving, export actions.
* **Component Palette:** Drag-and-drop or click-to-add SCADA elements.
* **Design Canvas:** Interactive editing grid with positioning, resizing, and rotation handles.
* **Properties Panel:** Visual, geometric, and OPC-UA tag configuration controls.

## Component Categories
* **Geometric:** Rectangle, Circle, Triangle, Polygon
* **Information:** Label, Text box
* **Simple Interactive:** Boolean indicator, Multi-state indicator, Boolean button, Toggle button, Slider, Bar indicator, Numeric/Text display & inputs
* **Complex Components:** Digital valve, Analog valve, Soft-start motor, VFD motor, Gauge, Pump, Fan

## Client State & Save Workflow
* Tracks modification flags ("Unsaved changes").
* Serializes canvas state to JSON before submitting to the backend API.
* Provides a preview mode to test component visual behavior without active PLC connections.