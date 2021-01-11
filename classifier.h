#pragma once
#include <SFML/Graphics.hpp>
#include <torch/torch.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include "ImageProcessor.h"
#include "inf_types.h"
#include "custom_net_imp.h"
#include "Windows.h"
#include <algorithm>
#include <random>



class classifier
{
public:
	classifier();
	sf::Vector2f getClass(sf::Image image);
private:
	inf::image toInfImage(sf::Image image);
	torch::Tensor toTensor(inf::image image, torch::DeviceType deviceType);
	inf::ImageProcessor imp;
	std::shared_ptr<CustomNetImp> Net;
	torch::DeviceType device_type;
};

