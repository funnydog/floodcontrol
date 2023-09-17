#include <ctime>
#include <random>
#include <fstream>
#include <sstream>

#include "utility.hpp"

namespace
{
std::default_random_engine createRandomEngine()
{
	auto seed = static_cast<unsigned long>(std::time(nullptr));
	return std::default_random_engine(seed);
}
auto randomEngine = createRandomEngine();
}

namespace Utility
{
std::string loadFile(const std::filesystem::path &file)
{
	std::ifstream in(file);
	if (!in)
	{
		throw std::runtime_error(file.string() + " not found.");
	}

	std::stringstream buffer;
	buffer << in.rdbuf();

	return buffer.str();
}

int randomInt(int exclusiveMax)
{
	std::uniform_int_distribution<> distr(0, exclusiveMax - 1);
	return distr(randomEngine);
}

}
