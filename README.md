# OpenGLuitar3D

Playable virtual guitar implemented in 3D OpenGL using C++. Windows-only. All audio files provided by me since I couldn't find any free sample collection.

One core issue is that the application is limited to 1920x1080 monitors because there is no responsiveness built in.

Press `P` to turn on playmode.

## Libraries
- `glfw.3.3.8`
- `glew-2.2.0.2.2.0.1`
- `glm-0.9.9.800`
- `assimp-5.2.5`

## Installing
You need to have the `vcpkg` C++ manager to be able to successfully install `assimp`. `vcpkg.json` contains the library definition (manifest mode), so you can do `vcpkg install`.
`vcpkg` is also enabled in `.vcxproj` so running the project with `vcpkg` globally installed should download the package automatically in MSBuild mode.

[More on vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-powershell).

## Credits
[Model](https://www.turbosquid.com/3d-models/acoustic-guitar-1681137) (although the model is heavily modified to the project model loader's needs)
[Background image source](https://worldguitars.co.uk/pages/about-us)

## License
MIT.

## Preview

![App preview 1](https://raw.githubusercontent.com/lukascobbler/OpenGLuitar3D/refs/heads/master/preview1.png "App preview 1")
![App preview 2](https://raw.githubusercontent.com/lukascobbler/OpenGLuitar3D/refs/heads/master/preview2.png "App preview 2")
