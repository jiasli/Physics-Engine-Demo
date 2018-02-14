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



#include "SnippetCamera.h"
#include <ctype.h>
#include "foundation/PxMat33.h"

#include "glut.h"

using namespace physx;

namespace Snippets
{

	Camera::Camera(const PxVec3 &eye, const PxVec3& dir)
	{
		mEye = eye;
		mDir = dir.getNormalized();
		mMouseX = 0;
		mMouseY = 0;
	}

	void Camera::handleMouse(int button, int state, int x, int y)
	{
		PX_UNUSED(state);
		PX_UNUSED(button);
		mMouseX = x;
		mMouseY = y;
	}

	bool Camera::handleKey(unsigned char key, bool down)
	{
		if (down)
		{
			switch (key)
			{
			case 'w':	forwardVelocity = moveSpeed;	break;
			case 's':	forwardVelocity = -moveSpeed;	break;
			case 'a':	rightVelocity = -moveSpeed;		break;
			case 'd':	rightVelocity = moveSpeed;		break;
			default:								return false;
			}
		}
		else  // up
		{
			switch (key)
			{
			case 'w':	forwardVelocity = 0;	break;
			case 's':	forwardVelocity = 0;	break;
			case 'a':	rightVelocity = 0;		break;
			case 'd':	rightVelocity = 0;		break;
			default:							return false;
			}
		}
		return true;
	}

	bool Camera::handleSpecialKey(int key, bool down)
	{
		if (down)
		{
			switch (key)
			{
			case GLUT_KEY_UP: upRotateVelocity = rotateSpeed;	break;
			case GLUT_KEY_DOWN: upRotateVelocity = -rotateSpeed;	break;
			case GLUT_KEY_RIGHT: rightRotateVelocity = rotateSpeed;	break;
			case GLUT_KEY_LEFT: rightRotateVelocity = -rotateSpeed;	break;
			default:								return false;
			}
		}
		else  // up
		{
			switch (key)
			{
			case GLUT_KEY_UP: upRotateVelocity = 0;	break;
			case GLUT_KEY_DOWN: upRotateVelocity =0;	break;
			case GLUT_KEY_RIGHT: rightRotateVelocity = 0;	break;
			case GLUT_KEY_LEFT: rightRotateVelocity = 0;	break;
			default:							return false;
			}
		}
		return true;
	}

	void Camera::update()
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		mEye += mDir * forwardVelocity;
		mEye += viewY * rightVelocity;

		PxQuat qx(PxPi * rightRotateVelocity / 180.0f, PxVec3(0, -1, 0));
		mDir = qx.rotate(mDir);
		PxQuat qy(PxPi * upRotateVelocity / 180.0f, viewY);
		mDir = qy.rotate(mDir);

		mDir.normalize();
	}

	void Camera::handleAnalogMove(float x, float y)
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		mEye += mDir*y;
		mEye += viewY*x;
	}

	void Camera::handleMotion(int x, int y)
	{
		PxReal dx = (mMouseX - x) * 0.15f;
		PxReal dy = (mMouseY - y) * 0.15f;

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();

		PxQuat qx(PxPi * dx / 180.0f, PxVec3(0, 1, 0));
		mDir = qx.rotate(mDir);
		PxQuat qy(PxPi * dy / 180.0f, viewY);
		mDir = qy.rotate(mDir);

		mDir.normalize();

		mMouseX = x;
		mMouseY = y;
	}

	PxTransform Camera::getTransform() const
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0));

		if (viewY.normalize() < 1e-6f)
			return PxTransform(mEye);

		PxMat33 m(mDir.cross(viewY), viewY, -mDir);
		return PxTransform(mEye, PxQuat(m));
	}

	PxVec3 Camera::getEye() const
	{
		return mEye;
	}

	PxVec3 Camera::getDir() const
	{
		return mDir;
	}


}

