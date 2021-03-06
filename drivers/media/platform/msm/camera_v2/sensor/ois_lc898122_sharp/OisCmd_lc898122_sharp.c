//********************************************************************************
//
//		<< LC898122 Evaluation Soft >>
//	    Program Name	: OisCmd.c
//		Design			: Y.Yamada
//		History			: First edition						2009.07.31 Y.Tashita
//********************************************************************************
//**************************
//	Include Header File		
//**************************
#define		OISCMD

//#include	"Main.h"
//#include	"Cmd.h"
#include	"Ois_lc898122_sharp.h"
#include	"OisDef_lc898122_sharp.h"

//**************************
//	Local Function Prottype	
//**************************
void			MesFil_lc898122_sharp( unsigned char ) ;					// Measure Filter Setting
#ifdef	MODULE_CALIBRATION
 #ifndef	HALLADJ_HW
 void			LopIni( unsigned char ) ;					// Loop Gain Initialize
 #endif
 void			LopPar_lc898122_sharp( unsigned char ) ;					// Loop Gain Parameter initialize
 #ifndef	HALLADJ_HW
  void			LopSin_lc898122_sharp( unsigned char, unsigned char ) ;	// Loop Gain Sin Wave Output
  unsigned char	LopAdj_lc898122_sharp( unsigned char ) ;					// Loop Gain Adjust
  void			LopMes_lc898122_sharp( void ) ;							// Loop Gain Measure
 #endif
#endif
#ifndef	HALLADJ_HW
unsigned long	GinMes_lc898122_sharp( unsigned char ) ;					// Measure Result Getting
#endif
void			GyrCon_lc898122_sharp( unsigned char ) ;					// Gyro Filter Control
short			GenMes_lc898122_sharp( unsigned short, unsigned char ) ;	// General Measure
#ifndef	HALLADJ_HW
// unsigned long	TnePtp_lc898122_sharp( unsigned char, unsigned char ) ;	// Get Hall Peak to Peak Values
// unsigned char	TneCen_lc898122_sharp( unsigned char, UnDwdVal ) ;			// Tuning Hall Center
 unsigned long	TneOff_lc898122_sharp( UnDwdVal, unsigned char ) ;			// Hall Offset Tuning
 unsigned long	TneBia_lc898122_sharp( UnDwdVal, unsigned char ) ;			// Hall Bias Tuning
#endif

void 			StbOnn_lc898122_sharp( void ) ;							// Servo ON Slope mode

void			SetSineWave_lc898122_sharp(   unsigned char , unsigned char );
void			StartSineWave_lc898122_sharp( void );
void			StopSineWave_lc898122_sharp(  void );

void			SetMeasFil_lc898122_sharp(  unsigned char );
void			ClrMeasFil_lc898122_sharp( void );



//**************************
//	define					
//**************************
#define		MES_XG1			0								// LXG1 Measure Mode
#define		MES_XG2			1								// LXG2 Measure Mode

#define		HALL_ADJ		0
#define		LOOPGAIN		1
#define		THROUGH			2
#define		NOISE			3

// Measure Mode

 #define		TNE 			80							// Waiting Time For Movement
#ifdef	HALLADJ_HW

 #define     __MEASURE_LOOPGAIN      0x00
 #define     __MEASURE_BIASOFFSET    0x01

#else

 /******* Hall calibration Type 1 *******/
 #define		MARJIN			0x0300						// Marjin
 #define		BIAS_ADJ_BORDER	0x1998						// HALL_MAX_GAP < BIAS_ADJ_BORDER < HALL_MIN_GAP(80%)

 #define		HALL_MAX_GAP	BIAS_ADJ_BORDER - MARJIN
 #define		HALL_MIN_GAP	BIAS_ADJ_BORDER + MARJIN
 /***************************************/
 
 #define		BIAS_LIMIT		0xFFFF						// HALL BIAS LIMIT
 #define		OFFSET_DIV		2							// Divide Difference For Offset Step
 #define		TIME_OUT		40							// Time Out Count

 /******* Hall calibration Type 2 *******/
 #define		MARGIN			0x0300						// Margin

 #define		BIAS_ADJ_OVER	0xD998						// 85%
 #define		BIAS_ADJ_RANGE	0xCCCC						// 80%
 #define		BIAS_ADJ_SKIP	0xBFFF						// 75%
 #define		HALL_MAX_RANGE	BIAS_ADJ_RANGE + MARGIN
 #define		HALL_MIN_RANGE	BIAS_ADJ_RANGE - MARGIN

 #define		DECRE_CAL		0x0100						// decrease value
 /***************************************/
#endif

#ifdef H1COEF_CHANGER
 #ifdef	CORRECT_05DEG
  #define		MAXLMT		0x40000000				// 2.0
  #define		MINLMT		0x3F8CCCCD				// 1.1
  #define		CHGCOEF		0xBA712C28				// 
 #endif	//CORRECT_05DEG
 
 #ifdef	CORRECT_10DEG
  #define		MAXLMT		0x40000000				// 2.0
  #define		MINLMT		0x3F8CCCCD				// 1.1
  #define		CHGCOEF		0xBA4C71C7				// -0.00078
 #endif	//CORRECT_10DEG
 
 #ifdef	CORRECT_105DEG
  #define		MAXLMT		0x40400000				// 3.0
  #define		MINLMT		0x40000000				// 2.0
  #define		CHGCOEF		0xB9400000				// -0.00018
 #endif	//CORRECT_105DEG
 
 #ifdef	CORRECT_12DEG
  #define		MAXLMT		0x40400000				// 3.0
  #define		MINLMT		0x4019999A				// 2.4
  #define		CHGCOEF		0xB9A00000				// -0.00031
 #endif	//CORRECT_12DEG
 
 #define		MINLMT_MOV	0x00000000				// 0.0
 #define		CHGCOEF_MOV	0xB9700000				// -0.00023

#endif

//**************************
//	Global Variable			
//**************************
#ifdef	HALLADJ_HW
 unsigned char UcAdjBsy;

#else
 unsigned short	UsStpSiz_lc898122_sharp	= 0 ;							// Bias Step Size
 unsigned short	UsErrBia_lc898122_sharp, UsErrOfs_lc898122_sharp ;
#endif



//**************************
//	Const					
//**************************
// gxzoom Setting Value
#define		ZOOMTBL	16
const unsigned long	ClGyxZom_lc898122_sharp[ ZOOMTBL ]	= {
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000
	} ;

// gyzoom Setting Value
const unsigned long	ClGyyZom_lc898122_sharp[ ZOOMTBL ]	= {
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000,
		0x3F800000
	} ;

// DI Coefficient Setting Value
#define		COEFTBL	7
const unsigned long	ClDiCof_lc898122_sharp[ COEFTBL ]	= {
		DIFIL_S2,		/* 0 */
		DIFIL_S2,		/* 1 */
		DIFIL_S2,		/* 2 */
		DIFIL_S2,		/* 3 */
		DIFIL_S2,		/* 4 */
		DIFIL_S2,		/* 5 */
		DIFIL_S2		/* 6 */
	} ;
	

//********************************************************************************
// Function Name 	: TneRun_lc898122_sharp
// Retun Value		: Hall Tuning SUCCESS or FAILURE
// Argment Value	: NON
// Explanation		: Hall System Auto Adjustment Function
// History			: First edition 						2009.12.1 YS.Kim
//********************************************************************************
unsigned short	TneRun_lc898122_sharp( void )
{
	unsigned char	UcHlySts, UcHlxSts, UcAtxSts, UcAtySts ;
	unsigned short	UsFinSts , UsOscSts ; 								// Final Adjustment state
	unsigned char	UcDrvMod ;
#ifndef	HALLADJ_HW
	UnDwdVal		StTneVal ;
#endif

#ifdef	USE_EXTCLK_ALL	// 24MHz
	UsOscSts	= EXE_END ;
#else
 #ifdef	MODULE_CALIBRATION
	/* OSC adjustment */
	if( UcCvrCod_lc898122_sharp == CVER122 ) {
		UsOscSts	= OscAdj_lc898122_sharp() ;
	} else {
  #ifdef	OSC_I2CCK
	UsOscSts	= EXE_END ;
  #else
   #ifdef	OSC_EXCLK
	UsOscSts	= EXE_END ;
   #else
	UsOscSts	= OscAdj_lc898122_sharp() ;
   #endif
  #endif
	}
 #else
	UsOscSts	= EXE_END ;
 #endif
#endif
	
	UcDrvMod = UcPwmMod_lc898122_sharp ;
	if( UcDrvMod == PWMMOD_CVL )
	{
		DrvPwmSw_lc898122_sharp( Mpwm ) ;		/* PWM mode */
	}
	
#ifdef	HALLADJ_HW
	UcHlySts = BiasOffsetAdj_lc898122_sharp( Y_DIR , 0 ) ;
	WitTim_lc898122_sharp( TNE ) ;
	UcHlxSts = BiasOffsetAdj_lc898122_sharp( X_DIR , 0 ) ;
	WitTim_lc898122_sharp( TNE ) ;
	UcHlySts = BiasOffsetAdj_lc898122_sharp( Y_DIR , 1 ) ;
	WitTim_lc898122_sharp( TNE ) ;
	UcHlxSts = BiasOffsetAdj_lc898122_sharp( X_DIR , 1 ) ;

	SrvCon_lc898122_sharp( Y_DIR, OFF ) ;
	SrvCon_lc898122_sharp( X_DIR, OFF ) ;
	
	if( UcDrvMod == PWMMOD_CVL )
	{
		DrvPwmSw_lc898122_sharp( Mlnp ) ;		/* PWM mode */
	}
	
  #ifdef	NEUTRAL_CENTER
	TneHvc_lc898122_sharp();
  #endif	// NEUTRAL_CENTER	
#else
//	StbOnnN_lc898122_sharp( OFF , ON ) ;				/* Y OFF, X ON */
	WitTim_lc898122_sharp( TNE ) ;

	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( Y_DIR , PTP_BEFORE ) ;
//	UcHlySts	= TneCen_lc898122_sharp( Y_DIR, StTneVal ) ;
	UcHlySts	= TneCen_lc898122_sharp( Y2_DIR, StTneVal ) ;
	
	StbOnnN_lc898122_sharp( ON , OFF ) ;				/* Y ON, X OFF */
	WitTim_lc898122_sharp( TNE ) ;

	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( X_DIR , PTP_BEFORE ) ;
//	UcHlxSts	= TneCen_lc898122_sharp( X_DIR, StTneVal ) ;
	UcHlxSts	= TneCen_lc898122_sharp( X2_DIR, StTneVal ) ;

	StbOnnN_lc898122_sharp( OFF , ON ) ;				/* Y OFF, X ON */
	WitTim_lc898122_sharp( TNE ) ;

	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( Y_DIR , PTP_AFTER ) ;
//	UcHlySts	= TneCen_lc898122_sharp( Y_DIR, StTneVal ) ;
	UcHlySts	= TneCen_lc898122_sharp( Y2_DIR, StTneVal ) ;

	StbOnnN_lc898122_sharp( ON , OFF ) ;				/* Y ON, X OFF */
	WitTim_lc898122_sharp( TNE ) ;

	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( X_DIR , PTP_AFTER ) ;
//	UcHlxSts	= TneCen_lc898122_sharp( X_DIR, StTneVal ) ;
	UcHlxSts	= TneCen_lc898122_sharp( X2_DIR, StTneVal ) ;

	SrvCon_lc898122_sharp( Y_DIR, OFF ) ;
	SrvCon_lc898122_sharp( X_DIR, OFF ) ;
	
	if( UcDrvMod == PWMMOD_CVL )
	{
		DrvPwmSw_lc898122_sharp( Mlnp ) ;		/* PWM mode */
	}
	
  #ifdef	NEUTRAL_CENTER
	TneHvc_lc898122_sharp();
  #endif	//NEUTRAL_CENTER
#endif
	

	WitTim_lc898122_sharp( TNE ) ;

	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	StAdjPar_lc898122_sharp.StHalAdj.UsAdxOff = (unsigned short)((unsigned long)0x00010000 - (unsigned long)StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna ) ;
	StAdjPar_lc898122_sharp.StHalAdj.UsAdyOff = (unsigned short)((unsigned long)0x00010000 - (unsigned long)StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna ) ;
	
	RamWriteA_lc898122_sharp( OFF0Z,  StAdjPar_lc898122_sharp.StHalAdj.UsAdxOff ) ;		// 0x1450
	RamWriteA_lc898122_sharp( OFF1Z,  StAdjPar_lc898122_sharp.StHalAdj.UsAdyOff ) ;		// 0x14D0

	RamReadA_lc898122_sharp( DAXHLO, &StAdjPar_lc898122_sharp.StHalAdj.UsHlxOff ) ;		// 0x1479
	RamReadA_lc898122_sharp( DAXHLB, &StAdjPar_lc898122_sharp.StHalAdj.UsHlxGan ) ;		// 0x147A
	RamReadA_lc898122_sharp( DAYHLO, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyOff ) ;		// 0x14F9
	RamReadA_lc898122_sharp( DAYHLB, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyGan ) ;		// 0x14FA
	RamReadA_lc898122_sharp( OFF0Z, &StAdjPar_lc898122_sharp.StHalAdj.UsAdxOff ) ;		// 0x1450
	RamReadA_lc898122_sharp( OFF1Z, &StAdjPar_lc898122_sharp.StHalAdj.UsAdyOff ) ;		// 0x14D0
	
	RamAccFixMod_lc898122_sharp( OFF ) ;									// Float mode
	
	StbOnn_lc898122_sharp() ;												// Slope Mode

	
	WitTim_lc898122_sharp( TNE ) ;

#ifdef	MODULE_CALIBRATION
	// X Loop Gain Adjust
	UcAtxSts	= LopGan_lc898122_sharp( X_DIR ) ;


	// Y Loop Gain Adjust
	UcAtySts	= LopGan_lc898122_sharp( Y_DIR ) ;
#else		//  default value
	RamAccFixMod_lc898122_sharp( ON ) ;									// Fix mode
	RamReadA_lc898122_sharp( sxg, &StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgVal ) ;			// 0x10D3
	RamReadA_lc898122_sharp( syg, &StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygVal ) ;			// 0x11D3
	RamAccFixMod_lc898122_sharp( OFF ) ;									// Float mode
	UcAtxSts	= EXE_END ;
	UcAtySts	= EXE_END ;
#endif
	

	TneGvc_lc898122_sharp() ;


	UsFinSts	= (unsigned short)( UcHlxSts - EXE_END ) + (unsigned short)( UcHlySts - EXE_END ) + (unsigned short)( UcAtxSts - EXE_END ) + (unsigned short)( UcAtySts - EXE_END ) + ( UsOscSts - (unsigned short)EXE_END ) + (unsigned short)EXE_END ;


	
/* 表示 */
	
	return( UsFinSts ) ;
}


#ifndef	HALLADJ_HW

//********************************************************************************
// Function Name 	: TnePtp_lc898122_sharp
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: X,Y Direction, Adjust Before After Parameter
// Explanation		: Measuring Hall Paek To Peak
// History			: First edition 						2009.12.1 YS.Kim
//********************************************************************************
 
unsigned long	TnePtp_lc898122_sharp ( unsigned char	UcDirSel, unsigned char	UcBfrAft )
{
	UnDwdVal		StTneVal ;

	MesFil_lc898122_sharp( THROUGH ) ;					// 測定用フィルターを設定する。


	if ( !UcDirSel ) {
		RamWrite32A_lc898122_sharp( sxsin , HALL_H_VAL );		// 0x10D5
		SetSinWavePara_lc898122_sharp( 0x0A , XHALWAVE ); 
	}else{
	    RamWrite32A_lc898122_sharp( sysin , HALL_H_VAL ); 		// 0x11D5
		SetSinWavePara_lc898122_sharp( 0x0A , YHALWAVE ); 
	}

	if ( !UcDirSel ) {					// AXIS X
		RegWriteA_lc898122_sharp( WC_MES1ADD0,  ( unsigned char )AD0Z ) ;							/* 0x0194	*/
		RegWriteA_lc898122_sharp( WC_MES1ADD1,  ( unsigned char )(( AD0Z >> 8 ) & 0x0001 ) ) ;		/* 0x0195	*/
	} else {							// AXIS Y
		RegWriteA_lc898122_sharp( WC_MES1ADD0,  ( unsigned char )AD1Z ) ;							/* 0x0194	*/
		RegWriteA_lc898122_sharp( WC_MES1ADD1,  ( unsigned char )(( AD1Z >> 8 ) & 0x0001 ) ) ;		/* 0x0195	*/
	}

	RegWriteA_lc898122_sharp( WC_MESLOOP1	, 0x00 );			// 0x0193	CmMesLoop[15:8]
	RegWriteA_lc898122_sharp( WC_MESLOOP0	, 0x01);			// 0x0192	CmMesLoop[7:0]
	
	RamWrite32A_lc898122_sharp( msmean	, 0x3F800000 );			// 0x1230	1/CmMesLoop[15:0]
	
	RamWrite32A_lc898122_sharp( MSMAX1, 	0x00000000 ) ;		// 0x1050
	RamWrite32A_lc898122_sharp( MSMAX1AV, 	0x00000000 ) ;		// 0x1051
	RamWrite32A_lc898122_sharp( MSMIN1, 	0x00000000 ) ;		// 0x1060
	RamWrite32A_lc898122_sharp( MSMIN1AV, 	0x00000000 ) ;		// 0x1061
	
	RegWriteA_lc898122_sharp( WC_MESABS, 0x00 ) ;				// 0x0198	none ABS
	BsyWit_lc898122_sharp( WC_MESMODE, 0x02 ) ;				// 0x0190		Sine wave Measure

	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	RamReadA_lc898122_sharp( MSMAX1AV, &StTneVal.StDwdVal.UsHigVal ) ;		// 0x1051
	RamReadA_lc898122_sharp( MSMIN1AV, &StTneVal.StDwdVal.UsLowVal ) ;		// 0x1061

	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode

	if ( !UcDirSel ) {					// AXIS X
		SetSinWavePara_lc898122_sharp( 0x00 , XHALWAVE ); 	/* STOP */
	}else{
		SetSinWavePara_lc898122_sharp( 0x00 , YHALWAVE ); 	/* STOP */
	}

	if( UcBfrAft == 0 ) {
		if( UcDirSel == X_DIR ) {
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxCen	= ( ( signed short )StTneVal.StDwdVal.UsHigVal + ( signed short )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxMax	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxMin	= StTneVal.StDwdVal.UsLowVal ;
		} else {
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyCen	= ( ( signed short )StTneVal.StDwdVal.UsHigVal + ( signed short )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyMax	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyMin	= StTneVal.StDwdVal.UsLowVal ;
		}
	} else {
		if( UcDirSel == X_DIR ){
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna	= ( ( signed short )StTneVal.StDwdVal.UsHigVal + ( signed short )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxMxa	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxMna	= StTneVal.StDwdVal.UsLowVal ;
		} else {
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna	= ( ( signed short )StTneVal.StDwdVal.UsHigVal + ( signed short )StTneVal.StDwdVal.UsLowVal ) / 2 ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyMxa	= StTneVal.StDwdVal.UsHigVal ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyMna	= StTneVal.StDwdVal.UsLowVal ;
		}
	}

	StTneVal.StDwdVal.UsHigVal	= 0x7fff - StTneVal.StDwdVal.UsHigVal ;		// Maximum Gap = Maximum - Hall Peak Top
	StTneVal.StDwdVal.UsLowVal	= StTneVal.StDwdVal.UsLowVal - 0x7fff ; 	// Minimum Gap = Hall Peak Bottom - Minimum

	
	return( StTneVal.UlDwdVal ) ;
}

//********************************************************************************
// Function Name 	: TneCen_lc898122_sharp
// Retun Value		: Hall Center Tuning Result
// Argment Value	: X,Y Direction, Hall Top & Bottom Gaps
// Explanation		: Hall Center Tuning Function
// History			: First edition 						2009.12.1 YS.Kim
//********************************************************************************
unsigned short	UsValBef_lc898122_sharp,UsValNow_lc898122_sharp ;
unsigned char	TneCen_lc898122_sharp( unsigned char	UcTneAxs, UnDwdVal	StTneVal )
{
	unsigned char 	UcTneRst, UcTmeOut, UcTofRst ;
	unsigned short	UsOffDif ;
	unsigned short	UsBiasVal ;

	UsErrBia_lc898122_sharp	= 0 ;
	UsErrOfs_lc898122_sharp	= 0 ;
	UcTmeOut	= 1 ;
	UsStpSiz_lc898122_sharp	= 1 ;
	UcTneRst	= FAILURE ;
	UcTofRst	= FAILURE ;

	while ( UcTneRst && UcTmeOut )
	{
		if( UcTofRst == FAILURE ) {
			StTneVal.UlDwdVal	= TneOff_lc898122_sharp( StTneVal, UcTneAxs ) ;
		} else {
			StTneVal.UlDwdVal	= TneBia_lc898122_sharp( StTneVal, UcTneAxs ) ;
			UcTofRst	= FAILURE ;
		}

		if( !( UcTneAxs & 0xF0 ) )
		{
			if ( StTneVal.StDwdVal.UsHigVal > StTneVal.StDwdVal.UsLowVal ) {									// Check Offset Tuning Result
				UsOffDif	= ( StTneVal.StDwdVal.UsHigVal - StTneVal.StDwdVal.UsLowVal ) / 2 ;
			} else {
				UsOffDif	= ( StTneVal.StDwdVal.UsLowVal - StTneVal.StDwdVal.UsHigVal ) / 2 ;
			}

			if( UsOffDif < MARJIN ) {
				UcTofRst	= SUCCESS ;
			} else {
				UcTofRst	= FAILURE ;
			}

			if ( ( StTneVal.StDwdVal.UsHigVal < HALL_MIN_GAP && StTneVal.StDwdVal.UsLowVal < HALL_MIN_GAP )		// Check Tuning Result 
			&& ( StTneVal.StDwdVal.UsHigVal > HALL_MAX_GAP && StTneVal.StDwdVal.UsLowVal > HALL_MAX_GAP ) ) {
				UcTneRst	= SUCCESS ;
				break ;
			} else if ( UsStpSiz_lc898122_sharp == 0 ) {
				UcTneRst	= SUCCESS ;
				break ;
			} else {
				UcTneRst	= FAILURE ;
				UcTmeOut++ ;
			}
		}else{
			if( (StTneVal.StDwdVal.UsHigVal > MARGIN ) && (StTneVal.StDwdVal.UsLowVal > MARGIN ) )	/* position check */
			{
				UcTofRst	= SUCCESS ;
				UsValBef_lc898122_sharp = UsValNow_lc898122_sharp = 0x0000 ;
			}else if( (StTneVal.StDwdVal.UsHigVal <= MARGIN ) && (StTneVal.StDwdVal.UsLowVal <= MARGIN ) ){
				UcTofRst	= SUCCESS ;
				UcTneRst	= FAILURE ;
			}else if( ((unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > BIAS_ADJ_OVER ) {
				UcTofRst	= SUCCESS ;
				UcTneRst	= FAILURE ;
			}else{
				UcTofRst	= FAILURE ;

				
				UsValBef_lc898122_sharp = UsValNow_lc898122_sharp ;

				RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
				
				if( !( UcTneAxs & 0x0F ) ) {
					RamReadA_lc898122_sharp( DAXHLO, &UsValNow_lc898122_sharp ) ;				// 0x1479	Hall X Offset Read
				}else{
					RamReadA_lc898122_sharp( DAYHLO, &UsValNow_lc898122_sharp ) ;				// 0x14F9	Hall Y Offset Read
				}
				if( ((( UsValBef_lc898122_sharp & 0xFF00 ) == 0x8000 ) && ( UsValNow_lc898122_sharp & 0xFF00 ) == 0x8000 )
				 || ((( UsValBef_lc898122_sharp & 0xFF00 ) == 0x7F00 ) && ( UsValNow_lc898122_sharp & 0xFF00 ) == 0x7F00 ) )
				{
					if( !( UcTneAxs & 0x0F ) ) {
						RamReadA_lc898122_sharp( DAXHLB, &UsBiasVal ) ;		// 0x147A	Hall X Bias Read
					}else{
						RamReadA_lc898122_sharp( DAYHLB, &UsBiasVal ) ;		// 0x14FA	Hall Y Bias Read
					}
					if( UsBiasVal > 0x8000 )
					{
						UsBiasVal -= 0x8000 ;
					}
					else
					{
						UsBiasVal += 0x8000 ;
					}
					if( UsBiasVal > DECRE_CAL )
					{
						UsBiasVal -= DECRE_CAL ;
					}
					UsBiasVal += 0x8000 ;
					
					if( !( UcTneAxs & 0x0F ) ) {
						RamWriteA_lc898122_sharp( DAXHLB, UsBiasVal ) ;		// 0x147A	Hall X Bias
					}else{
						RamWriteA_lc898122_sharp( DAYHLB, UsBiasVal ) ;		// 0x14FA	Hall Y Bias
					}
				}

				RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
				
			}
			
			if((( (unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) < HALL_MAX_RANGE )
			&& (( (unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > HALL_MIN_RANGE ) ) {
				if(UcTofRst	== SUCCESS)
				{
					UcTneRst	= SUCCESS ;
					break ;
				}
			}
			UcTneRst	= FAILURE ;
			UcTmeOut++ ;
		}

		if( UcTneAxs & 0xF0 )
		{
			if ( ( UcTmeOut / 2 ) == TIME_OUT ) {
				UcTmeOut	= 0 ;
			}		 																							// Set Time Out Count
		}else{
			if ( UcTmeOut == TIME_OUT ) {
				UcTmeOut	= 0 ;
			}		 																							// Set Time Out Count
		}
	}

	if( UcTneRst == FAILURE ) {
		if( !( UcTneAxs & 0x0F ) ) {
			UcTneRst					= EXE_HXADJ ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxGan	= 0xFFFF ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlxOff	= 0xFFFF ;
		} else {
			UcTneRst					= EXE_HYADJ ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyGan	= 0xFFFF ;
			StAdjPar_lc898122_sharp.StHalAdj.UsHlyOff	= 0xFFFF ;
		}
	} else {
		UcTneRst	= EXE_END ;
	}

	return( UcTneRst ) ;
}



//********************************************************************************
// Function Name 	: TneBia_lc898122_sharp
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: Hall Top & Bottom Gaps , X,Y Direction
// Explanation		: Hall Bias Tuning Function
// History			: First edition 						2009.12.1 YS.Kim
//********************************************************************************
unsigned long	TneBia_lc898122_sharp( UnDwdVal	StTneVal, unsigned char	UcTneAxs )
{
	long					SlSetBia ;
	unsigned short			UsSetBia ;
	unsigned char			UcChkFst ;
	static unsigned short	UsTneVax ;							// Variable For 1/2 Searching
	unsigned short			UsDecCal ;

	UcChkFst	= 1 ;

	if ( UsStpSiz_lc898122_sharp == 1) {
		UsTneVax	= 2 ;

		if( UcTneAxs & 0xF0 ){
			if ( ((unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > BIAS_ADJ_OVER ) {
				UcChkFst	= 0 ;

				RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
				
				if ( !( UcTneAxs & 0x0F ) ) {							// Initializing Hall Offset & Bias, Step Size
					RamReadA_lc898122_sharp( DAXHLB, &UsSetBia ) ;		// 0x147A	Hall X Bias Read
				} else {
					RamReadA_lc898122_sharp( DAYHLB, &UsSetBia ) ;		// 0x14FA	Hall Y Bias Read
				}
				if( UsSetBia > 0x8000 )
				{
					UsSetBia -= 0x8000 ;
				}
				else
				{
					UsSetBia += 0x8000 ;
				}
				if( !UcChkFst )	{
					UsDecCal = ( DECRE_CAL << 3 ) ;
				}else{
					UsDecCal = DECRE_CAL ;
				}
				if( UsSetBia > UsDecCal )
				{
					UsSetBia -= UsDecCal ;
				}
				UsSetBia += 0x8000 ;
				if ( !( UcTneAxs & 0x0F ) ) {							// Initializing Hall Offset & Bias, Step Size
					RamWriteA_lc898122_sharp( DAXHLB, UsSetBia ) ;				// 0x147A	Hall X Bias
					RamWriteA_lc898122_sharp( DAXHLO, 0x0000 ) ;				// 0x1479	Hall X Offset 0x0000
				} else {
					RamWriteA_lc898122_sharp( DAYHLB, UsSetBia ) ;				// 0x14FA	Hall Y Bias
					RamWriteA_lc898122_sharp( DAYHLO, 0x0000 ) ;				// 0x14F9	Hall Y Offset 0x0000
				}
				UsStpSiz_lc898122_sharp	= BIAS_LIMIT / UsTneVax ;

				RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
				
			}
		}else{
			if ( ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal ) / 2 < BIAS_ADJ_BORDER ) {
				UcChkFst	= 0 ;
			}

			if ( !UcTneAxs ) {										// Initializing Hall Offset & Bias, Step Size
				
				RamWrite32A_lc898122_sharp( DAXHLB, 0xBF800000 ) ; 	// 0x147A	Hall X Bias 0x8001
				RamWrite32A_lc898122_sharp( DAXHLO, 0x00000000 ) ;		// 0x1479	Hall X Offset 0x0000

				UsStpSiz_lc898122_sharp	= BIAS_LIMIT / UsTneVax ;
			} else {
				RamWrite32A_lc898122_sharp( DAYHLB, 0xBF800000 ) ; 	// 0x14FA	Hall Y Bias 0x8001
				RamWrite32A_lc898122_sharp( DAYHLO, 0x00000000 ) ;		// 0x14F9	 Y Offset 0x0000
				UsStpSiz_lc898122_sharp	= BIAS_LIMIT / UsTneVax ;
			}
		}
	}

	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	if ( !( UcTneAxs & 0x0F ) ) {
		RamReadA_lc898122_sharp( DAXHLB, &UsSetBia ) ;					// 0x147A	Hall X Bias Read
		SlSetBia	= ( long )UsSetBia ;
	} else {
		RamReadA_lc898122_sharp( DAYHLB, &UsSetBia ) ;					// 0x14FA	Hall Y Bias Read
		SlSetBia	= ( long )UsSetBia ;
	}

	if( SlSetBia >= 0x00008000 ) {
		SlSetBia	|= 0xFFFF0000 ;
	}

	if( UcChkFst ) {
		if( UcTneAxs & 0xF0 )
		{
			if ( ((unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) < BIAS_ADJ_RANGE ) {	// Calculatiton For Hall BIAS 1/2 Searching
				if( ((unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) < BIAS_ADJ_SKIP ) {
					SlSetBia	+= 0x0400 ;
				}else{
					SlSetBia	+= 0x0100 ;
				}
			} else {
				if( ((unsigned short)0xFFFF - ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal )) > BIAS_ADJ_OVER ) {
					SlSetBia	-= 0x0400 ;
				}else{
					SlSetBia	-= 0x0100 ;
				}
			}
			UsStpSiz_lc898122_sharp	= 0x0200 ;
			
		}else{
			if ( ( StTneVal.StDwdVal.UsHigVal + StTneVal.StDwdVal.UsLowVal ) / 2 > BIAS_ADJ_BORDER ) {	// Calculatiton For Hall BIAS 1/2 Searching
				SlSetBia	+= UsStpSiz_lc898122_sharp ;
			} else {
				SlSetBia	-= UsStpSiz_lc898122_sharp ;
			}

			UsTneVax	= UsTneVax * 2 ;
			UsStpSiz_lc898122_sharp	= BIAS_LIMIT / UsTneVax ;
		}
	}

	if( SlSetBia > ( long )0x00007FFF ) {
		SlSetBia	= 0x00007FFF ;
	} else if( SlSetBia < ( long )0xFFFF8001 ) {
		SlSetBia	= 0xFFFF8001 ;
	}

	if ( !( UcTneAxs & 0x0F ) ) {
		RamWriteA_lc898122_sharp( DAXHLB, SlSetBia ) ;		// 0x147A	Hall X Bias Ram Write
	} else {
		RamWriteA_lc898122_sharp( DAYHLB, SlSetBia ) ;		// 0x14FA	Hall Y Bias Ram Write
	}
	
	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode

	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( UcTneAxs & 0x0F , PTP_AFTER ) ;

	return( StTneVal.UlDwdVal ) ;
}



//********************************************************************************
// Function Name 	: TneOff_lc898122_sharp
// Retun Value		: Hall Top & Bottom Gaps
// Argment Value	: Hall Top & Bottom Gaps , X,Y Direction
// Explanation		: Hall Offset Tuning Function
// History			: First edition 						2009.12.1 YS.Kim
//********************************************************************************
unsigned long	TneOff_lc898122_sharp( UnDwdVal	StTneVal, unsigned char	UcTneAxs )
{
	long			SlSetOff ;
	unsigned short	UsSetOff ;

	UcTneAxs &= 0x0F ;
	
	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
		
	if ( !UcTneAxs ) {																			// Initializing Hall Offset & Bias
		RamReadA_lc898122_sharp( DAXHLO, &UsSetOff ) ;				// 0x1479	Hall X Offset Read
		SlSetOff	= ( long )UsSetOff ;
	} else {
		RamReadA_lc898122_sharp( DAYHLO, &UsSetOff ) ;				// 0x14F9	Hall Y Offset Read
		SlSetOff	= ( long )UsSetOff ;
	}

	if( SlSetOff > 0x00008000 ) {
		SlSetOff	|= 0xFFFF0000 ;
	}

	if ( StTneVal.StDwdVal.UsHigVal > StTneVal.StDwdVal.UsLowVal ) {
		SlSetOff	+= ( StTneVal.StDwdVal.UsHigVal - StTneVal.StDwdVal.UsLowVal ) / OFFSET_DIV ;	// Calculating Value For Increase Step
	} else {
		SlSetOff	-= ( StTneVal.StDwdVal.UsLowVal - StTneVal.StDwdVal.UsHigVal ) / OFFSET_DIV ;	// Calculating Value For Decrease Step
	}

	if( SlSetOff > ( long )0x00007FFF ) {
		SlSetOff	= 0x00007FFF ;
	} else if( SlSetOff < ( long )0xFFFF8001 ) {
		SlSetOff	= 0xFFFF8001 ;
	}

	if ( !UcTneAxs ) {
		RamWriteA_lc898122_sharp( DAXHLO, SlSetOff ) ;		// 0x1479	Hall X Offset Ram Write
	} else {
		RamWriteA_lc898122_sharp( DAYHLO, SlSetOff ) ;		// 0x14F9	Hall Y Offset Ram Write
	}

	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
	StTneVal.UlDwdVal	= TnePtp_lc898122_sharp( UcTneAxs, PTP_AFTER ) ;

	return( StTneVal.UlDwdVal ) ;
}

#endif

//********************************************************************************
// Function Name 	: MesFil_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Measure Filter Mode
// Explanation		: Measure Filter Setting Function
// History			: First edition 						2009.07.31  Y.Tashita
//********************************************************************************
void	MesFil_lc898122_sharp( unsigned char	UcMesMod )
{
#ifdef	USE_EXTCLK_ALL	// 24MHz
	if( !UcMesMod ) {								// Hall Bias&Offset Adjust
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3D1E5A40 ) ;		// 0x10F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3D1E5A40 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F6C34C0 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F800000 ) ;		// 0x10F5	Through
		RamWrite32A_lc898122_sharp( mes1bb, 0x00000000 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x00000000 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3D1E5A40 ) ;		// 0x11F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3D1E5A40 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F6C34C0 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F800000 ) ;		// 0x11F5	Through
		RamWrite32A_lc898122_sharp( mes2bb, 0x00000000 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x00000000 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == LOOPGAIN ) {				// Loop Gain Adjust
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3E587E00 ) ;		// 0x10F0	LPF1000Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3E587E00 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F13C100 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F7DF500 ) ;		// 0x10F5	HPF30Hz
		RamWrite32A_lc898122_sharp( mes1bb, 0xBF7DF500 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x3F7BEA40 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3E587E00 ) ;		// 0x11F0	LPF1000Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3E587E00 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F13C100 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F7DF500 ) ;		// 0x11F5	HPF30Hz
		RamWrite32A_lc898122_sharp( mes2bb, 0xBF7DF500 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x3F7BEA40 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == THROUGH ) {				// for Through
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3F800000 ) ;		// 0x10F0	Through
		RamWrite32A_lc898122_sharp( mes1ab, 0x00000000 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x00000000 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F800000 ) ;		// 0x10F5	Through
		RamWrite32A_lc898122_sharp( mes1bb, 0x00000000 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x00000000 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3F800000 ) ;		// 0x11F0	Through
		RamWrite32A_lc898122_sharp( mes2ab, 0x00000000 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x00000000 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F800000 ) ;		// 0x11F5	Through
		RamWrite32A_lc898122_sharp( mes2bb, 0x00000000 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x00000000 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == NOISE ) {				// SINE WAVE TEST for NOISE
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3D1E5A40 ) ;		// 0x10F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3D1E5A40 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F6C34C0 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3D1E5A40 ) ;		// 0x10F5	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1bb, 0x3D1E5A40 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x3F6C34C0 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3D1E5A40 ) ;		// 0x11F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3D1E5A40 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F6C34C0 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3D1E5A40 ) ;		// 0x11F5	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2bb, 0x3D1E5A40 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x3F6C34C0 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
	}
#else
	if( !UcMesMod ) {								// Hall Bias&Offset Adjust
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3CA175C0 ) ;		// 0x10F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3CA175C0 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F75E8C0 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F800000 ) ;		// 0x10F5	Through
		RamWrite32A_lc898122_sharp( mes1bb, 0x00000000 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x00000000 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3CA175C0 ) ;		// 0x11F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3CA175C0 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F75E8C0 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F800000 ) ;		// 0x11F5	Through
		RamWrite32A_lc898122_sharp( mes2bb, 0x00000000 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x00000000 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == LOOPGAIN ) {				// Loop Gain Adjust
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3DF21080 ) ;		// 0x10F0	LPF1000Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3DF21080 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F437BC0 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F7EF980 ) ;		// 0x10F5	HPF30Hz
		RamWrite32A_lc898122_sharp( mes1bb, 0xBF7EF980 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x3F7DF300 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3DF21080 ) ;		// 0x11F0	LPF1000Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3DF21080 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F437BC0 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F7EF980 ) ;		// 0x11F5	HPF30Hz
		RamWrite32A_lc898122_sharp( mes2bb, 0xBF7EF980 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x3F7DF300 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == THROUGH ) {				// for Through
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3F800000 ) ;		// 0x10F0	Through
		RamWrite32A_lc898122_sharp( mes1ab, 0x00000000 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x00000000 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3F800000 ) ;		// 0x10F5	Through
		RamWrite32A_lc898122_sharp( mes1bb, 0x00000000 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x00000000 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3F800000 ) ;		// 0x11F0	Through
		RamWrite32A_lc898122_sharp( mes2ab, 0x00000000 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x00000000 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3F800000 ) ;		// 0x11F5	Through
		RamWrite32A_lc898122_sharp( mes2bb, 0x00000000 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x00000000 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
		
	} else if( UcMesMod == NOISE ) {				// SINE WAVE TEST for NOISE
		// Measure Filter1 Setting
		RamWrite32A_lc898122_sharp( mes1aa, 0x3CA175C0 ) ;		// 0x10F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1ab, 0x3CA175C0 ) ;		// 0x10F1
		RamWrite32A_lc898122_sharp( mes1ac, 0x3F75E8C0 ) ;		// 0x10F2
		RamWrite32A_lc898122_sharp( mes1ad, 0x00000000 ) ;		// 0x10F3
		RamWrite32A_lc898122_sharp( mes1ae, 0x00000000 ) ;		// 0x10F4
		RamWrite32A_lc898122_sharp( mes1ba, 0x3CA175C0 ) ;		// 0x10F5	LPF150Hz
		RamWrite32A_lc898122_sharp( mes1bb, 0x3CA175C0 ) ;		// 0x10F6
		RamWrite32A_lc898122_sharp( mes1bc, 0x3F75E8C0 ) ;		// 0x10F7
		RamWrite32A_lc898122_sharp( mes1bd, 0x00000000 ) ;		// 0x10F8
		RamWrite32A_lc898122_sharp( mes1be, 0x00000000 ) ;		// 0x10F9
		
		// Measure Filter2 Setting
		RamWrite32A_lc898122_sharp( mes2aa, 0x3CA175C0 ) ;		// 0x11F0	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2ab, 0x3CA175C0 ) ;		// 0x11F1
		RamWrite32A_lc898122_sharp( mes2ac, 0x3F75E8C0 ) ;		// 0x11F2
		RamWrite32A_lc898122_sharp( mes2ad, 0x00000000 ) ;		// 0x11F3
		RamWrite32A_lc898122_sharp( mes2ae, 0x00000000 ) ;		// 0x11F4
		RamWrite32A_lc898122_sharp( mes2ba, 0x3CA175C0 ) ;		// 0x11F5	LPF150Hz
		RamWrite32A_lc898122_sharp( mes2bb, 0x3CA175C0 ) ;		// 0x11F6
		RamWrite32A_lc898122_sharp( mes2bc, 0x3F75E8C0 ) ;		// 0x11F7
		RamWrite32A_lc898122_sharp( mes2bd, 0x00000000 ) ;		// 0x11F8
		RamWrite32A_lc898122_sharp( mes2be, 0x00000000 ) ;		// 0x11F9
	}
#endif
}



//********************************************************************************
// Function Name 	: SrvCon_lc898122_sharp
// Retun Value		: NON
// Argment Value	: X or Y Select, Servo ON/OFF
// Explanation		: Servo ON,OFF Function
// History			: First edition 						2013.01.09 Y.Shigeoka
//********************************************************************************
void	SrvCon_lc898122_sharp( unsigned char	UcDirSel, unsigned char	UcSwcCon )
{
	if( UcSwcCon ) {
		if( !UcDirSel ) {						// X Direction
			RegWriteA_lc898122_sharp( WH_EQSWX , 0x03 ) ;			// 0x0170
			RamWrite32A_lc898122_sharp( sxggf, 0x00000000 ) ;		// 0x10B5
		} else {								// Y Direction
			RegWriteA_lc898122_sharp( WH_EQSWY , 0x03 ) ;			// 0x0171
			RamWrite32A_lc898122_sharp( syggf, 0x00000000 ) ;		// 0x11B5
		}
	} else {
		if( !UcDirSel ) {						// X Direction
			RegWriteA_lc898122_sharp( WH_EQSWX , 0x02 ) ;			// 0x0170
			RamWrite32A_lc898122_sharp( SXLMT, 0x00000000 ) ;		// 0x1477
		} else {								// Y Direction
			RegWriteA_lc898122_sharp( WH_EQSWY , 0x02 ) ;			// 0x0171
			RamWrite32A_lc898122_sharp( SYLMT, 0x00000000 ) ;		// 0x14F7
		}
	}
}



#ifdef	MODULE_CALIBRATION
//********************************************************************************
// Function Name 	: LopGan_lc898122_sharp
// Retun Value		: Execute Result
// Argment Value	: X,Y Direction
// Explanation		: Loop Gain Adjust Function
// History			: First edition 						2009.07.31 Y.Tashita
//********************************************************************************
unsigned char	LopGan_lc898122_sharp( unsigned char	UcDirSel )
{
	unsigned char	UcLpAdjSts ;
	
 #ifdef	HALLADJ_HW
	UcLpAdjSts	= LoopGainAdj_lc898122_sharp( UcDirSel ) ;
 #else
	MesFil_lc898122_sharp( LOOPGAIN ) ;

	// Servo ON
	SrvCon_lc898122_sharp( X_DIR, ON ) ;
	SrvCon_lc898122_sharp( Y_DIR, ON ) ;

	// Wait 300ms
	WitTim_lc898122_sharp( 300 ) ;

	// Loop Gain Adjust Initialize
	LopIni( UcDirSel ) ;

	// Loop Gain Adjust
	UcLpAdjSts	= LopAdj_lc898122_sharp( UcDirSel ) ;
 #endif
	// Servo OFF
	SrvCon_lc898122_sharp( X_DIR, OFF ) ;
	SrvCon_lc898122_sharp( Y_DIR, OFF ) ;

	if( !UcLpAdjSts ) {
		return( EXE_END ) ;
	} else {
		if( !UcDirSel ) {
			return( EXE_LXADJ ) ;
		} else {
			return( EXE_LYADJ ) ;
		}
	}
}



 #ifndef	HALLADJ_HW
//********************************************************************************
// Function Name 	: LopIni
// Retun Value		: NON
// Argment Value	: X,Y Direction
// Explanation		: Loop Gain Adjust Initialize Function
// History			: First edition 						2009.07.31 Y.Tashita
//********************************************************************************
void	LopIni( unsigned char	UcDirSel )
{
	// Loop Gain Value Initialize
	LopPar_lc898122_sharp( UcDirSel ) ;

	// Sign Wave Output Setting
	LopSin_lc898122_sharp( UcDirSel, ON ) ;

}
 #endif


//********************************************************************************
// Function Name 	: LopPar_lc898122_sharp
// Retun Value		: NON
// Argment Value	: X,Y Direction
// Explanation		: Loop Gain Adjust Parameter Initialize Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	LopPar_lc898122_sharp( unsigned char	UcDirSel )
{
	unsigned short	UsLopGan_lc898122_sharp ;

	RamAccFixMod_lc898122_sharp( ON ) ;						// Fix mode
	
	if( !UcDirSel ) {
		UsLopGan_lc898122_sharp	= SXGAIN_LOP ;
		RamWriteA_lc898122_sharp( sxg, UsLopGan_lc898122_sharp ) ;			/* 0x10D3 */
	} else {
		UsLopGan_lc898122_sharp	= SYGAIN_LOP ;
		RamWriteA_lc898122_sharp( syg, UsLopGan_lc898122_sharp ) ;			/* 0x11D3 */
	}

	RamAccFixMod_lc898122_sharp( OFF ) ;						// Float mode
}



 #ifndef	HALLADJ_HW
//********************************************************************************
// Function Name 	: LopSin_lc898122_sharp
// Retun Value		: NON
// Argment Value	: X,Y Direction, ON/OFF Switch
// Explanation		: Loop Gain Adjust Sign Wave Initialize Function
// History			: First edition 						2009.07.31 Y.Tashita
//********************************************************************************
void	LopSin_lc898122_sharp( unsigned char	UcDirSel, unsigned char	UcSonOff )
{
	unsigned short		UsFreqVal ;
	unsigned char		UcEqSwX , UcEqSwY ;
	
	RegReadA_lc898122_sharp( WH_EQSWX, &UcEqSwX ) ;				/* 0x0170	*/
	RegReadA_lc898122_sharp( WH_EQSWY, &UcEqSwY ) ;				/* 0x0171	*/
		
	if( UcSonOff ) {
		
  #ifdef	USE_EXTCLK_ALL	// 24MHz
		/* Freq = CmSinFrq * 11.718kHz / 65536 / 16 */
   #ifdef	ACTREG_6P5OHM
//		UsFreqVal	=	0x30EE ;				/* 139.9Hz */
		UsFreqVal	=	0x29F1 ;				/* 119.9Hz */
   #else
		UsFreqVal	=	0x3B6B ;				/* 169.9Hz */
   #endif
  #else
		/* Freq = CmSinFrq * 23.4375kHz / 65536 / 16 */
   #ifdef	ACTREG_6P5OHM
//		UsFreqVal	=	0x1877 ;				/* 139.9Hz */
//		UsFreqVal	=	0x16B7 ;				/* 130.0Hz */
		UsFreqVal	=	0x14F8 ;				/* 119.9Hz */
   #else
		UsFreqVal	=	0x1DB5 ;				/* 169.9Hz */
   #endif
  #endif
		
		RegWriteA_lc898122_sharp( WC_SINFRQ0,	(unsigned char)UsFreqVal ) ;				// 0x0181		Freq L
		RegWriteA_lc898122_sharp( WC_SINFRQ1,	(unsigned char)(UsFreqVal >> 8) ) ;			// 0x0182		Freq H
		
		if( !UcDirSel ) {

			UcEqSwX |= 0x10 ;
			UcEqSwY &= ~EQSINSW ;
			
//			RamWrite32A_lc898122_sharp( sxsin, 0x3CE6E1C0 ) ;				// 0x10D5		-31dB
//			RamWrite32A_lc898122_sharp( sxsin, 0x3CA3D70A ) ;				// 0x10D5		-34dB
                     RamWrite32A_lc898122_sharp( sxsin, 0x3BB84480 ) ;                     // 0x10D5		-37dB
//			RamWrite32A_lc898122_sharp( sxsin, 0x3BB84480 ) ;				// 0x10D5		-45dB
		} else {

			UcEqSwX &= ~EQSINSW ;
			UcEqSwY |= 0x10 ;
//			RamWrite32A_lc898122_sharp( sysin, 0x3CE6E1C0 ) ;				// 0x11D5		-31dB
//			RamWrite32A_lc898122_sharp( sysin, 0x3CA3D70A ) ;				// 0x11D5		-34dB
                     RamWrite32A_lc898122_sharp( sysin, 0x3BB84480 ) ;				                            // 0x11D5		       -37dB
//			RamWrite32A_lc898122_sharp( sysin, 0x3BB84480 ) ;				// 0x10D5		-45dB
		}
		RegWriteA_lc898122_sharp( WC_SINPHSX, 0x00 ) ;					/* 0x0183	X Sine phase */
		RegWriteA_lc898122_sharp( WC_SINPHSY, 0x00 ) ;					/* 0x0184	Y Sine phase */
		RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	Switch control */
		RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	Switch control */
		RegWriteA_lc898122_sharp( WC_SINON,     0x01 ) ;				/* 0x0180	Sine wave  */
	} else {
		UcEqSwX &= ~EQSINSW ;
		UcEqSwY &= ~EQSINSW ;
		RegWriteA_lc898122_sharp( WC_SINON,     0x00 ) ;				/* 0x0180	Sine wave  */
		if( !UcDirSel ) {
			RamWrite32A_lc898122_sharp( sxsin, 0x00000000 ) ;			// 0x10D5
		} else {
			RamWrite32A_lc898122_sharp( sysin, 0x00000000 ) ;			// 0x11D5
		}
		RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	Switch control */
		RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	Switch control */
	}
}



//********************************************************************************
// Function Name 	: LopAdj_lc898122_sharp
// Retun Value		: Command Status
// Argment Value	: X,Y Direction
// Explanation		: Loop Gain Adjust Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
unsigned char	LopAdj_lc898122_sharp( unsigned char	UcDirSel )
{
	unsigned char	UcAdjSts	= FAILURE ;
	unsigned short	UsRtnVal ;
	float			SfCmpVal ;
	unsigned char	UcIdxCnt ;
	unsigned char	UcIdxCn1 ;
	unsigned char	UcIdxCn2 ;
	UnFltVal		UnAdcXg1, UnAdcXg2 , UnRtnVa ;

	float			DfGanVal[ 5 ] ;
	float			DfTemVal ;

	if( !UcDirSel ) {
		RegWriteA_lc898122_sharp( WC_MES1ADD0, (unsigned char)SXGZ ) ;							// 0x0194
		RegWriteA_lc898122_sharp( WC_MES1ADD1, (unsigned char)(( SXGZ >> 8 ) & 0x0001 ) ) ;	// 0x0195
		RegWriteA_lc898122_sharp( WC_MES2ADD0, (unsigned char)SXG3Z ) ;						// 0x0196
		RegWriteA_lc898122_sharp( WC_MES2ADD1, (unsigned char)(( SXG3Z >> 8 ) & 0x0001 ) ) ;	// 0x0197
	} else {
		RegWriteA_lc898122_sharp( WC_MES1ADD0, (unsigned char)SYGZ ) ;							// 0x0194
		RegWriteA_lc898122_sharp( WC_MES1ADD1, (unsigned char)(( SYGZ >> 8 ) & 0x0001 ) ) ;	// 0x0195
		RegWriteA_lc898122_sharp( WC_MES2ADD0, (unsigned char)SYG3Z ) ;						// 0x0196
		RegWriteA_lc898122_sharp( WC_MES2ADD1, (unsigned char)(( SYG3Z >> 8 ) & 0x0001 ) ) ;	// 0x0197
	}
	
	// 5 Times Average Value Calculation
	for( UcIdxCnt = 0 ; UcIdxCnt < 5 ; UcIdxCnt++ )
	{
		LopMes_lc898122_sharp( ) ;																// Loop Gain Mesurement Start

		UnAdcXg1.UlLngVal	= GinMes_lc898122_sharp( MES_XG1 ) ;										// LXG1 Measure
		UnAdcXg2.UlLngVal	= GinMes_lc898122_sharp( MES_XG2 ) ;										// LXG2 Measure

		SfCmpVal	= UnAdcXg2.SfFltVal / UnAdcXg1.SfFltVal ;					// Compare Coefficient Value

		if( !UcDirSel ) {
			RamRead32A_lc898122_sharp( sxg, &UnRtnVa.UlLngVal ) ;									// 0x10D3
		} else {
			RamRead32A_lc898122_sharp( syg, &UnRtnVa.UlLngVal ) ;									// 0x11D3
		}
		UnRtnVa.SfFltVal	=  UnRtnVa.SfFltVal * SfCmpVal ;

		DfGanVal[ UcIdxCnt ]	= UnRtnVa.SfFltVal ;
	}

	for( UcIdxCn1 = 0 ; UcIdxCn1 < 4 ; UcIdxCn1++ )
	{
		for( UcIdxCn2 = UcIdxCn1+1 ; UcIdxCn2 < 5 ; UcIdxCn2++ )
		{
			if( DfGanVal[ UcIdxCn1 ] > DfGanVal[ UcIdxCn2 ] ) {
				DfTemVal				= DfGanVal[ UcIdxCn1 ] ;
				DfGanVal[ UcIdxCn1 ]	= DfGanVal[ UcIdxCn2 ] ;
				DfGanVal[ UcIdxCn2 ]	= DfTemVal ;
			}
		}
	}

	UnRtnVa.SfFltVal	=  ( DfGanVal[ 1 ] + DfGanVal[ 2 ] + DfGanVal[ 3 ] ) / 3  ;

	LopSin_lc898122_sharp( UcDirSel, OFF ) ;

	if( UnRtnVa.UlLngVal < 0x3F800000 ) {									// Adjust Error
		UcAdjSts	= SUCCESS ;												// Status OK
	}

	if( UcAdjSts ) {
		if( !UcDirSel ) {
			RamWrite32A_lc898122_sharp( sxg, 0x3F800000 ) ;						// 0x10D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgVal	= 0x7FFF ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgSts	= 0x0000 ;
		} else {
			RamWrite32A_lc898122_sharp( syg, 0x3F800000 ) ;						// 0x11D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygVal	= 0x7FFF ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygSts	= 0x0000 ;
		}
	} else {
		if( !UcDirSel ) {
			RamWrite32A_lc898122_sharp( sxg, UnRtnVa.UlLngVal ) ;						// 0x10D3
			RamAccFixMod_lc898122_sharp( ON ) ;								// Fix mode
			RamReadA_lc898122_sharp( sxg, &UsRtnVal ) ;						// 0x10D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgVal	= UsRtnVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgSts	= 0xFFFF ;
		} else {
			RamWrite32A_lc898122_sharp( syg, UnRtnVa.UlLngVal ) ;						// 0x11D3
			RamAccFixMod_lc898122_sharp( ON ) ;								// Fix mode
			RamReadA_lc898122_sharp( syg, &UsRtnVal ) ;						// 0x11D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygVal	= UsRtnVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygSts	= 0xFFFF ;
		}
		RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	}
	return( UcAdjSts ) ;
}


//********************************************************************************
// Function Name 	: LopMes_lc898122_sharp
// Retun Value		: void
// Argment Value	: void
// Explanation		: Loop Gain Adjust Measure Setting
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	LopMes_lc898122_sharp( void )
{
	ClrGyr_lc898122_sharp( 0x1000 , CLR_FRAM1 );					// Measure Filter RAM Clear
	RamWrite32A_lc898122_sharp( MSABS1AV, 0x00000000 ) ;			// 0x1041	Clear
	RamWrite32A_lc898122_sharp( MSABS2AV, 0x00000000 ) ;			// 0x1141	Clear
	RegWriteA_lc898122_sharp( WC_MESLOOP1, 0x04 ) ;				// 0x0193
	RegWriteA_lc898122_sharp( WC_MESLOOP0, 0x00 ) ;				// 0x0192	1024 Times Measure
	RamWrite32A_lc898122_sharp( msmean	, 0x3A800000 );				// 0x1230	1/CmMesLoop[15:0]
	RegWriteA_lc898122_sharp( WC_MESABS, 0x01 ) ;					// 0x0198	ABS
	RegWriteA_lc898122_sharp( WC_MESWAIT,     0x00 ) ;				/* 0x0199	0 cross wait */
	BsyWit_lc898122_sharp( WC_MESMODE, 0x01 ) ;					// 0x0190	Sin Wave Measure
}


//********************************************************************************
// Function Name 	: GinMes_lc898122_sharp
// Retun Value		: Measure Result
// Argment Value	: MES1/MES2 Select
// Explanation		: Measure Result Read
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
unsigned long	GinMes_lc898122_sharp( unsigned char	UcXg1Xg2 )
{
	unsigned long	UlMesVal ;

	if( !UcXg1Xg2 ) {
		RamRead32A_lc898122_sharp( MSABS1AV, &UlMesVal ) ;			// 0x1041
	} else {
		RamRead32A_lc898122_sharp( MSABS2AV, &UlMesVal ) ;			// 0x1141
	}
	
	return( UlMesVal ) ;
}

 #endif
#endif

//********************************************************************************
// Function Name 	: TneGvc_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Tunes the Gyro VC offset
// History			: First edition 						2013.01.15  Y.Shigeoka
//********************************************************************************
#define	LIMITH		0x0FA0
#define	LIMITL		0xF060
#define	INITVAL		0x0000
unsigned char	TneGvc_lc898122_sharp( void )
{
	unsigned char  UcRsltSts;
	
	
	// A/D Offset Clear
	RegWriteA_lc898122_sharp( IZAH,	(unsigned char)(INITVAL >> 8) ) ;	// 0x02A0		Set Offset High byte
	RegWriteA_lc898122_sharp( IZAL,	(unsigned char)INITVAL ) ;			// 0x02A1		Set Offset Low byte
	RegWriteA_lc898122_sharp( IZBH,	(unsigned char)(INITVAL >> 8) ) ;	// 0x02A2		Set Offset High byte
	RegWriteA_lc898122_sharp( IZBL,	(unsigned char)INITVAL ) ;			// 0x02A3		Set Offset Low byte
	
	MesFil_lc898122_sharp( THROUGH ) ;				// 測定用フィルターを設定する。
	//////////
	// X
	//////////
	RegWriteA_lc898122_sharp( WC_MES1ADD0, 0x00 ) ;		// 0x0194
	RegWriteA_lc898122_sharp( WC_MES1ADD1, 0x00 ) ;		// 0x0195
	ClrGyr_lc898122_sharp( 0x1000 , CLR_FRAM1 );					// Measure Filter RAM Clear
	StAdjPar_lc898122_sharp.StGvcOff.UsGxoVal = (unsigned short)GenMes_lc898122_sharp( AD2Z, 0 );		// 64回の平均値測定	GYRMON1(0x1110) <- GXADZ(0x144A)
	RegWriteA_lc898122_sharp( IZAH, (unsigned char)(StAdjPar_lc898122_sharp.StGvcOff.UsGxoVal >> 8) ) ;	// 0x02A0		Set Offset High byte
	RegWriteA_lc898122_sharp( IZAL, (unsigned char)(StAdjPar_lc898122_sharp.StGvcOff.UsGxoVal) ) ;		// 0x02A1		Set Offset Low byte
	//////////
	// Y
	//////////
	RegWriteA_lc898122_sharp( WC_MES1ADD0, 0x00 ) ;		// 0x0194
	RegWriteA_lc898122_sharp( WC_MES1ADD1, 0x00 ) ;		// 0x0195
	ClrGyr_lc898122_sharp( 0x1000 , CLR_FRAM1 );					// Measure Filter RAM Clear
	StAdjPar_lc898122_sharp.StGvcOff.UsGyoVal = (unsigned short)GenMes_lc898122_sharp( AD3Z, 0 );		// 64回の平均値測定	GYRMON2(0x1111) <- GYADZ(0x14CA)
	RegWriteA_lc898122_sharp( IZBH, (unsigned char)(StAdjPar_lc898122_sharp.StGvcOff.UsGyoVal >> 8) ) ;	// 0x02A2		Set Offset High byte
	RegWriteA_lc898122_sharp( IZBL, (unsigned char)(StAdjPar_lc898122_sharp.StGvcOff.UsGyoVal) ) ;		// 0x02A3		Set Offset Low byte
	
	UcRsltSts = EXE_END ;						/* Clear Status */

	StAdjPar_lc898122_sharp.StGvcOff.UsGxoSts	= 0xFFFF ;
	if(( (short)StAdjPar_lc898122_sharp.StGvcOff.UsGxoVal < (short)LIMITL ) || ( (short)StAdjPar_lc898122_sharp.StGvcOff.UsGxoVal > (short)LIMITH ))
	{
		UcRsltSts |= EXE_GXADJ ;
		StAdjPar_lc898122_sharp.StGvcOff.UsGxoSts	= 0x0000 ;
	}
	
	StAdjPar_lc898122_sharp.StGvcOff.UsGyoSts	= 0xFFFF ;
	if(( (short)StAdjPar_lc898122_sharp.StGvcOff.UsGyoVal < (short)LIMITL ) || ( (short)StAdjPar_lc898122_sharp.StGvcOff.UsGyoVal > (short)LIMITH ))
	{
		UcRsltSts |= EXE_GYADJ ;
		StAdjPar_lc898122_sharp.StGvcOff.UsGyoSts	= 0x0000 ;
	}
	return( UcRsltSts );
		
}



//********************************************************************************
// Function Name 	: RtnCen_lc898122_sharp
// Retun Value		: Command Status
// Argment Value	: Command Parameter
// Explanation		: Return to center Command Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
unsigned char	RtnCen_lc898122_sharp( unsigned char	UcCmdPar )
{
	unsigned char	UcCmdSts ;

	UcCmdSts	= EXE_END ;

	GyrCon_lc898122_sharp( OFF ) ;											// Gyro OFF

	if( !UcCmdPar ) {										// X,Y Centering

		StbOnn_lc898122_sharp() ;											// Slope Mode
		
	} else if( UcCmdPar == 0x01 ) {							// X Centering Only

		SrvCon_lc898122_sharp( X_DIR, ON ) ;								// X only Servo ON
		SrvCon_lc898122_sharp( Y_DIR, OFF ) ;
	} else if( UcCmdPar == 0x02 ) {							// Y Centering Only

		SrvCon_lc898122_sharp( X_DIR, OFF ) ;								// Y only Servo ON
		SrvCon_lc898122_sharp( Y_DIR, ON ) ;
	}

	return( UcCmdSts ) ;
}



//********************************************************************************
// Function Name 	: GyrCon_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Gyro Filter ON or OFF
// Explanation		: Gyro Filter Control Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	GyrCon_lc898122_sharp( unsigned char	UcGyrCon_lc898122_sharp )
{
	// Return HPF Setting
	RegWriteA_lc898122_sharp( WG_SHTON, 0x00 ) ;									// 0x0107
	
	if( UcGyrCon_lc898122_sharp == ON ) {												// Gyro ON

		
#ifdef	GAIN_CONT
		/* Gain3 Register */
//		AutoGainControlSw_lc898122_sharp( ON ) ;											/* Auto Gain Control Mode ON */
#endif
		ClrGyr_lc898122_sharp( 0x000E , CLR_FRAM1 );		// Gyro Delay RAM Clear

		RamWrite32A_lc898122_sharp( sxggf, 0x3F800000 ) ;	// 0x10B5
		RamWrite32A_lc898122_sharp( syggf, 0x3F800000 ) ;	// 0x11B5
		
	} else if( UcGyrCon_lc898122_sharp == SPC ) {										// Gyro ON for LINE

		
#ifdef	GAIN_CONT
		/* Gain3 Register */
//		AutoGainControlSw_lc898122_sharp( ON ) ;											/* Auto Gain Control Mode ON */
#endif

		RamWrite32A_lc898122_sharp( sxggf, 0x3F800000 ) ;	// 0x10B5
		RamWrite32A_lc898122_sharp( syggf, 0x3F800000 ) ;	// 0x11B5
		

	} else {															// Gyro OFF
		
		RamWrite32A_lc898122_sharp( sxggf, 0x00000000 ) ;	// 0x10B5
		RamWrite32A_lc898122_sharp( syggf, 0x00000000 ) ;	// 0x11B5
		

#ifdef	GAIN_CONT
		/* Gain3 Register */
//		AutoGainControlSw_lc898122_sharp( OFF ) ;											/* Auto Gain Control Mode OFF */
#endif
	}
}



//********************************************************************************
// Function Name 	: OisEna_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	OisEna_lc898122_sharp( void )
{
	// Servo ON
	SrvCon_lc898122_sharp( X_DIR, ON ) ;
	SrvCon_lc898122_sharp( Y_DIR, ON ) ;

	GyrCon_lc898122_sharp( ON ) ;
}

//********************************************************************************
// Function Name 	: OisEna_lc898122_sharpLin_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: OIS Enable Control Function for Line adjustment
// History			: First edition 						2013.09.05 Y.Shigeoka
//********************************************************************************
void	OisEna_lc898122_sharpLin_lc898122_sharp( void )
{
	// Servo ON
	SrvCon_lc898122_sharp( X_DIR, ON ) ;
	SrvCon_lc898122_sharp( Y_DIR, ON ) ;

	GyrCon_lc898122_sharp( SPC ) ;
}



//********************************************************************************
// Function Name 	: TimPro_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Timer Interrupt Process Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	TimPro_lc898122_sharp( void )
{
#ifdef	MODULE_CALIBRATION
	if( UcOscAdjFlg_lc898122_sharp )
	{
		if( UcOscAdjFlg_lc898122_sharp == MEASSTR )
		{
			RegWriteA_lc898122_sharp( OSCCNTEN, 0x01 ) ;		// 0x0258	OSC Cnt enable
			UcOscAdjFlg_lc898122_sharp = MEASCNT ;
		}
		else if( UcOscAdjFlg_lc898122_sharp == MEASCNT )
		{
			RegWriteA_lc898122_sharp( OSCCNTEN, 0x00 ) ;		// 0x0258	OSC Cnt disable
			UcOscAdjFlg_lc898122_sharp = MEASFIX ;
		}
	}
#endif
}



//********************************************************************************
// Function Name 	: S2cPro_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: S2 Command Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	S2cPro_lc898122_sharp( unsigned char uc_mode )
{
	if( uc_mode == 1 )
	{
#ifdef H1COEF_CHANGER
		SetH1cMod_lc898122_sharp( S2MODE ) ;							/* cancel Lvl change */
#endif
		// HPF→Through Setting
		RegWriteA_lc898122_sharp( WG_SHTON, 0x11 ) ;							// 0x0107
		RamWrite32A_lc898122_sharp( gxh1c, DIFIL_S2 );							// 0x1012
		RamWrite32A_lc898122_sharp( gyh1c, DIFIL_S2 );							// 0x1112
	}
	else
	{
		RamWrite32A_lc898122_sharp( gxh1c, UlH1Coefval_lc898122_sharp );							// 0x1012
		RamWrite32A_lc898122_sharp( gyh1c, UlH1Coefval_lc898122_sharp );							// 0x1112
		// HPF→Through Setting
		RegWriteA_lc898122_sharp( WG_SHTON, 0x00 ) ;							// 0x0107

#ifdef H1COEF_CHANGER
		SetH1cMod_lc898122_sharp( UcH1LvlMod_lc898122_sharp ) ;							/* Re-setting */
#endif
	}
	
}


//********************************************************************************
// Function Name 	: GenMes_lc898122_sharp
// Retun Value		: A/D Convert Result
// Argment Value	: Measure Filter Input Signal Ram Address
// Explanation		: General Measure Function
// History			: First edition 						2013.01.10 Y.Shigeoka
//********************************************************************************
short	GenMes_lc898122_sharp( unsigned short	UsRamAdd, unsigned char	UcMesMod )
{
	short	SsMesRlt ;

	RegWriteA_lc898122_sharp( WC_MES1ADD0, (unsigned char)UsRamAdd ) ;							// 0x0194
	RegWriteA_lc898122_sharp( WC_MES1ADD1, (unsigned char)(( UsRamAdd >> 8 ) & 0x0001 ) ) ;	// 0x0195
	RamWrite32A_lc898122_sharp( MSABS1AV, 0x00000000 ) ;				// 0x1041	Clear
	
	if( !UcMesMod ) {
		RegWriteA_lc898122_sharp( WC_MESLOOP1, 0x04 ) ;				// 0x0193
		RegWriteA_lc898122_sharp( WC_MESLOOP0, 0x00 ) ;				// 0x0192	1024 Times Measure
		RamWrite32A_lc898122_sharp( msmean	, 0x3A7FFFF7 );				// 0x1230	1/CmMesLoop[15:0]
	} else {
		RegWriteA_lc898122_sharp( WC_MESLOOP1, 0x01 ) ;				// 0x0193
		RegWriteA_lc898122_sharp( WC_MESLOOP0, 0x00 ) ;				// 0x0192	1 Times Measure
		RamWrite32A_lc898122_sharp( msmean	, 0x3F800000 );				// 0x1230	1/CmMesLoop[15:0]
	}

	RegWriteA_lc898122_sharp( WC_MESABS, 0x00 ) ;						// 0x0198	none ABS
	BsyWit_lc898122_sharp( WC_MESMODE, 0x01 ) ;						// 0x0190	normal Measure

	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	RamReadA_lc898122_sharp( MSABS1AV, ( unsigned short * )&SsMesRlt ) ;	// 0x1041

	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
	return( SsMesRlt ) ;
}


//********************************************************************************
// Function Name 	: SetSinWavePara_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Sine wave Test Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
#ifdef	USE_EXTCLK_ALL	// 24MHz
	/********* Parameter Setting *********/
	/* Servo Sampling Clock		=	11.71875kHz						*/
	/* Freq						=	CmSinFreq*Fs/65536/16			*/
	/* 05 00 XX MM 				XX:Freq MM:Sin or Circle */
const unsigned short	CucFreqVal_lc898122_sharp[ 17 ]	= {
		0xFFFF,				//  0:  Stop
		0x0059,				//  1: 0.994653Hz
		0x00B2,				//  2: 1.989305Hz
		0x010C,				//  3: 2.995133Hz	
		0x0165,				//  4: 3.989786Hz
		0x01BF,				//  5: 4.995614Hz
		0x0218,				//  6: 5.990267Hz
		0x0272,				//  7: 6.996095Hz
		0x02CB,				//  8: 7.990748Hz
		0x0325,				//  9: 8.996576Hz
		0x037E,				//  A: 9.991229Hz
		0x03D8,				//  B: 10.99706Hz
		0x0431,				//  C: 11.99171Hz
		0x048B,				//  D: 12.99754Hz
		0x04E4,				//  E: 13.99219Hz
		0x053E,				//  F: 14.99802Hz
		0x0597				// 10: 15.99267Hz
	} ;
#else
	/********* Parameter Setting *********/
	/* Servo Sampling Clock		=	23.4375kHz						*/
	/* Freq						=	CmSinFreq*Fs/65536/16			*/
	/* 05 00 XX MM 				XX:Freq MM:Sin or Circle */
const unsigned short	CucFreqVal_lc898122_sharp[ 17 ]	= {
		0xFFFF,				//  0:  Stop
		0x002C,				//  1: 0.983477Hz
		0x0059,				//  2: 1.989305Hz
		0x0086,				//  3: 2.995133Hz	
		0x00B2,				//  4: 3.97861Hz
		0x00DF,				//  5: 4.984438Hz
		0x010C,				//  6: 5.990267Hz
		0x0139,				//  7: 6.996095Hz
		0x0165,				//  8: 7.979572Hz
		0x0192,				//  9: 8.9854Hz
		0x01BF,				//  A: 9.991229Hz
		0x01EC,				//  B: 10.99706Hz
		0x0218,				//  C: 11.98053Hz
		0x0245,				//  D: 12.98636Hz
		0x0272,				//  E: 13.99219Hz
		0x029F,				//  F: 14.99802Hz
		0x02CB				// 10: 15.9815Hz
	} ;
#endif
	
#define		USE_SINLPF			/* if sin or circle movement is used LPF , this define has to enable */
	
/* 振幅はsxsin(0x10D5),sysin(0x11D5)で調整 */
void	SetSinWavePara_lc898122_sharp( unsigned char UcTableVal ,  unsigned char UcMethodVal )
{
	unsigned short	UsFreqDat ;
	unsigned char	UcEqSwX , UcEqSwY ;

	
	if(UcTableVal > 0x10 )
		UcTableVal = 0x10 ;			/* Limit */
	UsFreqDat = CucFreqVal_lc898122_sharp[ UcTableVal ] ;	
	
	if( UcMethodVal == SINEWAVE) {
		RegWriteA_lc898122_sharp( WC_SINPHSX, 0x00 ) ;					/* 0x0183	*/
		RegWriteA_lc898122_sharp( WC_SINPHSY, 0x00 ) ;					/* 0x0184	*/
	}else if( UcMethodVal == CIRCWAVE ){
		RegWriteA_lc898122_sharp( WC_SINPHSX,	0x00 ) ;				/* 0x0183	*/
		RegWriteA_lc898122_sharp( WC_SINPHSY,	0x20 ) ;				/* 0x0184	*/
	}else{
		RegWriteA_lc898122_sharp( WC_SINPHSX, 0x00 ) ;					/* 0x0183	*/
		RegWriteA_lc898122_sharp( WC_SINPHSY, 0x00 ) ;					/* 0x0184	*/
	}

#ifdef	USE_SINLPF
	if(( UcMethodVal != XHALWAVE ) && ( UcMethodVal != YHALWAVE )) {
		MesFil_lc898122_sharp( NOISE ) ;			/* LPF */
	}
#endif

	if( UsFreqDat == 0xFFFF )			/* Sine波中止 */
	{

		RegReadA_lc898122_sharp( WH_EQSWX, &UcEqSwX ) ;				/* 0x0170	*/
		RegReadA_lc898122_sharp( WH_EQSWY, &UcEqSwY ) ;				/* 0x0171	*/
		UcEqSwX &= ~EQSINSW ;
		UcEqSwY &= ~EQSINSW ;
		RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	*/
		RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	*/
		
#ifdef	USE_SINLPF
		if(( UcMethodVal != XHALWAVE ) && ( UcMethodVal != YHALWAVE )) {
			RegWriteA_lc898122_sharp( WC_DPON,     0x00 ) ;			/* 0x0105	Data pass off */
			RegWriteA_lc898122_sharp( WC_DPO1ADD0, 0x00 ) ;			/* 0x01B8	output initial */
			RegWriteA_lc898122_sharp( WC_DPO1ADD1, 0x00 ) ;			/* 0x01B9	output initial */
			RegWriteA_lc898122_sharp( WC_DPO2ADD0, 0x00 ) ;			/* 0x01BA	output initial */
			RegWriteA_lc898122_sharp( WC_DPO2ADD1, 0x00 ) ;			/* 0x01BB	output initial */
			RegWriteA_lc898122_sharp( WC_DPI1ADD0, 0x00 ) ;			/* 0x01B0	input initial */
			RegWriteA_lc898122_sharp( WC_DPI1ADD1, 0x00 ) ;			/* 0x01B1	input initial */
			RegWriteA_lc898122_sharp( WC_DPI2ADD0, 0x00 ) ;			/* 0x01B2	input initial */
			RegWriteA_lc898122_sharp( WC_DPI2ADD1, 0x00 ) ;			/* 0x01B3	input initial */
			
			/* Ram Access */
			RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
			
			RamWriteA_lc898122_sharp( SXOFFZ1, UsCntXof_lc898122_sharp ) ;			/* 0x1461	set optical value */
			RamWriteA_lc898122_sharp( SYOFFZ1, UsCntYof_lc898122_sharp ) ;			/* 0x14E1	set optical value */
			
			/* Ram Access */
			RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
			RegWriteA_lc898122_sharp( WC_MES1ADD0,  0x00 ) ;			/* 0x0194	*/
			RegWriteA_lc898122_sharp( WC_MES1ADD1,  0x00 ) ;			/* 0x0195	*/
			RegWriteA_lc898122_sharp( WC_MES2ADD0,  0x00 ) ;			/* 0x0196	*/
			RegWriteA_lc898122_sharp( WC_MES2ADD1,  0x00 ) ;			/* 0x0197	*/
			
		}
#endif
		RegWriteA_lc898122_sharp( WC_SINON,     0x00 ) ;			/* 0x0180	Sine wave  */
		
	}
	else
	{
		
		RegReadA_lc898122_sharp( WH_EQSWX, &UcEqSwX ) ;				/* 0x0170	*/
		RegReadA_lc898122_sharp( WH_EQSWY, &UcEqSwY ) ;				/* 0x0171	*/
		
		if(( UcMethodVal != XHALWAVE ) && ( UcMethodVal != YHALWAVE )) {
#ifdef	USE_SINLPF
			RegWriteA_lc898122_sharp( WC_DPI1ADD0,  ( unsigned char )MES1BZ2 ) ;						/* 0x01B0	input Meas-Fil */
			RegWriteA_lc898122_sharp( WC_DPI1ADD1,  ( unsigned char )(( MES1BZ2 >> 8 ) & 0x0001 ) ) ;	/* 0x01B1	input Meas-Fil */
			RegWriteA_lc898122_sharp( WC_DPI2ADD0,  ( unsigned char )MES2BZ2 ) ;						/* 0x01B2	input Meas-Fil */
			RegWriteA_lc898122_sharp( WC_DPI2ADD1,  ( unsigned char )(( MES2BZ2 >> 8 ) & 0x0001 ) ) ;	/* 0x01B3	input Meas-Fil */
			RegWriteA_lc898122_sharp( WC_DPO1ADD0, ( unsigned char )SXOFFZ1 ) ;						/* 0x01B8	output SXOFFZ1 */
			RegWriteA_lc898122_sharp( WC_DPO1ADD1, ( unsigned char )(( SXOFFZ1 >> 8 ) & 0x0001 ) ) ;	/* 0x01B9	output SXOFFZ1 */
			RegWriteA_lc898122_sharp( WC_DPO2ADD0, ( unsigned char )SYOFFZ1 ) ;						/* 0x01BA	output SYOFFZ1 */
			RegWriteA_lc898122_sharp( WC_DPO2ADD1, ( unsigned char )(( SYOFFZ1 >> 8 ) & 0x0001 ) ) ;	/* 0x01BA	output SYOFFZ1 */
			
			RegWriteA_lc898122_sharp( WC_MES1ADD0,  ( unsigned char )SINXZ ) ;							/* 0x0194	*/
			RegWriteA_lc898122_sharp( WC_MES1ADD1,  ( unsigned char )(( SINXZ >> 8 ) & 0x0001 ) ) ;	/* 0x0195	*/
			RegWriteA_lc898122_sharp( WC_MES2ADD0,  ( unsigned char )SINYZ ) ;							/* 0x0196	*/
			RegWriteA_lc898122_sharp( WC_MES2ADD1,  ( unsigned char )(( SINYZ >> 8 ) & 0x0001 ) ) ;	/* 0x0197	*/
			
			RegWriteA_lc898122_sharp( WC_DPON,     0x03 ) ;			/* 0x0105	Data pass[1:0] on */
			
			UcEqSwX &= ~EQSINSW ;
			UcEqSwY &= ~EQSINSW ;
#else
			UcEqSwX |= 0x08 ;
			UcEqSwY |= 0x08 ;
#endif

		}else{
			if( UcMethodVal == XHALWAVE ){
		    	UcEqSwX = 0x22 ;				/* SW[5] */
//		    	UcEqSwY = 0x03 ;
			}else{
//				UcEqSwX = 0x03 ;
				UcEqSwY = 0x22 ;				/* SW[5] */
			}
		}
		
		RegWriteA_lc898122_sharp( WC_SINFRQ0,	(unsigned char)UsFreqDat ) ;				// 0x0181		Freq L
		RegWriteA_lc898122_sharp( WC_SINFRQ1,	(unsigned char)(UsFreqDat >> 8) ) ;			// 0x0182		Freq H
		RegWriteA_lc898122_sharp( WC_MESSINMODE,     0x00 ) ;			/* 0x0191	Sine 0 cross  */

		RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	*/
		RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	*/

		RegWriteA_lc898122_sharp( WC_SINON,     0x01 ) ;			/* 0x0180	Sine wave  */
		
	}
	
	
}




#ifdef STANDBY_MODE
//********************************************************************************
// Function Name 	: SetStandby_lc898122_sharp
// Retun Value		: NON
// Argment Value	: 0:Standby ON 1:Standby OFF 2:Standby2 ON 3:Standby2 OFF 
//					: 4:Standby3 ON 5:Standby3 OFF
// Explanation		: Set Standby
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	SetStandby_lc898122_sharp( unsigned char UcContMode )
{
	unsigned char	UcStbb0 , UcClkon ;
	
	switch(UcContMode)
	{
	case STB1_ON:

#ifdef	AF_PWMMODE
#else
		RegWriteA_lc898122_sharp( DRVFCAF	, 0x00 );				// 0x0081	Drv.MODEAF=0,Drv.ENAAF=0,MODE-0
#endif
		RegWriteA_lc898122_sharp( STBB0 	, 0x00 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( STBB1 	, 0x00 );		// 0x0264 	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( PWMA 	, 0x00 );		// 0x0010		PWM Standby
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( CVA,  0x00 ) ;			/* 0x0020	LINEAR PWM mode standby	*/
		DrvSw_lc898122_sharp( OFF ) ;						/* Driver OFF */
		AfDrvSw_lc898122_sharp( OFF ) ;					/* AF Driver OFF */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
//		RegWriteA_lc898122_sharp( DACMONFC, 0x01 ) ;		// 0x0032	DAC Monitor Standby
		SelectGySleep_lc898122_sharp( ON ) ;				/* Gyro Sleep */
		break ;
	case STB1_OFF:
		SelectGySleep_lc898122_sharp( OFF ) ;				/* Gyro Wake Up */
//		RegWriteA_lc898122_sharp( DACMONFC, 0x81 ) ;		// 0x0032	DAC Monitor Active
		RegWriteA_lc898122_sharp( PWMMONA, 0x80 ) ;		/* 0x0030	Monitor Active	*/
		DrvSw_lc898122_sharp( ON ) ;						/* Driver Mode setting */
		AfDrvSw_lc898122_sharp( ON ) ;						/* AF Driver Mode setting */
		RegWriteA_lc898122_sharp( CVA		, 0xC0 );		// 0x0020	Linear PWM mode enable
		
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( PWMA		, 0xC0 );		// 0x0010	PWM enable
		RegWriteA_lc898122_sharp( STBB1	, 0x05 ) ;		// 0x0264	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( STBB0	, 0xDF );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
	case STB2_ON:
#ifdef	AF_PWMMODE
#else
		RegWriteA_lc898122_sharp( DRVFCAF	, 0x00 );				// 0x0081	Drv.MODEAF=0,Drv.ENAAF=0,MODE-0
#endif
		RegWriteA_lc898122_sharp( STBB0 	, 0x00 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( STBB1 	, 0x00 );		// 0x0264 	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( PWMA 	, 0x00 );		// 0x0010		PWM Standby
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( CVA,  0x00 ) ;			/* 0x0020	LINEAR PWM mode standby	*/
		DrvSw_lc898122_sharp( OFF ) ;						/* Drvier Block Ena=0 */
		AfDrvSw_lc898122_sharp( OFF ) ;					/* AF Drvier Block Ena=0 */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
//		RegWriteA_lc898122_sharp( DACMONFC, 0x01 ) ;		// 0x0032	DAC Monitor Standby
		SelectGySleep_lc898122_sharp( ON ) ;				/* Gyro Sleep */
		RegWriteA_lc898122_sharp( CLKON, 0x00 ) ;			/* 0x020B	Servo & PWM Clock OFF + D-Gyro I/F OFF	*/
		break ;
	case STB2_OFF:
		RegWriteA_lc898122_sharp( CLKON,	0x1F ) ;		// 0x020B	[ - | - | CmOpafClkOn | CmAfpwmClkOn | CMGifClkOn  | CmScmClkOn  | CmSrvClkOn  | CmPwmClkOn  ]
		SelectGySleep_lc898122_sharp( OFF ) ;				/* Gyro Wake Up */
//		RegWriteA_lc898122_sharp( DACMONFC, 0x81 ) ;		// 0x0032	DAC Monitor Active
		RegWriteA_lc898122_sharp( PWMMONA, 0x80 ) ;		/* 0x0030	Monitor Active	*/
		DrvSw_lc898122_sharp( ON ) ;						/* Driver Mode setting */
		AfDrvSw_lc898122_sharp( ON ) ;						/* AF Driver Mode setting */
		RegWriteA_lc898122_sharp( CVA, 	0xC0 );			// 0x0020	Linear PWM mode enable
		
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( PWMA	, 	0xC0 );			// 0x0010	PWM enable
		RegWriteA_lc898122_sharp( STBB1	, 0x05 ) ;		// 0x0264	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( STBB0	, 0xDF );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
	case STB3_ON:
#ifdef	AF_PWMMODE
#else
		RegWriteA_lc898122_sharp( DRVFCAF	, 0x00 );				// 0x0081	Drv.MODEAF=0,Drv.ENAAF=0,MODE-0
#endif
		RegWriteA_lc898122_sharp( STBB0 	, 0x00 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( STBB1 	, 0x00 );		// 0x0264 	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( PWMA 	, 0x00 );			// 0x0010		PWM Standby
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( CVA,  0x00 ) ;			/* 0x0020	LINEAR PWM mode standby	*/
		DrvSw_lc898122_sharp( OFF ) ;						/* Drvier Block Ena=0 */
		AfDrvSw_lc898122_sharp( OFF ) ;					/* AF Drvier Block Ena=0 */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
//		RegWriteA_lc898122_sharp( DACMONFC, 0x01 ) ;		// 0x0032	DAC Monitor Standby
		SelectGySleep_lc898122_sharp( ON ) ;				/* Gyro Sleep */
		RegWriteA_lc898122_sharp( CLKON, 0x00 ) ;			/* 0x020B	Servo & PWM Clock OFF + D-Gyro I/F OFF	*/
		RegWriteA_lc898122_sharp( I2CSEL, 0x01 ) ;			/* 0x0248	I2C Noise Cancel circuit OFF	*/
		RegWriteA_lc898122_sharp( OSCSTOP, 0x02 ) ;		// 0x0256	Source Clock Input OFF
		break ;
	case STB3_OFF:
		RegWriteA_lc898122_sharp( OSCSTOP, 0x00 ) ;		// 0x0256	Source Clock Input ON
		RegWriteA_lc898122_sharp( I2CSEL, 0x00 ) ;			/* 0x0248	I2C Noise Cancel circuit ON	*/
		RegWriteA_lc898122_sharp( CLKON,	0x1F ) ;		// 0x020B	[ - | - | - | - | CMGifClkOn  | CmScmClkOn  | CmSrvClkOn  | CmPwmClkOn  ]
		SelectGySleep_lc898122_sharp( OFF ) ;				/* Gyro Wake Up */
//		RegWriteA_lc898122_sharp( DACMONFC, 0x81 ) ;		// 0x0032	DAC Monitor Active
		RegWriteA_lc898122_sharp( PWMMONA, 0x80 ) ;		/* 0x0030	Monitor Active	*/
		DrvSw_lc898122_sharp( ON ) ;						/* Driver Mode setting */
		AfDrvSw_lc898122_sharp( ON ) ;						/* AF Driver Mode setting */
		RegWriteA_lc898122_sharp( CVA, 	0xC0 );			// 0x0020	Linear PWM mode enable
		
		RegWriteA_lc898122_sharp( PWMAAF,	0x00 );			// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( PWMA	, 	0xC0 );			// 0x0010	PWM enable
		RegWriteA_lc898122_sharp( STBB1	, 0x05 ) ;		// 0x0264	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( STBB0	, 0xDF );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
		
	case STB4_ON:
#ifdef	AF_PWMMODE
#else
		RegWriteA_lc898122_sharp( DRVFCAF	, 0x00 );				// 0x0081	Drv.MODEAF=0,Drv.ENAAF=0,MODE-0
#endif
		RegWriteA_lc898122_sharp( STBB0 	, 0x00 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( STBB1 	, 0x00 );		// 0x0264 	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( PWMA 	, 0x00 );		// 0x0010		PWM Standby
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( CVA,  	0x00 ) ;		/* 0x0020	LINEAR PWM mode standby	*/
		DrvSw_lc898122_sharp( OFF ) ;						/* Drvier Block Ena=0 */
		AfDrvSw_lc898122_sharp( OFF ) ;					/* AF Drvier Block Ena=0 */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
//		RegWriteA_lc898122_sharp( DACMONFC, 0x01 ) ;		// 0x0032	DAC Monitor Standby
		GyOutSignalCont_lc898122_sharp( ) ;				/* Gyro Continuos mode */
		RegWriteA_lc898122_sharp( CLKON, 0x04 ) ;			/* 0x020B	Servo & PWM Clock OFF + D-Gyro I/F ON	*/
		break ;
	case STB4_OFF:
		RegWriteA_lc898122_sharp( CLKON,	0x1F ) ;		// 0x020B	[ - | - | - | - | CMGifClkOn  | CmScmClkOn  | CmSrvClkOn  | CmPwmClkOn  ]
		SelectGySleep_lc898122_sharp( OFF ) ;				/* Gyro OIS mode */
//		RegWriteA_lc898122_sharp( DACMONFC, 0x81 ) ;		// 0x0032	DAC Monitor Active
		RegWriteA_lc898122_sharp( PWMMONA, 0x80 ) ;		/* 0x0030	Monitor Active	*/
		DrvSw_lc898122_sharp( ON ) ;						/* Driver Mode setting */
		AfDrvSw_lc898122_sharp( ON ) ;						/* AF Driver Mode setting */
		RegWriteA_lc898122_sharp( CVA, 	0xC0 );			// 0x0020	Linear PWM mode enable
		
		RegWriteA_lc898122_sharp( PWMAAF, 	0x00 );			// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( PWMA	, 	0xC0 );			// 0x0010	PWM enable
		RegWriteA_lc898122_sharp( STBB1	, 0x05 ) ;		// 0x0264	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegWriteA_lc898122_sharp( STBB0	, 0xDF );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
		
		/************** special mode ************/
	case STB2_OISON:
		RegReadA_lc898122_sharp( STBB0 	, &UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		UcStbb0 &= 0x80 ;
		RegWriteA_lc898122_sharp( STBB0 	, UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( PWMA 	, 0x00 );		// 0x0010		PWM Standby
		RegWriteA_lc898122_sharp( CVA,  0x00 ) ;			/* 0x0020	LINEAR PWM mode standby	*/
		DrvSw_lc898122_sharp( OFF ) ;						/* Drvier Block Ena=0 */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
//		RegWriteA_lc898122_sharp( DACMONFC, 0x01 ) ;		// 0x0032	DAC Monitor Standby
		SelectGySleep_lc898122_sharp( ON ) ;				/* Gyro Sleep */
		RegReadA_lc898122_sharp( CLKON, &UcClkon ) ;		/* 0x020B	PWM Clock OFF + D-Gyro I/F OFF	SRVCLK can't OFF */
		UcClkon &= 0x1A ;
		RegWriteA_lc898122_sharp( CLKON, UcClkon ) ;		/* 0x020B	PWM Clock OFF + D-Gyro I/F OFF	SRVCLK can't OFF */
		break ;
	case STB2_OISOFF:
		RegReadA_lc898122_sharp( CLKON, &UcClkon ) ;		/* 0x020B	PWM Clock OFF + D-Gyro I/F ON  */
		UcClkon |= 0x05 ;
		RegWriteA_lc898122_sharp( CLKON,	UcClkon ) ;		// 0x020B	[ - | - | CmOpafClkOn | CmAfpwmClkOn | CMGifClkOn  | CmScmClkOn  | CmSrvClkOn  | CmPwmClkOn  ]
		SelectGySleep_lc898122_sharp( OFF ) ;				/* Gyro Wake Up */
//		RegWriteA_lc898122_sharp( DACMONFC, 0x81 ) ;		// 0x0032	DAC Monitor Active
		RegWriteA_lc898122_sharp( PWMMONA, 0x80 ) ;		/* 0x0030	Monitor Active	*/
		DrvSw_lc898122_sharp( ON ) ;						/* Driver Mode setting */
		RegWriteA_lc898122_sharp( CVA, 	0xC0 );			// 0x0020	Linear PWM mode enable
		RegWriteA_lc898122_sharp( PWMA	, 	0xC0 );			// 0x0010	PWM enable
		RegReadA_lc898122_sharp( STBB0	, &UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		UcStbb0 |= 0x5F ;
		RegWriteA_lc898122_sharp( STBB0	, UcStbb0 );	// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
		
	case STB2_AFON:
#ifdef	AF_PWMMODE
#else
		RegWriteA_lc898122_sharp( DRVFCAF	, 0x00 );				// 0x0081	Drv.MODEAF=0,Drv.ENAAF=0,MODE-0
#endif
		RegReadA_lc898122_sharp( STBB0 	, &UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		UcStbb0 &= 0x7F ;
		RegWriteA_lc898122_sharp( STBB0 	, UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		RegWriteA_lc898122_sharp( STBB1 	, 0x00 );		// 0x0264 	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
              //#ifdef	AF_PWMMODE
		//RegWriteA_lc898122_sharp( PWMAAF 	, 0x80 );		// 0x0090	AF PWM enable
              //#else	//AF_PWMMODE
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		//#endif	//AF_PWMMODE
		AfDrvSw_lc898122_sharp( OFF ) ;					/* AF Drvier Block Ena=0 */
#ifdef	MONITOR_OFF
#else
		RegWriteA_lc898122_sharp( PWMMONA, 0x00 ) ;		// 0x0030	Monitor Standby
#endif
		RegReadA_lc898122_sharp( CLKON, &UcClkon ) ;		/* 0x020B	OPAF Clock OFF + AFPWM OFF	SRVCLK can't OFF	*/
		UcClkon &= 0x07 ;
		RegWriteA_lc898122_sharp( CLKON, UcClkon ) ;		/* 0x020B	OPAF Clock OFF + AFPWM OFF	SRVCLK can't OFF	*/
		break ;
	case STB2_AFOFF:
		RegReadA_lc898122_sharp( CLKON, &UcClkon ) ;		/* 0x020B	OPAF Clock ON + AFPWM ON  */
		UcClkon |= 0x18 ;
		RegWriteA_lc898122_sharp( CLKON,	UcClkon ) ;		// 0x020B	[ - | - | CmOpafClkOn | CmAfpwmClkOn | CMGifClkOn  | CmScmClkOn  | CmSrvClkOn  | CmPwmClkOn  ]
		AfDrvSw_lc898122_sharp( ON ) ;						/* AF Driver Mode setting */
		RegWriteA_lc898122_sharp( PWMAAF 	, 0x00 );		// 0x0090		AF PWM Standby
		RegWriteA_lc898122_sharp( STBB1	, 0x05 ) ;		// 0x0264	[ - | - | - | - ][ - | STBAFOP1 | - | STBAFDAC ]
		RegReadA_lc898122_sharp( STBB0	, &UcStbb0 );		// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		UcStbb0 |= 0x80 ;
		RegWriteA_lc898122_sharp( STBB0	, UcStbb0 );	// 0x0250 	[ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ]
		break ;
		/************** special mode ************/
	}
}
#endif

//********************************************************************************
// Function Name 	: SetZsp_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Set Zoom Step parameter Function
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	SetZsp_lc898122_sharp( unsigned char	UcZoomStepDat )
{
	unsigned long	UlGyrZmx, UlGyrZmy, UlGyrZrx, UlGyrZry ;

	
	/* Zoom Step */
	if(UcZoomStepDat > (ZOOMTBL - 1))
		UcZoomStepDat = (ZOOMTBL -1) ;										/* 上限をZOOMTBL-1に設定する */

	if( UcZoomStepDat == 0 )				/* initial setting	*/
	{
		UlGyrZmx	= ClGyxZom_lc898122_sharp[ 0 ] ;		// Same Wide Coefficient
		UlGyrZmy	= ClGyyZom_lc898122_sharp[ 0 ] ;		// Same Wide Coefficient
		/* Initial Rate value = 1 */
	}
	else
	{
		UlGyrZmx	= ClGyxZom_lc898122_sharp[ UcZoomStepDat ] ;
		UlGyrZmy	= ClGyyZom_lc898122_sharp[ UcZoomStepDat ] ;
		
		
	}
	
	// Zoom Value Setting
	RamWrite32A_lc898122_sharp( gxlens, UlGyrZmx ) ;		/* 0x1022 */
	RamWrite32A_lc898122_sharp( gylens, UlGyrZmy ) ;		/* 0x1122 */

	RamRead32A_lc898122_sharp( gxlens, &UlGyrZrx ) ;		/* 0x1022 */
	RamRead32A_lc898122_sharp( gylens, &UlGyrZry ) ;		/* 0x1122 */

	// Zoom Value Setting Error Check
	if( UlGyrZmx != UlGyrZrx ) {
		RamWrite32A_lc898122_sharp( gxlens, UlGyrZmx ) ;		/* 0x1022 */
	}

	if( UlGyrZmy != UlGyrZry ) {
		RamWrite32A_lc898122_sharp( gylens, UlGyrZmy ) ;		/* 0x1122 */
	}

}

//********************************************************************************
// Function Name 	: StbOnn_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Stabilizer For Servo On Function
// History			: First edition 						2013.01.09 Y.Shigeoka
//********************************************************************************
 
void StbOnn_lc898122_sharp( void )
{
	unsigned char	UcRegValx,UcRegValy;					// Registor value 
	unsigned char	UcRegIni ;
#ifdef CONFIG_IMX214_OIS_SHARP
	int zte_times = 500;
#endif

	RegReadA_lc898122_sharp( WH_EQSWX , &UcRegValx ) ;			// 0x0170
	RegReadA_lc898122_sharp( WH_EQSWY , &UcRegValy ) ;			// 0x0171
	
	if( (( UcRegValx & 0x01 ) != 0x01 ) && (( UcRegValy & 0x01 ) != 0x01 ))
	{
		
		RegWriteA_lc898122_sharp( WH_SMTSRVON,	0x01 ) ;				// 0x017C		Smooth Servo ON
		
		SrvCon_lc898122_sharp( X_DIR, ON ) ;
		SrvCon_lc898122_sharp( Y_DIR, ON ) ;
		
		UcRegIni = 0x11;
		while( (UcRegIni & 0x77) != 0x66 )
		{
			RegReadA_lc898122_sharp( RH_SMTSRVSTT,	&UcRegIni ) ;		// 0x01F8		Smooth Servo phase read
#ifdef CONFIG_IMX214_OIS_SHARP
			if (--zte_times == 0)
				break;
#endif
		}
		RegWriteA_lc898122_sharp( WH_SMTSRVON,	0x00 ) ;				// 0x017C		Smooth Servo OFF
		
	}
	else
	{
		SrvCon_lc898122_sharp( X_DIR, ON ) ;
		SrvCon_lc898122_sharp( Y_DIR, ON ) ;
	}
}

//********************************************************************************
// Function Name 	: StbOnnN_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Stabilizer For Servo On Function
// History			: First edition 						2013.10.09 Y.Shigeoka
//********************************************************************************
 
void StbOnnN_lc898122_sharp( unsigned char UcStbY , unsigned char UcStbX )
{
	unsigned char	UcRegIni ;
	unsigned char	UcSttMsk = 0 ;
	
	
	RegWriteA_lc898122_sharp( WH_SMTSRVON,	0x01 ) ;				// 0x017C		Smooth Servo ON
	if( UcStbX == ON )	UcSttMsk |= 0x07 ;
	if( UcStbY == ON )	UcSttMsk |= 0x70 ;
	
	SrvCon_lc898122_sharp( X_DIR, UcStbX ) ;
	SrvCon_lc898122_sharp( Y_DIR, UcStbY ) ;
	
	UcRegIni = 0x11;
	while( (UcRegIni & UcSttMsk) != ( 0x66 & UcSttMsk ) )
	{
		RegReadA_lc898122_sharp( RH_SMTSRVSTT,	&UcRegIni ) ;		// 0x01F8		Smooth Servo phase read
	}
	RegWriteA_lc898122_sharp( WH_SMTSRVON,	0x00 ) ;				// 0x017C		Smooth Servo OFF
		
}

//********************************************************************************
// Function Name 	: OptCen_lc898122_sharp
// Retun Value		: NON
// Argment Value	: UcOptMode 0:Set 1:Save&Set
//					: UsOptXval Xaxis offset
//					: UsOptYval Yaxis offset
// Explanation		: Send Optical Center
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
void	OptCen_lc898122_sharp( unsigned char UcOptmode , unsigned short UsOptXval , unsigned short UsOptYval )
{
	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	switch ( UcOptmode ) {
		case VAL_SET :
			RamWriteA_lc898122_sharp( SXOFFZ1   , UsOptXval ) ;		/* 0x1461	Check Hall X optical center */
			RamWriteA_lc898122_sharp( SYOFFZ1   , UsOptYval ) ;		/* 0x14E1	Check Hall Y optical center */
			break ;
		case VAL_FIX :
			UsCntXof_lc898122_sharp = UsOptXval ;
			UsCntYof_lc898122_sharp = UsOptYval ;
			RamWriteA_lc898122_sharp( SXOFFZ1   , UsCntXof_lc898122_sharp ) ;		/* 0x1461	Check Hall X optical center */
			RamWriteA_lc898122_sharp( SYOFFZ1   , UsCntYof_lc898122_sharp ) ;		/* 0x14E1	Check Hall Y optical center */

			break ;
		case VAL_SPC :
			RamReadA_lc898122_sharp( SXOFFZ1   , &UsOptXval ) ;		/* 0x1461	Check Hall X optical center */
			RamReadA_lc898122_sharp( SYOFFZ1   , &UsOptYval ) ;		/* 0x14E1	Check Hall Y optical center */
			UsCntXof_lc898122_sharp = UsOptXval ;
			UsCntYof_lc898122_sharp = UsOptYval ;


			break ;
	}

	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
}

#ifdef	MODULE_CALIBRATION
//********************************************************************************
// Function Name 	: OscAdj_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: OSC Clock adjustment
// History			: First edition 						2013.01.15 Y.Shigeoka
//********************************************************************************
#define	RRATETABLE	8
#define	CRATETABLE	16
const signed char	ScRselRate[ RRATETABLE ]	= {
		-12,			/* -12% */
		 -9,			/*  -9% */
		 -6,			/*  -6% */
		 -3,			/*  -3% */
		  0,			/*   0% */
		  3,			/*   3% */
		  7,			/*   7% */
		 11				/*  11% */
	} ;
const signed char	ScCselRate[ CRATETABLE ]	= {
		-14,			/* -14% */
		-12,			/* -12% */
		-10,			/* -10% */
		 -8,			/*  -8% */
		 -6,			/*  -6% */
		 -4,			/*  -4% */
		 -2,			/*  -2% */
		  0,			/*   0% */
		  0,			/*   0% */
		  2,			/*   2% */
		  4,			/*   4% */
		  6,			/*   6% */
		  8,			/*   8% */
		 10,			/*  10% */
		 12,			/*  12% */
		 14				/*  14% */
	} ;
	

#define	TARGET_FREQ		48000.0F/* 48MHz */
//#define	TARGET_FREQ		24000.0F/* 24MHz */
#define	START_RSEL		0x04	/* Typ */
#define	START_CSEL		0x08	/* Typ bit4:OSCPMSEL */
#define	MEAS_MAX		32		/* 上限32回 */
/* Measure Status (UcClkJdg) */
#define	UNDR_MEAS		0x00
#define	FIX_MEAS		0x01
#define	JST_FIX			0x03
#define	OVR_MEAS		0x80
/* Measure Check Flag (UcMeasFlg) */
#define	RSELFX			0x08
#define	RSEL1ST			0x01
#define	RSEL2ND			0x02
#define	CSELFX			0x80
#define	CSELPLS			0x10
#define	CSELMNS			0x20

unsigned short	OscAdj_lc898122_sharp( void )
{
	unsigned char	UcMeasFlg ;									/* Measure check flag */
	UnWrdVal		StClkVal ;									/* Measure value */
	unsigned char	UcMeasCnt ;									/* Measure counter */
	unsigned char	UcOscrsel , UcOsccsel ;						/* Reg set value */
	unsigned char	UcSrvDivBk ;								/* back up value */
	unsigned char	UcClkJdg ;									/* State flag */
	float			FcalA,FcalB ;								/* calcurate value */
	signed char		ScTblRate_Val, ScTblRate_Now, ScTblRate_Tgt ;	/* rate */
	float			FlRatePbk,FlRateMbk ;							/* Rate bk  */
	unsigned char	UcOsccselP , UcOsccselM ;					/* Reg set value */
	unsigned short	UsResult ;

	UcMeasFlg = 0 ;						/* Clear Measure check flag */
	UcMeasCnt = 0;						/* Clear Measure counter */
	UcClkJdg = UNDR_MEAS;				/* under Measure */
	UcOscrsel = START_RSEL ;
	UcOsccsel = START_CSEL ;
	
	RegReadA_lc898122_sharp( SRVDIV, &UcSrvDivBk ) ;	/* 0x0211 */
	RegWriteA_lc898122_sharp( SRVDIV,	0x00 ) ;		// 0x0211	 SRV Clock = Xtalck
	RegWriteA_lc898122_sharp( OSCSET, ( UcOscrsel << 5 ) | (UcOsccsel << 1 ) ) ;	// 0x0257	 
	
	while( UcClkJdg == UNDR_MEAS )
	{
		UcMeasCnt++ ;						/* Measure count up */
		UcOscAdjFlg_lc898122_sharp = MEASSTR ;				// Start trigger ON
		
		while( UcOscAdjFlg_lc898122_sharp != MEASFIX )
		{
			;
		}
		
		UcOscAdjFlg_lc898122_sharp = 0x00 ;				// Clear Flag
		RegReadA_lc898122_sharp( OSCCK_CNTR0, &StClkVal.StWrdVal.UcLowVal ) ;		/* 0x025E */
		RegReadA_lc898122_sharp( OSCCK_CNTR1, &StClkVal.StWrdVal.UcHigVal ) ;		/* 0x025F */
		
		FcalA = (float)StClkVal.UsWrdVal ;
		FcalB = TARGET_FREQ / FcalA ;
		FcalB =  FcalB - 1.0F ;
		FcalB *= 100.0F ;
		
		if( FcalB == 0.0F )
		{
			UcClkJdg = JST_FIX ;					/* Just 36MHz */
			UcMeasFlg |= ( CSELFX | RSELFX ) ;		/* Fix Flag */
			break ;
		}

		/* Rsel check */
		if( !(UcMeasFlg & RSELFX) )
		{
			if(UcMeasFlg & RSEL1ST)
			{
				UcMeasFlg |= ( RSELFX | RSEL2ND ) ;
			}
			else
			{
				UcMeasFlg |= RSEL1ST ;
			}
			ScTblRate_Now = ScRselRate[ UcOscrsel ] ;					/* 今のRate */
			ScTblRate_Tgt = ScTblRate_Now + (short)FcalB ;
			if( ScTblRate_Now > ScTblRate_Tgt )
			{
				while(1)
				{
					if( UcOscrsel == 0 )
					{
						break;
					}
					UcOscrsel -= 1 ;
					ScTblRate_Val = ScRselRate[ UcOscrsel ] ;	
					if( ScTblRate_Tgt >= ScTblRate_Val )
					{
						break;
					}
				}
			}
			else if( ScTblRate_Now < ScTblRate_Tgt )
			{
				while(1)
				{
					if(UcOscrsel == (RRATETABLE - 1))
					{
						break;
					}
					UcOscrsel += 1 ;
					ScTblRate_Val = ScRselRate[ UcOscrsel ] ;	
					if( ScTblRate_Tgt <= ScTblRate_Val )
					{
						break;
					}
				}
			}
			else
			{
				;
			}
		}
		else
		{		
		/* Csel check */
			if( FcalB > 0 )			/* Plus */
			{
				UcMeasFlg |= CSELPLS ;
				FlRatePbk = FcalB ;
				UcOsccselP = UcOsccsel ;
				if( UcMeasFlg & CSELMNS)
				{
					UcMeasFlg |= CSELFX ;
					UcClkJdg = FIX_MEAS ;			/* OK */
				}
				else if(UcOsccsel == (CRATETABLE - 1))
				{
					if(UcOscrsel < ( RRATETABLE - 1 ))
					{
						UcOscrsel += 1 ;
						UcOsccsel = START_CSEL ;
						UcMeasFlg = 0 ;			/* Clear */
					}
					else
					{
						UcClkJdg = OVR_MEAS ;			/* Over */
					}
				}
				else
				{
					UcOsccsel += 1 ;
				}
			}
			else					/* Minus */
			{
				UcMeasFlg |= CSELMNS ;
				FlRateMbk = (-1)*FcalB ;
				UcOsccselM = UcOsccsel ;
				if( UcMeasFlg & CSELPLS)
				{
					UcMeasFlg |= CSELFX ;
					UcClkJdg = FIX_MEAS ;			/* OK */
				}
				else if(UcOsccsel == 0x00)
				{
					if(UcOscrsel > 0)
					{
						UcOscrsel -= 1 ;
						UcOsccsel = START_CSEL ;
						UcMeasFlg = 0 ;			/* Clear */
					}
					else
					{
					UcClkJdg = OVR_MEAS ;			/* Over */
					}
				}
				else
				{
					UcOsccsel -= 1 ;
				}
			}
			if(UcMeasCnt >= MEAS_MAX)
			{
				UcClkJdg = OVR_MEAS ;			/* Over */
			}
		}	
		RegWriteA_lc898122_sharp( OSCSET, ( UcOscrsel << 5 ) | (UcOsccsel << 1 ) ) ;	// 0x0257	 
	}
	
	UsResult = EXE_END ;
	
	if(UcClkJdg == FIX_MEAS)
	{
		if( FlRatePbk < FlRateMbk )
		{
			UcOsccsel = UcOsccselP ; 
		}
		else
		{
			UcOsccsel = UcOsccselM ; 
		}
	
		RegWriteA_lc898122_sharp( OSCSET, ( UcOscrsel << 5 ) | (UcOsccsel << 1 ) ) ;	// 0x0264	 

	}
	StAdjPar_lc898122_sharp.UcOscVal = ( ( UcOscrsel << 5 ) | (UcOsccsel << 1 ) );
	
	if(UcClkJdg == OVR_MEAS)
	{
		UsResult = EXE_OCADJ ;
		StAdjPar_lc898122_sharp.UcOscVal = 0x00 ;
	}
	RegWriteA_lc898122_sharp( SRVDIV,	UcSrvDivBk ) ;		// 0x0211	 SRV Clock set
	return( UsResult );
}

//********************************************************************************
// Function Name 	: OscAdj_lc898122_sharpA
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: OSC Clock adjustment
// History			: First edition 						2013.12.05 Y.Shigeoka
//********************************************************************************
#define	OSCSET_INI		0x90
#define	OSCMODE_DEF		0x00
#define	OSCMODE_I2C		0x60
#define	OSCMODE_EXC		0x70
unsigned short	OscAdj_lc898122_sharpA( unsigned short UsTgtCnt )
{
	unsigned char	UcOscSetVal ;								/* Reg set value */
	unsigned char	UcSrvDivBk ;								/* back up value */
 #ifdef	OSC_EXCLK
	unsigned char	UcClkSelBk ;								/* back up value */
	unsigned char	UcPwmDivBk ;								/* back up value */
 #endif
	unsigned char	UcClkJdg ;									/* State flag */
	unsigned char	UcOscMod ;									/* OSC mode */
	unsigned short	UsResult ;


	RegReadA_lc898122_sharp( SRVDIV, &UcSrvDivBk ) ;			/* 0x0211 	Srv Div back UP */
	RegWriteA_lc898122_sharp( SRVDIV,	0x00 ) ;				// 0x0211	 SRV Clock = Xtalck
	RegWriteA_lc898122_sharp( OSCSET, OSCSET_INI ) ;			// 0x0257	 
 #ifdef	OSC_EXCLK
	RegReadA_lc898122_sharp( PWMDIV, &UcPwmDivBk ) ;			/* 0x0210 	Pwm Div back UP */
	RegReadA_lc898122_sharp( CLKSEL, &UcClkSelBk ) ;			/* 0x020C 	ClkSel back UP */
	RegWriteA_lc898122_sharp( PWMDIV, 0x00 ) ;					/* 0x0210 	Pwm Div 1/1 */
	RegWriteA_lc898122_sharp( CLKSEL, UcClkSelBk | 0x01 ) ;	/* 0x020C 	ClkSel re-store */
	UcOscMod = OSCMODE_EXC ;
 #else
	UcOscMod = OSCMODE_I2C ;
 #endif
	RegWriteA_lc898122_sharp( OSCMAMODE, UcOscMod ) ;								// 0x0265
	RegWriteA_lc898122_sharp( OSCTCNTL, (unsigned char)UsTgtCnt ) ;				// 0x0266
	RegWriteA_lc898122_sharp( OSCTCNTH, (unsigned char)( UsTgtCnt >> 8 ) ) ;		// 0x0267
	UcClkJdg = 0 ;
	
	while( UcClkJdg == 0 )
	{
		RegWriteA_lc898122_sharp( OSCCNTEN, 0x01 ) ;		// 0x0258	 Trigger ON
		RegWriteA_lc898122_sharp( OSCCNTEN, 0x00 ) ;		// 0x0258	 Trigger OFF
		RegReadA_lc898122_sharp(  OSCMAMODE, &UcClkJdg ) ;	// 0x0265	 Status Read
		UcClkJdg &= 0x03;
	}
	
	UsResult = EXE_END ;
	
	RegReadA_lc898122_sharp( OSCSET, &UcOscSetVal ) ;		// 0x0257	 
	StAdjPar_lc898122_sharp.UcOscVal = UcOscSetVal ;
	RegWriteA_lc898122_sharp( OSCMAMODE, OSCMODE_DEF ) ;	// 0x0265
	
	if(UcClkJdg == 0x01)
	{
		UsResult = EXE_OCADJ ;
	}
	RegWriteA_lc898122_sharp( SRVDIV,	UcSrvDivBk ) ;		// 0x0211	 SRV Clock set
 #ifdef	OSC_EXCLK
	RegWriteA_lc898122_sharp( PWMDIV, UcPwmDivBk ) ;		/* 0x0210 	Pwm Div re-store */
	RegWriteA_lc898122_sharp( CLKSEL, UcClkSelBk ) ;		/* 0x020C 	ClkSel re-store */
 #endif
	return( UsResult );
}

#endif


#ifdef HALLADJ_HW
//==============================================================================
//  Function    :   SetSineWave_lc898122_sharp()
//  inputs      :   UcJikuSel   0: X-Axis
//                              1: Y-Axis
//                  UcMeasMode  0: Loop Gain frequency setting
//                              1: Bias/Offset frequency setting
//  outputs     :   void
//  explanation :   Initializes sine wave settings:
//                      Sine Table, Amplitue, Offset, Frequency
//  revisions   :   First Edition                          2013.01.15 Y.Shigeoka
//==============================================================================
void SetSineWave_lc898122_sharp( unsigned char UcJikuSel , unsigned char UcMeasMode )
{
 #ifdef	USE_EXTCLK_ALL	// 24MHz
    unsigned short  UsFRQ[]   = { 0x30EE/*139.9Hz*/ , 0x037E/*10Hz*/ } ;          // { Loop Gain setting , Bias/Offset setting}
 #else
    unsigned short  UsFRQ[]   = { 0x1877/*139.9Hz*/ , 0x01BF/*10Hz*/ } ;          // { Loop Gain setting , Bias/Offset setting}
 #endif
	unsigned long   UlAMP[2][2]   = {{ 0x3CA3D70A , 0x3CA3D70A } ,		// Loop Gain   { X amp , Y amp }
									 { 0x3F800000 , 0x3F800000 } };		// Bias/offset { X amp , Y amp }
	unsigned char	UcEqSwX , UcEqSwY ;

    UcMeasMode &= 0x01;
    UcJikuSel  &= 0x01;

	/* Phase parameter 0deg */
	RegWriteA_lc898122_sharp( WC_SINPHSX, 0x00 ) ;					/* 0x0183	*/
	RegWriteA_lc898122_sharp( WC_SINPHSY, 0x00 ) ;					/* 0x0184	*/
	
	/* wait 0 cross */
	RegWriteA_lc898122_sharp( WC_MESSINMODE,     0x00 ) ;			/* 0x0191	Sine 0 cross  */
	RegWriteA_lc898122_sharp( WC_MESWAIT,     0x00 ) ;				/* 0x0199	0 cross wait */
	
    /* Manually Set Amplitude */
	RamWrite32A_lc898122_sharp( sxsin, UlAMP[UcMeasMode][X_DIR] ) ;				// 0x10D5
	RamWrite32A_lc898122_sharp( sysin, UlAMP[UcMeasMode][Y_DIR] ) ;				// 0x11D5

	/* Freq */
	RegWriteA_lc898122_sharp( WC_SINFRQ0,	(unsigned char)UsFRQ[UcMeasMode] ) ;				// 0x0181		Freq L
	RegWriteA_lc898122_sharp( WC_SINFRQ1,	(unsigned char)(UsFRQ[UcMeasMode] >> 8) ) ;			// 0x0182		Freq H

    /* Clear Optional Sine wave input address */
	RegReadA_lc898122_sharp( WH_EQSWX, &UcEqSwX ) ;				/* 0x0170	*/
	RegReadA_lc898122_sharp( WH_EQSWY, &UcEqSwY ) ;				/* 0x0171	*/
    if( !UcMeasMode && !UcJikuSel )       // Loop Gain mode  X-axis
    {
		UcEqSwX |= 0x10 ;				/* SW[4] */
		UcEqSwY &= ~EQSINSW ;
    }
    else if( !UcMeasMode && UcJikuSel )   // Loop Gain mode Y-Axis
    {
		UcEqSwX &= ~EQSINSW ;
		UcEqSwY |= 0x10 ;				/* SW[4] */
    }
    else if( UcMeasMode && !UcJikuSel )   // Bias/Offset mode X-Axis
    {
    	UcEqSwX = 0x22 ;				/* SW[5] */
    	UcEqSwY = 0x03 ;
    }
    else                    // Bias/Offset mode Y-Axis
    {
		UcEqSwX = 0x03 ;
		UcEqSwY = 0x22 ;				/* SW[5] */

    }
	RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	*/
	RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	*/
}

//==============================================================================
//  Function    :   StartSineWave_lc898122_sharp()
//  inputs      :   none
//  outputs     :   void
//  explanation :   Starts sine wave
//  revisions   :   First Edition                          2011.04.13 d.yamagata
//==============================================================================
void StartSineWave_lc898122_sharp( void )
{
    /* Start Sine Wave */
	RegWriteA_lc898122_sharp( WC_SINON,     0x01 ) ;				/* 0x0180	Sine wave  */

}

//==============================================================================
//  Function    :   StopSineWave_lc898122_sharp()
//  inputs      :   void
//  outputs     :   void
//  explanation :   Stops sine wave
//  revisions   :   First Edition                          2013.01.15 Y.Shigeoka
//==============================================================================
void StopSineWave_lc898122_sharp( void )
{
	unsigned char		UcEqSwX , UcEqSwY ;
	
	RegWriteA_lc898122_sharp( WC_SINON,     0x00 ) ;				/* 0x0180	Sine wave Stop */
	RegReadA_lc898122_sharp( WH_EQSWX, &UcEqSwX ) ;				/* 0x0170	*/
	RegReadA_lc898122_sharp( WH_EQSWY, &UcEqSwY ) ;				/* 0x0171	*/
	UcEqSwX &= ~EQSINSW ;
	UcEqSwY &= ~EQSINSW ;
	RegWriteA_lc898122_sharp( WH_EQSWX, UcEqSwX ) ;				/* 0x0170	Switch control */
	RegWriteA_lc898122_sharp( WH_EQSWY, UcEqSwY ) ;				/* 0x0171	Switch control */

}

//==============================================================================
//  Function    :   SetMeaseFil_LoopGain()
//  inputs      :   UcJikuSel   0: X-Axis
//                              1: Y-Axis
//                  UcMeasMode  0: Loop Gain frequency setting
//                              1: Bias/Offset frequency setting
//                  UcFilSel
//  outputs     :   void
//  explanation :
//  revisions   :   First Edition                          2013.01.15 Y.Shigeoka
//==============================================================================
void SetMeasFil_lc898122_sharp( unsigned char UcFilSel )
{
	
	MesFil_lc898122_sharp( UcFilSel ) ;					/* Set Measure filter */

}

//==============================================================================
//  Function    :   ClrMeasFil_lc898122_sharp()
//  inputs      :   void
//  outputs     :   void
//  explanation :
//  revisions   :   First Edition                          2013.01.15 Y.Shigeoka
//==============================================================================
void ClrMeasFil_lc898122_sharp( void )
{
    /* Measure Filters clear */
	ClrGyr_lc898122_sharp( 0x1000 , CLR_FRAM1 );		// MEAS-FIL Delay RAM Clear
 
}

 #ifdef	MODULE_CALIBRATION
//==============================================================================
//  Function    :   LoopGainAdj_lc898122_sharp()
//  inputs      :   UcJikuSel   0: X-Axis, 1: Y-Axis
//  outputs     :   void
//  explanation :
//  revisions   :   First Edition                          2011.04.13 d.yamagata
//==============================================================================
unsigned char	 LoopGainAdj_lc898122_sharp( unsigned char UcJikuSel)
{

	unsigned short	UsRltVal ;
	unsigned char	UcAdjSts	= FAILURE ;
	
    UcJikuSel &= 0x01;

	StbOnn_lc898122_sharp() ;											// Slope Mode
	
	// Wait 200ms
	WitTim_lc898122_sharp( 200 ) ;
	
	/* set start gain */
	LopPar_lc898122_sharp( UcJikuSel ) ;
	
	/* set sine wave */
    SetSineWave_lc898122_sharp( UcJikuSel , __MEASURE_LOOPGAIN );

	/* Measure count */
	RegWriteA_lc898122_sharp( WC_MESLOOP1, 0x00 ) ;				// 0x0193
	RegWriteA_lc898122_sharp( WC_MESLOOP0, 0x40 ) ;				// 0x0192	64 Times Measure
	RamWrite32A_lc898122_sharp( msmean	, 0x3C800000 );				// 0x1230	1/CmMesLoop[15:0]
	RegWriteA_lc898122_sharp( WC_MESABS, 0x01 ) ;					// 0x0198	ABS
	
    /* Set Adjustment Limits */
    RamWrite32A_lc898122_sharp( LGMax  , 0x3F800000 );		// 0x1092	Loop gain adjustment max limit
    RamWrite32A_lc898122_sharp( LGMin  , 0x3E000100 );		// 0x1091	Loop gain adjustment min limit
    RegWriteA_lc898122_sharp( WC_AMJLOOP1, 0x00 );			// 0x01A3	Time-Out time
    RegWriteA_lc898122_sharp( WC_AMJLOOP0, 0x41 );			// 0x01A2	Time-Out time
    RegWriteA_lc898122_sharp( WC_AMJIDL1, 0x00 );			// 0x01A5	wait
    RegWriteA_lc898122_sharp( WC_AMJIDL0, 0x00 );			// 0x01A4	wait

    /* set Measure Filter */
    SetMeasFil_lc898122_sharp( LOOPGAIN );

    /* Clear Measure Filters */
    ClrMeasFil_lc898122_sharp();

    /* Start Sine Wave */
    StartSineWave_lc898122_sharp();

    /* Enable Loop Gain Adjustment */
    /* Check Busy Flag */
	BsyWit_lc898122_sharp( WC_AMJMODE, (0x0E | ( UcJikuSel << 4 )) ) ;				// 0x01A0	Loop Gain adjustment

	RegReadA_lc898122_sharp( RC_AMJERROR, &UcAdjBsy ) ;							// 0x01AD

	/* Ram Access */
	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	if( UcAdjBsy )
	{
		if( UcJikuSel == X_DIR )
		{
			RamReadA_lc898122_sharp( sxg, &UsRltVal ) ;						// 0x10D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgVal	= UsRltVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgSts	= 0x0000 ;
		} else {
			RamReadA_lc898122_sharp( syg, &UsRltVal ) ;						// 0x11D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygVal	= UsRltVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygSts	= 0x0000 ;
		}

	}
	else
	{
		if( UcJikuSel == X_DIR )
		{
			RamReadA_lc898122_sharp( sxg, &UsRltVal ) ;						// 0x10D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgVal	= UsRltVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLxgSts	= 0xFFFF ;
		} else {
			RamReadA_lc898122_sharp( syg, &UsRltVal ) ;						// 0x11D3
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygVal	= UsRltVal ;
			StAdjPar_lc898122_sharp.StLopGan_lc898122_sharp.UsLygSts	= 0xFFFF ;
		}
		UcAdjSts	= SUCCESS ;												// Status OK
	}

	/* Ram Access */
	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
    /* Stop Sine Wave */
    StopSineWave_lc898122_sharp();

	return( UcAdjSts ) ;
}
 #endif

//==============================================================================
//  Function    :   BiasOffsetAdj_lc898122_sharp()
//  inputs      :   UcJikuSel   0: X-Axis, 1: Y-Axis	UcMeasCnt  :Measure Count
//  outputs     :   Result status
//  explanation :
//  revisions   :   First Edition                          2013.01.16 Y.Shigeoka
//==============================================================================
unsigned char  BiasOffsetAdj_lc898122_sharp( unsigned char UcJikuSel , unsigned char UcMeasCnt )
{
	unsigned char 	UcHadjRst ;
									/*	   STEP         OFSTH        OFSTL        AMPH         AMPL                 (80%)*/
	unsigned long  UlTgtVal[2][5]    =  {{ 0x3F800000 , 0x3D200140 , 0xBD200140 , 0x3F547AE1 , 0x3F451EB8 },	/* ROUGH */
										 { 0x3F000000 , 0x3D200140 , 0xBD200140 , 0x3F50A3D7 , 0x3F48F5C3 }} ;	/* FINE */

	if(UcMeasCnt > 1)		UcMeasCnt = 1 ;
	
    UcJikuSel &= 0x01;

    /* Set Sine Wave */
    SetSineWave_lc898122_sharp( UcJikuSel , __MEASURE_BIASOFFSET );

	/* Measure count */
	RegWriteA_lc898122_sharp( WC_MESLOOP1, 0x00 ) ;				// 0x0193
	RegWriteA_lc898122_sharp( WC_MESLOOP0, 0x04 ) ;				// 0x0192	4 Times Measure
	RamWrite32A_lc898122_sharp( msmean	, 0x3E000000 );				// 0x10AE	1/CmMesLoop[15:0]/2
	RegWriteA_lc898122_sharp( WC_MESABS, 0x00 ) ;					// 0x0198	ABS

    /* Set Adjustment Limits */
    RamWrite32A_lc898122_sharp( HOStp  , UlTgtVal[UcMeasCnt][0] );		// 0x1070	Hall Offset Stp
    RamWrite32A_lc898122_sharp( HOMax  , UlTgtVal[UcMeasCnt][1] );		// 0x1072	Hall Offset max limit
    RamWrite32A_lc898122_sharp( HOMin  , UlTgtVal[UcMeasCnt][2] );		// 0x1071	Hall Offset min limit
    RamWrite32A_lc898122_sharp( HBStp  , UlTgtVal[UcMeasCnt][0] );		// 0x1080	Hall Bias Stp
    RamWrite32A_lc898122_sharp( HBMax  , UlTgtVal[UcMeasCnt][3] );		// 0x1082	Hall Bias max limit
    RamWrite32A_lc898122_sharp( HBMin  , UlTgtVal[UcMeasCnt][4] );		// 0x1081	Hall Bias min limit

	RegWriteA_lc898122_sharp( WC_AMJLOOP1, 0x00 );			// 0x01A3	Time-Out time
    RegWriteA_lc898122_sharp( WC_AMJLOOP0, 0x40 );			// 0x01A2	Time-Out time
    RegWriteA_lc898122_sharp( WC_AMJIDL1, 0x00 );			// 0x01A5	wait
    RegWriteA_lc898122_sharp( WC_AMJIDL0, 0x00 );			// 0x01A4	wait
	
    /* Set Measure Filter */
    SetMeasFil_lc898122_sharp( HALL_ADJ );

    /* Clear Measure Filters */
    ClrMeasFil_lc898122_sharp();

    /* Start Sine Wave */
    StartSineWave_lc898122_sharp();

    /* Check Busy Flag */
	BsyWit_lc898122_sharp( WC_AMJMODE, (0x0C | ( UcJikuSel << 4 )) ) ;				// 0x01A0	Hall bais/offset ppt adjustment
	
	RegReadA_lc898122_sharp( RC_AMJERROR, &UcAdjBsy ) ;							// 0x01AD
	
	if( UcAdjBsy )
	{
		if( UcJikuSel == X_DIR )
		{
			UcHadjRst = EXE_HXADJ ;
		}
		else
		{
			UcHadjRst = EXE_HYADJ ;
		}

	}
	else
	{
		UcHadjRst = EXE_END ;
	}

    /* Stop Sine Wave */
    StopSineWave_lc898122_sharp();

    /* Set Servo Filter */
	
	/* Ram Access */
	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	if( UcJikuSel == X_DIR )
	{
		RamReadA_lc898122_sharp( MSPP1AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlxMxa  ) ;	 	// 0x1042 Max width value
		RamReadA_lc898122_sharp( MSCT1AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna  ) ;	 	// 0x1052 offset value
	}
	else
	{
//		RamReadA_lc898122_sharp( MSPP2AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyMxa  ) ;	 	// 0x1142 Max width value
//		RamReadA_lc898122_sharp( MSCT2AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna  ) ;	 	// 0x1152 offset value
		RamReadA_lc898122_sharp( MSPP1AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyMxa  ) ;	 	// 0x1042 Max width value
		RamReadA_lc898122_sharp( MSCT1AV, &StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna  ) ;	 	// 0x1052 offset value
	}

	StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna = (unsigned short)((signed short)StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna << 1 ) ;
	StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna = (unsigned short)((signed short)StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna << 1 ) ;
	/* Ram Access */
	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
	return( UcHadjRst ) ;
}

#endif


//********************************************************************************
// Function Name 	: GyrGan_lc898122_sharp
// Retun Value		: NON
// Argment Value	: UcGygmode 0:Set 1:Save&Set
//					: UlGygXval Xaxis Gain
//					: UlGygYval Yaxis Gain
// Explanation		: Send Gyro Gain
// History			: First edition 						2011.02.09 Y.Shigeoka
//********************************************************************************
void	GyrGan_lc898122_sharp( unsigned char UcGygmode , unsigned long UlGygXval , unsigned long UlGygYval )
{
	switch ( UcGygmode ) {
		case VAL_SET :
			RamWrite32A_lc898122_sharp( gxzoom, UlGygXval ) ;		// 0x1020
			RamWrite32A_lc898122_sharp( gyzoom, UlGygYval ) ;		// 0x1120
			break ;
		case VAL_FIX :
			RamWrite32A_lc898122_sharp( gxzoom, UlGygXval ) ;		// 0x1020
			RamWrite32A_lc898122_sharp( gyzoom, UlGygYval ) ;		// 0x1120

			break ;
		case VAL_SPC :
			RamRead32A_lc898122_sharp( gxzoom, &UlGygXval ) ;		// 0x1020
			RamRead32A_lc898122_sharp( gyzoom, &UlGygYval ) ;		// 0x1120
		
			break ;
	}

}

//********************************************************************************
// Function Name 	: SetPanTiltMode_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Pan-Tilt Enable/Disable
// History			: First edition 						2013.01.09 Y.Shigeoka
//********************************************************************************
void	SetPanTiltMode_lc898122_sharp( unsigned char UcPnTmod )
{
	switch ( UcPnTmod ) {
		case OFF :
			RegWriteA_lc898122_sharp( WG_PANON, 0x00 ) ;			// 0x0109	X,Y Pan/Tilt Function OFF
			break ;
		case ON :
#ifdef	NEW_PTST
			RegWriteA_lc898122_sharp( WG_PANON, 0x10 ) ;			// 0x0109	X,Y New Pan/Tilt Function ON
#else
			RegWriteA_lc898122_sharp( WG_PANON, 0x01 ) ;			// 0x0109	X,Y Pan/Tilt Function ON
#endif
			break ;
	}

}


#ifdef GAIN_CONT
//********************************************************************************
// Function Name 	: TriSts_lc898122_sharp
// Retun Value		: Tripod Status
//					: bit0( 1:Y Tripod ON / 0:OFF)
//					: bit4( 1:X Tripod ON / 0:OFF)
//					: bit7( 1:Tripod ENABLE  / 0:DISABLE)
// Argment Value	: NON
// Explanation		: Read Status of Tripod mode Function
// History			: First edition 						2013.02.18 Y.Shigeoka
//********************************************************************************
unsigned char	TriSts_lc898122_sharp( void )
{
	unsigned char UcRsltSts = 0;
	unsigned char UcVal ;

	RegReadA_lc898122_sharp( WG_ADJGANGXATO, &UcVal ) ;	// 0x0129
	if( UcVal & 0x03 ){						// Gain control enable?
		RegReadA_lc898122_sharp( RG_LEVJUGE, &UcVal ) ;	// 0x01F4
		UcRsltSts = UcVal & 0x11 ;			// bit0, bit4 set
		UcRsltSts |= 0x80 ;					// bit7 ON
	}
	return( UcRsltSts ) ;
}
#endif

//********************************************************************************
// Function Name 	: DrvPwmSw_lc898122_sharp
// Retun Value		: Mode Status
//					: bit4( 1:PWM / 0:LinearPwm)
// Argment Value	: NON
// Explanation		: Select Driver mode Function
// History			: First edition 						2013.02.18 Y.Shigeoka
//********************************************************************************
unsigned char	DrvPwmSw_lc898122_sharp( unsigned char UcSelPwmMod )
{

	switch ( UcSelPwmMod ) {
		case Mlnp :
			RegWriteA_lc898122_sharp( DRVFC	, 0xF0 );			// 0x0001	Drv.MODE=1,Drv.BLK=1,MODE2,LCEN
			UcPwmMod_lc898122_sharp = PWMMOD_CVL ;
			break ;
		
		case Mpwm :
#ifdef	PWM_BREAK
			RegWriteA_lc898122_sharp( DRVFC	, 0x00 );			// 0x0001	Drv.MODE=0,Drv.BLK=0,MODE0B
#else
			RegWriteA_lc898122_sharp( DRVFC	, 0xC0 );			// 0x0001	Drv.MODE=1,Drv.BLK=1,MODE1
#endif
			UcPwmMod_lc898122_sharp = PWMMOD_PWM ;
 			break ;
	}
	
	return( UcSelPwmMod << 4 ) ;
}

 #ifdef	NEUTRAL_CENTER
//********************************************************************************
// Function Name 	: TneHvc_lc898122_sharp
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Tunes the Hall VC offset
// History			: First edition 						2013.03.13	T.Tokoro
//********************************************************************************
unsigned char	TneHvc_lc898122_sharp( void )
{
	unsigned char	UcRsltSts;
	unsigned short	UsMesRlt1 ;
	unsigned short	UsMesRlt2 ;
	
	SrvCon_lc898122_sharp( X_DIR, OFF ) ;				// X Servo OFF
	SrvCon_lc898122_sharp( Y_DIR, OFF ) ;				// Y Servo OFF
	
	WitTim_lc898122_sharp( 500 ) ;
	
	//平均値測定
	
	MesFil_lc898122_sharp( THROUGH ) ;					// Set Measure Filter
	
	RegWriteA_lc898122_sharp( WC_MESLOOP1	, 0x00 );			// 0x0193	CmMesLoop[15:8]
	RegWriteA_lc898122_sharp( WC_MESLOOP0	, 0x40);			// 0x0192	CmMesLoop[7:0]

	RamWrite32A_lc898122_sharp( msmean	, 0x3C800000 );			// 0x1230	1/CmMesLoop[15:0]
	
	RegWriteA_lc898122_sharp( WC_MES1ADD0,  ( unsigned char )AD0Z ) ;							/* 0x0194	*/
	RegWriteA_lc898122_sharp( WC_MES1ADD1,  ( unsigned char )(( AD0Z >> 8 ) & 0x0001 ) ) ;		/* 0x0195	*/
	RegWriteA_lc898122_sharp( WC_MES2ADD0,  ( unsigned char )AD1Z ) ;							/* 0x0196	*/
	RegWriteA_lc898122_sharp( WC_MES2ADD1,  ( unsigned char )(( AD1Z >> 8 ) & 0x0001 ) ) ;		/* 0x0197	*/
	
	RamWrite32A_lc898122_sharp( MSABS1AV, 	0x00000000 ) ;		// 0x1041
	RamWrite32A_lc898122_sharp( MSABS2AV, 	0x00000000 ) ;		// 0x1141
	
	RegWriteA_lc898122_sharp( WC_MESABS, 0x00 ) ;				// 0x0198	none ABS
	
	BsyWit_lc898122_sharp( WC_MESMODE, 0x01 ) ;				// 0x0190		Normal Measure
	
	RamAccFixMod_lc898122_sharp( ON ) ;							// Fix mode
	
	RamReadA_lc898122_sharp( MSABS1AV, &UsMesRlt1 ) ;			// 0x1041	Measure Result
	RamReadA_lc898122_sharp( MSABS2AV, &UsMesRlt2 ) ;			// 0x1141	Measure Result

	RamAccFixMod_lc898122_sharp( OFF ) ;							// Float mode
	
	StAdjPar_lc898122_sharp.StHalAdj.UsHlxCna = UsMesRlt1;			//Measure Result Store
	StAdjPar_lc898122_sharp.StHalAdj.UsHlxCen = UsMesRlt1;			//Measure Result Store
	
	StAdjPar_lc898122_sharp.StHalAdj.UsHlyCna = UsMesRlt2;			//Measure Result Store
	StAdjPar_lc898122_sharp.StHalAdj.UsHlyCen = UsMesRlt2;			//Measure Result Store
	
	UcRsltSts = EXE_END ;				// Clear Status
	
	return( UcRsltSts );
}
 #endif	//NEUTRAL_CENTER

//********************************************************************************
// Function Name 	: SetGcf_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Set DI filter coefficient Function
// History			: First edition 						2013.03.22 Y.Shigeoka
//********************************************************************************
void	SetGcf_lc898122_sharp( unsigned char	UcSetNum )
{
	
	/* Zoom Step */
	if(UcSetNum > (COEFTBL - 1))
		UcSetNum = (COEFTBL -1) ;			/* 上限をCOEFTBL-1に設定する */

	UlH1Coefval_lc898122_sharp	= ClDiCof_lc898122_sharp[ UcSetNum ] ;
		
	// Zoom Value Setting
	RamWrite32A_lc898122_sharp( gxh1c, UlH1Coefval_lc898122_sharp ) ;		/* 0x1012 */
	RamWrite32A_lc898122_sharp( gyh1c, UlH1Coefval_lc898122_sharp ) ;		/* 0x1112 */

#ifdef H1COEF_CHANGER
		SetH1cMod_lc898122_sharp( UcSetNum ) ;							/* Re-setting */
#endif

}

#ifdef H1COEF_CHANGER
//********************************************************************************
// Function Name 	: SetH1cMod_lc898122_sharp
// Retun Value		: NON
// Argment Value	: Command Parameter
// Explanation		: Set H1C coefficient Level chang Function
// History			: First edition 						2013.04.18 Y.Shigeoka
//********************************************************************************
void	SetH1cMod_lc898122_sharp( unsigned char	UcSetNum )
{
	
	switch( UcSetNum ){
	case ( ACTMODE ):				// initial 
		IniPtMovMod_lc898122_sharp( OFF ) ;							// Pan/Tilt setting (Still)
		
		/* enable setting */
		/* Zoom Step */
		UlH1Coefval_lc898122_sharp	= ClDiCof_lc898122_sharp[ 0 ] ;
			
		UcH1LvlMod_lc898122_sharp = 0 ;
		
		// Limit value Value Setting
		RamWrite32A_lc898122_sharp( gxlmt6L, MINLMT ) ;		/* 0x102D L-Limit */
		RamWrite32A_lc898122_sharp( gxlmt6H, MAXLMT ) ;		/* 0x102E H-Limit */

		RamWrite32A_lc898122_sharp( gylmt6L, MINLMT ) ;		/* 0x112D L-Limit */
		RamWrite32A_lc898122_sharp( gylmt6H, MAXLMT ) ;		/* 0x112E H-Limit */

		RamWrite32A_lc898122_sharp( gxhc_tmp, 	UlH1Coefval_lc898122_sharp ) ;	/* 0x100E Base Coef */
		RamWrite32A_lc898122_sharp( gxmg, 		CHGCOEF ) ;		/* 0x10AA Change coefficient gain */

		RamWrite32A_lc898122_sharp( gyhc_tmp, 	UlH1Coefval_lc898122_sharp ) ;	/* 0x110E Base Coef */
		RamWrite32A_lc898122_sharp( gymg, 		CHGCOEF ) ;		/* 0x11AA Change coefficient gain */
		
		RegWriteA_lc898122_sharp( WG_HCHR, 0x12 ) ;			// 0x011B	GmHChrOn[1]=1 Sw ON
		break ;
		
	case( S2MODE ):				// cancel lvl change mode 
		RegWriteA_lc898122_sharp( WG_HCHR, 0x10 ) ;			// 0x011B	GmHChrOn[1]=0 Sw OFF
		break ;
		
	case( MOVMODE ):			// Movie mode 
		IniPtMovMod_lc898122_sharp( ON ) ;							// Pan/Tilt setting (Movie)
		
		RamWrite32A_lc898122_sharp( gxlmt6L, MINLMT_MOV ) ;	/* 0x102D L-Limit */
		RamWrite32A_lc898122_sharp( gylmt6L, MINLMT_MOV ) ;	/* 0x112D L-Limit */

		RamWrite32A_lc898122_sharp( gxmg, CHGCOEF_MOV ) ;		/* 0x10AA Change coefficient gain */
		RamWrite32A_lc898122_sharp( gymg, CHGCOEF_MOV ) ;		/* 0x11AA Change coefficient gain */
			
		RamWrite32A_lc898122_sharp( gxhc_tmp, UlH1Coefval_lc898122_sharp ) ;		/* 0x100E Base Coef */
		RamWrite32A_lc898122_sharp( gyhc_tmp, UlH1Coefval_lc898122_sharp ) ;		/* 0x110E Base Coef */
		
		RegWriteA_lc898122_sharp( WG_HCHR, 0x12 ) ;			// 0x011B	GmHChrOn[1]=1 Sw ON
		break ;
		
	default :
		IniPtMovMod_lc898122_sharp( OFF ) ;							// Pan/Tilt setting (Still)
		
		UcH1LvlMod_lc898122_sharp = UcSetNum ;
			
		RamWrite32A_lc898122_sharp( gxlmt6L, MINLMT ) ;		/* 0x102D L-Limit */
		RamWrite32A_lc898122_sharp( gylmt6L, MINLMT ) ;		/* 0x112D L-Limit */
		
		RamWrite32A_lc898122_sharp( gxmg, 	CHGCOEF ) ;			/* 0x10AA Change coefficient gain */
		RamWrite32A_lc898122_sharp( gymg, 	CHGCOEF ) ;			/* 0x11AA Change coefficient gain */
			
		RamWrite32A_lc898122_sharp( gxhc_tmp, UlH1Coefval_lc898122_sharp ) ;		/* 0x100E Base Coef */
		RamWrite32A_lc898122_sharp( gyhc_tmp, UlH1Coefval_lc898122_sharp ) ;		/* 0x110E Base Coef */
		
		RegWriteA_lc898122_sharp( WG_HCHR, 0x12 ) ;			// 0x011B	GmHChrOn[1]=1 Sw ON
		break ;
	}
}
#endif

//********************************************************************************
// Function Name 	: RdFwVr_lc898122_sharp
// Retun Value		: Firmware version
// Argment Value	: NON
// Explanation		: Read Fw Version Function
// History			: First edition 						2013.05.07 Y.Shigeoka
//********************************************************************************
unsigned short	RdFwVr_lc898122_sharp( void )
{
	return( FW_VER ) ;
}
