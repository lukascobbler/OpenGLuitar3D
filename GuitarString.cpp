#include "GuitarString.h"
#include <glm/glm.hpp>

void GuitarString::computeFretMiddles()
{
    fretMiddles.clear();
    fretMiddles.reserve(21);

    glm::vec3 bridge(x0, y0, z0); // smaller y
    glm::vec3 nut(x1, y1, z1);    // bigger y

    fretMiddles.push_back({ -1.0f, nut.x, nut.y - 0.002f, nut.z + 0.0009f });

    glm::vec3 prevPos = nut;

    for (int n = 0; n <= 20; n++)
    {
        float f = 1.0f - 1.0f / std::pow(2.0f, n / 12.3f);

        glm::vec3 pos = nut + f * (bridge - nut);

        glm::vec3 mid = (prevPos + pos) * 0.5f;

        if (n > 0) fretMiddles.push_back({ (float)(n - 1), mid.x, mid.y - 0.0025f, mid.z + 0.0009f });

        prevPos = pos;
    }
}
