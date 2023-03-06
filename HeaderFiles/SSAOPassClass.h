// GLM Mathematics
#include <glm/glm.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>


#include "Light.h"
#include "Shader2.h"

using namespace std;

class SSAOPassClass
{
	private:

		//FBO, texture, and RBO reference
		unsigned int ambientOcclusionFrameBuffer;
		unsigned int ambientOcclusionTexture;
		unsigned int depthRenderBuffer;
	

		const unsigned int TEX_WIDTH = 1024, TEX_HEIGHT = 1024;

		//Shader reference
		Shader2* simpleSSAOShader;

		//Models reference used for rendering
		vector<MyModelClass*> myModels;

		glm::mat4 ViewMatrix;


		void initSSAOTextureAndRBO()
		{
			// The framebuffer object (FBO) is a concept in OpenGL that allows you to render to textures and other attachments, 
			// instead of directly rendering to the screen. It is essentially an abstraction of the rendering process that provides 
			// a way to render to a texture, render buffer, or multiple render targets

			// Generate the ambient occlusion texture
			glGenTextures(1, &ambientOcclusionTexture);
			glBindTexture(GL_TEXTURE_2D, ambientOcclusionTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEX_WIDTH, TEX_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// Create a framebuffer object for the ambient occlusion pass
			glGenFramebuffers(1, &ambientOcclusionFrameBuffer);										// Generates 1 framebuffer and assigns it to the address of ambientOcclusionFrameBuffer
			glBindFramebuffer(GL_FRAMEBUFFER, ambientOcclusionFrameBuffer);							// Binds the framebuffer object ambientOcclusionFrameBuffer to the framebuffer target GL_FRAMEBUFFER, 
																									// meaning that all subsequent rendering commands will render to the textures attached to this 
																									// framebuffer object until the target is changed or the framebuffer is unbound.

			// Attach ambient occlusion texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ambientOcclusionTexture, 0);

			// Attach the depth render buffer object to the current framebuffer object (FBO) as the depth attachment. 
			// This means that the depth render buffer will be used as the depth buffer for the FBO. In the context of the SSAO pass,
			// this allows the SSAO fragment shader to access the depth information stored in the depth render buffer.

			// Generate depth render buffer
			glGenRenderbuffers(1, &depthRenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,  TEX_WIDTH, TEX_HEIGHT);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			// attach depth render buffer to framebuffer
    		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);


			 // check for framebuffer completeness
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			}

			// reset default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}


		void renderScene()
		{
			// To render to an FBO texture, you need to bind the FBO, then render the objects.
			simpleSSAOShader->Use();

			glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);												// Set the dimensions of the viewport to render to the texture
			glBindFramebuffer(GL_FRAMEBUFFER, ambientOcclusionFrameBuffer);							// Bind SSAO FBO for rendering
   		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);										// Clear the color and depth buffers of the SSAO FBO								
			renderModels();																			// Render the models in the scene	
			
			// reset viewport
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 1400, 800);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void renderModels()
		{
			//Render the scene just for depth to be used for SSAO.
			for (int i = 0; i < myModels.size(); i++)
			{
				myModels[i]->renderForAmbientOcclusion(*simpleSSAOShader);
			}
		}

	public:

		SSAOPassClass(glm::mat4 &ViewMatrix, vector<MyModelClass*> myModels)
		{
			//Pass the reference to the models vector
			this->myModels = myModels;
			//Pass the reference to the viewmatrix
			this->ViewMatrix = ViewMatrix;
			//Create a new shader for SSAO
			simpleSSAOShader = new Shader2("Shaders/simpleSSAOShader.vs", "Shaders/simpleSSAOShader.frag");
			
			initSSAOTextureAndRBO();
		}

		unsigned int* getDepthMap()
		{
			return &ambientOcclusionTexture;
		}

		void render()
		{
			renderScene();
		}

};
