/*
	RenderGirl - OpenCL raytracer renderer
	Copyright (c) 2014, Henrique Jung, All rights reserved.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this program.
*/

#ifndef __OBJLOADER__
#define __OBJLOADER__

#include <string>
#include <vector>
#include "RenderGirlShared.h"

// strip a path string from the file name (the last name)
void RemoveFileName(std::string &path);


/* Load materials with names from a MTL file, the amount of materials on the returning pointer
	is the size of the vector of strings, not very pretty I know*/
Material* LoadMTL(std::vector<std::string>& materialName, const char* file);

/* Loads an obj file providing a path. WARNING: it's your responsability to delete this memory. 
	This loader does not implements the full specification of the format. Return NULL for an error. */
Scene3D* LoadOBJ(const char* fileName);

/*
Analyse the definition of a face and return the proper face definition.
OBJ file format has 4 different types of face definitions:
0 = unidentified
1 = f v1			(vertex only)
2 = f v1/vt1		(vertex and texture)
3 = f v1//vn1		(vertex and normal)
4 = f v1/vt1/vn1	(vertex, texture and normal)
*/
int GetFaceDefinition(char* objContent, int counter, int size);


#endif // __OBJLOADER__