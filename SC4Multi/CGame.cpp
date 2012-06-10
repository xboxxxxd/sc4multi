#include "Shared.h"
#include "CGame.h"

long CGame::GetCash()      { return *(long *)( (*(long *)0x00b4418c) + 0x00000030 ); }
int CGame::GetPopulation() { return *(int *)( (*(int *)0x00b441cc) + 0x000000a4 ); }

float CGame::GetTaxIA()
{
}

float CGame::GetTaxID()
{
}

float CGame::GetTaxIM()
{
}

float CGame::GetTaxIHT()
{
}

float CGame::GetTaxR1()
{
}

float CGame::GetTaxR2()
{
}

float CGame::GetTaxR3()
{
}

float CGame::GetTaxCS1()
{
}

float CGame::GetTaxCS2()
{
}

float CGame::GetTaxCS3()
{
}

float CGame::GetTaxCO2()
{
}

float CGame::GetTaxCO3()
{
}

void CGame::SetTaxIA( float fTaxAg )    {  }
void CGame::SetTaxID( float fTaxDirt )  {  }
void CGame::SetTaxIM( float fTaxManu )  {  }
void CGame::SetTaxIHT( float fTaxTech ) {  }

void CGame::SetTaxR1( float fTaxPoor ) {  }
void CGame::SetTaxR2( float fTaxMid )  {  }
void CGame::SetTaxR3( float fTaxRich ) {  }

void CGame::SetTaxCS1( float fTaxServPoor )   {  }
void CGame::SetTaxCS2( float fTaxServMid )    {  }
void CGame::SetTaxCS3( float fTaxServRich )   {  }
void CGame::SetTaxCO2( float fTaxOfficeMid )  {  }
void CGame::SetTaxCO3( float fTaxOfficeRich ) {  }

void CGame::DisableCheats()
{
}

void CGame::EnableCheats()
{
}

bool CGame::CheatsEnabled()
{
}

void CGame::SetCash( long cash ) { *(long *)( (*(long *)0x00b4418c) + 0x00000030 ) = cash; }