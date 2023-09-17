#pragma once

#include <filesystem>
#include <string>

#include <glm/glm.hpp>

enum class ShaderType
{
	Vertex,
	Fragment,
	Geometry,
	Compute,
};

class ShaderUniform
{
public:
	explicit ShaderUniform(int location);

	void set(const glm::mat4 &matrix);
	void set(int value);

private:
	int mLocation;
};

class Shader
{
public:
	Shader();
	~Shader();

	void attach(const std::string &source, ShaderType shaderType);
	void attachFile(const std::filesystem::path &filename, ShaderType shaderType);
	void link();

	ShaderUniform getUniform(const std::string &name) const;

	static void bind(const Shader *shader);
private:
	void checkCompilation();
	void checkLink();

private:
	unsigned mProgram;
};
