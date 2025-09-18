/* Start Header ************************************************************************/
/*!
\file		collision.cpp
\author		
\par		
\date		September, 17, 2025
\brief		This file declares the static and dynamic collision functions, as well as
			the main AABB-AABB collision function implementation.
*/
/* End Header **************************************************************************/

#include "collision.h"
#include <cmath> //can change to include own math library
#include <algorithm>

//all can change to fit our game later
f64 g_fixedDT = 0.016;
f64 g_dt = 0.016;
f64 g_appTime = 0.0;

//AABB
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
{
    const Vec2& min1 = aabb1.getMin();
    const Vec2& max1 = aabb1.getMax();
    const Vec2& min2 = aabb2.getMin();
    const Vec2& max2 = aabb2.getMax();

    if (max1.getX() < min2.getX() || min1.getX() > max2.getX())
        return false;
    if (max1.getY() < min2.getY() || min1.getY() > max2.getY())
        return false;

    return true; 
}
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1, const Vec2& vel1, const AABB& aabb2, const Vec2& vel2, float& firstTimeOfCollision)
{
    float vx = vel2.getX() - vel1.getX();
    float vy = vel2.getY() - vel1.getY();

    const Vec2& min1 = aabb1.getMin();
    const Vec2& max1 = aabb1.getMax();
    const Vec2& min2 = aabb2.getMin();
    const Vec2& max2 = aabb2.getMax();

    float txEntry, txExit, tyEntry, tyExit;

    if (vx > 0.0f) {
        txEntry = (min1.getX() - max2.getX()) / vx;
        txExit  = (max1.getX() - min2.getX()) / vx;
    } else if (vx < 0.0f) {
        txEntry = (max1.getX() - min2.getX()) / vx;
        txExit  = (min1.getX() - max2.getX()) / vx;
    } else {
        txEntry = -INFINITY; //infinity might need to change 
        txExit = INFINITY; //infinity might need to change
    }

    if (vy > 0.0f) {
        tyEntry = (min1.getY() - max2.getY()) / vy;
        tyExit  = (max1.getY() - min2.getY()) / vy;
    } else if (vy < 0.0f) {
        tyEntry = (max1.getY() - min2.getY()) / vy;
        tyExit  = (min1.getY() - max2.getY()) / vy;
    } else {
        tyEntry = -INFINITY; //infinity might need to change
        tyExit = INFINITY; //infinity might need to change
    }

    float entryTime = std::max(txEntry, tyEntry);
    float exitTime = std::min(txExit, tyExit);

    if (entryTime > exitTime || entryTime > 1.0f || entryTime < 0.0f)
        return false;

    firstTimeOfCollision = entryTime;
    return true;
}

//circles
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2)
{
    float dx = c2.getCenter().getX() - c1.getCenter().getX();
    float dy = c2.getCenter().getY() - c1.getCenter().getY();
    float distanceSq = dx*dx + dy*dy;
    float radiusSum = c1.getRadius() + c2.getRadius();
    return distanceSq <= radiusSum * radiusSum;
}
bool CollisionIntersection_CircleCircle_Dynamic(const Circle& c1, const Vec2& vel1, const Circle& c2, const Vec2& vel2, float& firstTimeOfCollision)
{
    Vec2 relVel(vel2.getX() - vel1.getX(), vel2.getY() - vel1.getY());
    float dx = c2.getCenter().getX() - c1.getCenter().getX();
    float dy = c2.getCenter().getY() - c1.getCenter().getY();
    float radiusSum = c1.getRadius() + c2.getRadius();

    float a = relVel.getX()*relVel.getX() + relVel.getY()*relVel.getY();
    float b = 2 * (dx*relVel.getX() + dy*relVel.getY());
    float c = dx*dx + dy*dy - radiusSum*radiusSum;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0.0f || a == 0.0f)
        return false;

    float t = (-b - sqrt(discriminant)) / (2*a);
    if (t < 0.0f || t > 1.0f)
        return false;

    firstTimeOfCollision = t;
    return true;
}


//AABB with circles 
bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb)
{
    float closestX = std::max(aabb.getMin().getX(), std::min(c.getCenter().getX(), aabb.getMax().getX()));
    float closestY = std::max(aabb.getMin().getY(), std::min(c.getCenter().getY(), aabb.getMax().getY()));

    float dx = c.getCenter().getX() - closestX;
    float dy = c.getCenter().getY() - closestY;

    return (dx*dx + dy*dy) <= c.getRadius()*c.getRadius();
}
bool CollisionIntersection_CircleAABB_Dynamic(const Circle& c, const Vec2& velC, const AABB& aabb, const Vec2& velAABB, float& firstTimeOfCollision)
{
    float relVelX = velC.getX() - velAABB.getX();
    float relVelY = velC.getY() - velAABB.getY();

    float minX = aabb.getMin().getX() - c.getRadius();
    float minY = aabb.getMin().getY() - c.getRadius();
    float maxX = aabb.getMax().getX() + c.getRadius();
    float maxY = aabb.getMax().getY() + c.getRadius();

    float cx = c.getCenter().getX();
    float cy = c.getCenter().getY();

    float txEntry, txExit, tyEntry, tyExit;

    if (relVelX > 0.0f) {
        txEntry = minX - cx;
        txExit  = maxX - cx;
    } else if (relVelX < 0.0f) {
        txEntry = maxX - cx;
        txExit  = minX - cx;
    } else {
        txEntry = -INFINITY; //infinity might need to change
        txExit  = INFINITY; //infinity might need to change
    }

    if (relVelY > 0.0f) {
        tyEntry = minY - cy;
        tyExit  = maxY - cy;
    } else if (relVelY < 0.0f) {
        tyEntry = maxY - cy;
        tyExit  = minY - cy;
    } else {
        tyEntry = -1e30f;
        tyExit  = 1e30f;
    }

    if (relVelX != 0.0f) {
        txEntry /= relVelX;
        txExit  /= relVelX;
    }
    if (relVelY != 0.0f) {
        tyEntry /= relVelY;
        tyExit  /= relVelY;
    }

    float entryTime = (txEntry > tyEntry) ? txEntry : tyEntry;
    float exitTime  = (txExit < tyExit) ? txExit : tyExit;

    if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f)
        return false;

    firstTimeOfCollision = entryTime;
    return true;
}

