#pragma once
#include <iostream>

struct ClassA
{
public:
	void A() {}
};

struct ClassB : ClassA
{
public:
	void B() {}
};

struct ClassC : ClassB
{
public:
	void C() {}
};

struct ClassD : ClassC
{
public:
	void D() {}
};

class VirtualClassExample : ClassD
{
public:
	virtual ~VirtualClassExample() {};

	VirtualClassExample()
	{
		char MessageBuffer[MAX_PATH]{};

		sprintf_s(MessageBuffer, MAX_PATH, "Example virtual class address: 0x%p\n\nPress CTRL+C to copy it", this);

		MessageBoxA(NULL, MessageBuffer, "Example Virtual Class", MB_OK | MB_ICONINFORMATION);
	}
};

static VirtualClassExample ExampleInstance;