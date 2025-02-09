#pragma once
#include "Kargono/Core/Base.h"

#include <string>

namespace Kargono
{
	enum class CardinalDirection
	{
		None = 0,
		North,
		East,
		South,
		West,
	};

	enum class Direction
	{
		None = 0,
		Up,
		Down,
		Left,
		Right,
		Forward,
		Backward,
	};
}

namespace Kargono::Utility
{
	inline const char* CardinalDirectionToString(CardinalDirection direction)
	{
		switch (direction)
		{
		case CardinalDirection::North: return "North";
		case CardinalDirection::East: return "East";
		case CardinalDirection::South: return "South";
		case CardinalDirection::West: return "West";
		case CardinalDirection::None:
		default:
			KG_ERROR("Unknown Data Type sent to CardinalDirectionToString Function");
			break;
		}
		KG_ERROR("Unknown Data Type sent to CardinalDirectionToString Function");
		return "None";
	}

	inline CardinalDirection StringToCardinalDirection(std::string_view string)
	{
		if (string == "North") { return CardinalDirection::North; }
		if (string == "East") { return CardinalDirection::East; }
		if (string == "South") { return CardinalDirection::South; }
		if (string == "West") { return CardinalDirection::West; }

		if (string == "None") 
		{ 
			KG_ERROR("None is not a valid Cardinal Direction");
		}

		KG_ERROR("Unknown Data Type sent to StringToCardinalDirection Function");
		return CardinalDirection::None;
	}

	inline const char* DirectionToString(Direction direction)
	{
		switch (direction)
		{
		case Direction::Up: return "Up";
		case Direction::Down: return "Down";
		case Direction::Left: return "Left";
		case Direction::Right: return "Right";
		case Direction::Forward: return "Forward";
		case Direction::Backward: return "Backward";
		case Direction::None:
		default:
			KG_ERROR("Unknown Data Type sent to DirectionToString Function");
			break;
		}
		KG_ERROR("Unknown Data Type sent to DirectionToString Function");
		return "None";
	}

	inline Direction StringToDirection(std::string_view string)
	{
		if (string == "Up") { return Direction::Up; }
		if (string == "Down") { return Direction::Down; }
		if (string == "Left") { return Direction::Left; }
		if (string == "Right") { return Direction::Right; }
		if (string == "Forward") { return Direction::Forward; }
		if (string == "Backward") { return Direction::Backward; }

		if (string == "None")
		{
			KG_ERROR("None is not a valid Direction");
		}

		KG_ERROR("Unknown Data Type sent to StringToDirection Function");
		return Direction::None;
	}


}
