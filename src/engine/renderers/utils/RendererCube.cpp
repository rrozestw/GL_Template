#include "RendererCube.hpp"
#include "../../input/Input.hpp"

#include "../../helpers/GLUtilities.hpp"
#include "../../helpers/Logger.hpp"

#include <stdio.h>
#include <vector>
#ifdef _WIN32
#define M_PI_2 1.57079632679
#endif

#include <chrono>
#include <thread>

RendererCube::~RendererCube(){}

RendererCube::RendererCube(Config & config, const std::string & cubemapName, const std::string & shaderName, const int width, const int height, const GLenum format, const GLenum type, const GLenum preciseFormat) : Renderer(config) {
	
	_resultFramebuffer = std::make_shared<Framebuffer>(width, height, format, type, preciseFormat, GL_LINEAR, GL_CLAMP_TO_EDGE, false);
	
	_program = Resources::manager().getProgram(shaderName, "object_basic", shaderName);
	_cubemap = Object(_program, "skybox", {}, {{cubemapName, true }});
	
	checkGLError();

	// GL options
	glEnable(GL_DEPTH_TEST);
	checkGLError();
	
	checkGLError();
	
}


void RendererCube::draw() {
	drawCube(_resultFramebuffer->width(), _resultFramebuffer->height(), "cubemap-output-default");
}

void RendererCube::drawCube(const int localWidth, const int localHeight, const std::string & localOutputPath) {
	glDisable(GL_DEPTH_TEST);
	
	_resultFramebuffer->bind();

	glViewport(0,0,localWidth,localHeight);
	
	const glm::mat4 projection = glm::perspective((float)M_PI_2, (float)_resultFramebuffer->width()/(float)_resultFramebuffer->height(), 0.1f, 200.0f);
	const glm::vec3 ups[6] = { glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,-1.0,0.0),glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,0.0,-1.0) };
	const glm::vec3 centers[6] = { glm::vec3(1.0,0.0,0.0), glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,-1.0,0.0) };
	const std::string suffixes[6] = { "px", "nx", "pz", "nz", "py", "ny"};

	// Loop over the faces. Instead we could use a geometry shader and multiple output layers, one for each face with the corresponding view transformation.
	
	for(size_t i = 0; i < 6; ++i){
		
		const glm::mat4 view = glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), centers[i], ups[i]);
		
		glClearColor(0.0f,0.0f,i,0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		_cubemap.draw(view, projection);
		glFlush();
		glFinish();
		
		const std::string outputPathComplete = localOutputPath + "-" + suffixes[i];
		GLUtilities::saveFramebuffer(_resultFramebuffer, localWidth, localHeight, outputPathComplete, false);
		
	}
	
	_resultFramebuffer->unbind();
	
	glEnable(GL_DEPTH_TEST);
	
}

void RendererCube::update(){
	Renderer::update();
	// Update nothing.
}

void RendererCube::physics(double fullTime, double frameTime){
	// Simulate nothing.
}


void RendererCube::clean() const {
	Renderer::clean();
	// Clean objects.
	_cubemap.clean();
	_resultFramebuffer->clean();
	
}


void RendererCube::resize(int width, int height){
	// Do nothing.
}



