#include "classifier.h"

classifier::classifier()
{
	if (torch::cuda::is_available() && 1) {
		device_type = torch::kCUDA;
		std::cout << "cuda is available" << std::endl;
	}
	else {
		device_type = torch::kCPU;
		std::cout << "cuda is not available" << std::endl;
	}
	torch::Device device(device_type);

	Net = std::make_shared<CustomNetImp>();
	torch::load(Net, "vehiclePro.pt");
	Net->to(device);
}

sf::Vector2f classifier::getClass(sf::Image image)
{
	inf::image infImage = imp.bicubicResize(toInfImage(image), 100, 100);
	torch::Tensor input = toTensor(infImage, device_type);
	torch::Tensor output = Net->forward(input);
	sf::Vector2f resultVec(output[0].item<float>(),output[1].item<float>());
	return resultVec;
}

inf::image classifier::toInfImage(sf::Image image)
{
	inf::image result;
	imp.copyBufferToImage((void*)image.getPixelsPtr(), image.getSize().x, image.getSize().y, inf::RGBA8, &result);
	return result;
}

torch::Tensor classifier::toTensor(inf::image image, torch::DeviceType deviceType)
{
	const sf::Uint8* buff = (sf::Uint8*)image.buffer;
	int sizeX = image.sizeX;
	int sizeY = image.sizeY;
	int sizeTotalImage = sizeX * sizeY;
	int sizeTotalBuff = sizeX * sizeY * 4;
	int sizeTotalTensor = sizeX * sizeY * 3;



	torch::Tensor result = torch::empty({ 1, 3, sizeX, sizeY });


	// REDUCE THE CHANNEL SIZE TO 3
	int oldBuffSize = image.sizeX * image.sizeY * 4;
	sf::Uint8* oldBuff = (sf::Uint8*)image.buffer;

	int newBuffSize = image.sizeX * image.sizeY * 3;
	int newBuffChannelSize = newBuffSize / 3;
	sf::Uint8* newBuff = (sf::Uint8*)malloc(newBuffSize);

	int a = 0;
	for (int i = 0; i < oldBuffSize; i++)
	{
		if (i % 4 == 3)
		{
			a++;
		}
		else
		{
			newBuff[(i % 4) * newBuffChannelSize + a] = oldBuff[i];
		}
	}
	auto tensor_image = torch::from_blob(newBuff, { 3 ,sizeX, sizeY }, at::kByte);

	tensor_image = tensor_image.permute({ 0,2,1 });
	result[0] = tensor_image;
	result = result.toType(torch::kFloat).div(255);
	//result.sub_(0.5).div_(0.5);


	free(newBuff);
	return result.to(deviceType);
}
