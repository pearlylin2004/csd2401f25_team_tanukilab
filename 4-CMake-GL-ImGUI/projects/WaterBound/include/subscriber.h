/* Start Header ************************************************************************/
/*!
\file       subscriber.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       November, 4th, 2025
\brief

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "message.h"

class subscriber {
public:
    virtual ~subscriber() = default;
    virtual void onNotify(const Message& msg) = 0;
};
