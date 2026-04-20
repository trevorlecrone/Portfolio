# Command Line Image editor
A command line tool that can perform various operations on .bmp images including dithering, saturation changes, channel extracion, edge detection etc. Was an excersize in understanding pixel manipulation and convolution matricies.

Should build with provided VisualStudio files in VisualStudio 2022.

For example, using the following image, the Edge Detection and Quantize functions were run
![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/rubiks.bmp?raw=true)

Edge Detection output (.\ImageEditor.exe -input rubiks.bmp -edgeDetect -output edgeRubiks.bmp)
![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/edgeRubiks.bmp?raw=true)

Quantize (2 bits) output (.\ImageEditor.exe -input rubiks.bmp -quantize 2 -output quantRubiks.bmp)
![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/quantRubiks.bmp?raw=true)

