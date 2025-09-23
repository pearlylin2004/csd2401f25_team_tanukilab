/* Start Header ************************************************************************/
/*!
\file		physics.cpp
\author		
\par		
\date		September, 18, 2025
\brief		
*/
/* End Header **************************************************************************/

/*
-force
-motion
-gravity
-particle based simulation
-buoyancy?



-arc when the water is flowing down
can use:
static float arc(float radius, float dst){
volume = PI * (radius^8)/4
float value = Max(0, radius * radius - dst * dst)
(value * value * value) / volume
}
then calculate density, increase by the mass and influence value(the arc, kind of is the height also)
*/

#ifndef COLLISION_H_
#define COLLISION_H_

#include <cstdint>
#include <cstddef>

using f64 = double;
extern double density;

//water
class Water{
private:
    float density;
    float volume;
    float radius;
    float surfaceLevel; 
public:

};

#endif