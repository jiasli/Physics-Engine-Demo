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


#ifndef PHYSX_SNIPPET_CAMERA_H
#define PHYSX_SNIPPET_CAMERA_H

#include "foundation/PxTransform.h"

namespace Snippets
{
	class Camera
	{
	public:
		Camera(const physx::PxVec3 &eye, const physx::PxVec3& dir);

		void				handleMouse(int button, int state, int x, int y);
		bool				handleKey(unsigned char key, bool down);
		bool				handleSpecialKey(int key, bool down);

		void update();
		void				handleMotion(int x, int y);
		void				handleAnalogMove(float x, float y);

		physx::PxVec3		getEye()	const;
		physx::PxVec3		getDir()	const;
		physx::PxTransform	getTransform() const;

		physx::PxVec3	mEye;
		physx::PxVec3	mDir;
		int				mMouseX;
		int				mMouseY;

		// Continuous movement.
		float moveSpeed = 0.1;
		float forwardVelocity = 0;
		float rightVelocity = 0;

		float rotateSpeed = 0.3;
		float rightRotateVelocity = 0;
		float upRotateVelocity = 0;
	};


}


#endif //PHYSX_SNIPPET_CAMERA_H