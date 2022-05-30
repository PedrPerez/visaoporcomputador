
#include "ImageDb.h"

ImageDb::ImageDb(string filePath, string category) {
	this->filePath = filePath;
	this->category = category;
}

bool ImageDb::parseMetaDataFile(string filepath, vector<ImageDb>& out) {

	ifstream f(filepath);
	string line;
	bool ret = true;

	if (f.is_open() == false) return false;

	while (getline(f, line)) { //read data from file object and put it into string.
		char str[1024];
		strcpy(str, line.c_str());

		char * img = strtok(str, ";");
		if (img == NULL) { ret = false;  break; }

		char* tipo = strtok(NULL,";");
		if (tipo == NULL) { ret = false;  break; }

		if(strcmp(img,"imagem") != 0 && strcmp(tipo,"tipo") != 0){
			out.push_back(ImageDb("db\\" + string(img), "db\\" + string(tipo)));
		}
	}
	
	f.close();

	return ret;
}