#pragma once

#include <array>
#include <functional>
#include <span>
#include <tuple>

#include "Shapes/Shape.h"
#include "Shapes/Circle.h"
#include "Shapes/Box.h"
#include "Shapes/Polygon.h"
#include "Shapes/Point.h"

#include "CollisionArbiter.h"

namespace vlx
{
	class CollisionTable
	{
	public:
		using Matrix = std::array<std::function<void(
			CollisionArbiter&, const Shape&, const Shape&)>, Shape::Count * Shape::Count>;

		using Face = std::array<Vector2f, 2>;
		using VectorSpan = std::span<const Vector2f>;

	public:
		static void Collide(CollisionArbiter&, 
			const Shape& s1, typename Shape::Type st1, 
			const Shape& s2, typename Shape::Type st2);

	private:
		/// <summary>
		///		
		///	 ___Collision Table___
		///		 _____________
		///		|	C	B	P	X 
		///		|C	CC	CB	CP	CX
		///		|B	BC	BB	BP	BX
		///		|P	PC	PB	PP	PX
		///		|X	XC	XB	XP	XX
		///   
		/// </summary>

		static void CircleToCircle	(CollisionArbiter&, const Shape&, const Shape&);
		static void CircleToBox		(CollisionArbiter&, const Shape&, const Shape&);
		static void CircleToPoint	(CollisionArbiter&, const Shape&, const Shape&);
		static void CircleToConvex	(CollisionArbiter&, const Shape&, const Shape&);

		static void BoxToCircle		(CollisionArbiter&, const Shape&, const Shape&);
		static void BoxToBox		(CollisionArbiter&, const Shape&, const Shape&);
		static void BoxToPoint		(CollisionArbiter&, const Shape&, const Shape&);
		static void BoxToConvex		(CollisionArbiter&, const Shape&, const Shape&);

		static void PointToCircle	(CollisionArbiter&, const Shape&, const Shape&);
		static void PointToBox		(CollisionArbiter&, const Shape&, const Shape&);
		static void PointToPoint	(CollisionArbiter&, const Shape&, const Shape&);
		static void PointToConvex	(CollisionArbiter&, const Shape&, const Shape&);

		static void ConvexToCircle	(CollisionArbiter&, const Shape&, const Shape&);
		static void ConvexToBox		(CollisionArbiter&, const Shape&, const Shape&);
		static void ConvexToPoint	(CollisionArbiter&, const Shape&, const Shape&);
		static void ConvexToConvex	(CollisionArbiter&, const Shape&, const Shape&);

	private:
		static constexpr bool BoxBiasGreaterThan(float a, float b, float c);
		static constexpr bool BiasGreaterThan(float a, float b);

	private:
		static auto BoxFindIncidentFace(const Mat2f& rot, const Mat2f& rot_tsp, 
			const Vector2f& half, const Vector2f& pos, const Vector2f& normal) -> Face;

	private:
		static Vector2f GetSupport(VectorSpan vertices, const Vector2f& dir);

		static std::tuple<float, uint32_t> FindAxisLeastPenetration(
			const Shape& s1, VectorSpan v1, VectorSpan n1,
			const Shape& s2, VectorSpan v2, VectorSpan n2);

		static auto FindIncidentFace(
			const Shape& inc, VectorSpan inc_vertices, VectorSpan inc_normals,
			const Shape& ref, const Vector2f& ref_normal) -> Face;

		static int Clip(Face& face, const Vector2f& n, float c);

	private:
		static Matrix table;
	};

	constexpr bool CollisionTable::BoxBiasGreaterThan(float a, float b, float c)
	{
		constexpr float k_bias_relative = 0.95f;
		constexpr float k_bias_absolute = 0.01f;

		return a >= b * k_bias_relative + c * k_bias_absolute;
	}

	constexpr bool CollisionTable::BiasGreaterThan(float a, float b)
	{
		constexpr float k_bias_relative = 0.95f;
		constexpr float k_bias_absolute = 0.01f;

		return a >= b * k_bias_relative + a * k_bias_absolute;
	}
}