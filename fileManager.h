#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <experimental/filesystem>
#include "json.hpp"
#include <SFML/Graphics.hpp>
#define DATA_PATH "B:/nuImagesDataset"

class fileManager
{
public:
	

	struct frameData
	{
		std::string camFrontName;
		std::string sample_data_token;
	};

	fileManager();
	// THIS MAP KEEPS IFSTREAM FOR EACH FILE BEING READ BECAUSE IT WOULD BE REALLY HARD TO NAVIGATE TO THE LAST POINT EACH TIME
	// <name, ifstream>
	std::map<std::string, std::ifstream*> files;

	// Holds the filenames in CAM_FRONT
	std::vector<std::string> camFrontNames;
	

	// ADD A NEW FILE WITH THE FILE PATH
	void addFile(std::string fileName, std::string filePath);

	// Size = number of objects to be read, 0 reads the whole file
	nlohmann::json readFromFile(int size, std::ifstream* readFile);

	// size = how many objects to look through, obj = found object
	// find the object in which "key" = "value"
	bool findObjectInFile(int size, std::ifstream* readFile, nlohmann::json& jsonObj, std::string key, std::string value);
	bool findObject(nlohmann::json& readObj, nlohmann::json& jsonObj, std::string key, std::string value);
	bool findAllObjects(nlohmann::json& readObj, std::vector<nlohmann::json>& objects, std::string key, std::string value);


	std::string getCategoryName(std::string token);
	
	// returns the next cam front image
	sf::Texture* loadNextCamFront();

	// return name of the current cam front image
	std::string getCamFrontName();
	

	// sample_data.json
	nlohmann::json sampleData;
	// object_ann.json
	nlohmann::json objectAnn;

private:
	// Category.json
	nlohmann::json categories;
	
	

	int camFrontNamesNext = 5000;

	void initFiles();
	// Loads file names into a vector in a given directory
	void loadNames(std::string path, std::vector<std::string>&);
};

