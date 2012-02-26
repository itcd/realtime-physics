#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <vector>
#include <algorithm>

template<class T>
struct Simplex
{
	std::vector<T> points;

	T getA()
	{
		if (points.size() > 0)
		{
			return points[points.size() - 1];
		}
		else
		{
			printf("Error in Simplex.getA()\n");
			return T();
		}
	}

	T getB()
	{
		if (points.size() > 1)
		{
			return points[points.size() - 2];
		}
		else
		{
			printf("Error in Simplex.getB()\n");
			return T();
		}
	}

	T getC()
	{
		if (points.size() > 2)
		{
			return points[points.size() - 3];
		}
		else
		{
			printf("Error in Simplex.getC()\n");
			return T();
		}
	}

	T getD()
	{
		if (points.size() > 3)
		{
			return points[points.size() - 4];
		}
		else
		{
			printf("Error in Simplex.getD()\n");
			return T();
		}
	}

	void remove(T point)
	{
		auto r = std::find(points.begin(), points.end(), point);
		if (r != points.end())
		{
			points.erase(r);
		}
	}

	void add(T point)
	{
		points.push_back(point);
	}

	void clear()
	{
		points.clear();
	}
};

#endif // SIMPLEX_H
