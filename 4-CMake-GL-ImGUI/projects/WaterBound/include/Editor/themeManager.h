/* Start Header ************************************************************************/
/*!
\file       ui.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 90%
			Pearly Lin Lee Ying, p.lin, 2401591
			- 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 24th, 2025
\brief      Declaration of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem

class ThemeManager {
	void themeSinLeDark();
	void themeSinLeLight();
	void themeForest();
};