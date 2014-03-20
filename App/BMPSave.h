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


#include <Windows.h>


/* As with the OBJ loader, this writer is not suppose to be comprehensive*/
void SaveBMP(const char* path, const int width, const int height, const BYTE* pixels)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));

	// header
	bmfh.bfType = 19778; // bmp magic number
	bmfh.bfSize = sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER)+(width * height * 3);
	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	// info header
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biBitCount = 24;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biPlanes = 1;
	bmih.biSizeImage = width * height * 3;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biCompression = BI_RGB;
	// write file
	//TODO: make a proper BMP writer
	FILE* bmpImageFile = fopen(path, "w");
	fwrite(&bmfh, sizeof (BITMAPFILEHEADER), 1, bmpImageFile);
	fwrite(&bmih, sizeof (BITMAPINFOHEADER), 1, bmpImageFile);
	fwrite(pixels, sizeof(char), width * height * 3, bmpImageFile);
		
	fclose(bmpImageFile);
}

/* Workaround method to convert the data, this method make a copy,
	so you have to delete it after using. 
	Oh shit this is a really uggly name for a function*/
BYTE* UChar4ToBYTE(const cl_uchar4* originalData, int x, int y)
{
	BYTE* newData = new BYTE[x * y * 3];
	for (int t = 0, i = 0; t < x * y; t+=1,i+=3)
	{
		newData[i] = originalData[t].s[0];
		newData[i + 1] = originalData[t].s[1];
		newData[i + 2] = originalData[t].s[2];
	}

	return newData;
}