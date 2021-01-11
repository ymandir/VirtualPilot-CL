#include "ImageProcessor.h"

using namespace inf;

ImageProcessor::ImageProcessor()
{

}


ImageProcessor::~ImageProcessor()
{

}

void ImageProcessor::copyBufferToImage(void* buffer, unsigned int sizeX, unsigned int sizeY, imgFormat format, inf::image* target)
{

	switch (format)
	{
	case RGBA8:
		target->buffer = (void*)malloc(sizeof(std::int8_t) * sizeX * sizeY * 4);
		copyBuffer8(buffer, target->buffer, sizeX * sizeY * 4);
		target->channelCount = 4;
		break;
	case RGB8:
		target->buffer = (void*)malloc(sizeof(std::int8_t) * sizeX * sizeY * 3);
		copyBuffer8(buffer, target->buffer, sizeX * sizeY * 3);
		target->channelCount = 3;
		break;
	case BGR8:
		target->buffer = (void*)malloc(sizeof(std::int8_t) * sizeX * sizeY * 3);
		copyBuffer8(buffer, target->buffer, sizeX * sizeY * 3);
		target->channelCount = 3;
		break;
	}
	target->format = format;
	target->sizeX = sizeX;
	target->sizeY = sizeY;

}

void inf::ImageProcessor::copyImage(image source, image* target)
{
	switch (source.format)
	{
	case RGBA8:
		target->buffer = (void*)malloc(sizeof(std::int8_t) * source.sizeX * source.sizeY * source.channelCount);
		copyBuffer8(source.buffer, target->buffer, source.sizeX * source.sizeY * source.channelCount);
		target->channelCount = 4;
		break;
	}
}


inline void inf::ImageProcessor::copyBuffer8(void* buffer, void* target, unsigned int size)
{
	std::int8_t* buffer_ptr = (std::int8_t*)buffer;
	std::int8_t* target_ptr = (std::int8_t*)target;

	for (int i = 0; i < size; i++)
	{
		target_ptr[i] = buffer_ptr[i];
	}
}

inline void inf::ImageProcessor::copyBuffer16(void* buffer, void* target, unsigned int size)
{
	std::int16_t* buffer_ptr = (std::int16_t*)buffer;
	std::int16_t* target_ptr = (std::int16_t*)target;

	for (int i = 0; i < size; i++)
	{
		target_ptr[i] = buffer_ptr[i];
	}
}



image ImageProcessor::applyFilter(kernel kernel, unsigned int strife, unsigned int padding, image image)
{
	inf::image result;
	result.buffer = new std::uint8_t[image.sizeX * image.sizeY * image.channelCount];

	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;

	int cc = image.channelCount;
	int line = image.sizeX * cc;

	for (int y = padding; y + strife < image.sizeY - padding; y = y + strife)
	{
		for (int x = padding; x + strife < image.sizeX - padding; x = x + strife)
		{
			int index = x * cc + y * line;

			result_ptr[index] = (std::uint8_t)getKernelResult(&buffer_ptr[index], cc, image.sizeX, 1, kernel);
			result_ptr[index + 1] = (std::uint8_t)getKernelResult(&buffer_ptr[index], cc, image.sizeX, 2, kernel);
			result_ptr[index + 2] = (std::uint8_t)getKernelResult(&buffer_ptr[index], cc, image.sizeX, 3, kernel);
			result_ptr[index + 3] = 255;

		}
		int a;
	}

	return result;
}


image ImageProcessor::makeGrayScale(image image)
{
	inf::image result;
	result.buffer = new std::uint8_t[image.sizeX * image.sizeY * image.channelCount];

	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;

	int cc = image.channelCount;
	int line = image.sizeX * cc;
	int bufferIndex = 0;
	int sum = 0;
	int average = 0;

	for (int y = 0; y < image.sizeY; y++)
	{
		for (int x = 0; x < image.sizeX; x++)
		{
			bufferIndex = x * cc + y * line;
			sum = buffer_ptr[bufferIndex + 2] + buffer_ptr[bufferIndex + 1] + buffer_ptr[bufferIndex];
			average = sum / 3;

			result_ptr[bufferIndex] = average;
			result_ptr[bufferIndex + 1] = average;
			result_ptr[bufferIndex + 2] = average;
			result_ptr[bufferIndex + 3] = 255;

		}
	}
	result.channelCount = 4;
	result.sizeX = image.sizeX;
	result.sizeY = image.sizeY;
	result.format = RGBA8;


	return result;

}



//TODO(YIGIT) : DEBUG INTENSELY
inline float inf::ImageProcessor::getKernelResult(std::uint8_t* point, unsigned int cc, unsigned int sizeX, unsigned int channel, kernel kernel)
{
	//STD::VECTOR IS NOT USED BECAUSE OF PERFORMANCE CONCERNS
	int* values = new int[kernel.size * kernel.size];
	int valuesNext = 0;
	point = point - kernel.size / 2;


	for (unsigned int y = 0; y < kernel.size; y++)
	{
		for (unsigned int x = 0; x < kernel.size; x++)
		{
			unsigned int bufferIndex = y * sizeX * cc + x * cc;

			values[valuesNext] = ((int)*(point + bufferIndex + channel) * kernel.values[valuesNext]) * kernel.div;
			valuesNext++;
		}
	}

	float sum = 0;

	for (int i = 0; i < valuesNext; i++)
	{
		sum = sum + values[i];
	}

	float mean = sum / (kernel.size * kernel.size);
	delete values;

	if (mean > 255) { return 255; }
	else { return mean; }

}

//STATUS: KIND OF WORKS?
//TODO(yigit): TEST
image inf::ImageProcessor::maxPool(int size, image image, int channel)
{
	inf::image result;

	//IMAGE GETS CROPPED TO FIT THE POOL KERNEL
	int croppedSizeX = image.sizeX - image.sizeX % size;
	int croppedSizeY = image.sizeY - image.sizeY % size;
	int resultNext = 0;

	result.sizeX = croppedSizeX / size * 2;
	result.sizeY = croppedSizeY / size * 2;
	result.format = RGBA8;
	result.channelCount = image.channelCount;

	result.buffer = new std::uint8_t[result.sizeX * result.sizeY * result.channelCount];
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;
	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;

	//TODO(yigit) : EVALUATE IF "+1" IS NECESSARY
	for (int y = 0; y < croppedSizeY / size; y++)
	{
		for (int x = 0; x < croppedSizeX / size; x++)
		{
			//INDEX OF THE RESULT IMAGE
			resultNext = x * result.channelCount * 2 + result.sizeX * y * result.channelCount * 2;

			//THIS GETS US TO THE CLUSTER
			// bufferIndex : index of first pixel in the cluster
			int bufferIndex = y * image.sizeX * image.channelCount * size + x * size * image.channelCount;

			//PERFORMANCE_POTENTIAL 
			std::vector<int> innerClusters[4];
			//RELATIVE TO 0,0 OF THE ORIGINAL IMAGE BUT ONLY USED INSIDE THE 
			int innerFullBufferIndex = bufferIndex;
			for (int innerY = 0; innerY < size; innerY++)
			{
				for (int innerX = 0; innerX < size; innerX++)
				{
					//WE CALCULATE THE INNERBUFFERINDEX
					innerFullBufferIndex = bufferIndex + innerY * image.sizeX * image.channelCount + innerX * image.channelCount;

					// BETWEEN 0-4 
					int innerIndex = 0;

					if (innerY > (size / 2) - 1 && innerX > (size / 2) - 1) { innerIndex = 3; }
					else if (innerY > (size / 2) - 1) { innerIndex = 2; }
					else if (innerX > (size / 2) - 1) { innerIndex = 1; }
					else { innerIndex = 0; }

					innerClusters[innerIndex].push_back(buffer_ptr[innerFullBufferIndex + channel]);
				}
			}

			//INNER CLUSTERS ARE FILLED BY THIS POINT
			//WE FIND THE MAX VALUES
			int maximums[4] = { 0,0,0,0 };
			for (int i = 0; i < 4; i++)
			{
				for (int x : innerClusters[i])
				{
					if (x > maximums[i]) { maximums[i] = x; }
				}
			}
			//CLEAR INNERCLUSTERS AS WE DO NOT NEED THEM ANYMORE
			for (int i = 0; i < 4; i++) { innerClusters[i].clear(); }


			//WE CAN APPLY IT TO ALL CHANNELS SINCE IT IS ONLY ONE AND WE WANT TO DISPLAY A B/W IMAGE
			for (int i = 0; i < 3; i++)
			{
				result_ptr[resultNext + i] = maximums[0];
				result_ptr[resultNext + result.channelCount + i] = maximums[1];
				result_ptr[resultNext + result.sizeX * result.channelCount + i] = maximums[2];
				result_ptr[resultNext + result.sizeX * result.channelCount + result.channelCount + i] = maximums[3];
			}




		}
	}

	return result;


}

image inf::ImageProcessor::minPool(int size, image image, int channel)
{
	inf::image result;

	//IMAGE GETS CROPPED TO FIT THE POOL KERNEL
	int croppedSizeX = image.sizeX - image.sizeX % size;
	int croppedSizeY = image.sizeY - image.sizeY % size;
	int resultNext = 0;

	result.sizeX = croppedSizeX / size * 2;
	result.sizeY = croppedSizeY / size * 2;
	result.format = RGBA8;
	result.channelCount = image.channelCount;

	result.buffer = new std::uint8_t[result.sizeX * result.sizeY * result.channelCount];
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;
	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;

	//TODO(yigit) : EVALUATE IF "+1" IS NECESSARY
	for (int y = 0; y < croppedSizeY / size; y++)
	{
		for (int x = 0; x < croppedSizeX / size; x++)
		{
			//INDEX OF THE RESULT IMAGE
			resultNext = x * result.channelCount * 2 + result.sizeX * y * result.channelCount * 2;

			//THIS GETS US TO THE CLUSTER
			// bufferIndex : index of first pixel in the cluster
			int bufferIndex = y * image.sizeX * image.channelCount * size + x * size * image.channelCount;

			//PERFORMANCE_POTENTIAL 
			std::vector<int> innerClusters[4];
			//RELATIVE TO 0,0 OF THE ORIGINAL IMAGE BUT ONLY USED INSIDE THE CLUSTER
			int innerFullBufferIndex = bufferIndex;
			for (int innerY = 0; innerY < size; innerY++)
			{
				for (int innerX = 0; innerX < size; innerX++)
				{
					//WE CALCULATE THE INNERBUFFERINDEX
					innerFullBufferIndex = bufferIndex + innerY * image.sizeX * image.channelCount + innerX * image.channelCount;

					// BETWEEN 0-4 
					int innerIndex = 0;

					if (innerY > (size / 2) - 1 && innerX > (size / 2) - 1) { innerIndex = 3; }
					else if (innerY > (size / 2) - 1) { innerIndex = 2; }
					else if (innerX > (size / 2) - 1) { innerIndex = 1; }
					else { innerIndex = 0; }

					innerClusters[innerIndex].push_back(buffer_ptr[innerFullBufferIndex + channel]);
				}
			}

			//INNER CLUSTERS ARE FILLED BY THIS POINT
			//WE FIND THE MAX VALUES
			int minimums[4] = { 255,255,255,255 };
			for (int i = 0; i < 4; i++)
			{
				for (int x : innerClusters[i])
				{
					if (x < minimums[i]) { minimums[i] = x; }
				}
			}
			//CLEAR INNERCLUSTERS AS WE DO NOT NEED THEM ANYMORE
			for (int i = 0; i < 4; i++) { innerClusters[i].clear(); }


			//WE CAN APPLY IT TO ALL CHANNELS SINCE IT IS ONLY ONE AND WE WANT TO DISPLAY A B/W IMAGE
			for (int i = 0; i < 3; i++)
			{
				result_ptr[resultNext + i] = minimums[0];
				result_ptr[resultNext + result.channelCount + i] = minimums[1];
				result_ptr[resultNext + result.sizeX * result.channelCount + i] = minimums[2];
				result_ptr[resultNext + result.sizeX * result.channelCount + result.channelCount + i] = minimums[3];
			}


		}
	}

	return result;

}

image inf::ImageProcessor::avgPool(int size, image image, int channel)
{
	inf::image result;

	//IMAGE GETS CROPPED TO FIT THE POOL KERNEL
	int croppedSizeX = image.sizeX - image.sizeX % size;
	int croppedSizeY = image.sizeY - image.sizeY % size;
	int resultNext = 0;

	result.sizeX = croppedSizeX / size * 2;
	result.sizeY = croppedSizeY / size * 2;
	result.format = RGBA8;
	result.channelCount = image.channelCount;

	result.buffer = new std::uint8_t[result.sizeX * result.sizeY * result.channelCount];
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;
	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;

	//TODO(yigit) : EVALUATE IF "+1" IS NECESSARY
	for (int y = 0; y < croppedSizeY / size; y++)
	{
		for (int x = 0; x < croppedSizeX / size; x++)
		{
			//INDEX OF THE RESULT IMAGE
			resultNext = x * result.channelCount * 2 + result.sizeX * y * result.channelCount * 2;

			//THIS GETS US TO THE CLUSTER
			// bufferIndex : index of first pixel in the cluster
			int bufferIndex = y * image.sizeX * image.channelCount * size + x * size * image.channelCount;

			//PERFORMANCE_POTENTIAL 
			std::vector<int> innerClusters[4];
			//RELATIVE TO 0,0 OF THE ORIGINAL IMAGE BUT ONLY USED INSIDE THE CLUSTER
			int innerFullBufferIndex = bufferIndex;
			for (int innerY = 0; innerY < size; innerY++)
			{
				for (int innerX = 0; innerX < size; innerX++)
				{
					//WE CALCULATE THE INNERBUFFERINDEX
					innerFullBufferIndex = bufferIndex + innerY * image.sizeX * image.channelCount + innerX * image.channelCount;

					// BETWEEN 0-4 
					int innerIndex = 0;

					if (innerY > (size / 2) - 1 && innerX > (size / 2) - 1) { innerIndex = 3; }
					else if (innerY > (size / 2) - 1) { innerIndex = 2; }
					else if (innerX > (size / 2) - 1) { innerIndex = 1; }
					else { innerIndex = 0; }

					innerClusters[innerIndex].push_back(buffer_ptr[innerFullBufferIndex + channel]);
				}
			}

			//INNER CLUSTERS ARE FILLED BY THIS POINT
			int sums[4] = { 0,0,0,0 };
			int averages[4] = { 0,0,0,0 };
			for (int i = 0; i < 4; i++)
			{
				for (int x : innerClusters[i])
				{
					sums[i] = sums[i] + x;
				}
			}

			//CALCULATE AVERAGES
			for (int i = 0; i < 4; i++) { averages[i] = sums[i] / innerClusters->size(); }
			//CLEAR INNERCLUSTERS AS WE DO NOT NEED THEM ANYMORE
			for (int i = 0; i < 4; i++) { innerClusters[i].clear(); }


			//WE CAN APPLY IT TO ALL CHANNELS SINCE IT IS ONLY ONE AND WE WANT TO DISPLAY A B/W IMAGE
			for (int i = 0; i < 3; i++)
			{
				result_ptr[resultNext + i] = averages[0];
				result_ptr[resultNext + result.channelCount + i] = averages[1];
				result_ptr[resultNext + result.sizeX * result.channelCount + i] = averages[2];
				result_ptr[resultNext + result.sizeX * result.channelCount + result.channelCount + i] = averages[3];
			}


		}
	}

	return result;
}


//TODO(yigit) : ALMOST WORKS, DEBUG LATER
image inf::ImageProcessor::bicubicResize(image image, int sizeX, int sizeY)
{

	if (image.sizeX == sizeX && image.sizeY == sizeY) { return image; }


	while (image.sizeX <= sizeX || image.sizeY <= sizeY)
	{
		inf::image newImage = resize2x(image);
		free(image.buffer);
		image.sizeX = newImage.sizeX;
		image.sizeY = newImage.sizeY;
		image.buffer = newImage.buffer;
		image.channelCount = newImage.channelCount;
		image.format = newImage.format;
	}


	float* resizeMap = (float*)malloc(sizeX * sizeY * sizeof(float) * 2);

	inf::image result;

	result.sizeX = sizeX;
	result.sizeY = sizeY;
	result.format = RGBA8;
	result.channelCount = image.channelCount;

	result.buffer = new std::uint8_t[result.sizeX * result.sizeY * result.channelCount];
	std::uint8_t* result_ptr = (std::uint8_t*)result.buffer;
	std::uint8_t* buffer_ptr = (std::uint8_t*)image.buffer;

	//resizeMap[0][0] = x of point(0,0) 
	//resizeMap[0][1] = y of point(0,0) 

	//point(x,y).x = resizeMap[y][2x]  and  point(x,y).y = resizeMap[y][2x +  1]

	//TODO(yigit) : DEBUG LATER TO SEE IF CASTING IS REQUIRED
	float incX = (float)image.sizeX / (float)sizeX;
	float incY = (float)image.sizeY / (float)sizeY;

	float offsetX = 0;
	float offsetY = 0;

	for (int y = 0; y < sizeY; y++)
	{

		for (int x = 0; x < sizeX; x++)
		{
			resizeMap[y * sizeX * 2 + 2 * x] = offsetX;
			resizeMap[y * sizeX * 2 + 2 * x + 1] = offsetY;
			offsetX = offsetX + incX;
		}
		offsetY = offsetY + incY;
		offsetX = 0;

	}

	//BY THIS POINT COORDINATES OF THE RESIZE MAP MUST BE CALCULATED
	//WE TRAVERSE THE RESIZE MAP
	for (int channel = 0; channel < image.channelCount; channel++)
	{
		for (int y = 0; y < sizeY; y++)
		{
			for (int x = 0; x < sizeX; x++)
			{
				float pointX = resizeMap[y * sizeX * 2 + 2 * x];
				float pointY = resizeMap[y * sizeX * 2 + 2 * x + 1];

				//TODO(yigit) : DEBUG THIS
				int originalX = pointX;
				int originalY = pointY;

				int bufferIndex = originalY * image.sizeX * image.channelCount + originalX * image.channelCount + channel;

				float p[4];
				p[0] = buffer_ptr[bufferIndex];
				p[1] = buffer_ptr[bufferIndex + image.channelCount];
				p[2] = buffer_ptr[bufferIndex + image.sizeX * image.channelCount];
				p[3] = buffer_ptr[bufferIndex + image.sizeX * image.channelCount + image.channelCount];

				float offsetX = pointX - (originalX);
				float offsetY = pointY - (originalY);


				float A, B, C, D;

				/*
				p[0]    A     p[1]

				C			    D

				p[2]    B      p[3]
				*/

				A = p[1] * offsetX + p[0] * (1 - offsetX);
				B = p[3] * offsetX + p[2] * (1 - offsetX);
				C = p[2] * offsetY + p[0] * (1 - offsetY);
				D = p[3] * offsetY + p[1] * (1 - offsetY);


				float finalValue = (((1 - offsetY) * A + offsetY * B) + ((1 - offsetX) * C + offsetX * D)) / 2;
				int resultBufferIndex = y * result.sizeX * result.channelCount + x * result.channelCount + channel;
				result_ptr[resultBufferIndex] = finalValue;
			}
		}
	}

	free(resizeMap);

	return result;

}

image inf::ImageProcessor::resize2x(image image)
{
	inf::image newImage;
	newImage.sizeX = image.sizeX * 2;
	newImage.sizeY = image.sizeY * 2;
	newImage.channelCount = 4;
	newImage.format = inf::RGBA8;
	uint8_t* oldBuffer = (uint8_t*)image.buffer;
	uint8_t* newBuffer = (uint8_t*)malloc(newImage.sizeX * newImage.sizeY * newImage.channelCount);
	newImage.buffer = newBuffer;
	for (int y = 0; y < newImage.sizeY; y++)
	{
		for (int x = 0; x < newImage.sizeX; x++)
		{
			int bigImagePtr = y * newImage.sizeX * newImage.channelCount + x * newImage.channelCount;
			int smallImagePtr = (y / 2) * image.sizeX * image.channelCount + (x / 2) * image.channelCount;
			newBuffer[bigImagePtr] = oldBuffer[smallImagePtr];
			newBuffer[bigImagePtr + 1] = oldBuffer[smallImagePtr + 1];
			newBuffer[bigImagePtr + 2] = oldBuffer[smallImagePtr + 2];
			newBuffer[bigImagePtr + 3] = oldBuffer[smallImagePtr + 3];
		}
	}


	return newImage;
}



