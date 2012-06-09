#include "Shared.h"

#ifndef CGameDef

	#define CGameDef
	class CGame
	{
		public:
			static long GetCash();
			static int GetPopulation(); // read-only

			static float GetTaxIA();
			static float GetTaxID();
			static float GetTaxIM();
			static float GetTaxIHT();

			static float GetTaxR1();
			static float GetTaxR2();
			static float GetTaxR3();

			static float GetTaxCS1();
			static float GetTaxCS2();
			static float GetTaxCS3();

			static float GetTaxCO2();
			static float GetTaxCO3();

		public:
			static void SetCash( long cash );

			static void SetTaxIA( float fTaxAg );
			static void SetTaxID( float fTaxDirt );
			static void SetTaxIM( float fTaxManu );
			static void SetTaxIHT( float fTaxTech );

			static void SetTaxR1( float fTaxPoor );
			static void SetTaxR2( float fTaxMid );
			static void SetTaxR3( float fTaxRich );

			static void SetTaxCS1( float fTaxServPoor );
			static void SetTaxCS2( float fTaxServMid );
			static void SetTaxCS3( float fTaxServRich );

			static void SetTaxCO2( float fTaxOfficeMid );
			static void SetTaxCO3( float fTaxOfficeRich );

		public:
			static void DisableCheats();
			static void EnableCheats();
			static bool CheatsEnabled();
	};

#endif