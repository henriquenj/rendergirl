/*
	RenderGirl - OpenCL raytracer renderer
	Copyright(C) Henrique Jung

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "RenderGirlCore.h"


#include <Windows.h>
#include <commdlg.h> // For GetSaveFileName(), GetOpenFileName(), Etc.

// File Dialog Types:
enum FileDialogType { DialogOpen, DialogSave };

// ShowFileDialog function written thousand of years ago by Guilherme Lampert

/// Shows a file dialog window using a specific system service. Once the user selects a file from
/// the window and clicks the ok button, the function returns with the full name of the selected file.
/// The returned string should not be used directly, instead copy it to some other buffer, since it exists as
/// a per-thread static variable. In the case of the user closing the dialog window before selecting a file,
/// zero is returned. Zero is also returned if an error occurs.
/// You can pass many file filters as arguments. Use the '|' symbol to combine them. Example:
/// Parameter 'filterDesc': "Text Files (*.txt) | Bitmap Files (*.bmp)"
/// Parameter 'extFilter':  "*.txt | *.bmp"
const char * ShowFileDialog(/* Optional */void * window, int type, const char * filterDesc, const char * extFilter)
{
#if defined (_WIN32)

	// Reused From The Tile Map Editor...

	BOOL result;
	OPENFILENAMEA open_params;
	memset(&open_params, 0, sizeof(OPENFILENAMEA));

	// This should always use thread local storage.
	static char full_file_name[MAX_PATH];

	// Clear full_file_name every call:
	memset(full_file_name, 0, MAX_PATH);

	char * ext = const_cast<char *>(extFilter);
	char * filter = const_cast<char *>(filterDesc);

	char sorted_filter[4096]; // FIXME: Make buffer size safe some day.
	char * sorted_ptr = sorted_filter;

	for (;;) // Sort the seach filter:
	{
		while ((*filter != '|') && (*filter != '\0'))
			*sorted_ptr++ = *filter++;

		while (*(sorted_ptr - 1) == ' ') // Remove white spaces.
			--sorted_ptr;

		*sorted_ptr++ = '\0';

		while ((*ext != '|') && (*ext != '\0'))
			*sorted_ptr++ = *ext++;

		while (*(sorted_ptr - 1) == ' ') // Remove white spaces.
			--sorted_ptr;

		*sorted_ptr++ = '\0';

		if ((*filter == '|') || (*ext == '|'))
		{
			// Skip white spaces:

			while (*(++filter) == ' ')
				;
			while (*(++ext) == ' ')
				;
		}
		else
		{
			*sorted_ptr++ = '\0';
			break;
		}
	}

	// Make the system call:
	open_params.lStructSize = sizeof(OPENFILENAMEA);
	open_params.hwndOwner = reinterpret_cast<HWND>(window);
	open_params.lpstrFilter = sorted_filter;
	open_params.lpstrFile = full_file_name;
	open_params.nMaxFile = MAX_PATH;
	open_params.Flags = (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY);

	if (type == DialogSave)
		result = GetSaveFileNameA(&open_params);

	else if (type == DialogOpen)
		result = GetOpenFileNameA(&open_params);

	else result = FALSE;

	return ((result == FALSE) ? 0 : full_file_name);

#else
#	error ShowFileDialog() not implemented on this platform.
#endif
}


/* Loads an obj file providing a path. WARNING: it's your responsability to delete this memory. 
	This loader does not implements the full specification of the format. Return NULL for an error. */
Scene3D* LoadOBJ(const char* fileName)
{

	/* Please bear in mind that this loader is not suppose to be comprehensive, 
		since RenderGirl is suppose to work with other 3D softwares.
		You may notice the lack of asserts. */

	FILE* objFile = fopen(fileName, "r");

	if (objFile == NULL)
	{
		return NULL;
	}

	fseek(objFile, 0, SEEK_END);
	long size = ftell(objFile);
	fseek(objFile, 0, SEEK_SET);

	// raw content of obj file
	char* objContent = new char[size];
	fread(objContent, sizeof(char), size, objFile);
	fclose(objFile);

	// now put the data on a nice Scene3D
	Scene3D* scene = new Scene3D();
	memset(scene, 0, sizeof(Scene3D));


	/* First pass, count objects */
	int counter = 0;
	while (counter < size)
	{
		if (objContent[counter] == 'v' && objContent[counter + 1] == 'n')
			scene->normalSize++;
		else if (objContent[counter] == 'f')
			scene->facesSize++;
		else if (objContent[counter] == 'v' && objContent[counter + 1] == ' ')
			scene->verticesSize++;

		// jump line
		while (true)
		{
			if (objContent[counter] == 10)
			{
				counter++;
				break;
			}
			counter++;
		}
	}

	// alloc enough memory
	scene->faces = new cl_int3[scene->facesSize];
	scene->normal = new cl_float3[scene->normalSize];
	scene->vertices = new cl_float3[scene->verticesSize];

	int faceCount, verticesCount, normalCount;
	faceCount = verticesCount = normalCount = 0;

	/* Second pass, organize the data*/
	counter = 0;

	while (counter < size)
	{
		// normals
		if (objContent[counter] == 'v' && objContent[counter + 1] == 'n')
		{
			counter+=3;
			sscanf(objContent + counter, "%f %f %f", &(scene->normal[normalCount].s[0]),
													&(scene->normal[normalCount].s[1]),
													&(scene->normal[normalCount].s[2]));
			normalCount++;
		}
		// faces
		else if (objContent[counter] == 'f')
		{
			counter+=2;
			cl_int temp = 0; // to fill with descarted data
			sscanf(objContent + counter, "%i/%i/%i %i/%i/%i %i/%i/%i",
				&(scene->faces[faceCount].s[0]),&temp,&temp,
				&(scene->faces[faceCount].s[1]),&temp,&temp,
				&(scene->faces[faceCount].s[2]), &temp, &temp);

			// make C-like indexes since OBJ file format use indexes starting in 1
			scene->faces[faceCount].s[0]--;
			scene->faces[faceCount].s[1]--;
			scene->faces[faceCount].s[2]--;
			faceCount++;
		}
		//vertices
		else if (objContent[counter] == 'v' && objContent[counter + 1] == ' ')
		{
			counter+=2;
			// load line
			sscanf(objContent + counter, "%f %f %f", &(scene->vertices[verticesCount].s[0]),
													&(scene->vertices[verticesCount].s[1]),
													&(scene->vertices[verticesCount].s[2]));
			verticesCount++;
		}

		// jump line
		while (true)
		{
			if (objContent[counter] == 10)
			{
				counter++;
				break;
			}
			counter++;
		}

	}
	
	delete objContent;
	return scene;


}