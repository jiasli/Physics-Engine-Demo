/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
 // Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
 // Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifdef RENDER_SNIPPET

#include <vector>

#include "PxPhysicsAPI.h"

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetRender/SnippetRender.h"
#include "../SnippetRender/SnippetCamera.h"

#include "pic.h"


using namespace physx;

extern void initPhysics();
extern void stepPhysics();
extern void cleanupPhysics();
extern void keyPress(const char key, const PxTransform& camera);

int saveScreenToFile = 0;

namespace
{
	Snippets::Camera*	sCamera;

	/* Write a screenshot, in the PPM format, to the specified filename, in PPM format */
	void saveScreenshot(int windowWidth, int windowHeight, char *filename)
	{
		if (filename == NULL)
			return;

		// Allocate a picture buffer 
		Pic * in = pic_alloc(windowWidth, windowHeight, 3, NULL);

		printf("File to save to: %s\n", filename);

		for (int i = windowHeight - 1; i >= 0; i--)
		{
			glReadPixels(0, windowHeight - i - 1, windowWidth, 1, GL_RGB, GL_UNSIGNED_BYTE,
				&in->pix[i*in->nx*in->bpp]);
		}

		if (ppm_write(filename, in))
			printf("File saved Successfully\n");
		else
			printf("Error in Saving\n");

		pic_free(in);
	}

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}

	void keyboardCallback(unsigned char key, int x, int y)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		if (key == 27)
			exit(0);

		if (key == 'p') saveScreenToFile = 1 - saveScreenToFile;

		if (!sCamera->handleKey(key, true))
		{
			PxVec3 viewY = sCamera->mDir.cross(PxVec3(0, 1, 0)).getNormalized();

			PxMat33 m(viewY, viewY.cross(sCamera->mDir), -sCamera->mDir);
			PxTransform tr(sCamera->mEye + sCamera->mDir, PxQuat(m));
			keyPress(key, tr);
		}

	}

	void keyboardUpCallback(unsigned char key, int x, int y)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		sCamera->handleKey(key, false);
	}

	void keyboardSpecialCallback(int key, int x, int y)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		sCamera->handleSpecialKey(key, true);
	}

	void keyboardSpecialUpCallback(int key, int x, int y)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		sCamera->handleSpecialKey(key, false);
	}


	void mouseCallback(int button, int state, int x, int y)
	{
		sCamera->handleMouse(button, state, x, y);
	}

	void idleCallback()
	{
		static int picCounter = 0;
		if (saveScreenToFile)
		{

			char s[20] = "SS\\picxxxx.ppm";
			int i;

			// save screen to file
			s[6] = 48 + (picCounter / 1000);
			s[7] = 48 + (picCounter % 1000) / 100;
			s[8] = 48 + (picCounter % 100) / 10;
			s[9] = 48 + picCounter % 10;

			saveScreenshot((float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT), s);
			// saveScreenToFile=0; // save only once, change this if you want continuos image generation (i.e. animation)
			picCounter++;


			if (picCounter >= 300) // allow only 300 snapshots
			{
				//exit(0);
			}
		}
		glutPostRedisplay();
	}

	void renderCallback()
	{
		sCamera->update();

		Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC | PxActorTypeSelectionFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC | PxActorTypeSelectionFlag::eRIGID_STATIC, (PxActor**)&actors[0], nbActors);
			Snippets::renderActors(&actors[0], PxU32(actors.size()), true, PxVec3(0.88, 0.69, 0.49));
		}

		Snippets::finishRender();

		stepPhysics();
	}

	void exitCallback(void)
	{
		delete sCamera;
		cleanupPhysics();
	}
}

void renderLoop()
{
	// Local to the tower
	sCamera = new Snippets::Camera(PxVec3(10, 5, 5), PxVec3(-0.7, 0, -1));

	Snippets::setupDefaultWindow("PhysX Snippet Serialization");
	Snippets::setupDefaultRenderState();

	glutFullScreen();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);

	glutKeyboardFunc(keyboardCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(keyboardSpecialCallback);
	glutSpecialUpFunc(keyboardSpecialUpCallback);

	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0, 0);

	atexit(exitCallback);

	glutMainLoop();
}
#endif
