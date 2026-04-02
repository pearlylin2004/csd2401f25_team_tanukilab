/* Start Header ************************************************************************/
/*!
\file       camera.cpp
\author     Seow Sin Le, s.sinle, 2401084
			- 100% of the file
\par        s.sinle@digipen.edu
\date       March, 8th, 2026
\brief      Used for camera component, which is used to set up the camera's position, zoom, and aspect ratio. The camera system updates the camera's view and projection matrices based on the camera component attached to game objects. If no camera component is present, it sets up a default camera.
Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"

bool Camera::camPresent = false;
int Camera::screenWidth = 0;
int Camera::screenHeight = 0;

void Camera::update(GameObjectManager& manager)
{
	std::vector<GameObject*> objects;
	manager.getAllGameObjects(objects);
	camPresent = false;
	for (GameObject* obj : objects)
	{
		if (obj->hasComponent<camera>())
		{
			const camera* camInfo = obj->getComponent<camera>();
			const Transform* transform = obj->getComponent<Transform>();
			renderer::cam.ar = 2;
			std::cout << "Camera found, Aspect Ratio: " << renderer::cam.ar << "\n";
			if (camInfo->useScreenSize)
			{
				renderer::cam.width = (screenWidth);
				//renderer::cam.ar = static_cast<GLfloat>(screenWidth) / screenHeight;
			}
			else
			{
				renderer::cam.width = camInfo->width;
				//renderer::cam.ar = camInfo->width / camInfo->height;
			}
			renderer::cam.campos = glm::vec3(transform->x, transform->y, transform->z);
			renderer::cam.zoom = camInfo->zoom;
			renderer::cam.minZoom = camInfo->minZoom;
			renderer::cam.maxZoom = camInfo->maxZoom;
			float halfH = camInfo->zoom;
			float halfW = camInfo->zoom * renderer::cam.ar;

			if (camInfo->mapWidth >= halfW)
				renderer::cam.campos.x = glm::clamp(renderer::cam.campos.x,
					-camInfo->mapWidth + halfW, camInfo->mapWidth - halfW);
			else
				renderer::cam.campos.x = 0;

			if (camInfo->mapHeight >= halfH)
				renderer::cam.campos.y = glm::clamp(renderer::cam.campos.y,
					-camInfo->mapHeight + halfH, camInfo->mapHeight - halfH);
			else
				renderer::cam.campos.y = 0;

			camPresent = true;
		}
	}
	if (!camPresent)
		camDefault();
}

void Camera::camDefault()
{
	
	//renderer::cam.ar = static_cast<GLfloat>(screenWidth) / screenHeight;
	renderer::cam.ar = 2;
	std::cout << "No camera component found, using default camera. Aspect Ratio: " << renderer::cam.ar << "\n";
	renderer::cam.width = (screenWidth);
	renderer::cam.rot = 0;
	renderer::cam.zoom = 10;
	renderer::cam.campos = glm::vec3(0, 0, 0);
	renderer::cam.up = glm::vec3(0, 1, 0);
	renderer::cam.zoomFactor = 0.1f;
	renderer::cam.minZoom = 0.1f;
	renderer::cam.maxZoom = 50.f;
}
void Camera::onResize(int w, int h)
{
	screenHeight = h;
	screenWidth = w;
}