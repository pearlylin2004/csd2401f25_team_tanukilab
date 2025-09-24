/* Start Header ************************************************************************/
/*!
\file		ui.h
\author
\par
\date		September, 24, 2025
\brief
*/
/* End Header **************************************************************************/

#ifndef UI_H
#define UI_H
#include <imgui.h>
#include <iostream>
#include <vector>

namespace UI{
    void drawDemo(float& rotationSpeed, float& angleOfRotation);
    void drawControls();
	void drawDebug(std::string messages);
}
#endif