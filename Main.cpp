#include <SFML/Graphics.hpp>
#include <iostream>
#include "json.hpp"
#include <fstream>
#include <sstream>



// THIS VECTOR KEEPS IFSTREAM FOR EACH FILE BEING READ BECAUSE IT WOULD BE REALLY HARD TO NAVIGATE TO THE LAST POINT EACH TIME
// <name, ifstream>
std::map<std::string,std::ifstream*> files;

// ADD A NEW FILE WITH THE FILE PATH
void addFile(std::string fileName,std::string filePath)
{
    files.insert(std::pair<std::string, std::ifstream*>(fileName, new std::ifstream(filePath)));
}
void initFiles()
{
    std::string dataPath = "B:/nuScenesDataset";
    addFile("sample_data", dataPath.append("/1.0-trainval-meta/v1.0-trainval/sample_data.json"));
    addFile("sample", dataPath.append("/1.0-trainval-meta/v1.0-trainval/sample.json"));
    addFile("category", dataPath.append("/1.0-trainval-meta/v1.0-trainval/category.json"));
    addFile("sample_annotation", dataPath.append("/1.0-trainval-meta/v1.0-trainval/sample_annotation.json"));
}


// Size = number of objects to be read
nlohmann::json readFromFile(int size, std::ifstream* readFile)
{
    std::stringstream result;
    std::string line;

    int objectIndex = 0;
    bool stopOnObjectEnd = false;
    bool stop = false;
    
    // Get one line out of the loop to determine if an extra "[" is needed
    std::getline(*readFile, line);
    if (line[0] != '[') { line = "[{"; }
    result << line;
    objectIndex++;

    while (std::getline(*readFile, line) && !stop)
    {
    
        if (line[0] == '}') 
        { 
           objectIndex++;
           if (stopOnObjectEnd)
           {
                line.pop_back();
                line.append("]");
                stop = true;
           }
        }

        result << line;
  
        if (objectIndex == size) { stopOnObjectEnd = true; }
    }
    nlohmann::json jsonFile;
    std::string debugString = result.str();
    jsonFile = nlohmann::json::parse(result);
    return jsonFile;
}

// size = how many objects to look through, obj = found object
bool findObject(int size, std::ifstream* readFile, nlohmann::json& jsonObj, std::string key, std::string value)
{
    int portion = 10000;
    if (size < portion) { portion = size; }
    nlohmann::json jsonFile = readFromFile(portion, readFile);
    std::string valueString = "\"" + value + "\"";
    
    // remove all the spaces
    std::remove(valueString.begin(),valueString.end(),' ');
    valueString.pop_back();
    while (valueString[valueString.size()-1] != '"')
    {
        valueString.pop_back();
    }

    std::remove(valueString.begin(),valueString.end(),' ');

    bool exit = false;
    int i = 0;
    while (!exit)
    {
        for (int i = 0; i < size; i++)
        {
            std::string keyString = jsonFile[i][key].dump();
            if (!keyString.compare(valueString))
            {
                jsonObj = jsonFile[i];
                return true;
            }
        }
        jsonFile = readFromFile(portion, readFile);
        i++;
        if (portion * i > size) { exit = true; }
    }
    return false;
}

int main()
{
    initFiles();

    
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "VP_CL");

    nlohmann::json jsonFile;
    std::string fileName = "samples/CAM_FRONT/n008 - 2018 - 08 - 01 - 15 - 16 - 36 - 0400__CAM_FRONT__1533151061512404.jpg";
    if (findObject(500000, files.at("sample_data"), jsonFile, "filename", fileName))
    {
        std::cout << jsonFile.dump() << std::endl;
    }
    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.display();
    }

    return 0;
}