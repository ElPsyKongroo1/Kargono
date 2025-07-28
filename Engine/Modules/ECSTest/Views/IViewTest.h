#pragma once

#include "Modules/ECSTest/CommonTest.h"

#include <iterator>
#include <vector> // TODO: Maybe delete this?
#include <span>

namespace Kargono::ECS
{
	class IView
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		IView() = default;
		virtual ~IView() = default;
    public:
        //==============================
        // Enable For-Loop / Iterator Usage
        //==============================
       // virtual PackedIterator begin() const = 0;
        //virtual PackedIterator end() const = 0;
	};
}