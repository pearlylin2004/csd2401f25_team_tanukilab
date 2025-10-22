/* Start Header ************************************************************************/
/*!
\file       input.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 24th, 2025
\brief      Declaration of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
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
    enum class ShapeType { Leaves, Bullet };
    extern float xAdd, yAdd; //temp

    void drawPerformance();
    void drawControls(std::vector<Collision::Circle>& circles);
	void drawDebug();
    void shapeControl(Transform& t1, Transform& t2);
    //void dockingSetUp();
}
#endif