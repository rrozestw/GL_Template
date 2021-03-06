#ifndef BoxBlur_h
#define BoxBlur_h
#include "Blur.hpp"


class BoxBlur : public Blur {

public:
	
	BoxBlur(int width, int height, bool approximate, GLuint format, GLuint type, GLuint preciseFormat);

	/// Draw function
	void process(const GLuint textureId);
	
	/// Clean function
	void clean() const;

	/// Handle screen resizing
	void resize(int width, int height);

private:
	
	ScreenQuad _blurScreen;
	std::shared_ptr<Framebuffer> _finalFramebuffer;
	
};

#endif
