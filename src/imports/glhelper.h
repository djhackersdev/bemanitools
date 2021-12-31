#include <GL/gl.h>

void glFramebufferTexture2DEXT(GLenum target,
 	GLenum attachment,
 	GLenum textarget,
 	GLuint texture,
 	GLint level);

void glGenFramebuffersEXT(GLsizei n,
  	GLuint *ids);
