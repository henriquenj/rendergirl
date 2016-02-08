
REM This script will copy the RenderGirl plugin files to Blender user addons directory
REM called by VS when compilation is complete

REM grab general context, including how the DLL was compiled (debug or release, 32 or 64 bit)
set output_dir=%1
set target_name=%2
set project_dir=%cd%

REM Blender holds the user scripts on
REM %appdata%\Blender Foundation\Blender\<blender-version>\scripts\addons
REM The trick part is to fetch the blender version folder

cd "%appdata%\Blender Foundation\Blender"

REM This for loop will grab the first item in reverse-alphabetically order
REM which is our Blender version
FOR /F %%I IN ('DIR *.* /B /O:G-N') DO (
	set b_version=%%I
	goto :break
	)

:break
set b_addon_path="%appdata%\Blender Foundation\Blender\%b_version%\scripts\addons"

cd %b_addon_path%
REM Remove any previous RenderGirl plugin
if exist "RenderGirl" rd /S /Q "RenderGirl"
REM containing folder is ready
mkdir RenderGirl

cd RenderGirl
set render_girl_path=%cd%

REM Go back to project to move the binaries generated by VS
cd %project_dir%
cd ..\lib\%output_dir%
copy %target_name%.dll "%render_girl_path%"

REM Now move python scripts
cd ..\..\BlenderPlugin
copy *.py "%render_girl_path%"

REM Move .cl files
cd ..\Core
copy *.cl "%render_girl_path%"

echo "Files installed on %render_girl_path%"