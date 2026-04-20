# Command Line Ray Tracer
A Command line ray tracer that accepts .scn files (formatted text documents provided by the instructor) and generates .png outputs based on the objects in the scene. Scene files are collections of lights (spot lights, point lights and directional lights), and surfaces (spheres, standard triangles, and triangles with normals assigned to each vertex) that have some material components (color, specularity, index of refraction etc.) The tool parses the scene file and then generates the .png output using 1 sample ray per pixel.

Improvements I plan to make (in addition to general code cleanup) include adding a pre-processor for .scn files to add bounding volume hierarchies to the scene to vastly speed up rendering, adding options for MSAA, and potentially writing a CUDA kernel to perform the ray tracing in parallel

Should build with provided VisualStudio files in VisualStudio 2022. -openmp flag may need manually to be set to parallelize

Here are a few sample outputs

.\RayTracer.exe ../../SceneFiles/Dragon.scn

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/dragon.png?raw=true)


.\RayTracer.exe ../../SceneFiles/bottle.scn

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/bottle.png?raw=true)


.\RayTracer.exe ../../SceneFiles/spheres2.scn

![alt text](https://github.com/trevorlecrone/PersonalAndSchool/blob/main/DemoImagesAndVideos/spheres2.png?raw=true)