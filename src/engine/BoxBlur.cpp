#include "BoxBlur.hpp"

#include <stdio.h>
#include <vector>


BoxBlur::BoxBlur(int width, int height,  bool approximate, GLuint format, GLuint type, GLuint preciseFormat) : Blur() {
	
	std::string blur_type_name = "box-blur-" + (approximate ? std::string("approx-") : "");
	switch (format) {
		case GL_RED:
			blur_type_name += "1";
			break;
		case GL_RG:
			blur_type_name += "2";
			break;
		case GL_RGB:
			blur_type_name += "3";
			break;
		default:
			blur_type_name += "4";
			break;
	}
	
	_blurScreen.init(blur_type_name);
	// Create one framebuffer.
	_finalFramebuffer = std::make_shared<Framebuffer>(width, height, format, type, preciseFormat, GL_LINEAR, GL_CLAMP_TO_EDGE, false);
	// Final combining buffer.
	_finalTexture = _finalFramebuffer->textureId();
	
	checkGLError();
	
}

/// Draw function
void BoxBlur::process(const GLuint textureId){
	_finalFramebuffer->bind();
	glViewport(0, 0, _finalFramebuffer->width(), _finalFramebuffer->height());
	glClear(GL_COLOR_BUFFER_BIT);
	_blurScreen.draw(textureId);
	_finalFramebuffer->unbind();
}


/// Clean function
void BoxBlur::clean() const {
	_blurScreen.clean();
	_finalFramebuffer->clean();
	Blur::clean();
}

/// Handle screen resizing
void BoxBlur::resize(int width, int height){
	_finalFramebuffer->resize(width, height);
}
