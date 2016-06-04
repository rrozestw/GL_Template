#include <stdio.h>
#include <iostream>
#include <vector>
// glm additional header to generate transformation matrices directly.
#include <glm/gtc/matrix_transform.hpp>

#include "helpers/ProgramUtilities.h"
#include "Renderer.h"

Renderer::Renderer(){}

Renderer::~Renderer(){}

void Renderer::init(int width, int height){
	_width = width;
	_height = height;

	updateProjectionMatrix();

	// initialize the timer
	_timer = glfwGetTime();

	
	// Query the renderer identifier, and the supported OpenGL version.
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;
	checkGLError();

	// GL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_FRAMEBUFFER_SRGB);
	checkGLError();
	
	
	// Setup light
	_light.position = glm::vec4(0.0f); // position will be updated at each frame
	_light.shininess = 250.0f;
	_light.Ia = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f);
	_light.Id = glm::vec4(0.7f, 0.8f, 0.9f, 0.0f);
	_light.Is = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	
	// Setup material
	_material.Ka = glm::vec4(0.3f,0.2f,0.0f,0.0f);
	_material.Kd = glm::vec4(1.0f, 0.5f, 0.0f, 0.0f);
	_material.Ks = glm::vec4(1.0f, 1.0f, 1.0f,0.0f);
	
	// Generate the buffer.
	glGenBuffers(1, &_ubo);
	// Bind the buffer.
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	
	// We need to know the alignment size if we want to store two uniform blocks in the same uniform buffer.
	GLint uboAlignSize = 0;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlignSize);
	// Compute the padding for the second block, it needs to be a multiple of uboAlignSize (typically uboAlignSize will be 256.)
	GLuint padding = 4*sizeof(glm::vec4) + 1*sizeof(float);
	padding = ((padding/uboAlignSize)+1)*uboAlignSize;
	
	// Allocate enough memory to hold the Light struct and Material structures.
	glBufferData(GL_UNIFORM_BUFFER, padding + 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	
	// Bind the range allocated to the light.
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, _ubo, 0, 4*sizeof(glm::vec4) + sizeof(float));
	// Submit the data.
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 4*sizeof(glm::vec4) + sizeof(float), &_light);
	
	
	// Bind the range allocated to the material.
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, _ubo, padding, 3*sizeof(glm::vec4));
	// Submit the data.
	glBufferSubData(GL_UNIFORM_BUFFER, padding, 3*sizeof(glm::vec4), &_material);
	
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	
	// Initialize objects.
	_suzanne.init();
	_skybox.init();
	
	checkGLError();
	
}


void Renderer::draw(){

	// Compute the time elapsed since last frame
	float elapsed = glfwGetTime() - _timer;
	_timer = glfwGetTime();

	// Physics simulation
	physics(elapsed);

	// Set the clear color to white.
	glClearColor(1.0f,1.0f,1.0f,0.0f);
	// Clear the color and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the light position (in view space).
	// Bind the buffer.
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	// Obtain a handle to the underlying memory.
	GLvoid * ptr = glMapBuffer(GL_UNIFORM_BUFFER,GL_WRITE_ONLY);
	// Copy the light position.
	memcpy(ptr, &(_light.position[0]), sizeof(glm::vec4));
	// Unmap, unbind.
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Draw objects.
	_suzanne.draw(elapsed, _camera._view, _projection);
	_skybox.draw(elapsed, _camera._view, _projection);
	
	// Update timer
	_timer = glfwGetTime();
}

void Renderer::physics(float elapsedTime){
	_camera.update(elapsedTime);
	// Compute the light position in view space.
	_light.position = _camera._view * glm::vec4(2.0f,2.0f,2.0f,1.0f);
}


void Renderer::clean(){
	// Clean objects.
	_suzanne.clean();
	_skybox.clean();
}


void Renderer::resize(int width, int height){
	_width = width;
	_height = height;
	//Update the size of the viewport.
	glViewport(0, 0, width, height);
	updateProjectionMatrix();
}

void Renderer::keyPressed(int key, int action){
	if(action == GLFW_PRESS){
		if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D || key == GLFW_KEY_Q || key == GLFW_KEY_E){
			_camera.registerMove(key, true);

		} else if(key == GLFW_KEY_R) {
			_camera.reset();

		} else {
			std::cout << "Key: " << key << " (" << char(key) << ")." << std::endl;
		}
	} else if(action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D || key == GLFW_KEY_Q || key == GLFW_KEY_E){
			_camera.registerMove(key, false);
		}
	}
}

void Renderer::buttonPressed(int button, int action, double x, double y){
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			// We normalize the x and y values to the [-1, 1] range.
			float xPosition =  fmax(fmin(1.0f,2.0f * (float)x / _width - 1.0),-1.0f);
			float yPosition =  fmax(fmin(1.0f,2.0f * (float)y / _height - 1.0),-1.0f);
			_camera.startLeftMouse(xPosition, yPosition);
			return;
		} else if (action == GLFW_RELEASE) {
			_camera.endLeftMouse();
		}
	}
	std::cout << "Button: " << button << ", action: " << action << std::endl;            
}

void Renderer::mousePosition(int x, int y, bool leftPress, bool rightPress){
	if (leftPress){
		// We normalize the x and y values to the [-1, 1] range.
        float xPosition =  fmax(fmin(1.0f,2.0f * (float)x / _width - 1.0),-1.0f);
		float yPosition =  fmax(fmin(1.0f,2.0f * (float)y / _height - 1.0),-1.0f);
		_camera.leftMouseTo(xPosition, yPosition); 
    }
}

void Renderer::updateProjectionMatrix(){
	// Perspective projection.
	_projection = glm::perspective(45.0f, float(_width) / float(_height), 0.1f, 100.f);
}



