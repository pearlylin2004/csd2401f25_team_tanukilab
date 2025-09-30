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
#include "collision.h" //for using circles
#include <iostream>
#include <vector>
#include <string>

namespace UI{
    extern std::vector<std::string> debugMessages;
    void drawDemo(float& rotationSpeed, float& angleOfRotation);
    void drawControls(std::vector<Collision::Circle>& circles);
	void drawDebug(std::vector<std::string> debugMessages);
}
#endif