#pragma once
#include "libs.h"


class ImageDb
{

private:
	string filePath;
	string category;


public:
	inline ImageDb() { this->filePath = ""; this->category = ""; }
	ImageDb(string filePath, string category);
	inline string getFilePath() { return this->filePath; }
	inline string getCategory() { return this->category;  }

	static bool parseMetaDataFile(string filepath, vector<ImageDb>& out );
};

