#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "json.hpp"
#include <SFML/Graphics.hpp>
#define DATA_PATH "B:/nuScenesDataset/1.0-trainval01"

class fileManager
{
public:
	
	struct annotation
	{
		

	};

	struct frameData
	{
		std::string camFrontName;
		std::string sample_data_token;
	};

	fileManager();
	// THIS VECTOR KEEPS IFSTREAM FOR EACH FILE BEING READ BECAUSE IT WOULD BE REALLY HARD TO NAVIGATE TO THE LAST POINT EACH TIME
	// <name, ifstream>
	std::map<std::string, std::ifstream*> files;

	// Holds the filenames in CAM_FRONT
	std::vector<std::string> camFrontNames;
	

	// ADD A NEW FILE WITH THE FILE PATH
	void addFile(std::string fileName, std::string filePath);

	// Size = number of objects to be read
	nlohmann::json readFromFile(int size, std::ifstream* readFile);

	// size = how many objects to look through, obj = found object
	// find the object in which "key" = "value"
	bool findObject(int size, std::ifstream* readFile, nlohmann::json& jsonObj, std::string key, std::string value);
	
	// returns the next cam front image
	sf::Texture* loadNextCamFront();

	// return name of the current cam front image
	std::string getCamFrontName();
	



private:
	// Category.json
	nlohmann::json categories;


	int camFrontNamesNext = 0;

	void initFiles();
	// Loads file names into a vector in a given directory
	void loadNames(std::string path, std::vector<std::string>&);
};

