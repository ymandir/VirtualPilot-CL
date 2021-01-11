#if !defined(CUSTOM_NET_IMP_H)

#include <torch/torch.h>


struct SequentialWrapperImpl : torch::nn::SequentialImpl
{
	using SequentialImpl::SequentialImpl;
	torch::Tensor forward(torch::Tensor Input)
	{
		return SequentialImpl::forward(Input);
	}
};

TORCH_MODULE(SequentialWrapper);


class CustomNetImp : public torch::nn::Module
{
public:
	CustomNetImp()
	{
		c1 = register_module("c1", torch::nn::Conv2d(torch::nn::Conv2dOptions(3, 64, 3).padding(1)));
		b1 = register_module("b1", torch::nn::BatchNorm2d(64));
		m1 = register_module("m1", torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(4).stride(2)));

		c2 = register_module("c2", torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 128, 3).padding(1)));
		b2 = register_module("b2", torch::nn::BatchNorm2d(128));
		m2 = register_module("m2", torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(4).stride(2)));

		c3 = register_module("c3", torch::nn::Conv2d(torch::nn::Conv2dOptions(128, 256, 3).padding(1)));
		b3 = register_module("b3", torch::nn::BatchNorm2d(256));
		m3 = register_module("m3", torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(4).stride(2)));

		avgP = register_module("adaptive_avg_pool", torch::nn::AdaptiveAvgPool2d(torch::nn::AdaptiveAvgPool2dOptions({ 7, 7 })));

		convOutputSize = ConvOutputSize(3, 100, 100);
		std::cout << convOutputSize << std::endl;

		fc1 = register_module("fc1", torch::nn::Linear(convOutputSize, 256));
		;
		fc2 = register_module("fc2", torch::nn::Linear(256, 64));

		fc3 = register_module("fc3", torch::nn::Linear(64, 16));

		fc4 = register_module("fc4", torch::nn::Linear(16, 2));

	}

	torch::Tensor forward(torch::Tensor x) {
		// Use one of many tensor manipulation functions
		x = c1->forward(x);
		x = torch::relu(x);
		x = b1->forward(x);
		x = m1->forward(x);

		x = c2->forward(x);
		x = torch::relu(x);
		x = b2->forward(x);
		x = m2->forward(x);

		x = c3->forward(x);
		x = torch::relu(x);
		x = b3->forward(x);
		x = m3->forward(x);

		x = avgP->forward(x);

		x = x.flatten();
		x = fc1->forward(x);
		x = torch::relu(x);
		x = fc2->forward(x);
		x = torch::relu(x);
		x = torch::dropout(x, 0.25, true);
		x = fc3->forward(x);
		x = torch::relu(x);
		x = fc4->forward(x);
		x = torch::relu(x);
		return torch::softmax(x, 0);
	}





private:

	int64_t ConvOutputSize(int64_t Channels, int64_t Height, int64_t Width)
	{
		torch::Tensor dummy = torch::ones({ 1, Channels, Width, Height });
		dummy = c1->forward(dummy);
		dummy = m1->forward(dummy);
		dummy = c2->forward(dummy);
		dummy = m2->forward(dummy);
		dummy = c3->forward(dummy);
		dummy = m3->forward(dummy);
		dummy = avgP->forward(dummy);
		return dummy.numel();
	}


	torch::nn::Linear fc1{ nullptr }, fc2{ nullptr }, fc3{ nullptr }, fc4{ nullptr };

	torch::nn::BatchNorm2d b1{ nullptr }, b2{ nullptr }, b3{ nullptr };
	torch::nn::Conv2d c1{ nullptr }, c2{ nullptr }, c3{ nullptr };
	torch::nn::MaxPool2d m1{ nullptr }, m2{ nullptr }, m3{ nullptr };
	torch::nn::AdaptiveAvgPool2d avgP{ nullptr };
	int convOutputSize = 1;
};

#define CUSTOM_NET_IMP_H
#endif