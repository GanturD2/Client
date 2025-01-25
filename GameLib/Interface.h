#pragma once

class IBackground : public CSingleton<IBackground>
{
public:
	IBackground() = default;
	~IBackground() = default;

	virtual bool IsBlock(int x, int y) = 0;
};