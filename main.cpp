#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stl.h"
#include <glm/glm.hpp>
#include <objLoader/OBJ_Loader.h>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#define TINYPLY_IMPLEMENTATION
#include <tinyply.h>
#include <texture.h>

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* PARTICULES */
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
	float size;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0, 1);
	std::uniform_real_distribution<float> distributionWorld(-1, 1);

	std::vector<Particule> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particule{
				{
				distributionWorld(generator),
				distributionWorld(generator),
				distributionWorld(generator)
				},
				{
				distribution01(generator),
				distribution01(generator),
				distribution01(generator)
				},
				{0.f, 0.f, 0.f},
				distribution01(generator)
				});
	}

	return p;
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	const size_t nParticules = 1000;
	//auto particules = MakeParticules(nParticules);
	/*auto triangles = ReadStl("logo.stl");*/

	objl::Loader loader;
	bool loaded = loader.LoadFile("Statuette.obj");
	auto triangles = loader.LoadedVertices;
	auto indices = loader.LoadedIndices;

	if (!loaded) {
		std::cerr << "Error Import: Il y a eu un érreur lors de l'import" << std::endl;
	}
	std::cerr << "Info Import : NbrTriangle = " << triangles.size() << std::endl;




	Image image = LoadImage("result.bmp");
	//Image image = LoadImage("sheep_normal.bmp");
	glEnable(GL_DEPTH_TEST);

	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureStorage2D(tex, 1, GL_RGB8, image.width, image.height);

	GLuint texUnit = 0;
	glTextureSubImage2D(tex, 0, 0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.data.data());
	glBindTextureUnit(texUnit, tex);

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");

	const auto program = AttachAndLink({vertex, fragment});

	glUseProgram(program);


	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), particules.data(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(objl::Vertex) * triangles.size(), triangles.data(), GL_STATIC_DRAW);

	// Bindings
	const auto index = glGetAttribLocation(program, "pos");
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), 0);
	glEnableVertexAttribArray(index);

	const auto index_uv = glGetAttribLocation(program, "vertexUV");
	glVertexAttribPointer(index_uv, 2, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), (void*)offsetof(objl::Vertex, TextureCoordinate));
	glEnableVertexAttribArray(index_uv);


	// Buffers
	GLuint indexVertBuffer;
	glGenBuffers(1, &indexVertBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVertBuffer);
	//glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), particules.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glEnable(GL_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		//glUniform2d(glGetUniformLocation(program, "mousePos"), xpos, ypos);

		//Bind Texture
		//auto id = glGetUniformLocation(program, "tex");
		//glUniform1i(id, texUnit);

		float t = glfwGetTime();
		glUniform1f(glGetUniformLocation(program, "time"), t);

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glm::mat4 trans(1);
		trans = glm::scale(trans, glm::vec3(0.3f, 0.3f, 0.3f));
		trans *= glm::rotate(trans,
			t * glm::radians(180.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		glUniformMatrix4fv(glGetUniformLocation(program, "trans"), 1, GL_FALSE, glm::value_ptr(trans));

		glViewport(0, 0, width, height);

		/*for (int i = 0; i < particules.size(); i++)
		{
			particules[i].position += particules[i].speed * 0.5f;

			particules[i].speed += + 0.5f * 9.8f * glm::vec3(0, -0.0001, 0);

			if (particules[i].position.y < -1) {
				particules[i].position = glm::vec3((xpos / width)*2-1, ((ypos / height) * 2 - 1) *-1 , 1);
				particules[i].speed = glm::vec3(0.,0.,0.);
			}
		}*/

		//glBufferSubData(GL_ARRAY_BUFFER, 0, particules.size() * sizeof(Particule), particules.data());

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.5f, 0.8f, 0.3f, 1.0f);

		//glDrawArrays(GL_POINTS, 0, triangles.size());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
