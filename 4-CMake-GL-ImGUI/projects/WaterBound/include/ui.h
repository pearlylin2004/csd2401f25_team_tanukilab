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
#include <unordered_map>

namespace UI{
	extern std::vector<std::string> debugMessages;
    extern float xAdd, yAdd; //temp

    enum class ShapeType { 
        Leaves, 
        Bullet 
    };

	struct Range {
		float min;
		float max;
	};

    struct ShapeDefaults {
		float posX;
		float posY;
		float rotation;
		float scaleX;
		float scaleY;
		float scaleZ;

		// once json done, auto use this if not overriden by json
		// consider having like some reusable RANGE for certain game obj?
		// OPTIONAL slider ranges (min, max)
		Range posXRange = { -15.f, 15.f };
		Range posYRange = { -10.f, 10.f };
		Range rotRange = { 0.f, 360.f };
		Range scaleXRange = { 0.1f, 1.f };
		Range scaleYRange = { 0.1f, 1.f };
	};

	extern std::unordered_map<ShapeType, ShapeDefaults> shapePresets;

	// draw UI windows
    void drawPerformance();
    void drawControls(std::vector<Collision::Circle>& circles);
	void drawDebug();

	// edit shape windows
    void shapeControl(Transform& t1, Transform& t2);
    //void dockingSetUp();
}
#endif