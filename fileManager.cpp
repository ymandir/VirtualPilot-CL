#include "fileManager.h"
fileManager::fileManager()
{
    initFiles();

    std::string camFrontPath = DATA_PATH;
    camFrontPath.append("/samples/CAM_FRONT");
    loadNames(camFrontPath,camFrontNames);

    categories = readFromFile(0,files.at("category"));
    sampleData = readFromFile(0, files.at("sample_data"));
    objectAnn = readFromFile(0, files.at("object_ann"));
}

// ADD A NEW FILE WITH THE FILE PATH
void fileManager::addFile(std::string fileName, std::string filePath)
{
	files.insert(std::pair<std::string, std::ifstream*>(fileName, new std::ifstream(filePath)));
}

// Loads file names into a vector in a given directory
void fileManager::loadNames(std::string path, std::vector<std::string>& nameStack)
{
    for (const auto& entry : std::experimental::filesystem::directory_iterator(path))
    {
        std::string name = entry.path().string().substr(path.size() + 1, entry.path().string().size() - path.size() - 1);
        nameStack.push_back(name);
    }
}

void fileManager::initFiles()
{
    std::string dataPath = "B:/nuImagesDataset";
    addFile("sample_data", dataPath + "/v1.0-train/sample_data.json");
    addFile("category", dataPath + "/v1.0-train/category.json");
    addFile("sample", dataPath + "/v1.0-train/sample.json");
    addFile("object_ann", dataPath + "/v1.0-train/object_ann.json");
    addFile("sample_annotation", dataPath + "/v1.0-train/sample_annotation.json");
}


// Size = number of objects to be read
nlohmann::json fileManager::readFromFile(int size, std::ifstream* readFile)
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
    int openBracket = 0;
    
    while (std::getline(*readFile, line) && !stop)
    {
        if (line[0] == '{')
        {
            openBracket++;
        }
        
        if (line[0] == '}')
        {
            openBracket--;
            if (openBracket == 0) { objectIndex++; }
            if (stopOnObjectEnd)
            {
                line.pop_back();
                line.append("]");
                stop = true;
            }
        }
        else if (!line.compare("]")) { stop = true; }
        
        result << line;

        if (objectIndex == size) { stopOnObjectEnd = true; }
    }
    nlohmann::json jsonFile;
    std::string debugString = result.str();
    jsonFile = nlohmann::json::parse(result);
    return jsonFile;
}


// size = how many objects to look through, obj = found object
bool fileManager::findObjectInFile(int size, std::ifstream* readFile, nlohmann::json& jsonObj, std::string key, std::string value)
{
    if (size == 0) { size = 2147483647; }
    int portion = 100000;
    if (size < portion) { portion = size; }
    nlohmann::json jsonFile = readFromFile(portion, readFile);
    std::string valueString = "\"" + value + "\"";


    // only do white space removal if there are any whitespaces to remove
    if (valueString.find(" ") != std::string::npos)
    {
        // remove all the spaces
        std::remove(valueString.begin(), valueString.end(), ' ');
        valueString.pop_back();

        while (valueString[valueString.size() - 1] != '"')
        {
            valueString.pop_back();
        }

        std::remove(valueString.begin(), valueString.end(), ' ');
    }
    

    bool exit = false;
    int i = 0;
    while (!exit)
    {
        for (int i = 0; i < portion; i++)
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

bool fileManager::findObject(nlohmann::json& readObj, nlohmann::json& jsonObj, std::string key, std::string value)
{
    std::string valueString = "\"" + value + "\"";


    // only do white space removal if there are any whitespaces to remove
    if (valueString.find(" ") != std::string::npos)
    {
        // remove all the spaces
        std::remove(valueString.begin(), valueString.end(), ' ');
        valueString.pop_back();

        while (valueString[valueString.size() - 1] != '"')
        {
            valueString.pop_back();
        }

        std::remove(valueString.begin(), valueString.end(), ' ');
    }


    for (int i = 0; i < readObj.size(); i++)
    {
        std::string keyString = readObj[i][key].dump();
        if (!keyString.compare(valueString))
        {
            jsonObj = readObj[i];
            return true;
        }
    }

    return false;
}

bool fileManager::findAllObjects(nlohmann::json& readObj, std::vector<nlohmann::json>& objects, std::string key, std::string value)
{
    std::string valueString = "\"" + value + "\"";


    // only do white space removal if there are any whitespaces to remove
    if (valueString.find(" ") != std::string::npos)
    {
        // remove all the spaces
        std::remove(valueString.begin(), valueString.end(), ' ');
        valueString.pop_back();

        while (valueString[valueString.size() - 1] != '"')
        {
            valueString.pop_back();
        }

        std::remove(valueString.begin(), valueString.end(), ' ');
    }


    for (int i = 0; i < readObj.size(); i++)
    {
        std::string keyString = readObj[i][key].dump();
        if (!keyString.compare(valueString))
        {
            objects.push_back(readObj[i]);
        }
    }

    if (objects.size() > 0) { return true; }
    return false;
}

std::string fileManager::getCategoryName(std::string token)
{
    nlohmann::json jsonObj;
    findObject(categories, jsonObj, "token", token);
    std::string name = jsonObj["name"].dump();
    return name.substr(1,name.size()-2);
}

sf::Texture* fileManager::loadNextCamFront()
{
    sf::Texture* tex = new sf::Texture;
    std::string path = DATA_PATH;
    path.append("/samples/CAM_FRONT/");
    tex->loadFromFile(path + camFrontNames.at(camFrontNamesNext));
    camFrontNamesNext++;
    return tex;
}

std::string fileManager::getCamFrontName()
{
    return camFrontNames.at(camFrontNamesNext - 1);
}
