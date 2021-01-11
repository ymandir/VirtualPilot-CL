#pragma once
#include <iostream>


namespace inf
{
	enum imgFormat
	{
		RGBA8 = 0, RGB8, BGR8
	};


	struct kernel
	{
		std::int8_t* values;
		int size;
		float div = 1.0;
	};


	struct image
	{
		void* buffer;
		unsigned int sizeX;
		unsigned int sizeY;
		unsigned int channelCount = 0;
		imgFormat format;
	};

}