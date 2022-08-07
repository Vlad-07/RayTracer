#pragma once

void CrashProgram()
{
	*((unsigned int*)0) = 0xDEAD;
	abort();
}