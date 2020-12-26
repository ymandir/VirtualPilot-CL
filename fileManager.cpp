#include "fileManager.h"
fileManager::fileManager()
{
    initFiles();

    std::string camFrontPath = DATA_PATH;
    camFrontPath.append("/samples/CAM_FRONT");
    loadNames(camFrontPath,camFrontNames);

    categories = readFromFile(100,files.at("sample_data"));
}

// ADD A NEW FILE WITH THE FILE PATH
void fileManager::addFile(std::string fileName, std::string filePath)
{
	files.insert(std::pair<std::string, std::ifstream*>(fileName, new std::ifstream(filePath)));
}

// Loads file names into a vector in a given directory
void fileManager::loadNames(std::string path, std::vector<std::string>& nameStack)
{
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        nameStack.push_back(entry.path().string().substr(path.size() + 2, entry.path().string().size() - path.size() - 2));
    }
}

void fileManager::initFiles()
{
    std::string dataPath = "B:/nuScenesDataset";
    addFile("sample_data", dataPath + "/1.0-trainval-meta/v1.0-trainval/sample_data.json");
    addFile("category", dataPath + "/1.0-trainval-meta/v1.0-trainval/category.json");
    addFile("sample", dataPath + "/1.0-trainval-meta/v1.0-trainval/sample.json");
    addFile("sample_annotation", dataPath + "/1.0-trainval-meta/v1.0-trainval/sample_annotation.json");
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
        else if (line[0] == ']') { stop = true; }

        result << line;

        if (objectIndex == size) { stopOnObjectEnd = true; }
    }
    nlohmann::json jsonFile;
    std::string debugString = result.str();
    jsonFile = nlohmann::json::parse(result);
    return jsonFile;
}


// size = how many objects to look through, obj = found object
bool fileManager::findObject(int size, std::ifstream* readFile, nlohmann::json& jsonObj, std::string key, std::string value)
{
    int portion = 10000;
    if (size < portion) { portion = size; }
    nlohmann::json jsonFile = readFromFile(portion, readFile);
    std::string valueString = "\"" + value + "\"";

    // remove all the spaces
    std::remove(valueString.begin(), valueString.end(), ' ');
    valueString.pop_back();
    while (valueString[valueString.size() - 1] != '"')
    {
        valueString.pop_back();
    }

    std::remove(valueString.begin(), valueString.end(), ' ');

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
