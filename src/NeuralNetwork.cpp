#include "NeuralNetwork.h"

int NeuralNetwork::GetInputSize() {
	return inputSize;
}

int NeuralNetwork::GetOutputSize() {
	return outputSize;
}

NeuralNetwork::~NeuralNetwork() {};

FFN::FFN() {
	model = new mlpack::FFN<>;
	inputSize = 0;
	outputSize = 0;
}

FFN::FFN(int inputSize, int outputSize, int hiddenLayers) {
	init(inputSize, outputSize,	hiddenLayers);
}

FFN::FFN(const char* dataPath, const char* targetPath, int hiddenLayers) {
	// load data
	arma::mat rawFeatures, rawTargets, features, targets;
	mlpack::data::Load(dataPath, rawFeatures);
	mlpack::data::Load(targetPath, rawTargets);

	// get row of unique targets
	targets = arma::unique(rawTargets);

	// get first column of features
	features = rawFeatures.submat(arma::span(), arma::span(0));
	
	init(features.n_rows, targets.n_cols, hiddenLayers);
}

FFN::~FFN() {
	delete model;
}

void FFN::init(int inputSize, int outputSize, int hiddenLayers) {
	assert(inputSize > 0);
	assert(outputSize > 0);
	assert(hiddenLayers > 0);

	model = new mlpack::FFN<>;
	this->inputSize = inputSize;
	this->outputSize = outputSize;

	// calculate hidden layer sizes based on input and output sizes using linear interpolation
	float outputLayerIndex = hiddenLayers + 1, inputLayerSize = inputSize, outputLayerSize = outputSize;
	for (int i = 1; i < outputLayerIndex; i++) {
		model->Add<mlpack::Linear>((int)(inputLayerSize + (outputLayerSize - inputLayerSize) / outputLayerIndex * i));
		model->Add<mlpack::Sigmoid>();
	}
	// add output connection layer with softmax for classification
	model->Add<mlpack::Linear>(outputSize);
	model->Add<mlpack::LogSoftMax>();
}

void FFN::AddLayer(NeuralNetwork::LayerType layerType, int size) {
	assert(size > 0);
	assert(layerType != FFN::LSTM);
	
	switch (layerType)
	{
	case NeuralNetwork::Linear:
		model->Add<mlpack::Linear>(size);
		break;
	case NeuralNetwork::LinearNoBias:
		model->Add<mlpack::LinearNoBias>(size);
		break;
	case NeuralNetwork::LayerNorm:
		model->Add<mlpack::LayerNorm>(size);
		break;
	case NeuralNetwork::DropConnect:
		model->Add<mlpack::DropConnect>(size);
		break;
	default:
		break;
	}
}

void FFN::AddActivation(NeuralNetwork::ActivationFunction activationFunction) {
	switch (activationFunction)
	{
	case NeuralNetwork::Sigmoid:
		model->Add<mlpack::Sigmoid>();
		break;
	case NeuralNetwork::ReLU:
		model->Add<mlpack::ReLU>();
		break;
	case NeuralNetwork::LeakyReLU:
		model->Add<mlpack::LeakyReLU>();
		break;
	case NeuralNetwork::FlexibleReLU:
		model->Add<mlpack::FlexibleReLU>();
		break;
	case NeuralNetwork::PReLU:
		model->Add<mlpack::PReLU>();
		break;
	case NeuralNetwork::ELU:
		model->Add<mlpack::ELU>();
		break;
	case NeuralNetwork::CELU:
		model->Add<mlpack::CELU>();
		break;
	case NeuralNetwork::SELU:
		model->Add<mlpack::SELU>();
		break;
	case NeuralNetwork::LogSoftmax:
		model->Add<mlpack::LogSoftMax>();
		break;
	case NeuralNetwork::TanH:
		model->Add<mlpack::TanH>();
		break;
	case NeuralNetwork::ELiSH:
		model->Add<mlpack::Elish>();
		break;
	case NeuralNetwork::Elliot:
		model->Add<mlpack::Elliot>();
		break;
	case NeuralNetwork::FlattenTSwish:
		model->Add<mlpack::FTSwish>();
		break;
	case NeuralNetwork::HardSigmoid:
		model->Add<mlpack::HardSigmoid>();
		break;
	case NeuralNetwork::HardTanh:
		model->Add<mlpack::HardTanH>();
		break;
	case NeuralNetwork::HardSwish:
		model->Add<mlpack::HardSwish>();
		break;
	case NeuralNetwork::Dropout:
		model->Add<mlpack::Dropout>();
		break;
	default:
		break;
	}
}

int FFN::Predict(const std::vector<Arinc429>* data) {
	assert(data != nullptr);
	assert(data->size() == inputSize);

	// put data from vector into arma mat
	arma::mat inputFeatures(inputSize, 1), result;
	for (int i = 0; i < inputSize; i++) {
		inputFeatures(i, 0) = data->at(i).value;
	}

	model->Predict(inputFeatures, result);

	return result.index_max();
}

bool FFN::LoadModel(const char* path) {
	bool result = mlpack::data::Load(path, "FFN_Model", *(model));
	this->inputSize = model->InputDimensions()[0];
	return result;
}

bool FFN::SaveModel(const char* path) {
	return mlpack::data::Save(path, "FFN_Model", *(model));
}

double FFN::Train(const char* dataPath, const char* targetPath, int stride) {
	arma::mat trainingFeatures, trainingTargets;
	mlpack::data::Load(dataPath, trainingFeatures);
	mlpack::data::Load(targetPath, trainingTargets);

	double result = model->Train(trainingFeatures, trainingTargets);
	
	this->inputSize = model->InputDimensions()[0];
	this->outputSize = ((arma::mat)arma::unique(trainingTargets)).n_cols;

	return result;
}

double FFN::TestClassificationError(const char* dataPath, const char* targetPath, int stride) {
	arma::mat testFeatures, testTargets;
	mlpack::data::Load(dataPath, testFeatures);
	mlpack::data::Load(targetPath, testTargets);

	arma::mat predictionResults;
	model->Predict(testFeatures, predictionResults);

	arma::mat prediction = arma::zeros<arma::mat>(1, predictionResults.n_cols);

	for (size_t i = 0; i < predictionResults.n_cols; ++i)
	{
		prediction(i) = arma::as_scalar(arma::find(
			arma::max(predictionResults.col(i)) == predictionResults.col(i), 1));
	}

	size_t correct = arma::accu(prediction == testTargets);
	double classificationError = 1 - double(correct) / testFeatures.n_cols;

	return classificationError;
}

RNN::RNN() {
	model = new mlpack::RNN<>;
	inputSize = 0;
	outputSize = 0;
}

RNN::RNN(int inputSize, int outputSize, int hiddenLayers) {
	init(inputSize, outputSize, hiddenLayers);
}

RNN::RNN(const char* dataPath, const char* targetPath, int hiddenLayers) {
	// load data
	arma::mat rawFeatures, rawTargets, features, targets;
	mlpack::data::Load(dataPath, rawFeatures);
	mlpack::data::Load(targetPath, rawTargets);

	// get row of unique targets
	targets = arma::unique(rawTargets);

	// get first column of features
	features = rawFeatures.submat(arma::span(), arma::span(0));

	init(features.n_rows, targets.n_cols, hiddenLayers);
}

RNN::~RNN() {
	delete model;
}

void RNN::init(int inputSize, int outputSize, int hiddenLayers) {
	assert(inputSize > 0);
	assert(outputSize > 0);
	assert(hiddenLayers > 0);

	model = new mlpack::RNN<mlpack::NegativeLogLikelihood>;
	this->inputSize = inputSize;
	this->outputSize = outputSize;

	// calculate hidden layer sizes based on input and output sizes using linear interpolation
	float outputLayerIndex = hiddenLayers + 1, inputLayerSize = inputSize, outputLayerSize = outputSize;
	for (int i = 1; i < outputLayerIndex; i++) {
		model->Add<mlpack::LSTM>((int)(inputLayerSize + (outputLayerSize - inputLayerSize) / outputLayerIndex * i));
		model->Add<mlpack::Sigmoid>();
	}
	// add output connection layer with softmax for classification
	model->Add<mlpack::LSTM>(outputSize);
	model->Add<mlpack::LogSoftMax>();

}

void RNN::AddLayer(NeuralNetwork::LayerType layerType, int size) {
	assert(size > 0);

	switch (layerType)
	{
	case NeuralNetwork::Linear:
		model->Add<mlpack::Linear>(size);
		break;
	case NeuralNetwork::LinearNoBias:
		model->Add<mlpack::LinearNoBias>(size);
		break;
	case NeuralNetwork::LayerNorm:
		model->Add<mlpack::LayerNorm>(size);
		break;
	case NeuralNetwork::LSTM:
		model->Add<mlpack::LSTM>(size);
		break;
	case NeuralNetwork::DropConnect:
		model->Add<mlpack::DropConnect>(size);
		break;
	default:
		break;
	}
}

void RNN::AddActivation(NeuralNetwork::ActivationFunction activationFunction) {
	switch (activationFunction)
	{
	case NeuralNetwork::Sigmoid:
		model->Add<mlpack::Sigmoid>();
		break;
	case NeuralNetwork::ReLU:
		model->Add<mlpack::ReLU>();
		break;
	case NeuralNetwork::LeakyReLU:
		model->Add<mlpack::LeakyReLU>();
		break;
	case NeuralNetwork::FlexibleReLU:
		model->Add<mlpack::FlexibleReLU>();
		break;
	case NeuralNetwork::PReLU:
		model->Add<mlpack::PReLU>();
		break;
	case NeuralNetwork::ELU:
		model->Add<mlpack::ELU>();
		break;
	case NeuralNetwork::CELU:
		model->Add<mlpack::CELU>();
		break;
	case NeuralNetwork::SELU:
		model->Add<mlpack::SELU>();
		break;
	case NeuralNetwork::LogSoftmax:
		model->Add<mlpack::Softmax>();
		break;
	case NeuralNetwork::TanH:
		model->Add<mlpack::TanH>();
		break;
	case NeuralNetwork::ELiSH:
		model->Add<mlpack::Elish>();
		break;
	case NeuralNetwork::Elliot:
		model->Add<mlpack::Elliot>();
		break;
	case NeuralNetwork::FlattenTSwish:
		model->Add<mlpack::FTSwish>();
		break;
	case NeuralNetwork::HardSigmoid:
		model->Add<mlpack::HardSigmoid>();
		break;
	case NeuralNetwork::HardTanh:
		model->Add<mlpack::HardTanH>();
		break;
	case NeuralNetwork::HardSwish:
		model->Add<mlpack::HardSwish>();
		break;
	case NeuralNetwork::Dropout:
		model->Add<mlpack::Dropout>();
		break;
	default:
		break;
	}
}

int RNN::Predict(const std::vector<Arinc429>* data) {
	assert(data != nullptr);
	assert(data->size() % inputSize == 0);

	int rho = data->size() / inputSize;
	// put data into arma cube
	arma::cube inputFeatures(inputSize, 1, rho), result;
	for (int i = 0; i < rho; i++) {
		for (int j = 0; j < inputSize; j++) {
			inputFeatures(i, 0, j) = data->at(i * inputSize + j).value;
		}
	}

	model->Predict(inputFeatures, result);

	arma::mat resultForLastInSequence = result.subcube(arma::span(), arma::span(), arma::span(result.n_slices - 1));

	return resultForLastInSequence.index_max();
}

bool RNN::LoadModel(const char* path) {
	bool result = mlpack::data::Load(path, "RNN_Model", *(model));
	this->inputSize = model->InputDimensions()[0];
	return result;
}

bool RNN::SaveModel(const char* path) {
	return mlpack::data::Save(path, "RNN_Model", *(model));
}

double RNN::Train(const char* dataPath, const char* targetPath, int stride) {
	arma::mat rawFeatures, rawTargets;
	arma::cube trainingFeatures, trainingTargets;
	mlpack::data::Load(dataPath, rawFeatures);
	mlpack::data::Load(targetPath, rawTargets);

	assert(rawData.n_cols % stride == 0);

	int sequences = rawFeatures.n_cols / stride;
	// load data from mat into cube
	for (int i = 0; i < sequences; i++) {
		trainingFeatures.subcube(arma::span(), arma::span(i), arma::span()) =
			rawFeatures.submat(arma::span(), arma::span(sequences * stride, sequences * stride + stride));

		trainingTargets.subcube(arma::span(), arma::span(i), arma::span()) =
			rawTargets.submat(arma::span(), arma::span(sequences * stride, sequences * stride + stride));
	}

	return model->Train(trainingFeatures, trainingTargets);
}

double RNN::TestClassificationError(const char* dataPath, const char* targetPath, int stride) {
	arma::mat rawFeatures, rawTargets;
	arma::cube testFeatures, testTargets;
	mlpack::data::Load(dataPath, rawFeatures);
	mlpack::data::Load(targetPath, rawTargets);

	assert(rawData.n_cols % stride == 0);

	int sequences = rawFeatures.n_cols / stride;
	// load data from mat into cube
	for (int i = 0; i < sequences; i++) {
		testFeatures.subcube(arma::span(), arma::span(i), arma::span()) =
			rawFeatures.submat(arma::span(), arma::span(sequences * stride, sequences * stride + stride));

		testTargets.subcube(arma::span(), arma::span(i), arma::span()) =
			rawTargets.submat(arma::span(), arma::span(sequences * stride, sequences * stride + stride));
	}

	arma::cube predictionResults;
	model->Predict(testFeatures, predictionResults);

	arma::cube prediction = arma::zeros<arma::cube>(1, predictionResults.n_cols, predictionResults.n_slices);

	for (size_t i = 0; i < predictionResults.n_cols; ++i)
	{
		for (size_t j = 0; j < predictionResults.n_slices; j++) {
			prediction(0, i, j) = predictionResults.subcube(arma::span(), arma::span(i), arma::span(j)).index_max();
		}
	}

	size_t correct = arma::accu(prediction == testTargets);
	double classificationError = 1 - double(correct) / testFeatures.n_cols * testFeatures.n_slices;

	return 0;
}