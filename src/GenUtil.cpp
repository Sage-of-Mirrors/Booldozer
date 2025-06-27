#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>

#include <filesystem>
#include <fstream>
#include "GenUtil.hpp"

// Log handlers
std::fstream LGenUtility::Log;


// The following was copied from https://gist.github.com/kilfu0701/e279e35372066ae1832850c438d5611e
std::string LGenUtility::Utf8ToSjis(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "utf8");
    int length = src.extract(0, src.length(), NULL, "shift_jis");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "shift_jis");

    return std::string(result.begin(), result.end() - 1);
}

std::string LGenUtility::SjisToUtf8(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "shift_jis");
    int length = src.extract(0, src.length(), NULL, "utf8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf8");

    return std::string(result.begin(), result.end() - 1);
}

inline float max3(float p0, float p1, float p2){
    return glm::max(glm::max(p0, p1), p2);
}

inline float min3(float p0, float p1, float p2){
    return glm::min(glm::min(p0, p1), p2);
}

bool LGenUtility::TriBoxIntersect(glm::vec3 tri[3], glm::vec3 boxCenter, glm::vec3 boxExtents){
    int X = 0, Y = 1, Z = 2;

    glm::vec3 v0 = tri[0] - boxCenter;
    glm::vec3 v1 = tri[1] - boxCenter;
    glm::vec3 v2 = tri[2] - boxCenter;

    glm::vec3 f0 = tri[1] - tri[0];
    glm::vec3 f1 = tri[2] - tri[1];
    glm::vec3 f2 = tri[0] - tri[2];

    // axis 00
    glm::vec3 a00 = {0, -f0.z, f0.y};
    float p0 = glm::dot(v0, a00);
    float p1 = glm::dot(v1, a00);
    float p2 = glm::dot(v2, a00);
    float r = boxExtents.y * glm::abs(f0.z) + boxExtents.z * glm::abs(f0.y);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 01
    glm::vec3 a01 = {0, -f1.z, f1.y};
    p0 = glm::dot(v0, a01);
    p1 = glm::dot(v1, a01);
    p2 = glm::dot(v2, a01);
    r = boxExtents.y * glm::abs(f1.z) + boxExtents.z * glm::abs(f1.y);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 02
    glm::vec3 a02 = {0, -f2.z, f2.y};
    p0 = glm::dot(v0, a02);
    p1 = glm::dot(v1, a02);
    p2 = glm::dot(v2, a02);
    r = boxExtents.y * glm::abs(f2.z) + boxExtents.z * glm::abs(f2.y);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 10
    glm::vec3 a10 = {f0.z, 0, -f0.x};
    p0 = glm::dot(v0, a10);
    p1 = glm::dot(v1, a10);
    p2 = glm::dot(v2, a10);
    r = boxExtents.x * glm::abs(f0.z) + boxExtents.z * glm::abs(f0.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 11
    glm::vec3 a11 = {f1.z, 0, -f1.x};
    p0 = glm::dot(v0, a11);
    p1 = glm::dot(v1, a11);
    p2 = glm::dot(v2, a11);
    r = boxExtents.x * glm::abs(f1.z) + boxExtents.z * glm::abs(f1.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 12
    glm::vec3 a12 = {f2.z, 0, -f2.x};
    p0 = glm::dot(v0, a12);
    p1 = glm::dot(v1, a12);
    p2 = glm::dot(v2, a12);
    r = boxExtents.x * glm::abs(f2.z) + boxExtents.z * glm::abs(f2.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 20
    glm::vec3 a20 = {-f0.y, f0.x, 0};
    p0 = glm::dot(v0, a20);
    p1 = glm::dot(v1, a20);
    p2 = glm::dot(v2, a20);
    r = boxExtents.x * glm::abs(f0.y) + boxExtents.y * glm::abs(f0.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 21
    glm::vec3 a21 = {-f1.y, f1.x, 0};
    p0 = glm::dot(v0, a21);
    p1 = glm::dot(v1, a21);
    p2 = glm::dot(v2, a21);
    r = boxExtents.x * glm::abs(f1.y) + boxExtents.y * glm::abs(f1.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    // axis 22
    glm::vec3 a22 = {-f2.y, f2.x, 0};
    p0 = glm::dot(v0, a22);
    p1 = glm::dot(v1, a22);
    p2 = glm::dot(v2, a22);
    r = boxExtents.x * glm::abs(f2.y) + boxExtents.y * glm::abs(f2.x);
    if(glm::max(-max3(p0, p1 , p2), min3(p0, p1, p2)) > r){
        return false;
    }

    if(max3(v0.x, v1.x, v2.x) < -boxExtents.x || min3(v0.x, v1.x, v2.x) > boxExtents.x){
        return false;
    }

    if(max3(v0.y, v1.y, v2.y) < -boxExtents.y || min3(v0.y, v1.y, v2.y) > boxExtents.y){
        return false;
    }

    if(max3(v0.z, v1.z, v2.z) < -boxExtents.z || min3(v0.z, v1.z, v2.z) > boxExtents.z){
        return false;
    }

    glm::vec3 planeNormal = glm::cross(f0, f1);
    float planeDistance = glm::dot(planeNormal, tri[0]);

    r = boxExtents.x * glm::abs(planeNormal.x) + boxExtents.y * glm::abs(planeNormal.y) + boxExtents.z * glm::abs(planeNormal.z);

    if(planeDistance > r){
        return false;
    }
    return true;
}
