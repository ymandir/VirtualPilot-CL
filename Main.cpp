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
    // keeps track of how many grid saves were performed per object
    int gridSaveIndex = 0;
};

fileManager fm;

std::string currentSampleDataToken;
std::vector<ann> currentAnns;

sf::Texture cutBuffer(const sf::Uint8* buffer, int posX, int posY, int sizeX, int sizeY, int bigSizeX, int bigSizeY)
{
    sf::Texture texture;
    texture.create(bigSizeX, bigSizeY);
    texture.update(buffer);

    const sf::Uint8* buffer_ptr = buffer;
    sf::Uint8* small_buffer_ptr = (sf::Uint8*)malloc(sizeX * sizeY * 4 * sizeof(sf::Uint8));


    for (int y = 0; y < sizeY; y++)
    {
        for (int x = 0; x < sizeX; x++)
        {
            int bigX = posX + x;
            int bigY = posY + y;
            int bigIndex = bigX * 4 + bigY * bigSizeX * 4;
            int smallIndex = x * 4 + y * sizeX * 4;

            small_buffer_ptr[smallIndex] = buffer_ptr[bigIndex];
            small_buffer_ptr[smallIndex + 1] = buffer_ptr[bigIndex + 1];
            small_buffer_ptr[smallIndex + 2] = buffer_ptr[bigIndex + 2];
            small_buffer_ptr[smallIndex + 3] = buffer_ptr[bigIndex + 3];

        }
    }
    sf::Texture smallTexture;
    smallTexture.create(sizeX, sizeY);
    smallTexture.update(small_buffer_ptr);

    return smallTexture;
}

void findCurrentSampleDataToken()
{
    nlohmann::json foundObj;
    std::string camFrontName = fm.getCamFrontName();
    std::string samples = "samples/CAM_FRONT/";
    std::string searchString = samples + camFrontName;
    fm.findObject(fm.sampleData, foundObj, "filename", searchString);
    std::string sampleDataToken = foundObj["token"].dump();
    currentSampleDataToken = sampleDataToken.substr(1, sampleDataToken.size() - 2);
}

void findCurrentRects()
{
    currentAnns.clear();

    std::vector<nlohmann::json> objects;
    if (fm.findAllObjects(fm.objectAnn, objects, "sample_data_token", currentSampleDataToken))
    {
        //std::cout << "found" << std::endl;
    }

    for (nlohmann::json foundObj : objects)
    {
        std::string xMin = foundObj["bbox"][0].dump();
        std::string yMin = foundObj["bbox"][1].dump();
        std::string xMax = foundObj["bbox"][2].dump();
        std::string yMax = foundObj["bbox"][3].dump();

        ann objAnn;

        std::string cToken = foundObj["category_token"].dump();
        objAnn.categoryName = fm.getCategoryName(cToken.substr(1, cToken.size() - 2));

        int x = atoi(xMin.c_str());
        int y = atoi(yMin.c_str());
        int xSize = atoi(xMax.c_str()) - atoi(xMin.c_str());
        int ySize = atoi(yMax.c_str()) - atoi(yMin.c_str());

        objAnn.rect.setPosition(x, y);
        objAnn.rect.setSize(sf::Vector2f(xSize, ySize));
        objAnn.rect.setOutlineColor(sf::Color::White);
        objAnn.rect.setOutlineThickness(2);
        objAnn.rect.setFillColor(sf::Color::Transparent);
        if (!objAnn.categoryName.substr(0, 7).compare("vehicle"))
        {
            currentAnns.push_back(objAnn);
        }
    }
}



// check if to rects overlap
bool checkOverlap(sf::RectangleShape A, sf::RectangleShape B, int tolerance)
{

    sf::Vector2i posA;
    sf::Vector2i posB;
    sf::Vector2i sizeA;
    sf::Vector2i sizeB;

    if (A.getSize().x < B.getSize().x)
    {
        posA = sf::Vector2i(A.getPosition().x, A.getPosition().y);
        posB = sf::Vector2i(B.getPosition().x, B.getPosition().y);
        sizeA = sf::Vector2i(A.getSize().x, A.getSize().y);
        sizeB = sf::Vector2i(B.getSize().x, B.getSize().y);
    }
    else
    {
        posA = sf::Vector2i(B.getPosition().x, B.getPosition().y);
        posB = sf::Vector2i(A.getPosition().x, A.getPosition().y);
        sizeA = sf::Vector2i(B.getSize().x, B.getSize().y);
        sizeB = sf::Vector2i(A.getSize().x, A.getSize().y);
    }

    sf::RectangleShape a;
    sf::Vector2i point;
    std::vector<sf::Vector2i> points;
    points.push_back(sf::Vector2i(posA.x, posA.y));
    points.push_back(sf::Vector2i(posA.x, posA.y + sizeA.y));
    points.push_back(sf::Vector2i(posA.x + sizeA.x, posA.y + sizeA.y));
    points.push_back(sf::Vector2i(posA.x + sizeA.x, posA.y));

    for (auto point : points)
    {
        if (point.x > posB.x + tolerance && point.x < posB.x + sizeB.x - tolerance && point.y > posB.y + tolerance && point.y < posB.y + sizeB.y - tolerance)
        {
            
            return true;
        }
        if (posA.x > posB.x && posA.x + sizeA.x < posB.x + sizeB.x && posB.y > posA.y && posB.y + sizeB.y < posA.y + sizeA.y)
        {
            return true;
        }
    }
    return false;
}

int main()
{

    int frameIndex = 0;

    sf::Texture* camFront = fm.loadNextCamFront();

    std::vector<sf::RectangleShape> rects;
    std::vector<sf::Image> saveImagesVeh;
    std::vector<sf::Image> saveImagesNveh;
    int vehIndex = 0;
    int nVehIndex = 0;

    for (int y = 0; y < 9; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            sf::RectangleShape rect;
            rect.setFillColor(sf::Color::Transparent);
            rect.setSize(sf::Vector2f(100, 100));
            rect.setPosition(x * 100, y * 100);
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2);
            rects.push_back(rect);
        }
    }

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "VP_CL");

    nlohmann::json jsonFile;
    std::string fileName = fm.getCamFrontName();

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
                    camFront = fm.loadNextCamFront();
                    findCurrentSampleDataToken();
                    findCurrentRects();
                    std::cout << currentSampleDataToken << std::endl;

                    // SAVE IMAGES
                    // ON GRID VEHICLES
                    for (auto x : currentAnns)
                    {
                        //std::cout << x.rect.getPosition().x << " " << x.rect.getPosition().y << " " << x.rect.getSize().x << " " << x.rect.getSize().y << std::endl;
                        // check if the rect is in bounds
                        if (x.rect.getPosition().x + x.rect.getSize().x < 1600 && x.rect.getPosition().y + x.rect.getSize().y < 900
                            && x.rect.getPosition().x > 0 && x.rect.getPosition().y > 0 && x.rect.getSize().x > 40)
                        {
                            sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.rect.getPosition().x, x.rect.getPosition().y,
                                x.rect.getSize().x, x.rect.getSize().y, camFront->getSize().x, camFront->getSize().y);
                            sf::Image img = tex.copyToImage();
                            saveImagesVeh.push_back(img);
                        }
                    }


                    // OFF GRID NON VEHICLES
                    for (auto x : currentAnns)
                    {
                        //std::cout << x.rect.getPosition().x << " " << x.rect.getPosition().y << " " << x.rect.getSize().x << " " << x.rect.getSize().y << std::endl;
                        // check if the rect is in bounds
                        if (x.rect.getPosition().x + x.rect.getSize().x < 1500 && x.rect.getPosition().y + x.rect.getSize().y < 900
                            && x.rect.getPosition().x > 0 && x.rect.getPosition().y > 0)
                        {
                            bool overlap = false;
                            sf::RectangleShape alteredRect = x.rect;
                            alteredRect.move(100, 0);
                            for (auto y : currentAnns)
                            {
                                if (checkOverlap(y.rect, alteredRect, 0)) { overlap = true; break; }
                            }
                            if (!overlap)
                            {
                                sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.rect.getPosition().x + 100, x.rect.getPosition().y,
                                    x.rect.getSize().x, x.rect.getSize().y, camFront->getSize().x, camFront->getSize().y);
                                sf::Image img = tex.copyToImage();
                                //saveImagesNveh.push_back(img);
                            }
                        }
                    }

                    // ON GRID VEHICLES AND NON VEHICLES
                    int rectNvehIndex = 0;
                    int rectVehIndex = 0;
                    int index = 0;
                    for (auto x : rects)
                    {
                        bool overlap = false;
                        for (auto y : currentAnns)
                        {
                            if (checkOverlap(x, y.rect, 50) && y.gridSaveIndex < 3)
                            {
                                overlap = true;
                                if (x.getSize().x < 300 && x.getSize().x > 50) { y.gridSaveIndex++; }
                                break;
                            }
                        }
                        if (overlap && rectVehIndex < 5 && x.getSize().x < 300 && x.getSize().x > 50)
                        {
                            sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.getPosition().x, x.getPosition().y,
                                x.getSize().x, x.getSize().y, camFront->getSize().x, camFront->getSize().y);
                            sf::Image img = tex.copyToImage();
                            saveImagesVeh.push_back(img);
                            rectVehIndex++;
                        }
                        for (auto y : currentAnns)
                        {
                            if (checkOverlap(x, y.rect, 0)) { overlap = true; break; }
                        }
                        if (!overlap && rectNvehIndex < 300 && x.getPosition().y && currentAnns.size() > 0)
                        {
                            sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.getPosition().x, x.getPosition().y,
                                x.getSize().x, x.getSize().y, camFront->getSize().x, camFront->getSize().y);
                            sf::Image img = tex.copyToImage();
                            if (index % 4 == 0)
                            {
                                saveImagesNveh.push_back(img);
                            }
                            rectNvehIndex++;
                            index++;
                        }
                    }


                    for (auto x : saveImagesVeh)
                    {
                        std::string vehString = "vehicle";
                        x.saveToFile("B:/VirtualPilotData/Vehicle/" + vehString + std::to_string(vehIndex) + ".png");
                        vehIndex++;
                    }
                    saveImagesVeh.clear();

                    for (auto x : saveImagesNveh)
                    {
                        std::string nvehString = "non-vehicle";
                        x.saveToFile("B:/VirtualPilotData/nonVehicle/" + nvehString + std::to_string(nVehIndex) + ".png");
                        nVehIndex++;
                    }
                    saveImagesNveh.clear();
                    //SAVE IMAGES

                    frameIndex++;
                    
                }
            }
        }


        if (1)
        {
            camFront = fm.loadNextCamFront();
            findCurrentSampleDataToken();
            findCurrentRects();
            std::cout << currentSampleDataToken << std::endl;

            // SAVE IMAGES
            // ON GRID VEHICLES
            for (auto x : currentAnns)
            {
                //std::cout << x.rect.getPosition().x << " " << x.rect.getPosition().y << " " << x.rect.getSize().x << " " << x.rect.getSize().y << std::endl;
                // check if the rect is in bounds
                if (x.rect.getPosition().x + x.rect.getSize().x < 1600 && x.rect.getPosition().y + x.rect.getSize().y < 900
                    && x.rect.getPosition().x > 0 && x.rect.getPosition().y > 0 && x.rect.getSize().x > 40)
                {
                    sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.rect.getPosition().x, x.rect.getPosition().y,
                        x.rect.getSize().x, x.rect.getSize().y, camFront->getSize().x, camFront->getSize().y);
                    sf::Image img = tex.copyToImage();
                    saveImagesVeh.push_back(img);
                }
            }


            // OFF GRID NON VEHICLES
            for (auto x : currentAnns)
            {
                //std::cout << x.rect.getPosition().x << " " << x.rect.getPosition().y << " " << x.rect.getSize().x << " " << x.rect.getSize().y << std::endl;
                // check if the rect is in bounds
                if (x.rect.getPosition().x + x.rect.getSize().x < 1500 && x.rect.getPosition().y + x.rect.getSize().y < 900
                    && x.rect.getPosition().x > 0 && x.rect.getPosition().y > 0)
                {
                    bool overlap = false;
                    sf::RectangleShape alteredRect = x.rect;
                    alteredRect.move(100, 0);
                    for (auto y : currentAnns)
                    {
                        if (checkOverlap(y.rect, alteredRect, 0)) { overlap = true; break; }
                    }
                    if (!overlap)
                    {
                        sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.rect.getPosition().x + 100, x.rect.getPosition().y,
                            x.rect.getSize().x, x.rect.getSize().y, camFront->getSize().x, camFront->getSize().y);
                        sf::Image img = tex.copyToImage();
                        //saveImagesNveh.push_back(img);
                    }
                }
            }

            // ON GRID VEHICLES AND NON VEHICLES
            int rectNvehIndex = 0;
            int rectVehIndex = 0;
            int index = 0;
            for (auto x : rects)
            {
                bool overlap = false;
                for (auto y : currentAnns)
                {
                    if (checkOverlap(x, y.rect, 50) && y.gridSaveIndex < 3)
                    {
                        overlap = true;
                        if (x.getSize().x < 300 && x.getSize().x > 50) { y.gridSaveIndex++; }
                        break;
                    }
                }
                if (overlap && rectVehIndex < 5 && x.getSize().x < 300 && x.getSize().x > 50)
                {
                    sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.getPosition().x, x.getPosition().y,
                        x.getSize().x, x.getSize().y, camFront->getSize().x, camFront->getSize().y);
                    sf::Image img = tex.copyToImage();
                    saveImagesVeh.push_back(img);
                    rectVehIndex++;
                }
                for (auto y : currentAnns)
                {
                    if (checkOverlap(x, y.rect, 0)) { overlap = true; break; }
                }
                if (!overlap && rectNvehIndex < 300 && x.getPosition().y && currentAnns.size() > 0)
                {
                    sf::Texture tex = cutBuffer(camFront->copyToImage().getPixelsPtr(), x.getPosition().x, x.getPosition().y,
                        x.getSize().x, x.getSize().y, camFront->getSize().x, camFront->getSize().y);
                    sf::Image img = tex.copyToImage();
                    if (index % 4 == 0) 
                    {
                        saveImagesNveh.push_back(img);
                    }
                    rectNvehIndex++;
                    index++;
                }
            }


            for (auto x : saveImagesVeh)
            {
                std::string vehString = "vehicle";
                x.saveToFile("B:/VirtualPilotData/Vehicle/" + vehString + std::to_string(vehIndex) + ".png");
                vehIndex++;
            }
            saveImagesVeh.clear();

            for (auto x : saveImagesNveh)
            {
                std::string nvehString = "non-vehicle";
                x.saveToFile("B:/VirtualPilotData/nonVehicle/" + nvehString + std::to_string(nVehIndex) + ".png");
                nVehIndex++;
            }
            saveImagesNveh.clear();
            //SAVE IMAGES

            frameIndex++;
            std::cout << frameIndex << std::endl;
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
        for (auto x : rects)
        {
            window.draw(x);
        }
        window.display();

        if (frameIndex > 10000) { return 0; window.close(); break; }
    }

    return 0;
}