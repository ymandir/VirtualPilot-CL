#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "json.hpp"
#include "fileManager.h"






int main()
{
   
    fileManager fm;
 

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "VP_CL");

    nlohmann::json jsonFile;
    std::string fileName = "samples/CAM_FRONT/n008 - 2018 - 08 - 01 - 15 - 16 - 36 - 0400__CAM_FRONT__1533151061512404.jpg";
    if (fm.findObject(500, fm.files.at("sample_data"), jsonFile, "filename", fileName))
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