/* Start Header ************************************************************************/
/*!
\file       event.h
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
#include <string>
#include <memory>

// Basic Event type
class Event {
public:
    virtual ~Event() = default;
    virtual std::string GetName() const = 0;
};
