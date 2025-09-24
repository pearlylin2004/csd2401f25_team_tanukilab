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
#ifndef COLLISION_H
#define COLLISION_H

#include <cstdint>
#include <cstddef>
#include "Math_Lib.h"

namespace Collision {
    using f64 = double;
    extern f64 g_fixedDT;
    extern f64 g_dt;
    extern f64 g_appTime;

    //AABB
    class AABB
    {
    private:
        Vector2D min;
        Vector2D max;
    public:
        //constructor ... can add more in future 
        AABB(const Vector2D& _min, const Vector2D& _max) : min(_min), max(_max) {}

        //getters
        const Vector2D& getMin() const { return min; }
        const Vector2D& getMax() const { return max; }

        //setters
        void setMin(const Vector2D& _min) { min = _min; }
        void setMax(const Vector2D& _max) { max = _max; }

        //for imgui
        Vector2D& getMinRef() { return min; }
        Vector2D& getMaxRef() { return max; }
    };

    //circles
    class Circle {
    private:
        Vector2D center;
        float radius;
    public:
        //constructor .. can add more in future 
        Circle() : center(0, 0), radius(1) {}
        Circle(const Vector2D& _center, float _radius) : center(_center), radius(_radius) {}

        //getters
        const Vector2D& getCenter() const { return center; }
        float getRadius() const { return radius; }

        //setters
        void setCenter(const Vector2D& _center) { center = _center; }
        void setRadius(float _radius) { radius = _radius; }

        //for imgui
        Vector2D& getCenterRef() { return center; }
        float& getRadiusRef() { return radius; }
    };

    //collision function for AABB
    bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);
    bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2, float& firstTimeOfCollision);

    //collision function for circles
    bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2);
    bool CollisionIntersection_CircleCircle_Dynamic(const Circle& c1, const Vector2D& vel1, const Circle& c2, const Vector2D& vel2, float& firstTimeOfCollision);

    //circle and AABB
    bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb);
    bool CollisionIntersection_CircleAABB_Dynamic(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB, float& firstTimeOfCollision);

    //simple collision between 2 AABB d + s
    inline bool CollisionIntersection_RectRect(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2, float& firstTimeOfCollision)
    {
        //static intersection
        if (CollisionIntersection_RectRect_Static(aabb1, aabb2))
            return true;

        //dynamic intersection
        return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
    }

    //simple collision between circles d + s
    inline bool CollisionIntersection_CircleCircle(const Circle& c1, const Vector2D& vel1, const Circle& c2, const Vector2D& vel2, float& firstTimeOfCollision)
    {
        //static collision check
        if (CollisionIntersection_CircleCircle_Static(c1, c2))
            return true;

        //dynamic collision check
        return CollisionIntersection_CircleCircle_Dynamic(c1, vel1, c2, vel2, firstTimeOfCollision);
    }

    inline bool CollisionIntersection_CircleAABB(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB, float& firstTimeOfCollision) {
        if (CollisionIntersection_CircleAABB_Static(c, aabb))
            return true;

        return CollisionIntersection_CircleAABB_Dynamic(c, velC, aabb, velAABB, firstTimeOfCollision);
    }
}
#endif