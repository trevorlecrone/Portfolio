#pragma once
#include <iostream>
#include <cstdio>

using namespace std;

class LevelParser {
public:

	string* parse(const char* filename_, int &width, int &height, int &type) {
		printf("parsing input file...\n");
		FILE *fp;
		long length;
		char line[1024]; //Assumes no line is longer than 1024 characters!

		string fileName = filename_;

		// open the file containing the scene description
		fp = fopen(fileName.c_str(), "r");

		// check for errors in opening the file
		if (fp == NULL) {
			printf("Can't open file '%s'\n", fileName.c_str());
			return 0;  //Exit
		}

		// determine the file size (this is optional -- feel free to delete the 4 lines below)
		fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
		length = ftell(fp);  // return the value of the current position
		printf("File '%s' is %ld bytes long.\n\n", fileName.c_str(), length);
		fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
		fgets(line, 1024, fp);
		int x_size, y_size, l_type;
		sscanf(line, "%d %d %d", &x_size, &y_size, &l_type);
		width = x_size;
		height = y_size;
		type = l_type;
		string* lMatrix = new string[y_size];
		int i = 0;
		while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
			char characters[100];
			sscanf(line, "%s", characters);
			string cString = characters;
			lMatrix[i] = cString;
			i++;
		}
		fclose(fp);
		return lMatrix;
	}
};