#include "LSystem.h"

LSystem::LSystem(string Axiom) : m_Axiom(Axiom), m_CurrentSystem(Axiom)
{

}

void LSystem::Run(const int count)
{
	Reset();

	for (int i = 0; i < count; i++)
	{
		//Iterate();
	}
}

void LSystem::AddRule(char predecessor, string successor)
{
	rules.emplace(predecessor, successor);
}

void LSystem::Iterate(std::map<std::string, bool> map)
{
	std::string newString = "";

	// Go through the L-System string
	for (int i = 0; i < m_CurrentSystem.length(); ++i)
	{
		if (map["3DCylTree"])
		{
			Tree3D(m_CurrentSystem[i], newString);
		}
	}

	m_CurrentSystem = newString;
	newString = "";
}

void LSystem::Tree3D(char letter, std::string& newStr)
{
	switch (letter)
	{
	case 'A':
		newStr += "[&FA][>&FA][<&FA]";
		break;
	case '[':				// Save
		newStr += '[';
		break;
	case ']':				// Restore
		newStr += ']';
		break;
	case '&':				// Pitch
		newStr += "&";
		break;
	case '<':				// Roll left
		newStr += "<";
		break;
	case '>':				// Roll right
		newStr += ">";
		break;
	case 'F':
		newStr += "F";
		break;
	}
}

void LSystem::Reset()
{
	m_CurrentSystem = m_Axiom;
}
