#pragma once

#define MLPACK_ENABLE_ANN_SERIALIZATION

#include <mlpack/core.hpp>
#include <mlpack/methods/ann.hpp>
#include <mlpack/methods/ann/activation_functions/activation_functions.hpp>
#include <mlpack/methods/ann/layer/serialization.hpp>
#include "Arinc429.h"


class NeuralNetwork {
public:
	static enum LayerType {
		Linear, LinearNoBias, LayerNorm, LSTM, DropConnect
	};

	static enum ActivationFunction {
		Sigmoid, ReLU, LeakyReLU, FlexibleReLU, PReLU, ELU, CELU, SELU, LogSoftmax, TanH, ELiSH, Elliot, FlattenTSwish, HardSigmoid, HardTanh, HardSwish, Dropout
	};

	virtual ~NeuralNetwork();

	virtual void AddLayer(LayerType layerType, int size) = 0;

	virtual void AddActivation(ActivationFunction activationFunction) = 0;
	
	virtual int Predict(const std::vector<Arinc429>* data) = 0;

	virtual bool LoadModel(const char* path) = 0;

	virtual bool SaveModel(const char* path) = 0;

	virtual double Train(const char* dataPath, const char* targetPath, int stride = 1) = 0;

	virtual double TestClassificationError(const char* dataPath, const char* targetPath, int stride = 1) = 0;

	int GetInputSize();

	int GetOutputSize();

protected:
	int inputSize, outputSize;
};

class FFN : public NeuralNetwork {
public:
	FFN();

	FFN(int inputSize, int outputSize, int hiddenLayers = 1);

	FFN(const char* dataPath, const char* targetPath, int hiddenLayers = 1);

	~FFN();

	virtual void AddLayer(LayerType layerType, int size);

	virtual void AddActivation(ActivationFunction activationFunction);

	virtual int Predict(const std::vector<Arinc429>* data);

	virtual bool LoadModel(const char* path);

	virtual bool SaveModel(const char* path);

	virtual double Train(const char* dataPath, const char* targetPath, int stride = 1);

	virtual double TestClassificationError(const char* dataPath, const char* targetPath, int stride = 1);

private:
	mlpack::FFN<>* model;

	void init(int inputSize, int outputSize, int hiddenLayers);
};

class RNN : public NeuralNetwork {
public:
	RNN();

	RNN(int inputSize, int outputSize, int hiddenLayers = 1);

	RNN(const char* dataPath, const char* targetPath, int hiddenLayers = 1);

	~RNN();

	virtual void AddLayer(LayerType layerType, int size);

	virtual void AddActivation(ActivationFunction activationFunction);

	virtual int Predict(const std::vector<Arinc429>* data);

	virtual bool LoadModel(const char* path);

	virtual bool SaveModel(const char* path);

	virtual double Train(const char* dataPath, const char* targetPath, int stride = 1);

	virtual double TestClassificationError(const char* dataPath, const char* targetPath, int stride = 1);

private:
	mlpack::RNN<>* model;

	void init(int inputSize, int outputSize, int hiddenLayers);
};