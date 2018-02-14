#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "foundation/PxMemory.h"

#include "../SnippetUtils/SnippetUtils.h"
#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"


using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics = NULL;
PxCooking*				gCooking = NULL;
PxMaterial*				gMaterial = NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene = NULL;

PxVisualDebuggerConnection*
gConnection = NULL;

#define MAX_MEMBLOCKS 10
PxU8*					gMemBlocks[MAX_MEMBLOCKS];
PxU32					gMemBlockCount = 0;

void createPyramid(const PxTransform& t, PxU32 size, PxReal halfExtent);
void createPillar(const PxTransform& t, PxU32 size, PxReal halfExtent);
void createDominoCircle(const PxTransform& t);

void deserializeIndependentObjects(PxInputData& inputData)
{
	PxSerializationRegistry* sr = PxSerialization::createSerializationRegistry(*gPhysics);

	PxCollection* collection = PxSerialization::createCollectionFromXml(inputData, *gCooking, *sr);

	gScene->addCollection(*collection);
	collection->release();

	PxMaterial* material;
	gPhysics->getMaterials(&material, 1);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *material);
	gScene->addActor(*groundPlane);
	sr->release();
}


void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	PxProfileZoneManager* profileZoneManager = &PxProfileZoneManager::createProfileZoneManager(gFoundation);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, profileZoneManager);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	PxInitExtensions(*gPhysics);

	if (gPhysics->getPvdConnectionManager())
	{
		gPhysics->getVisualDebugger()->setVisualizeConstraints(true);
		gPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
		gPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
		gConnection = PxVisualDebuggerExt::createConnection(gPhysics->getPvdConnectionManager(), PVD_HOST, 5425, 10);
	}

	PxU32 numWorkers = PxMax(PxI32(SnippetUtils::getNbPhysicalCores()) - 1, 0);
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);

	printf("PxTolerancesScale length=%f mass=%f speed=%f\n", PxTolerancesScale().length, PxTolerancesScale().mass, PxTolerancesScale().speed);
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -9.81f, 0);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	createPyramid(PxTransform(PxVec3(0, 0, -10)), 10, 0.4f);
	createPillar(PxTransform(PxVec3(10, 0, -10)), 40, 0.4f);
	//createDominoCircle(PxTransform(0, 0, 0));
}

void stepPhysics()
{
	gScene->simulate(1.0f / 120.0f);
	gScene->fetchResults(true);
}

/**
Releases all physics objects, including memory blocks containing deserialized data
*/
void cleanupPhysics()
{
	gScene->release();
	gScene = NULL;
	gDispatcher->release();
	gDispatcher = NULL;
	PxCloseExtensions();
	PxProfileZoneManager* profileZoneManager = gPhysics->getProfileZoneManager();
	if (gConnection != NULL)
	{
		gConnection->release();
		gConnection = NULL;
	}

	gPhysics->release();	// releases all objects	
	gPhysics = NULL;
	gCooking->release();
	gCooking = NULL;
	profileZoneManager->release();

	// Now that the objects have been released, it's safe to release the space they occupy
	for (PxU32 i = 0; i < gMemBlockCount; i++)
	{
		free(gMemBlocks[i]);
	}
	gMemBlockCount = 0;

	gFoundation->release();
	gFoundation = NULL;
}

PxRigidDynamic* createBoxDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
{
	static int nbBalls = 0;
	printf("Number of balls: %d\n", ++nbBalls);
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 300.0f);
	dynamic->setLinearVelocity(velocity);
	dynamic->setSleepThreshold(0.02);
	dynamic->setSolverIterationCounts(8, 4);
	gScene->addActor(*dynamic);
	return dynamic;
}

PxRigidDynamic* createBallDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
{
	static int nbBalls = 0;
	printf("Number of balls: %d\n", ++nbBalls);
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 300.0f);
	dynamic->setLinearDamping(1.0f);
	dynamic->setAngularDamping(2.0f);
	dynamic->setLinearVelocity(velocity);
	dynamic->setSleepThreshold(1.0);
	dynamic->setSolverIterationCounts(8, 4);
	gScene->addActor(*dynamic);
	return dynamic;
}

void createPyramid(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	printf("Create stack with ContactOffset=%f, RestOffset=%f\n", shape->getContactOffset(), shape->getRestOffset());
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			for (PxU32 k = 0; k < size - i; k++)
			{
				PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), PxReal(k * 2) - PxReal(size - i)) * halfExtent);
				PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				body->setSleepThreshold(1.0);
				body->setSolverIterationCounts(8, 4);
				gScene->addActor(*body);
			}
		}
	}
	shape->release();
}

void createDominoCircle(const PxTransform& t)
{
	PxMaterial* material = gPhysics->createMaterial(0.01f, 0.01f, 0.1f);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.3, 1.0, 0.1), *material);
	
	PxReal radiusPeriodIncreament = 1.0;
	PxReal stepLength = 1.0;
	PxReal theta = 0;
	PxReal radius = 4;
	PxU32 n = 0;

	while (n < 500)
	{
		PxVec3 translate(radius, 1, 0);
		PxQuat rotateQuat(theta, PxVec3(0, 1, 0));
		rotateQuat.rotate(translate);
		PxTransform localTm(rotateQuat.rotate(translate), rotateQuat);
		PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
		body->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		body->setSolverIterationCounts(8, 4);
		gScene->addActor(*body);

		theta += stepLength / radius;
		//radius += radiusPeriodIncreament * stepLength / (2 * PxPi * radius);
		radius += 0.05;
		//printf("radius=%f, theta=%f\n", radius, theta);
		n++;
	}

}


void createPillar(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent * 2), *gMaterial);
	printf("Create pillar with ContactOffset=%f, RestOffset=%f\n", shape->getContactOffset(), shape->getRestOffset());
	for (PxU32 i = 0; i < size; i++)
	{
		PxTransform localTm(PxVec3(0, PxReal(i * 2 + 1), 0) * halfExtent);
		PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
		body->attachShape(*shape);
		PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		body->setSleepThreshold(1.0);
		body->setSolverIterationCounts(20, 10);
		gScene->addActor(*body);
		body->putToSleep();
	}
	shape->release();
}

void keyPress(const char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
		//case 'B':	createStack(PxTransform(PxVec3(0, 0, -10)), 10, 0.2f);						break;
	case ' ':	createBallDynamic(camera, PxSphereGeometry(0.2f), camera.rotate(PxVec3(0, 0, -1)) * 40);	break;
	}
}

int snippetMain(int, const char*const*)
{
	initPhysics();
	PxDefaultFileInputData inputStream("finalNoOffset.xml");
	deserializeIndependentObjects(inputStream);	

#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 250;
	for (PxU32 i = 0; i < frameCount; i++)
		stepPhysics();
	cleanupPhysics();
	printf("SnippetSerialization done.\n");
#endif

	return 0;
}
