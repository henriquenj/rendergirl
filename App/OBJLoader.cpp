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


#include "OBJLoader.h"

void RemoveFileName(std::string& path)

{
	while (true)
	{
		if (path[path.size() - 1] != '\\')
		{
			path.pop_back();
		}
		else
			break;
	}
}


Material* LoadMTL(std::vector<std::string>& materialName, const char* file)
{
	FILE* mtlFile = fopen(file, "r");

	if (mtlFile == NULL)
	{
		return NULL;
	}

	// store on a vector and later on put on pure C++ array
	std::vector<Material> materials;

	fseek(mtlFile, 0, SEEK_END);
	long size = ftell(mtlFile);
	fseek(mtlFile, 0, SEEK_SET);

	// raw content of mtl file
	char* mtlContent = new char[size];
	fread(mtlContent, sizeof(char), size, mtlFile);
	fclose(mtlFile);


	int counter = 0;
	int currentMaterial = 0; // index of current material
	Material tempMaterial;
	memset(&tempMaterial, 0, sizeof(tempMaterial));

	while (counter < size)
	{
		if (mtlContent[counter] == 'n' && mtlContent[counter + 1] == 'e' &&
			mtlContent[counter + 2] == 'w')
		{
			if (currentMaterial != 0)
			{
				// if this is the first added material, we don't have a previously one to add to the vector
				//...otherwise
				materials.push_back(tempMaterial);
				// reset material
				memset(&tempMaterial, 0, sizeof(tempMaterial));
			}
			counter += 7;
			std::string t_Name;
			for (int p = 0; mtlContent[counter + p] != 10; p++)
			{
				t_Name.push_back(mtlContent[counter + p]);
			}
			materialName.push_back(t_Name);
			currentMaterial++;
		}
		else if (mtlContent[counter] == 'K' && mtlContent[counter + 1] == 'a')
		{
			counter += 3;
			// ambient color
			sscanf(mtlContent + counter, "%f %f %f", &(tempMaterial.ambientColor.s[0]),
				&(tempMaterial.ambientColor.s[1]),
				&(tempMaterial.ambientColor.s[2]));
		}

		else if (mtlContent[counter] == 'K' && mtlContent[counter + 1] == 'd')
		{
			counter += 3;
			// diffuse color
			sscanf(mtlContent + counter, "%f %f %f", &(tempMaterial.diffuseColor.s[0]),
				&(tempMaterial.diffuseColor.s[1]),
				&(tempMaterial.diffuseColor.s[2]));
		}
		else if (mtlContent[counter] == 'K' && mtlContent[counter + 1] == 's')
		{
			counter += 3;
			// specular color
			sscanf(mtlContent + counter, "%f %f %f", &(tempMaterial.specularColor.s[0]),
				&(tempMaterial.specularColor.s[1]),
				&(tempMaterial.specularColor.s[2]));
		}

		// jump line
		while (true)
		{
			if (mtlContent[counter] == 10)
			{
				counter++;
				break;
			}
			counter++;
		}
	}

	// add the last material to the list
	materials.push_back(tempMaterial);

	// put into a pure C-array
	Material* r_material = new Material[materialName.size()];
	for (unsigned int p = 0; p < materialName.size(); p++)
	{
		memcpy(r_material + p, &materials[p], sizeof(Material));
	}

	delete[] mtlContent;
	return r_material;
}


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
		{
			scene->facesSize++;
		}
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
	scene->faces = new cl_int4[scene->facesSize];
	scene->normal = new cl_float3[scene->normalSize];
	scene->vertices = new cl_float3[scene->verticesSize];

	int faceCount, verticesCount, normalCount;
	faceCount = verticesCount = normalCount = 0;

	/* Second pass, organize the data*/
	counter = 0;

	std::vector<std::string> materialNames;
	int currentMaterial = -1; //-1 means faces with no material

	while (counter < size)
	{
		// normals
		if (objContent[counter] == 'v' && objContent[counter + 1] == 'n')
		{
			counter += 3;
			sscanf(objContent + counter, "%f %f %f", &(scene->normal[normalCount].s[0]),
				&(scene->normal[normalCount].s[1]),
				&(scene->normal[normalCount].s[2]));
			normalCount++;
		}
		// faces
		else if (objContent[counter] == 'f')
		{
			/* check face defintion on each new face (some programs output with all kinds of face definitons)*/
			int faceDefinition = GetFaceDefinition(objContent, counter);
			counter += 2;
			cl_int temp = 0; // to fill with descarted data

			if (faceDefinition == 1)
			{
				sscanf(objContent + counter, "%i %i %i",
					&(scene->faces[faceCount].s[0]),
					&(scene->faces[faceCount].s[1]),
					&(scene->faces[faceCount].s[2]));
			}
			else if (faceDefinition == 2)
			{
				sscanf(objContent + counter, "%i/%i %i/%i %i/%i",
					&(scene->faces[faceCount].s[0]), &temp,
					&(scene->faces[faceCount].s[1]), &temp,
					&(scene->faces[faceCount].s[2]), &temp);
			}
			else if (faceDefinition == 3)
			{
				sscanf(objContent + counter, "%i//%i %i//%i %i//%i",
					&(scene->faces[faceCount].s[0]), &temp,
					&(scene->faces[faceCount].s[1]), &temp,
					&(scene->faces[faceCount].s[2]), &temp);
			}
			else
			{
				sscanf(objContent + counter, "%i/%i/%i %i/%i/%i %i/%i/%i",
					&(scene->faces[faceCount].s[0]), &temp, &temp,
					&(scene->faces[faceCount].s[1]), &temp, &temp,
					&(scene->faces[faceCount].s[2]), &temp, &temp);
			}
			// make C-like indexes since OBJ file format use indexes starting in 1
			scene->faces[faceCount].s[0]--;
			scene->faces[faceCount].s[1]--;
			scene->faces[faceCount].s[2]--;
			scene->faces[faceCount].s[3] = currentMaterial;
			faceCount++;
		}
		//vertices
		else if (objContent[counter] == 'v' && objContent[counter + 1] == ' ')
		{
			counter += 2;
			// load line
			sscanf(objContent + counter, "%f %f %f", &(scene->vertices[verticesCount].s[0]),
				&(scene->vertices[verticesCount].s[1]),
				&(scene->vertices[verticesCount].s[2]));
			verticesCount++;
		}
		// material name
		else if (objContent[counter] == 'u' && objContent[counter + 1] == 's' &&
			objContent[counter + 2] == 'e')
		{
			//subsequent faces will have this material
			counter += 7;
			std::string t_Name;
			for (int p = 0; objContent[counter + p] != 10; p++)
			{
				t_Name.push_back(objContent[counter + p]);
			}
			for (unsigned int p = 0; p < materialNames.size(); p++)
			{
				// compare strings until find this particular material
				if (t_Name.compare(materialNames[p]) == 0)
				{
					// p is the index of our material
					currentMaterial = p;
					break;
				}
			}
		}
		// mtl lib
		else if (objContent[counter] == 'm' && objContent[counter + 1] == 't')
		{
			counter += 7;
			/* since I don't now the size of the name, let's do something a little bit different*/
			std::string mtlFile;
			for (int i = 0; objContent[counter + i] != 10; i++)
			{
				mtlFile.push_back(objContent[counter + i]);
			}
			// setup material path
			std::string mtlPath(fileName);
			RemoveFileName(mtlPath);
			mtlPath += mtlFile;
			/* We NEED the name of the materials to syncronize the indexes with the names later on.
			You may ask: why not put a std::string inside the material struct? Well, this struct
			is suppose to fit into an OpenCL device, so I can't use that kind of object there
			*/
			// fill material information
			scene->materials = LoadMTL(materialNames, mtlPath.c_str());
			if (scene->materials != NULL)
				scene->materialSize = materialNames.size();

			/* to find the names of the materials, just look at the materialName vector, they have the same indexes
			like this: materialName[i] matches the materials[i]   */

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

	delete[] objContent;
	return scene;

}


int GetFaceDefinition(char* objContent, int counter)
{

	counter += 2;
	// keep walking until finds something to checks the face definition type
	for (int p = counter;; p++)
	{
		if (objContent[p] == ' ') // vertex only
		{
			return 1;
			break;
		}
		else if (objContent[p] == '/' && objContent[p + 1] == '/')//vertex and normal mode
		{
			return 3;
			break;
		}
		else if (objContent[p] == '/')
		{
			while (true)
			{
				p++;
				if (objContent[p] == '/') // vertex, texture and normal mode
				{
					return 4;
					break;
				}
				else if (objContent[p] == ' ') //vertex and texture
				{
					return 2;
					break;
				}
			}
			break;
		}

	}

}