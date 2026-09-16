# SCADA Mimic Maker Server — Functional Description

The server is a local backend that hosts the web-based SCADA Mimic Maker IDE, manages project data, and provides an API for creating, editing, saving, loading, and exporting SCADA applications.

The server does not communicate with PLCs. PLC communication belongs exclusively to the generated SCADA application.

## Main Responsibilities
* Serve the IDE static files (HTML, CSS, JavaScript, images).
* Provide an HTTP API for the frontend.
* Create, open, rename, and delete projects.
* Save and load SCADA screens, components, and properties.
* Export projects as standalone SCADA applications.
* Validate incoming project data and store it in SQLite.

## API Specification
| Method | Endpoint | Function |
|---|---|---|
| `GET` | `/api/projects` | List projects |
| `POST` | `/api/projects` | Create a project |
| `GET` | `/api/projects/:id` | Load a project |
| `PUT` | `/api/projects/:id` | Update project metadata |
| `DELETE` | `/api/projects/:id` | Delete a project |
| `GET` | `/api/projects/:id/screens` | List project screens |
| `POST` | `/api/projects/:id/screens` | Create a screen |
| `PUT` | `/api/screens/:id` | Save screen changes |
| `DELETE` | `/api/screens/:id` | Delete a screen |
| `POST` | `/api/projects/:id/export` | Export the project |

## Storage Strategy
* **Database:** SQLite
* **Initial Schema:** Storing project definitions as JSON documents or normalized tables (`projects`, `screens`, `components`, `component_properties`).