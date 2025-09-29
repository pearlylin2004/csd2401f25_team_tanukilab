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
#include <iostream>

namespace Collision {
    //all can change to fit our game later
    f64 g_fixedDT = 0.016;
    f64 g_dt = 0.016;
    f64 g_appTime = 0.0;
    const float EPSILON = 1e-5f;

    //can add to math
    static inline float Dot(const Vector2D& a, const Vector2D& b) { return a.x * b.x + a.y * b.y; }
    static inline float LenSqr(const Vector2D& v) { return v.x * v.x + v.y * v.y; }
    static inline float Length(const Vector2D& v) { return std::sqrt(LenSqr(v)); }
    static inline Vector2D Mul(const Vector2D& v, float s) { return Vector2D{ v.x * s, v.y * s }; }
    static inline Vector2D SubV(const Vector2D& a, const Vector2D& b) { return Vector2D{ a.x - b.x, a.y - b.y }; }
    static inline Vector2D AddV(const Vector2D& a, const Vector2D& b) { return Vector2D{ a.x + b.x, a.y + b.y }; }
    static inline Vector2D NormalizeSafe(const Vector2D& v) {
        float L = Length(v);
        if (L <= EPSILON) return Vector2D{ 0,0 };
        return Vector2D{ v.x / L, v.y / L };
    }

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
        Vector2D relVel = { vel2.x - vel1.x, vel2.y - vel1.y };

        const Vector2D& min1 = aabb1.getMin();
        const Vector2D& max1 = aabb1.getMax();
        const Vector2D& min2 = aabb2.getMin();
        const Vector2D& max2 = aabb2.getMax();

        float txEntry, txExit, tyEntry, tyExit;

        if (relVel.x > 0.0f) {
            txEntry = (min1.x - max2.x) / relVel.x;
            txExit = (max1.x - min2.x) / relVel.x;
        }
        else if (relVel.x < 0.0f) {
            txEntry = (max1.x - min2.x) / relVel.x;
            txExit = (min1.x - max2.x) / relVel.x;
        }
        else {
            txEntry = -INFINITY;
            txExit = INFINITY;
        }

        if (relVel.y > 0.0f) {
            tyEntry = (min1.y - max2.y) / relVel.y;
            tyExit = (max1.y - min2.y) / relVel.y;
        }
        else if (relVel.y < 0.0f) {
            tyEntry = (max1.y - min2.y) / relVel.y;
            tyExit = (min1.y - max2.y) / relVel.y;
        }
        else {
            tyEntry = -INFINITY;
            tyExit = INFINITY;
        }

        float entryTime = std::max(txEntry, tyEntry);
        float exitTime = std::min(txExit, tyExit);

        if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f) {
            info.collided = false;
            return info;
        }

        Vector2D normal{ 0,0 };
        if (txEntry > tyEntry) {
            //collision on x axis
            if (relVel.x > 0.0f) normal = Vector2D{ -1, 0 }; //B moving right relative to A contact normal points left
            else normal = Vector2D{ 1, 0 };
        }
        else {
            //collision on y axis
            if (relVel.y > 0.0f) normal = Vector2D{ 0, -1 };
            else normal = Vector2D{ 0, 1 };
        }

        info.collided = true;
        info.timeOfImpact = entryTime;
        info.normal = normal;
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
                Vector2D n = NormalizeSafe(relPos);
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
        Vector2D contactPosRel = AddV(relPos, Mul(relVel, toi));
        Vector2D normal = NormalizeSafe(contactPosRel);
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
            Vector2D n = NormalizeSafe(Vector2D{ dx, dy });
            if (Length(n) <= EPSILON) {
                // if center inside, pick nearest axis normal by distance to edges
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
        return SubV(v, Mul(n, 2.0f * d));
    }

    //remove normal 
    static inline Vector2D RemoveNormalComponent(const Vector2D& v, const Vector2D& n) {
        float d = Dot(v, n);
        return SubV(v, Mul(n, d));
    }
    void Resolve_AABB_AABB(Collision::AABB& aabbA, Vector2D& velA, float massA,
        Collision::AABB& aabbB, Vector2D& velB, float massB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        // Move objects to time of impact (translate by velocity * toi)
        // Note: AABB stores min/max; move both min/max by delta
        Vector2D moveA = Mul(velA, info.timeOfImpact);
        Vector2D moveB = Mul(velB, info.timeOfImpact);
        aabbA.setMin(AddV(aabbA.getMin(), moveA)); aabbA.setMax(AddV(aabbA.getMax(), moveA));
        aabbB.setMin(AddV(aabbB.getMin(), moveB)); aabbB.setMax(AddV(aabbB.getMax(), moveB));

        // For dynamic-dynamic use impulse-like approach if mode != Stop and masses provided
        if (mode == CollisionResponseMode::Stop) {
            velA = Vector2D{ 0,0 };
            velB = Vector2D{ 0,0 };
            return;
        }

        // Compute relative velocity along normal
        Vector2D n = info.normal;
        Vector2D relVel = SubV(velB, velA);
        float velAlongNormal = Dot(relVel, n);

        if (mode == CollisionResponseMode::Bounce) {
            // if moving apart do nothing
            if (velAlongNormal > 0.0f) return;

            // compute impulse scalar for perfectly elastic-ish collision:
            // j = -(1 + e) * (v_rel·n) / (1/m1 + 1/m2)
            float invMassA = (massA > 0.0f) ? (1.0f / massA) : 0.0f;
            float invMassB = (massB > 0.0f) ? (1.0f / massB) : 0.0f;
            float j = -(1.0f + restitution) * velAlongNormal / (invMassA + invMassB + EPSILON);

            Vector2D impulse = Mul(n, j);
            // apply impulse
            velA = SubV(velA, Mul(impulse, invMassA));
            velB = AddV(velB, Mul(impulse, invMassB));
            return;
        }

        if (mode == CollisionResponseMode::Slide) {
            // remove normal component from velocity for the dynamic object(s)
            // For dynamic vs static (massB==0) stop/resist B accordingly
            // Apply to A and B both: project velocities onto tangent
            Vector2D tangentA = RemoveNormalComponent(velA, n);
            Vector2D tangentB = RemoveNormalComponent(velB, n);
            velA = tangentA;
            velB = tangentB;
            return;
        }
    }

    void Resolve_Circle_Circle(Collision::Circle& cA, Vector2D& velA, float massA,
        Collision::Circle& cB, Vector2D& velB, float massB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        // move to contact
        Vector2D deltaA = Mul(velA, info.timeOfImpact);
        Vector2D deltaB = Mul(velB, info.timeOfImpact);
        cA.setCenter(AddV(cA.getCenter(), deltaA));
        cB.setCenter(AddV(cB.getCenter(), deltaB));

        Vector2D n = info.normal;
        Vector2D relVel = SubV(velB, velA);
        float velAlongNormal = Dot(relVel, n);

        if (mode == CollisionResponseMode::Stop) {
            velA = Vector2D{ 0,0 };
            velB = Vector2D{ 0,0 };
            return;
        }

        if (mode == CollisionResponseMode::Bounce) {
            if (velAlongNormal > 0.0f) return;
            float invMassA = (massA > 0.0f) ? 1.0f / massA : 0.0f;
            float invMassB = (massB > 0.0f) ? 1.0f / massB : 0.0f;
            float j = -(1.0f + restitution) * velAlongNormal / (invMassA + invMassB + EPSILON);
            Vector2D impulse = Mul(n, j);
            velA = SubV(velA, Mul(impulse, invMassA));
            velB = AddV(velB, Mul(impulse, invMassB));
            return;
        }

        if (mode == CollisionResponseMode::Slide) {
            velA = RemoveNormalComponent(velA, n);
            velB = RemoveNormalComponent(velB, n);
            return;
        }
    }

    void Resolve_Circle_AABB(Collision::Circle& c, Vector2D& velC, float massC,
        Collision::AABB& aabb, Vector2D& velAABB, float massAABB,
        const CollisionInfo& info, CollisionResponseMode mode, float restitution)
    {
        if (!info.collided) return;

        // move to contact
        Vector2D deltaC = Mul(velC, info.timeOfImpact);
        Vector2D deltaA = Mul(velAABB, info.timeOfImpact);
        c.setCenter(AddV(c.getCenter(), deltaC));
        aabb.setMin(AddV(aabb.getMin(), deltaA));
        aabb.setMax(AddV(aabb.getMax(), deltaA));

        Vector2D n = info.normal;
        // relative velocity: aabb treated as B
        Vector2D relVel = SubV(velAABB, velC);
        float velAlongNormal = Dot(relVel, n);

        if (mode == CollisionResponseMode::Stop) {
            // if static AABB (massAABB==0) stop circle
            if (massAABB <= 0.0f) velC = Vector2D{ 0,0 };
            else {
                velC = Vector2D{ 0,0 };
                velAABB = Vector2D{ 0,0 };
            }
            return;
        }

        if (mode == CollisionResponseMode::Bounce) {
            // if moving away, nothing
            if (velAlongNormal > 0.0f) return;
            float invMassC = (massC > 0.0f) ? 1.0f / massC : 0.0f;
            float invMassA = (massAABB > 0.0f) ? 1.0f / massAABB : 0.0f;
            float j = -(1.0f + restitution) * velAlongNormal / (invMassC + invMassA + EPSILON);
            Vector2D impulse = Mul(n, j);
            velC = SubV(velC, Mul(impulse, invMassC));
            velAABB = AddV(velAABB, Mul(impulse, invMassA));
            return;
        }

        if (mode == CollisionResponseMode::Slide) {
            velC = RemoveNormalComponent(velC, n);
            if (massAABB > 0.0f) velAABB = RemoveNormalComponent(velAABB, n);
            return;
        }
    }
}