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

//all can change to fit our game later
f64 g_fixedDT = 0.016;
f64 g_dt = 0.016;
f64 g_appTime = 0.0;

//AABB
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
{
    const Vector2D& min1 = aabb1.getMin();
    const Vector2D& max1 = aabb1.getMax();
    const Vector2D& min2 = aabb2.getMin();
    const Vector2D& max2 = aabb2.getMax();

    if (max1.x < min2.x || min1.x > max2.x)
        return false;
    if (max1.y < min2.y || min1.y > max2.y)
        return false;

    return true; 
}
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2, float& firstTimeOfCollision)
{
    float vx = vel2.x - vel1.x;
    float vy = vel2.y - vel1.y;

    const Vector2D& min1 = aabb1.getMin();
    const Vector2D& max1 = aabb1.getMax();
    const Vector2D& min2 = aabb2.getMin();
    const Vector2D& max2 = aabb2.getMax();

    float txEntry, txExit, tyEntry, tyExit;

    if (vx > 0.0f) {
        txEntry = (min1.x - max2.x) / vx;
        txExit  = (max1.x - min2.x) / vx;
    } else if (vx < 0.0f) {
        txEntry = (max1.x - min2.x) / vx;
        txExit  = (min1.x - max2.x) / vx;
    } else {
        txEntry = -INFINITY; //infinity might need to change 
        txExit = INFINITY; //infinity might need to change
    }

    if (vy > 0.0f) {
        tyEntry = (min1.y - max2.y) / vy;
        tyExit  = (max1.y - min2.y) / vy;
    } else if (vy < 0.0f) {
        tyEntry = (max1.y - min2.y) / vy;
        tyExit  = (min1.y - max2.y) / vy;
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
    float dx = c2.getCenter().x - c1.getCenter().x;
    float dy = c2.getCenter().y - c1.getCenter().y;
    float distanceSq = dx*dx + dy*dy;
    float radiusSum = c1.getRadius() + c2.getRadius();
    return distanceSq <= radiusSum * radiusSum;
}
bool CollisionIntersection_CircleCircle_Dynamic(const Circle& c1, const Vector2D& vel1, const Circle& c2, const Vector2D& vel2, float& firstTimeOfCollision)
{
    Vector2D relVel(vel2.x - vel1.x, vel2.y - vel1.y);
    float dx = c2.getCenter().x - c1.getCenter().x;
    float dy = c2.getCenter().y - c1.getCenter().y;
    float radiusSum = c1.getRadius() + c2.getRadius();

    float a = relVel.x*relVel.x + relVel.y*relVel.y;
    float b = 2 * (dx*relVel.x + dy*relVel.y);
    float c = dx*dx + dy*dy - radiusSum*radiusSum;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0.0f || a == 0.0f)
        return false;

    float t = (-b - Math_Sqrt(discriminant)) / (2*a);
    if (t < 0.0f || t > 1.0f)
        return false;

    firstTimeOfCollision = t;
    return true;
}


//AABB with circles 
bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb)
{
    float minX = aabb.getMin().x - c.getRadius();
    float maxX = aabb.getMax().x + c.getRadius();
    float minY = aabb.getMin().y - c.getRadius();
    float maxY = aabb.getMax().y + c.getRadius();

    return (c.getCenter().x >= minX && c.getCenter().x <= maxX && c.getCenter().y >= minY && c.getCenter().y <= maxY);
}
bool CollisionIntersection_CircleAABB_Dynamic(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB, float& firstTimeOfCollision)
{
    float relVelX = velC.x - velAABB.x;
    float relVelY = velC.y - velAABB.y;

    float minX = aabb.getMin().x - c.getRadius();
    float minY = aabb.getMin().y - c.getRadius();
    float maxX = aabb.getMax().x + c.getRadius();
    float maxY = aabb.getMax().y + c.getRadius();

    float cx = c.getCenter().x;
    float cy = c.getCenter().y;

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
        tyEntry = -INFINITY; //infinity might need to change
        tyExit  = INFINITY; //infinity might need to change
    }

    if (relVelX != 0.0f) {
        txEntry /= relVelX;
        txExit  /= relVelX;
    }
    if (relVelY != 0.0f) {
        tyEntry /= relVelY;
        tyExit  /= relVelY;
    }

    float entryTime = Math_Max(txEntry, tyEntry);
    float exitTime = Math_Min(txExit, tyExit);

    if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f)
        return false;

    firstTimeOfCollision = entryTime;
    return true;
}

