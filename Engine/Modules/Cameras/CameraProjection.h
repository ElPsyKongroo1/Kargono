#pragma once

#include "Kargono/Math/Math.h"

namespace Kargono::Cameras
{
	class CameraProjection
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CameraProjection() = default;
		CameraProjection(const Math::mat4& projection) : m_Projection(projection) {}
		virtual ~CameraProjection() = default;
	public:
		//==============================
		// Getters/Setters
		//==============================
		const Math::mat4& GetProjection() const { return m_Projection; }
		void SetProjection(const Math::mat4& projection) { m_Projection = projection; }
	public:
		//==============================
		// Operator Overloads
		//==============================
		operator Math::mat4& () { return m_Projection; }
	private:
		//==============================
		// Internal Fields
		//==============================
		Math::mat4 m_Projection{ Math::mat4(1.0f) };
	};
}
