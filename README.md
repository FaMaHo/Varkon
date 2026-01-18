# Varkon3D

## Team members:
- Nour Mahfood
- ⁠Fatemeh Mahmoudzadeh
- ⁠Hussein Mokhadder
- Saeed Zayyad

## The story
(It's the 2D story, we might need to update it a bit for the 3D part)

Officer Jack at NASA was asked to go on a mission to Mars to check for any signs of vital life so that humans can expand their kingdom to Mars.

Unfortunately, Officer Jack accepted the mission without knowing that NASA had kept the Varkon, a hostile alien species, a secret from him so he wouldn’t hesitate.

On his way there, and right when he was about to land, Officer Jack was attacked by Varkon and decided to abort the landing. He flew his spaceship back up, only to find out he was being followed and surrounded by Varkon motherships that started spreading aliens all around.

Officer Jack had no idea what to do!!! And as we all know, he was panicking, pressing every button on his ship until he found the plasma button, and that’s when he realized he had been sent into this trap by NASA. A real "holy sh*t" moment for him.

Jack started eliminating those creatures with his weapon, but they kept coming in waves, and different kinds started showing up.

Now it’s your mission to help Officer Jack and bring him back to Earth safely.

## Game Engine structure
```
Varkon3D
└───GameEngine2025
    └───GameEngine
        ├───.vs
        ├───Debug
        ├───Dependencies
        │   ├───GLEW
        │   │   ├───include
        │   │   └───libs
        │   ├───GLFW
        │   │   ├───include
        │   │   └───lib-vc2015
        │   └───glm
        │       ├───core
        │       ├───gtc
        │       ├───gtx
        │       └───virtrev
        ├───GameEngine
        │   ├───Camera
        │   ├───Debug
        │   │   └───GameEngine.tlog
        │   ├───Graphics├───GameEngine
        │   ├───Camera
        │   ├───Debug
        │   │   └───GameEngine.tlog
        │   ├───Graphics
        │   ├───Model Loading
        │   ├───Resources
        │   │   ├───Models
        │   │   └───Textures
        │   ├───Shaders
        │   └───x64
        │       └───Debug
        │           └───GameEngine.tlog
        └───x64
            └───Debug
        │   ├───Model Loading
        │   ├───Resources
        │   │   ├───Models
        │   │   └───Textures
        │   ├───Shaders
        │   └───x64
        │       └───Debug
        │           └───GameEngine.tlog
        └───x64
            └───Debug
```
As we are ignoring files and folders like Dependencies and obj and bmp files (as they are heavy and make git crash), you can download the full project from here:
[link]
