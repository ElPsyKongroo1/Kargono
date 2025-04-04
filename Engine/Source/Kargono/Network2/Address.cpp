#include "kgpch.h"

#include "Address.h"

namespace Kargono::Network
{
	Address::Address()
	{
		m_Address = 0x7F000001; // 127.0.0.1
		m_Port = 3000;
	}

	Address::Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short m_Port)
	{
		SetAddress(a, b, c, d);
		m_Port = m_Port;
	}

	Address::Address(unsigned int m_Address, unsigned short m_Port)
	{
		m_Address = m_Address;
		m_Port = m_Port;
	}

	unsigned int Address::GetAddress() const
	{
		return m_Address;
	}

	unsigned char Address::GetA() const
	{
		return m_Address >> 24;
	}

	unsigned char Address::GetB() const
	{
		return (m_Address >> 16) & 0xFF;
	}

	unsigned char Address::GetC() const
	{
		return (m_Address >> 8) & 0xFF;
	}

	unsigned char Address::GetD() const
	{
		return m_Address & 0xFF;
	}

	unsigned short Address::GetPort() const
	{
		return m_Port;
	}

	void Address::SetAddress(unsigned int newAddress)
	{
		m_Address = newAddress;
	}

	void Address::SetAddress(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		m_Address = (a << 24) | (b << 16) | (c << 8) | d;
	}

	void Address::SetNewPort(unsigned short newPort)
	{
		m_Port = newPort;
	}

	bool Address::operator==(const Address& other) const
	{
		return (m_Address == other.m_Address) && (m_Port == other.m_Port);
	}
}
