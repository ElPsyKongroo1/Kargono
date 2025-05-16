#pragma once
#include "Kargono/Math/MathAliases.h"

namespace Kargono::Network
{
	class Address
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Address();
		Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short m_Port);
		Address(unsigned int m_Address, unsigned short m_Port);
		Address(Math::u8vec4 address, unsigned short port);

		//==============================
		// Getters/Setters
		//==============================
		// Manage Address
		unsigned int GetAddress() const;
		Math::u8vec4 GetAddressUVec4() const;
		unsigned char GetA() const;
		unsigned char GetB() const;
		unsigned char GetC() const;
		unsigned char GetD() const;
		void SetAddress(unsigned int newAddress);
		void SetAddress(Math::u8vec4 address);
		void SetAddress(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
		// Manage Port
		unsigned short GetPort() const;
		void SetNewPort(unsigned short newPort);

		//==============================
		// Operator Overloads
		//==============================
		bool operator==(const Address& other) const;

	private:
		//==============================
		// Internal Data
		//==============================
		unsigned int m_Address{ 0 };
		unsigned short m_Port{ 0 };
	};
}
