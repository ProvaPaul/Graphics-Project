# 🏫 Student Dormitory 3D Simulation

**OpenGL · C++14 · Visual Studio · Procedural Textures · Bezier Curves · Fractal Trees**

---

## 🎥 Project Demo

<h2 align="center">🎬 Student Dormitory 3D Simulation — Project VIDEO</h2>
<p align="center">
  <a href="https://youtu.be/v_2dDEgSfxU?si=GTG3r3Rr8nAYKb2f">
    <img src="https://img.youtube.com/vi/v_2dDEgSfxU/0.jpg" width="700">
  </a>
</p>

---
<!-- 
## 🖼️ Project Preview

> _Add screenshots to an `images/` folder in your repo and update the paths below._

| Exterior Campus | Interior Room |
|:-:|:-:|
| ![Exterior](images/exterior.png) | ![Interior](images/interior.png) |

| Lighting & Animation | Fractal Trees |
|:-:|:-:|
| ![Lighting](images/lighting.png) | ![Fractal](images/fractal.png) |

--- -->

## 📌 Project Description

This project is an interactive **3D student dormitory campus simulation** developed using OpenGL and C++14. It features a fully navigable environment with an exterior campus and multiple interior rooms, including real-time controls, lighting, animations, and procedural textures. Advanced techniques like **Bezier curves**, **ruled surfaces**, and **fractal modeling** are also implemented.

---

## 🚀 Features

### 🌆 Exterior Campus
- Ground, plaza, walkways, boundary walls, main gate
- Decorative elements: bench, fountain, trees, lamp posts
- Multi-building dormitory layout with architectural detailing
- Procedurally generated fractal trees

### 🏠 Interior Rooms
- Bedrooms, reading room, dining hall, TV lounge, washroom
- Corridor-based room navigation system

### 🎥 Camera & View System
- Free camera, orbit, and bird's-eye view modes
- 4-view split screen system
- Smooth transitions between scenes

### 💡 Lighting & Animation
- Per-room lighting control
- Day / Night mode toggle
- Animated fan & TV
- Emissive lighting effects

### 🧮 Geometry & Curves
- Primitive modeling: cube, sphere, cylinder, etc.
- Bezier curves and ruled surfaces
- Fractal tree generation

### 🎨 Texture System
- Fully procedural textures (no external image files)
- Materials: brick, grass, marble, wood, tile, concrete

---

## ⚙️ How to Download & Run

### 🔽 Clone Repository

```bash
git clone https://github.com/ProvaPaul/Graphics-Project-
```

### 🛠️ Setup (Visual Studio)

- Open the cloned project folder in **Visual Studio**
- Install if needed: **Desktop development with C++** workload
- Select configuration: **x64 + Debug** _(recommended)_ or **Release**

### ▶️ Build & Run

```
Build → Build Solution
Run   → Start Without Debugging  (Ctrl + F5)
```

---

## 🎮 Controls

### 🔹 Movement

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move forward / left / backward / right |
| `Mouse` | Look around |
| `Space` | Move up |
| `Shift` | Move down |
| `Esc` | Exit |

### 🔹 Navigation

| Key | Action |
|-----|--------|
| `R` | Enter corridor |
| `E` | Enter room |
| `P` / `B` | Go back |
| `Tab` | Switch room |

### 🔹 View & Lighting

| Key | Action |
|-----|--------|
| `L` | Toggle lights |
| `F` | Orbit view |
| `I` | Bird's-eye view |
| `T` | Toggle texture mode |
| `V` + `0–4` | Switch view modes |

---

## ⚙️ Project Structure

### 🔧 Core Modules

```
Graphics-Project/
│
├── main.cpp          # Main render loop
├── input.cpp         # Keyboard & mouse controls
├── shaders.cpp       # Shader programs
├── geometry.cpp      # Shape primitives
├── textures.cpp      # Procedural texture generation
│
├── scene/            # Campus & environment scene files
├── rooms/            # Individual room + corridor files
│
└── images/           # Screenshots (add yours here)
```

### 🧠 System Workflow

```
Initialize OpenGL & Shaders
        ↓
Generate Geometry & Textures
        ↓
Handle Input
        ↓
Update Animations
        ↓
Render Scene  ──→ (loop)
```

---

## 🐞 Troubleshooting

| Issue | Fix |
|-------|-----|
| `LNK1168` linker error | Close the running `.exe` instance, then rebuild |
| Zoom / camera stuck | Reset or switch view mode with `V + 0` |
| Textures not showing | Press `T` to toggle texture mode on |

---

## 🛠️ Tech Stack

| Layer | Technology |
|-------|-----------|
| Graphics API | OpenGL |
| Language | C++14 |
| IDE | Visual Studio |
| Math | GLM |
| Windowing | GLFW / GLUT |

---

## 📄 License

Developed as an academic graphics course project. Free to use for learning and reference.
