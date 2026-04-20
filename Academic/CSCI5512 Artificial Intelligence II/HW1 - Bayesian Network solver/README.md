Name: Trevor LeCrone

Uses a Gibbs sampler to evaluates probability "Rain" in the provided network:

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/Network.PNG?raw=true)

Running GibbsRain.py:

Simply run the file in a command line with the number of steps immediately afterward for example: `GibbsRain.py 10000`
If no argument is passed, the program defaults to 100 steps. No additional files are needed to run the file.
As noted in the in-line comments in my code, I don't track S or W in the state var since they never change.
