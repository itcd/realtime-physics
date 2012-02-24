#ifndef GEOMETRY_H
#define GEOMETRY_H

struct Geometry
{
	// a pure virtual function for drawing the object
	virtual void operator()() = 0;

	// get the inertial tensor of the geometry
	virtual glm::mat3 inertial_tensor() = 0;

	// get the radius of its bounding sphere
	virtual float radius() = 0;

	virtual void draw_sphere()
	{
		if (show_bounding_shpere())
		{
			glColor3d(1, 1, 0);
			glutWireSphere(radius(), 16, 16);
			glColor3d(1, 1, 1);
		}
	}

	virtual void draw_AABB()
	{
		if (show_bounding_AABB())
		{
			glColor3d(0, 1, 1);
			glPushMatrix();
			auto r = AABB_size();
			glScalef(r.x, r.y, r.z);
			glutWireCube(2);
			glPopMatrix();
			glColor3d(1, 1, 1);
		}
	}

	virtual std::vector<glm::vec3> vertices_object_space() = 0;

	virtual std::vector<glm::vec3> vertices(glm::quat orientation, glm::vec3 position)
	{
		auto v = vertices_object_space();
		std::vector<glm::vec3> v_oriented;
		for (auto i=v.begin(); i!=v.end(); i++)
		{
			auto p = glm::rotate(orientation, *i) + position;
			v_oriented.push_back(p);
		}
		return v_oriented;
	}

	virtual glm::vec3 AABB_size() = 0;

	virtual float mass()
	{
		return _mass;
	}

	Geometry(float mass = 1)
	{
		this->_mass = mass;
	}

protected:
	float _mass;
};

struct Cube : Geometry
{
	float size;

	Cube(float size = 0.25f)
	{
		this->size = size;
	}

	virtual void operator()()
	{
		glutSolidCube(size);
	}

	virtual glm::mat3 inertial_tensor()
	{
		auto v = mass() * size * size  / 6.f;
		return glm::mat3(v, 0, 0, 0, v, 0, 0, 0, v);
	}

	virtual float radius()
	{
		return size / 2.f * 1.732f;
	}

	virtual glm::vec3 AABB_size()
	{
		return glm::vec3(size / 2, size / 2, size / 2);
	}

	virtual std::vector<glm::vec3> vertices_object_space()
	{
		std::vector<glm::vec3> v;
		float s = size / 2.f;

		v.push_back(glm::vec3(s, -s, s));
		v.push_back(glm::vec3(s, -s, -s));
		v.push_back(glm::vec3(-s, -s, -s));
		v.push_back(glm::vec3(-s, -s, s));

		v.push_back(glm::vec3(s, s, s));
		v.push_back(glm::vec3(s, s, -s));
		v.push_back(glm::vec3(-s, s, -s));
		v.push_back(glm::vec3(-s, s, s));

		return v;
	}
};

struct Octahedron : Geometry
{
	float size;

	Octahedron(float size = 0.25f)
	{
		this->size = size;
	}

	virtual void operator()()
	{
		glPushMatrix();
		glScalef(size, size, size);
		glutSolidOctahedron();
		glPopMatrix();
	}

	virtual glm::mat3 inertial_tensor()
	{
		auto v = mass() * size * size  / 6.f;
		return glm::mat3(v, 0, 0, 0, v, 0, 0, 0, v);
	}

	virtual float radius()
	{
		return size;
	}

	virtual glm::vec3 AABB_size()
	{
		return glm::vec3(size, size, size);
	}

	virtual std::vector<glm::vec3> vertices_object_space()
	{
		std::vector<glm::vec3> v;
		float s = size;

		v.push_back(glm::vec3(s, 0, 0));
		v.push_back(glm::vec3(0, 0, -s));
		v.push_back(glm::vec3(-s, 0, 0));
		v.push_back(glm::vec3(0, 0, s));

		v.push_back(glm::vec3(0, s, 0));
		v.push_back(glm::vec3(0, -s, 0));

		return v;
	}
};

#endif // GEOMETRY_H
