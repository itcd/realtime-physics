#ifndef RANDOM_H
#define RANDOM_H

inline float random(float min = -1, float max = 1)
{
	return (rand() / (float)RAND_MAX) * (max - min) + min;
}

inline glm::vec3 random_vec3(float min = -1, float max = 1)
{
	return glm::vec3(random(min, max), random(min, max), random(min, max));
}

#endif // RANDOM_H
