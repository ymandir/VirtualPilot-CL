#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "json.hpp"
#include "fileManager.h"

struct ann
{
    std::string categoryName;
    sf::RectangleShape rect;
};

fileManager fm;

std::string currentSampleDataToken;
std::vector<ann> currentAnns;

void findCurrentSampleDataToken()
{
    nlohmann::json foundObj;
    std::string camFrontName = fm.getCamFrontName();
    std::string samples = "samples/CAM_FRONT/";
    std::string searchString = samples + camFrontName;
    fm.findObject(fm.sampleData,foundObj,"filename",searchString);
    std::string sampleDataToken = foundObj["token"].dump();
    currentSampleDataToken = sampleDataToken.substr(1,sampleDataToken.size()-2);
}

void findCurrentRects()
{
    currentAnns.clear();

    std::vector<nlohmann::json> objects;
    if (fm.findAllObjects(fm.objectAnn, objects, "sample_data_token", currentSampleDataToken))
    {
        std::cout << "found" << std::endl;
    }

    for (nlohmann::json foundObj : objects)
    {
        std::string xMin = foundObj["bbox"][0].dump();
        //x = x.substr(1,x.size()-2);
        std::string yMin = foundObj["bbox"][1].dump();
        //y = y.substr(1, y.size() - 2);
        std::string xMax = foundObj["bbox"][2].dump();
        //xSize = xSize.substr(1, xSize.size() - 2);
        std::string yMax = foundObj["bbox"][3].dump();
        //ySize = ySize.substr(1, ySize.size() - 2);

        ann objAnn;

        std::string cToken = foundObj["category_token"].dump();
        objAnn.categoryName = fm.getCategoryName(cToken.substr(1, cToken.size()-2));
        
        int x = atoi(xMin.c_str());
        int y = atoi(yMin.c_str());
        int xSize = atoi(xMax.c_str()) - atoi(xMin.c_str());
        int ySize = atoi(yMax.c_str()) - atoi(yMin.c_str());

        objAnn.rect.setPosition(x, y);
        objAnn.rect.setSize(sf::Vector2f(xSize, ySize));
        objAnn.rect.setOutlineColor(sf::Color::White);
        objAnn.rect.setOutlineThickness(2);
        objAnn.rect.setFillColor(sf::Color::Transparent);
        currentAnns.push_back(objAnn);
    }
}

int main()
{
   

    sf::Texture* camFront = fm.loadNextCamFront();


    sf::RenderWindow window(sf::VideoMode(1920, 1080), "VP_CL");

    nlohmann::json jsonFile;
    std::string fileName = fm.getCamFrontName();
    //if (fm.findObject(500, fm.files.at("sample_data"), jsonFile, "filename", fileName))
    //{
    //    std::cout << jsonFile.dump() << std::endl;
    //}
    
    sf::Font font;
    sf::Text annText;

    font.loadFromFile("font.ttf");
    annText.setFont(font);
    annText.setCharacterSize(10);
    annText.setFillColor(sf::Color::White);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                {
                    camFront =  fm.loadNextCamFront();
                    findCurrentSampleDataToken();
                    findCurrentRects();
                    std::cout << currentSampleDataToken << std::endl;
                }
            }
        }

        window.clear();
        window.draw(sf::Sprite(*camFront));
        for (auto x : currentAnns) 
        { 
            window.draw(x.rect);
            annText.setPosition(x.rect.getPosition());
            annText.setString(x.categoryName);
            window.draw(annText);
        }
        window.display();
    }

    return 0;
}