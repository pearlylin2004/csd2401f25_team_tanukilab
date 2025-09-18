/* Start Header ************************************************************************/
/*!
\file		collision.hpp
\author		
\par		
\date		September, 17, 2025
\brief		This file declares the static and dynamic collision functions, as well as
			the main AABB-AABB collision function implementation.
*/
/* End Header **************************************************************************/
#ifndef COLLISION_HPP_
#define COLLISION_HPP_

#include <cstdint>
#include <cstddef>

using f64 = double;
extern f64 g_fixedDT;
extern f64 g_dt;
extern f64 g_appTime;

//vector
class Vec2
{
private:
    float x;
    float y;
public:
    //constructors... can add more in future
    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    //getters
    float getX() const { return x; }
    float getY() const { return y; }

    //setters 
    void setX(float _x) { x = _x; }
    void setY(float _y) { y = _y; }
};

//AABB
class AABB
{
private:
    Vec2 min;
    Vec2 max;
public:
    //constructor ... can add more in future 
    AABB(const Vec2& _min, const Vec2& _max) : min(_min), max(_max) {}

    //getters
    const Vec2& getMin() const { return min; }
    const Vec2& getMax() const { return max; }

    //setters
    void setMin(const Vec2& _min) { min = _min; }
    void setMax(const Vec2& _max) { max = _max; }
};

//circles
class Circle{
private:
    Vec2 center;
    float radius;
public:
    //constructor .. can add more in future 
    Circle() : center(0, 0), radius(1) {}
    Circle(const Vec2& _center, float _radius) : center(_center), radius(_radius) {}

    //getters
    const Vec2& getCenter() const { return center; }
    float getRadius() const { return radius; }

    //setters
    void setCenter(const Vec2& _center) { center = _center; }
    void setRadius(float _radius) { radius = _radius; }
};

//collision function for AABB
bool CollisionIntersection_RectRect_Static(const AABB& aabb1,const AABB& aabb2);      
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1, const Vec2& vel1, const AABB& aabb2, const Vec2& vel2, float& firstTimeOfCollision);

//collision function for circles
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2);
bool CollisionIntersection_CircleCircle_Dynamic(const Circle& c1, const Vec2& vel1, const Circle& c2, const Vec2& vel2, float& firstTimeOfCollision);

//circle and AABB
bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb);
bool CollisionIntersection_CircleAABB_Dynamic(const Circle& c, const Vec2& velC, const AABB& aabb, const Vec2& velAABB, float& firstTimeOfCollision);

//simple collision between 2 AABB d + s
inline bool CollisionIntersection_RectRect(const AABB& aabb1, const Vec2& vel1, const AABB& aabb2, const Vec2& vel2, float& firstTimeOfCollision) 
{
    //static intersection
    if (CollisionIntersection_RectRect_Static(aabb1, aabb2))
        return true;

    //dynamic intersection
    return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}

//simple collision between circles d + s
inline bool CollisionIntersection_CircleCircle(const Circle& c1, const Vec2& vel1, const Circle& c2, const Vec2& vel2,float& firstTimeOfCollision)
{
    //static collision check
    if (CollisionIntersection_CircleCircle_Static(c1, c2))
        return true;

    //dynamic collision check
    return CollisionIntersection_CircleCircle_Dynamic(c1, vel1, c2, vel2, firstTimeOfCollision);
}

inline bool CollisionIntersection_CircleAABB(const Circle& c, const Vec2& velC,const AABB& aabb, const Vec2& velAABB,float& firstTimeOfCollision){
    if (CollisionIntersection_CircleAABB_Static(c, aabb))
        return true;

    return CollisionIntersection_CircleAABB_Dynamic(c, velC, aabb, velAABB, firstTimeOfCollision);
}

#endif