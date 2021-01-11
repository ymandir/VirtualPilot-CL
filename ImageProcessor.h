#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "inf_types.h"

namespace inf {

	class ImageProcessor
	{
	public:
		ImageProcessor();
		~ImageProcessor();
		void copyBufferToImage(void* buffer, unsigned int sizeX, unsigned int sizeY, imgFormat format, image*);
		void copyImage(image source, image* target);

		image applyFilter(kernel kernel, unsigned int strife, unsigned int padding, image image);

		image maxPool(int size, image image, int channel);
		image minPool(int size, image image, int channel);
		image avgPool(int size, image image, int channel);

		image makeGrayScale(image image);

		image bicubicResize(image image, int sizeX, int sizeY);
		image resize2x(image image);

	private:
		inline void copyBuffer8(void* buffer, void* target, unsigned int size);
		inline void copyBuffer16(void* buffer, void* target, unsigned int size);
		inline float getKernelResult(std::uint8_t* point, unsigned int channelCount, unsigned int channel, unsigned int sizeX, kernel kernel);

	};

}