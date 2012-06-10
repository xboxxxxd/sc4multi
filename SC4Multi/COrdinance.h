#include "Shared.h"

#ifndef COrdDef
	
	#define COrdDef
	class COrdinance
	{
		public:
			static BYTE CPR();
			static BYTE WaterConservation();
			static BYTE PaperReduction();
			static BYTE FreeClinics();
			static BYTE SmokeDetector();
			static BYTE HoodWatch();
			static BYTE TourismPromo();
			static BYTE JrSports();
			static BYTE PowerConservation();
			static BYTE Carpool();
			static BYTE Shuttle();
			static BYTE CleanAir();
			static BYTE YouthCurfew();
			static BYTE EmissionReduction();
			static BYTE TireRecycle();
			static BYTE NuclearFree();
			static BYTE ReadingCampaign();

		public:
			static void CPR( BYTE toEnable );
			static void WaterConservation( BYTE toEnable );
			static void PaperReduction( BYTE toEnable );
			static void FreeClinics( BYTE toEnable );
			static void SmokeDetector( BYTE toEnable );
			static void HoodWatch( BYTE toEnable );
			static void TourismPromo( BYTE toEnable );
			static void JrSports( BYTE toEnable );
			static void PowerConservation( BYTE toEnable );
			static void Carpool( BYTE toEnable );
			static void Shuttle( BYTE toEnable );
			static void CleanAir( BYTE toEnable );
			static void YouthCurfew( BYTE toEnable );
			static void EmissionReduction( BYTE toEnable );
			static void TireRecycle( BYTE toEnable );
			static void NuclearFree( BYTE toEnable );
			static void ReadingCampaign( BYTE toEnable );
	};

#endif