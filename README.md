Student Dormitory 3D Simulation (OpenGL, C++14)
🎥 Demo Video

[👉 (Replace YOUR_VIDEO_ID with your actual YouTube video ID)
](]([https://youtu.be/v_2dDEgSfxU?si=GTG3r3Rr8nAYKb2f](https://youtu.be/v_2dDEgSfxU?si=GTG3r3Rr8nAYKb2f)))
🖼️ Project Preview




👉 (Replace with your actual screenshot paths or upload images to GitHub and link them here)

⚙️ How to Download & Run
🔽 Clone Repository
git clone https://github.com/ProvaPaul/Graphics-Project-
🛠️ Setup (Visual Studio)
Open the project folder in Visual Studio
Install (if needed):
Desktop development with C++
Select:
x64 + Debug (recommended) or Release
▶️ Build & Run
Build → Build Solution
Run → Start Without Debugging (Ctrl + F5)
📌 Project Description

This project is an interactive 3D student dormitory campus simulation developed using OpenGL and C++14.
It features a fully navigable environment with exterior campus and multiple interior rooms, including real-time controls, lighting systems, animations, and procedural textures. Advanced techniques like Bezier curves, ruled surfaces, and fractal modeling are also used.

🚀 Features
🌆 Exterior Campus
Ground, plaza, walkways, boundary walls, main gate
Decorative elements (bench, fountain, trees, lamp posts)
Multi-building dormitory layout
Fractal trees and architectural detailing
🏠 Interior Rooms
Bedrooms, reading room, dining hall, TV lounge, washroom
Corridor-based room navigation
🎥 Camera & View System
Free camera, orbit, bird’s-eye modes
4-view split system
Smooth transitions between scenes
💡 Lighting & Animation
Per-room lighting
Day/Night mode
Animated fan & TV
Emissive lighting effects
🧮 Geometry & Curves
Primitive modeling (cube, sphere, etc.)
Bezier & ruled surfaces
Fractal tree generation
🎨 Texture System
Fully procedural textures
Brick, grass, marble, wood, tile, concrete
🎮 Controls
🔹 Movement
W A S D → Move
Mouse → Look
Space / Shift → Up / Down
Esc → Exit
🔹 Navigation
R → Corridor
E → Enter room
P / B → Back
Tab → Switch room
🔹 View & Lighting
L → Light toggle
F / I → Orbit / Bird view
T → Texture mode
V + 0–4 → View modes
⚙️ Project Structure
🔧 Core Modules
main.cpp → Render loop
input.cpp → Controls
shaders.cpp → Shaders
geometry.cpp → Shapes
textures.cpp → Textures
🏗️ Scene & Rooms
Scene files → Campus & environment
Room files → Individual rooms + corridor
🧠 System Workflow
Initialize OpenGL & shaders
Generate geometry & textures
Handle input
Update animations
Render scene
🐞 Troubleshooting
LNK1168 → Close running .exe
Zoom issue → Reset/change view
No texture → Press T
