# MazeGame expansion
An expansion on the initial maze game that a secondary sokoban game-mode. Sokoban levels are accessed via objects in the main maze game and grant bombs when completed. Used as an excersize in preserving level states and expanding on previous work (hence the VS solution also being called Project5). This was a group project whereas the inital maze game was individual. We used my original maze game as a basis as it was the most complete. I did the modelling and texturing work, and my group members wrote the function for the bomb and code and to load in and out of sokloban levels.

Improvements I plan to make (in addition to general code cleanup) include fixing key and bomb inventory rotation behavior, getting stuck in walls when moving too fast due to high frame rate, and decoupling the physics tick rate and rendering frame rate

Should build with provided VisualStudio files in VisualStudio 2022. Contents of RequiredResources directory need to be in the folder the executable is in

Here are a few sample screenshots

Gameboy objects load sokoban levels when interacted with.

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/GameBoy.PNG?raw=true)

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/Sokoban.PNG?raw=true)


Completing the Sokoban level gives a bomb, which can be used to remove cracked walls

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/Bomb.PNG?raw=true)

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/Goal2.PNG?raw=true)
