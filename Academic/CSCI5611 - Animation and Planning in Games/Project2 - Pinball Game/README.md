# Pinball game
Pinball game building off of collision detection library. Parses scene files to construct a board. Flippers are controlled with ctrl keys, gravity is turned on or off with "g". Different elements have different interactions, balls collide with each other, score bumpers, and red obsticles. Balls are teleported from black holes to white holes and maintain their current velocity, and do not interact with white holes

Should build with provided VisualStudio files in VisualStudio 2022, be sure to use a Debug build. contents of Levels directory needs to be in same location as executable. Release builds run collision checks too quickly and break line circle collisions.

Could be improved by fixing release build bug, and adding additional obsticles or interactable objects.

example play of map 2 (.\PinballGame.exe PinballGame2.txt)

https://github.com/user-attachments/assets/56969f36-4c72-4bbd-81db-be72d5e394b0
