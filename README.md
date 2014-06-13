Render Girl by Henrique Jung


Notes on the source code:

The only build system currently available is for Visual Studio 2013, located in the
VS2013 folder. The compiled software has only the Visual C++ Redistributable 2013 as dependency,
you can download it here  http://www.microsoft.com/en-gb/download/details.aspx?id=40784. The 
Raytracer.cl source file should be on the same folder as the compiled software.

In order to compile the software, simply open the RenderGirlOpenCL.sln file and select a platform
(either Win32 or x64) and press Build > Build Solution. The selected StartUp project should be
always be RenderGirlApp if you want to run the software from inside Visual Studio.

The VS solution is organized as follows: the wxWidgets solution folder contains the wxWidgets 
projects, the RenderGirlApp project contains the wxWidgets interface for the Core and the 
RenderGirlCore project contains the Core itself.

The source code folder structure is organized as follows:
/VS2013 				contains the VS2013 build files and the intermediate directories
/Core					RenderGirl Core source code, the Raytracer.cl and OpenCL headers and libraries
/App					wxWidgets interface source code, as well as wxWidgets itself

Upon compilation, the following folders are created
/lib					the final compiled core (a .lib on windows) goes to this folder
/bin					the final cimpiled wx interface goes to this folder


WARNING: Please be aware that the provided OBJ loader does not implement the full specification of
the format. The renderer only render triangles, so assure that you are loading
a triangulated mesh.

Licensed under LGPL.

For additional information about RenderGirl, newest versions or any issues related to it, 
please access the project page hosted at https://code.google.com/p/rendergirl/.