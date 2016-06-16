[![Build status](https://ci.appveyor.com/api/projects/status/bmyr05fi72274srf/branch/master?svg=true)](https://ci.appveyor.com/project/henriquenj/rendergirl/branch/master)

Render Girl
=======
RenderGirl is an ongoing effort to build an open source raytracer capable of running inside the OpenCL architecture. There are currently few options available for this kind of software, since most part of the implementations are either on CUDA, closed-source, or both.

The core part of the renderer is separated from the interfaces with the user in order to easily create additional front-end for the renderer; those front-ends can be applications such as 3DS Max, Blender and SketchUp.

Want to help? Know how to code? Have experience building plugins for one or more of the applications stated above? Fork my repository, pull requests are welcome.

For additional information please visit the [wiki pages](https://github.com/henriquenj/rendergirl/wiki)

***

**Notes on the source code**

The source code folder structure is organized as follows:
- **/VS2013**: contains the VS2013 build files and the intermediate directories
- **/Core**: RenderGirl Core source code, the Raytracer.cl and OpenCL headers and libraries
- **/App**: wxWidgets interface source code, as well as the submodule for wxWidgets itself
- **/BlenderPlugin**: Blender plugin source code, including C wrappers to the core and the python scripts that load the core into Blender.
- **/Console**: Source code for the console project

***Compilation of the Core, Console and App projects***

The only build system currently available is for Visual Studio 2013, located in the
VS2013 folder. The compiled software has only the [Visual C++ Redistributable 2013](http://www.microsoft.com/en-gb/download/details.aspx?id=40784) as dependency. The Raytracer.cl source file should be on the same folder as the compiled software.

In order to compile the software, simply open the RenderGirlOpenCL.sln file and select a platform (either Win32 or x64) and press Build > Build Solution. The selected StartUp project should either be RenderGirlApp or RenderGirlConsole if you want to run the software from inside Visual Studio.

The VS solution is organized as follows: the wxWidgets solution folder contains the wxWidgets projects, the RenderGirlApp project contains the wxWidgets interface, the RenderGirlConsole contains the console-only interface, the BlenderPlugin project contains the Blender C wrappers and deployment scripts and finally the RenderGirlCore project contains the Core itself.

Upon compilation, the following folders are created
- **/lib**: the final compiled core (a .lib on windows) goes to this folder
- **/bin**: the final compiled wx interface goes to this folder

***Compilation of the Blender plugin interface***

The compilation of the plugin follows the same approach. You must compile the BlenderPlugin project on VisualStudio, which will run the `deploy_blender_plugin.bat` script that will take care of moving the build artifacts and scripts to your Blender plugins folder. The folder should be `%appdata%\Blender Foundation\Blender\<blender-version>\scripts\addons`, so please check if it was installed correctly.

Within Blender, go to `File` > `User Preferences`, `Addon`  tab and filter the addons by "Testing", chances are that RenderGirl is the only testing addon within your Blender, so you can activate it from there. After activating, you may check the System Console Window for status of the renderer. To use RenderGirl you have to select it on the drop-down menu of renderers on Blender.

***

**WARNING:** Please be aware that the provided OBJ loader **does not** implement the full specification of the format. The renderer only render triangles, so assure that you are loading a triangulate mesh. To overcome this, you might use the Blender plugin interface.

Licensed under LGPL.

[Project page](https://github.com/henriquenj/rendergirl) hosted at Github.
