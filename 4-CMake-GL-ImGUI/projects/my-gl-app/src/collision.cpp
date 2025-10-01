/* Start Header ************************************************************************/
/*!
\file       collision.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      This file defines the static and dynamic collision functions, as well as
            the main AABB-AABB collision function implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "collision.h"

#include <iostream>

namespace Collision {
    //all can change to fit our game later
    f64 g_fixedDT = 0.016;
    f64 g_dt = 0.016;
    f64 g_appTime = 0.0;
    const float EPSILON = 1e-5f;

    //helper function to get an AABB from a renderer object
    AABB getObjectAABB(const Transform* transform) {
        float halfWidth = transform->scaleX * 0.5f;
        float halfHeight = transform->scaleY * 0.5f;

        Vector2D center(transform->x, transform->y);
        Vector2D min(center.x - halfWidth, center.y - halfHeight);
        Vector2D max(center.x + halfWidth, center.y + halfHeight);
        return Collision::AABB(min, max);
    }

    Circle getObjectCircle(const Transform* transform) {
        Vector2D center(transform->x, transform->y);
        float radius = std::max(transform->scaleX, transform->scaleY) * 0.5f;
        return Collision::Circle(center, radius);
    }

    //can add to math
    static inline float Dot(const Vector2D& a, const Vector2D& b) { return a.x * b.x + a.y * b.y; }
    static inline float LenSqr(const Vector2D& v) { return v.x * v.x + v.y * v.y; }
    static inline float Length(const Vector2D& v) { return std::sqrt(LenSqr(v)); }

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
    CollisionInfo CollisionIntersection_RectRect_Dynamic_Info(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2)
    {
        CollisionInfo info;
        info.collided = false;
        info.timeOfImpact = static_cast<float>(g_dt);
        info.normal = { 0, 0 };
        info.contactPoint = { 0, 0 };

        // relative velocity
        Vector2D Vb{ vel1.x - vel2.x, vel1.y - vel2.y };

        float tFirst = 0.0f;
        float tLast = g_dt;

        //x
        float x1 = aabb1.getMin().x;
        float x2 = aabb2.getMin().x;
        float dx1 = aabb1.getMax().x - aabb1.getMin().x;
        float dx2 = aabb2.getMax().x - aabb2.getMin().x;
        float tXFirst, tXLast;

        if (Vb.x < 0) {
            tXFirst = (x2 + dx2 - x1) / Vb.x;
            tXLast = (x2 - (x1 + dx1)) / Vb.x;
        }
        else if (Vb.x > 0) {
            tXFirst = (x2 - (x1 + dx1)) / Vb.x;
            tXLast = (x2 + dx2 - x1) / Vb.x;
        }
        else {
            // no relative motion in X → must already overlap
            if (x1 + dx1 <= x2 || x2 + dx2 <= x1) return info;
            tXFirst = 0.0f;
            tXLast = g_dt;
        }

        if (tXFirst > tXLast || tXLast < 0.0f || tXFirst > g_dt)
            return info;

        tFirst = std::max(tFirst, tXFirst);
        tLast = std::min(tLast, tXLast);

        //y
        float y1 = aabb1.getMin().y;
        float y2 = aabb2.getMin().y;
        float dy1 = aabb1.getMax().y - aabb1.getMin().y;
        float dy2 = aabb2.getMax().y - aabb2.getMin().y;
        float tYFirst, tYLast;

        if (Vb.y < 0) {
            tYFirst = (y2 + dy2 - y1) / Vb.y;
            tYLast = (y2 - (y1 + dy1)) / Vb.y;
        }
        else if (Vb.y > 0) {
            tYFirst = (y2 - (y1 + dy1)) / Vb.y;
            tYLast = (y2 + dy2 - y1) / Vb.y;
        }
        else {
            //no relative motion in Y must already overlap
            if (y1 + dy1 <= y2 || y2 + dy2 <= y1) return info;
            tYFirst = 0.0f;
            tYLast = g_dt;
        }

        if (tYFirst > tYLast || tYLast < 0.0f || tYFirst > g_dt)
            return info;

        tFirst = std::max(tFirst, tYFirst);
        tLast = std::min(tLast, tYLast);

        //final check
        if (tFirst < tLast && tFirst >= 0.0f && tFirst <= g_dt) {
            info.collided = true;
            info.timeOfImpact = tFirst;

            //determine collision normal based on which axis constrained first
            if (tXFirst > tYFirst) {
                info.normal = (Vb.x > 0) ? Vector2D{ -1, 0 } : Vector2D{ 1, 0 };
            }
            else {
                info.normal = (Vb.y > 0) ? Vector2D{ 0, -1 } : Vector2D{ 0, 1 };
            }

            //compute contact point
            Vector2D cp = aabb1.getMin();
            cp.x += vel1.x * tFirst;
            cp.y += vel1.y * tFirst;

            if (info.normal.x != 0) {
                cp.x = (info.normal.x > 0) ? aabb2.getMax().x : aabb2.getMin().x;
                cp.y += (vel1.y - vel2.y) * tFirst;
            }
            else if (info.normal.y != 0) {
                cp.y = (info.normal.y > 0) ? aabb2.getMax().y : aabb2.getMin().y;
                cp.x += (vel1.x - vel2.x) * tFirst;
            }

            info.contactPoint = cp;
        }

        return info;
    }

    //circles
    bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2)
    {
        float dx = c2.getCenter().x - c1.getCenter().x;
        float dy = c2.getCenter().y - c1.getCenter().y;
        float distanceSq = dx * dx + dy * dy;
        float radiusSum = c1.getRadius() + c2.getRadius();
        return distanceSq <= (radiusSum * radiusSum + EPSILON);
    }
    CollisionInfo CollisionIntersection_CircleCircle_Dynamic_Info(const Circle& c1, const Vector2D& vel1, const Circle& c2, const Vector2D& vel2)
    {
        CollisionInfo info;
        Vector2D relVel = { vel2.x - vel1.x, vel2.y - vel1.y };
        Vector2D relPos = { c2.getCenter().x - c1.getCenter().x, c2.getCenter().y - c1.getCenter().y };
        float r = c1.getRadius() + c2.getRadius();

        float a = Dot(relVel, relVel);
        float b = 2.0f * Dot(relPos, relVel);
        float c = Dot(relPos, relPos) - r * r;

        if (a <= EPSILON) {
            //relative velocity almost zero
            if (c <= 0.0f) {
                info.collided = true;
                info.timeOfImpact = 0.0f;
                Vector2D n = Vec_Normalize(&relPos);
                info.normal = (Length(relPos) > EPSILON) ? n : Vector2D{ 1,0 };
            }
            return info;
        }

        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) {
            info.collided = false;
            return info;
        }

        float sqrtD = std::sqrt(disc);
        float t1 = (-b - sqrtD) / (2.0f * a);
        float t2 = (-b + sqrtD) / (2.0f * a);

        float toi = INFINITY;
        if (t1 >= 0.0f && t1 <= 1.0f) toi = t1;
        else if (t2 >= 0.0f && t2 <= 1.0f) toi = t2;

        if (toi == INFINITY) {
            info.collided = false;
            return info;
        }

        //normal at contact
        Vector2D scaledVel = Vec_Scale(&relVel, toi);
        Vector2D contactPosRel = Vec_Add(&relPos, &scaledVel);
        Vector2D normal = Vec_Normalize(&contactPosRel);
        if (Length(normal) <= EPSILON) normal = Vector2D{ 1, 0 };

        info.collided = true;
        info.timeOfImpact = toi;
        info.normal = normal;
        return info;
    }

    //AABB with circles 
    bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb)
    {
        const Vector2D& center = c.getCenter();
        float radius = c.getRadius();

        //find closest point on AABB to circle center
        float closestX = Math_Max(aabb.getMin().x, Math_Min(center.x, aabb.getMax().x));
        float closestY = Math_Max(aabb.getMin().y, Math_Min(center.y, aabb.getMax().y));

        //distance between center to closest point
        float dx = center.x - closestX;
        float dy = center.y - closestY;

        return (dx * dx + dy * dy) <= (radius * radius + EPSILON);
    }
    CollisionInfo CollisionIntersection_CircleAABB_Dynamic_Info(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB)
    {
        CollisionInfo info;
        Vector2D relVel = { velC.x - velAABB.x, velC.y - velAABB.y };

        float closestX = Math_Max(aabb.getMin().x, Math_Min(c.getCenter().x, aabb.getMax().x));
        float closestY = Math_Max(aabb.getMin().y, Math_Min(c.getCenter().y, aabb.getMax().y));
        float dx = c.getCenter().x - closestX;
        float dy = c.getCenter().y - closestY;
        float distSq = dx * dx + dy * dy;
        if (distSq <= c.getRadius() * c.getRadius() + EPSILON) {
            info.collided = true;
            info.timeOfImpact = 0.0f;
            Vector2D temp = { dx, dy };
            Vector2D n = Vec_Normalize(&temp);
            if (Length(n) <= EPSILON) {
                //if center inside pick nearest axis normal by distance to edges
                float left = std::abs(c.getCenter().x - aabb.getMin().x);
                float right = std::abs(aabb.getMax().x - c.getCenter().x);
                float bottom = std::abs(c.getCenter().y - aabb.getMin().y);
                float top = std::abs(aabb.getMax().y - c.getCenter().y);
                float minv = std::min(std::min(left, right), std::min(bottom, top));
                if (minv == left) n = Vector2D{ -1,0 };
                else if (minv == right) n = Vector2D{ 1,0 };
                else if (minv == bottom) n = Vector2D{ 0,-1 };
                else n = Vector2D{ 0,1 };
            }
            info.normal = n;
            return info;
        }

        float minX = aabb.getMin().x - c.getRadius();
        float maxX = aabb.getMax().x + c.getRadius();
        float minY = aabb.getMin().y - c.getRadius();
        float maxY = aabb.getMax().y + c.getRadius();

        float txEntry, txExit, tyEntry, tyExit;
        if (relVel.x > 0.0f) {
            txEntry = (minX - c.getCenter().x) / relVel.x;
            txExit = (maxX - c.getCenter().x) / relVel.x;
        }
        else if (relVel.x < 0.0f) {
            txEntry = (maxX - c.getCenter().x) / relVel.x;
            txExit = (minX - c.getCenter().x) / relVel.x;
        }
        else {
            txEntry = -INFINITY; txExit = INFINITY;
        }

        if (relVel.y > 0.0f) {
            tyEntry = (minY - c.getCenter().y) / relVel.y;
            tyExit = (maxY - c.getCenter().y) / relVel.y;
        }
        else if (relVel.y < 0.0f) {
            tyEntry = (maxY - c.getCenter().y) / relVel.y;
            tyExit = (minY - c.getCenter().y) / relVel.y;
        }
        else {
            tyEntry = -INFINITY; tyExit = INFINITY;
        }

        float entryTime = std::max(txEntry, tyEntry);
        float exitTime = std::min(txExit, tyExit);

        if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f) {
            info.collided = false;
            return info;
        }

        Vector2D normal{ 0,0 };
        if (txEntry > tyEntry) {
            //x axis
            normal = (relVel.x > 0.0f) ? Vector2D{ -1,0 } : Vector2D{ 1,0 };
        }
        else {
            normal = (relVel.y > 0.0f) ? Vector2D{ 0,-1 } : Vector2D{ 0,1 };
        }

        info.collided = true;
        info.timeOfImpact = entryTime;
        info.normal = normal;
        return info;
    }

    //relecting
    static inline Vector2D Reflect(const Vector2D& v, const Vector2D& n) {
        float d = Dot(v, n);
        Vector2D scaledN = Vec_Scale(&n, 2.0f * d);
        return Vec_Sub(&v, &scaledN);
    }

    //remove normal 
    static inline Vector2D RemoveNormalComponent(const Vector2D& v, const Vector2D& n) {
        float d = Dot(v, n);
        Vector2D scaledN = Vec_Scale(&n, d);
        return Vec_Sub(&v, &scaledN);
    }
    void Resolve_AABB_AABB(Collision::AABB& aabbA, Vector2D& velA, float massA,
        Collision::AABB& aabbB, Vector2D& velB, float massB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        Vector2D moveA = Vec_Scale(&velA, info.timeOfImpact);
        Vector2D moveB = Vec_Scale(&velB, info.timeOfImpact);

        aabbA.setMin(Vec_Add(&aabbA.getMin(), &moveA));
        aabbA.setMax(Vec_Add(&aabbA.getMax(), &moveA));
        aabbB.setMin(Vec_Add(&aabbB.getMin(), &moveB));
        aabbB.setMax(Vec_Add(&aabbB.getMax(), &moveB));

        if (mode == CollisionResponseMode::Stop) {
            velA = Vector2D{ 0,0 };
            velB = Vector2D{ 0,0 };
            return;
        }
        else {
            //testing for broken collision 
            Vector2D relVel = Vec_Sub(&velB, &velA);
            float velAlongNormal = Dot(relVel, info.normal);
            if (velAlongNormal > 0) return; // moving apart
            float j = -(1.0f + restitution) * velAlongNormal / (1.0f / massA + 1.0f / massB);
            Vector2D impulse = Vec_Scale(&info.normal, j);
            Vector2D scaledImpulseA = Vec_Scale(&impulse, 1.0f / massA);
            Vector2D scaledImpulseB = Vec_Scale(&impulse, 1.0f / massB);
            if (massA > 0) {
                Vector2D tempA = Vec_Sub(&velA, &scaledImpulseA);
                velA = tempA;
            }
            if (massB > 0) {
                Vector2D tempB = Vec_Add(&velB, &scaledImpulseB);
                velB = tempB;
            }
        }

        Vector2D n = info.normal;
        Vector2D relVel = Vec_Sub(&velB, &velA);
        float velAlongNormal = Dot(relVel, n);
    }

    void Resolve_Circle_Circle(Collision::Circle& cA, Vector2D& velA, float massA,
        Collision::Circle& cB, Vector2D& velB, float massB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        //move to contact
        Vector2D deltaA = Vec_Scale(&velA, info.timeOfImpact);
        Vector2D deltaB = Vec_Scale(&velB, info.timeOfImpact);
        cA.setCenter(Vec_Add(&cA.getCenter(), &deltaA));
        cB.setCenter(Vec_Add(&cB.getCenter(), &deltaB));

        Vector2D n = info.normal;
        Vector2D relVel = Vec_Sub(&velB, &velA);
        float velAlongNormal = Dot(relVel, n);

        if (mode == CollisionResponseMode::Stop) {
            velA = Vector2D{ 0,0 };
            velB = Vector2D{ 0,0 };
            return;
        }
    }

    void Resolve_Circle_AABB(Collision::Circle& c, Vector2D& velC, float massC,
        Collision::AABB& aabb, Vector2D& velAABB, float massAABB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        //move to contact
        Vector2D deltaC = Vec_Scale(&velC, info.timeOfImpact);
        Vector2D deltaA = Vec_Scale(&velAABB, info.timeOfImpact);
        c.setCenter(Vec_Add(&c.getCenter(), &deltaC));
        aabb.setMin(Vec_Add(&aabb.getMin(), &deltaA));
        aabb.setMax(Vec_Add(&aabb.getMax(), &deltaA));

        Vector2D n = info.normal;
        //relative velocity: aabb treated as B
        Vector2D relVel = Vec_Sub(&velAABB, &velC);
        float velAlongNormal = Dot(relVel, n);

        if (mode == CollisionResponseMode::Stop) {
            if (massAABB <= 0.0f) velC = Vector2D{ 0,0 };
            else {
                velC = Vector2D{ 0,0 };
                velAABB = Vector2D{ 0,0 };
            }
            return;
        }
    }
}