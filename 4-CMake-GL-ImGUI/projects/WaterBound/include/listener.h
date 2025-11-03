/* Start Header ************************************************************************/
/*!
\file       listener.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Novemeber, 3rd, 2025
\brief      

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Event.h"

class Listener {
public:
	virtual ~Listener() = default; //virtual to allow proper cleanup of derived classes
	virtual void OnEvent(const Event& e) = 0; //virtual to allow derived classes to implement their own event handling
};
