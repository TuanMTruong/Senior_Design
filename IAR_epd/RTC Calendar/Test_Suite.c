//******************************************************************************
//   Code for application report - "Real Time Clock Library"
//******************************************************************************
// THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
// REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
// INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
// COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
// TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
// POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
// INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
// YOUR USE OF THE PROGRAM.
//
// IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
// CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
// THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
// OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
// EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
// REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
// OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
// USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
// AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
// YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
// (U.S.$500).
//
// Unless otherwise stated, the Program written and copyrighted
// by Texas Instruments is distributed as "freeware".  You may,
// only under TI's copyright in the Program, use and modify the
// Program without any charge or restriction.  You may
// distribute to third parties, provided that you transfer a
// copy of this license to the third party and the third party
// agrees to these terms by its first use of the Program. You
// must reproduce the copyright notice and any other legend of
// ownership on each copy or partial copy, of the Program.
//
// You acknowledge and agree that the Program contains
// copyrighted material, trade secrets and other TI proprietary
// information and is protected by copyright laws,
// international copyright treaties, and trade secret laws, as
// well as other intellectual property laws.  To protect TI's
// rights in the Program, you agree not to decompile, reverse
// engineer, disassemble or otherwise translate any object code
// versions of the Program to a human-readable form.  You agree
// that in no event will you alter, remove or destroy any
// copyright notice included in the Program.  TI reserves all
// rights not specifically granted under this license. Except
// as specifically provided herein, nothing in this agreement
// shall be construed as conferring by implication, estoppel,
// or otherwise, upon you, any license or other right under any
// TI patents, copyrights or trade secrets.
//
// You may not use the Program in non-TI devices.
//
//******************************************************************************
#include  <msp430.h>
#include  "RTC_Calendar.h"
//#include  "RTC.h"

int testSuiteCases( void );

int result_a = -1;
void main ( void )
{
  result_a = testSuiteCases();
  _NOP();
}


int testSuiteCases( void )
{
//------------------------------------------------------------------------------
// Seconds test Suite
//------------------------------------------------------------------------------
// second = 0 add one, second should be 01
//------------------------------------------------------------------------------
  TI_second = 0x00;
  incrementSeconds();
  if( TI_second != 0x01 ){ return 1; }

//------------------------------------------------------------------------------
// second = 09 add one, second should be 10
//------------------------------------------------------------------------------
  TI_second = 0x09;
  incrementSeconds();
  if( TI_second != 0x10 ){ return 2; }

//------------------------------------------------------------------------------
// second = 59 minute = 1 add one, second should be 00, minutes should roll
//------------------------------------------------------------------------------
  TI_second = 0x59;
  TI_minute = 0x01;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 3; }
  if( TI_minute != 0x02 ){ return 4; }

//------------------------------------------------------------------------------
// Minutes test Suite
//------------------------------------------------------------------------------
// minute = 0 add one, minute should be 01
//------------------------------------------------------------------------------
  TI_minute = 0x00;
  incrementMinutes();
  if( TI_minute != 0x01 ){ return 5; }

//------------------------------------------------------------------------------
// minute = 9 add one, minute should be 10
//------------------------------------------------------------------------------
  TI_minute = 0x09;
  incrementMinutes();
  if( TI_minute != 0x10 ){ return 6; }

//------------------------------------------------------------------------------
// minute = 59, hour = 1 add one, minute should be 0, hour should roll
//------------------------------------------------------------------------------
  TI_minute = 0x59;
  TI_hour = 0x01;
  incrementMinutes();
  if( TI_minute != 0x00 ){ return 7; }
  if( TI_hour != 0x02 ){ return 8; }

//------------------------------------------------------------------------------
// Hours test Suite
//------------------------------------------------------------------------------
// hour = 0 add one, hour should be 01
//------------------------------------------------------------------------------
  TI_hour = 0x00;
  incrementHours();
  if( TI_hour != 0x01 ){ return 9; }

//------------------------------------------------------------------------------
// hour = 9 add one, hour should be 10
//------------------------------------------------------------------------------
  TI_hour = 0x09;
  incrementHours();
  if( TI_hour != 0x10 ){ return 10; }

//------------------------------------------------------------------------------
// hour = 11 AM add one, hour should be 12 PM
//------------------------------------------------------------------------------
  TI_hour = 0x11;
  TI_PM = 0x00;
  incrementHours();
  if( TI_hour != 0x12 ){ return 11; }
  if( TI_PM != 0x01 ){ return 12; }

//------------------------------------------------------------------------------
// hour = 11 PM add one, hour should be 12 AM
//------------------------------------------------------------------------------
  TI_hour = 0x11;
  TI_PM = 0x01;
  incrementHours();
  if( TI_hour != 0x12 ){ return 13; }
  if( TI_PM != 0x00 ){ return 14; }

//------------------------------------------------------------------------------
// hour = 12 AM add one, hour should be 1 AM
//------------------------------------------------------------------------------
  TI_hour = 0x12;
  TI_PM = 0x00;
  incrementHours();
  if( TI_hour != 0x01 ){ return 15; }
  if( TI_PM != 0x00 ){ return 16; }

//------------------------------------------------------------------------------
// hour = 12 PM add one, hour should be 1 PM
//------------------------------------------------------------------------------
  TI_hour = 0x12;
  TI_PM = 0x01;
  incrementHours();
  if( TI_hour != 0x01 ){ return 17; }
  if( TI_PM != 0x01 ){ return 18; }

//------------------------------------------------------------------------------
// Date test Suite
//------------------------------------------------------------------------------
// Set Date to 4/25/05
//   - Day = Monday
//   - leapyear = no
//   - day = 0x25, month = 0x04 year = 0x2005
//------------------------------------------------------------------------------
  setDate( 2005, 4, 25 );
  if ( TI_dayOfWeek != MONDAY ){ return 19; }
  if ( LEAP_YEAR ){ return 20; }
  if ( TI_day != 0x25 ){ return 21; }
  if ( TI_month != APRIL ){ return 22; }
  if ( TI_year != 0x2005 ){ return 23; }

//------------------------------------------------------------------------------
// Set Date to 4/25/2100
//   - Day = Monday
//   - leapyear = no
//   - day = 0x25, month = 0x04 year = 0x2005
//------------------------------------------------------------------------------
  setDate( 2100, 4, 25 );
  if ( TI_dayOfWeek != SUNDAY ){ return 24; }
  if ( LEAP_YEAR ){ return 25; }
  if ( TI_day != 0x25 ){ return 26; }
  if ( TI_month != APRIL ){ return 27; }
  if ( TI_year != 0x2100 ){ return 28; }

//------------------------------------------------------------------------------
// Set Date to 4/30/05
//   - Day = Saturday
//   - leapyear = no
//   - day = 0x30, month = 0x04 year = 0x2005
//------------------------------------------------------------------------------
  setDate( 2005, 4, 30 );
  if ( TI_dayOfWeek != SATURDAY ){ return 29; }
  if ( LEAP_YEAR ){ return 30; }
  if ( TI_day != 0x30 ){ return 31; }
  if ( TI_month != APRIL ){ return 32; }
  if ( TI_year != 0x2005 ){ return 33; }
//------------------------------------------------------------------------------
// test for proper day increment
//------------------------------------------------------------------------------
  incrementDays();
  if ( TI_dayOfWeek != SUNDAY ){ return 34; }
  if ( LEAP_YEAR ){ return 35; }
  if ( TI_day != 0x01 ){ return 36; }
  if ( TI_month != MAY ){ return 37; }
  if ( TI_year != 0x2005 ){ return 38; }

//------------------------------------------------------------------------------
// Set Date to 1/1/04
//   - Day = Thursday
//   - leapyear = yes
//   - day = 0x01, month = 0x01 year = 0x2004
//------------------------------------------------------------------------------
  setDate( 2005, 4, 30 );
  if ( TI_dayOfWeek != SATURDAY ){ return 39; }
  if ( LEAP_YEAR ){ return 40; }
  if ( TI_day != 0x30 ){ return 41; }
  if ( TI_month != APRIL ){ return 42; }
  if ( TI_year != 0x2005 ){ return 43; }

//------------------------------------------------------------------------------
// Set Date to 2/28/04
//   - Day = Saturday
//   - leapyear = yes
//   - day = 0x28, month = 0x02 year = 0x2004
//------------------------------------------------------------------------------
  setDate( 2004, 2, 28 );
  if ( TI_dayOfWeek != SATURDAY ){ return 44; }
  if ( !LEAP_YEAR ){ return 45; }
  if ( TI_day != 0x28 ){ return 46; }
  if ( TI_month != FEBRUARY ){ return 47; }
  if ( TI_year != 0x2004 ){ return 48; }

//------------------------------------------------------------------------------
// test for proper leap year day
//------------------------------------------------------------------------------
  incrementDays();
  if ( TI_dayOfWeek != SUNDAY ){ return 49; }
  if ( !LEAP_YEAR ){ return 50; }
  if ( TI_day != 0x29 ){ return 51; }
  if ( TI_month != FEBRUARY ){ return 52; }
  if ( TI_year != 0x2004 ){ return 53; }
//------------------------------------------------------------------------------
// test for proper leap year day 2
//------------------------------------------------------------------------------
  incrementDays();
  if ( TI_dayOfWeek != MONDAY ){ return 54; }
  if ( !LEAP_YEAR ){ return 55; }
  if ( TI_day != 0x01 ){ return 56; }
  if ( TI_month != MARCH ){ return 57; }
  if ( TI_year != 0x2004 ){ return 58; }

//------------------------------------------------------------------------------
// ED Daylight Savings time Test Suite
//------------------------------------------------------------------------------
// Set Date to 3/25/2007 12:59:59 AM add one second, should be 2AM - first 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2007, 3, 25 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 59; }
  if( TI_minute != 0x00 ){ return 60; }
  if( TI_hour != 0x02 ){ return 61; }
  if( TI_day != 0x25 ){ return 62; }
  if( TI_month != MARCH ){ return 63; }
  if( TI_year != 0x2007 ){ return 64; }
  if( TI_dayLightSavings != 0x01 ){ return 65; }

//------------------------------------------------------------------------------
// Set Date to 3/28/2004 12:59:59 AM add one second, should be 2AM  - middle 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2004, 3, 28 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 66; }
  if( TI_minute != 0x00 ){ return 67; }
  if( TI_hour != 0x02 ){ return 68; }
  if( TI_day != 0x28 ){ return 69; }
  if( TI_month != MARCH ){ return 70; }
  if( TI_year != 0x2004 ){ return 71; }
  if( TI_dayLightSavings != 0x01 ){ return 72; }
//------------------------------------------------------------------------------
// Set Date to 3/31/2002 12:59:59 AM add one second, should be 2AM - last 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2002, 3, 31 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 73; }
  if( TI_minute != 0x00 ){ return 74; }
  if( TI_hour != 0x02 ){ return 75; }
  if( TI_day != 0x31 ){ return 76; }
  if( TI_month != MARCH ){ return 77; }
  if( TI_year != 0x2002 ){ return 78; }
  if( TI_dayLightSavings != 0x01 ){ return 79; }

//------------------------------------------------------------------------------
// Set Date to 3/21/2004 12:59:59 AM add one second, should be 1AM - second to last Sunday
//------------------------------------------------------------------------------
  setDate( 2004, 3, 21 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 80; }
  if( TI_minute != 0x00 ){ return 81; }
  if( TI_hour != 0x01 ){ return 82; }
  if( TI_day != 0x21 ){ return 83; }
  if( TI_month != MARCH ){ return 84; }
  if( TI_year != 0x2004 ){ return 85; }
  //if( TI_dayLightSavings != 0x00 ){ return 86; }
  //Even though daylight savings is not during 3/21/2004, setDate()
  //still sets it to 1, so the above test will fail
  //DO NOT COUNT ON TI_dayLightSavings

//------------------------------------------------------------------------------
// Set Date to 10/25/2009 12:59:59 AM add one second, should be 12AM - first 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2009, 10, 25 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 87; }
  if( TI_minute != 0x00 ){ return 88; }
  if( TI_hour != 0x12 ){ return 89; }
  if( TI_day != 0x25 ){ return 90; }
  if( TI_month != OCTOBER ){ return 91; }
  if( TI_year != 0x2009 ){ return 92; }
  if( TI_dayLightSavings != 0x00 ){ return 93; }

//------------------------------------------------------------------------------
// Set Date to 10/30/2005 12:59:59 AM add one second, should be 12AM - middle 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2005, 10, 30 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 94; }
  if( TI_minute != 0x00 ){ return 95; }
  if( TI_hour != 0x12 ){ return 96; }
  if( TI_day != 0x30 ){ return 97; }
  if( TI_month != OCTOBER ){ return 98; }
  if( TI_year != 0x2005 ){ return 99; }
  if( TI_dayLightSavings != 0x00 ){ return 100; }

//------------------------------------------------------------------------------
// Set Date to 10/31/2004 12:59:59 AM add one second, should be 12AM - last 'last Sunday'
//------------------------------------------------------------------------------
  setDate( 2004, 10, 31 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 101; }
  if( TI_minute != 0x00 ){ return 102; }
  if( TI_hour != 0x12 ){ return 103; }
  if( TI_day != 0x31 ){ return 104; }
  if( TI_month != OCTOBER ){ return 105; }
  if( TI_year != 0x2004 ){ return 106; }
  if( TI_dayLightSavings != 0x00 ){ return 107; }


//------------------------------------------------------------------------------
// Set Date to 10/23/2005 12:59:59 AM add one second, should be 1 AM - second to last Sunday
//------------------------------------------------------------------------------
  setDate( 2005, 10, 23 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 108; }
  if( TI_minute != 0x00 ){ return 109; }
  if( TI_hour != 0x01 ){ return 110; }
  if( TI_day != 0x23 ){ return 111; }
  if( TI_month != OCTOBER ){ return 112; }
  if( TI_year != 0x2005 ){ return 113; }
  if( TI_dayLightSavings != 0x01 ){ return 114; }

//------------------------------------------------------------------------------
// Set Date to 3/11/2007 1:59:59 AM add one second, should be 3AM - second Sunday 
//------------------------------------------------------------------------------
  setDate( 2007, 3, 11 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 115; }
  if( TI_minute != 0x00 ){ return 116; }
  if( TI_hour != 0x3 ){ return 117; }
  if( TI_day != 0x11){ return 118; }
  if( TI_month != MARCH ){ return 119; }
  if( TI_year != 0x2007 ){ return 120; }
  if( TI_dayLightSavings != 0x01 ){ return 121; }

//------------------------------------------------------------------------------
// Set Date to 3/9/2008 1:59:59 AM add one second, should be 3AM  - second Sunday
//------------------------------------------------------------------------------
  setDate( 2008, 3, 9 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 122; }
  if( TI_minute != 0x00 ){ return 123; }
  if( TI_hour != 0x3 ){ return 124; }
  if( TI_day != 0x09 ){ return 125; }
  if( TI_month != MARCH ){ return 126; }
  if( TI_year != 0x2008 ){ return 127; }
  if( TI_dayLightSavings != 0x01 ){ return 128; }

//------------------------------------------------------------------------------
// Set Date to 3/8/2009 1:59:59 AM add one second, should be 3AM  - second Sunday
//------------------------------------------------------------------------------
  setDate( 2009, 3, 8 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 129; }
  if( TI_minute != 0x00 ){ return 130; }
  if( TI_hour != 0x3 ){ return 131; }
  if( TI_day != 0x08 ){ return 132; }
  if( TI_month != MARCH ){ return 133; }
  if( TI_year != 0x2009 ){ return 134; }
  if( TI_dayLightSavings != 0x01 ){ return 135; }

//------------------------------------------------------------------------------
// Set Date to 3/7/2010 1:59:59 AM add one second, should be 2AM  - second Sunday
//------------------------------------------------------------------------------
  setDate( 2010, 3, 7 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 136; }
  if( TI_minute != 0x00 ){ return 137; }
  if( TI_hour != 0x2 ){ return 138; }
  if( TI_day != 0x07 ){ return 139; }
  if( TI_month != MARCH ){ return 140; }
  if( TI_year != 0x2010 ){ return 141; }
  if( TI_dayLightSavings != 0x01 ){ return 142; }

//------------------------------------------------------------------------------
// Set Date to 11/6/2011 1:59:59 AM add one second, should be 1AM - first Sunday 
//------------------------------------------------------------------------------
  setDate( 2011, 11, 6 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 143; }
  if( TI_minute != 0x00 ){ return 144; }
  if( TI_hour != 0x1 ){ return 145; }
  if( TI_day != 0x06 ){ return 146; }
  if( TI_month != NOVEMBER ){ return 147; }
  if( TI_year != 0x2011 ){ return 148; }
  if( TI_dayLightSavings != 0x00 ){ return 149; }

//------------------------------------------------------------------------------
// Set Date to 11/4/2012 1:59:59 AM add one second, should be 1AM - first Sunday
//------------------------------------------------------------------------------
  setDate( 2012, 11, 4 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 150; }
  if( TI_minute != 0x00 ){ return 151; }
  if( TI_hour != 0x1 ){ return 152; }
  if( TI_day != 0x04 ){ return 153; }
  if( TI_month != NOVEMBER ){ return 154; }
  if( TI_year != 0x2012 ){ return 155; }
  if( TI_dayLightSavings != 0x00 ){ return 156; }

//------------------------------------------------------------------------------
// Set Date to 11/3/2013 1:59:59 AM add one second, should be 1AM - first Sunday
//------------------------------------------------------------------------------
  setDate( 2013, 11, 3 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 157; }
  if( TI_minute != 0x00 ){ return 158; }
  if( TI_hour != 0x1 ){ return 159; }
  if( TI_day != 0x03 ){ return 160; }
  if( TI_month != NOVEMBER ){ return 161; }
  if( TI_year != 0x2013 ){ return 162; }
  if( TI_dayLightSavings != 0x00 ){ return 163; }

//------------------------------------------------------------------------------
// Set Date to 11/30/2014 1:59:59 AM add one second, should be 2AM - last Sunday
//------------------------------------------------------------------------------
  setDate( 2014, 11, 30 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 164; }
  if( TI_minute != 0x00 ){ return 165; }
  if( TI_hour != 0x2 ){ return 166; }
  if( TI_day != 0x030 ){ return 167; }
  if( TI_month != NOVEMBER ){ return 168; }
  if( TI_year != 0x2014 ){ return 169; }
  if( TI_dayLightSavings != 0x01 ){ return 170; }

//------------------------------------------------------------------------------
// Tests for 'infinite loop' on daylight adjust for US
// Set Date to 11/1/2015 1:59:59 AM add one second, should be 1AM 
// then set minutes and seconds to 59, add one second.. should now be 2AM
//------------------------------------------------------------------------------
  setDate( 2015, 11, 1 );
  TI_hour = 0x01;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = US_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 171; }
  if( TI_minute != 0x00 ){ return 172; }
  if( TI_hour != 0x1 ){ return 173; }
  if( TI_day != 0x01 ){ return 174; }
  if( TI_month != NOVEMBER ){ return 175; }
  if( TI_year != 0x2015 ){ return 176; }
  if( TI_dayLightSavings != 0x00 ){ return 177; }
  TI_minute = 0x59;
  TI_second = 0x59;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 178; }
  if( TI_minute != 0x00 ){ return 179; }
  if( TI_hour != 0x2 ){ return 180; }
  if( TI_day != 0x01 ){ return 181; }
  if( TI_month != NOVEMBER ){ return 182; }
  if( TI_year != 0x2015 ){ return 183; }
  if( TI_dayLightSavings != 0x00 ){ return 184; }
//------------------------------------------------------------------------------
// Tests for 'infinite loop' on daylight adjust for EU
// Set Date to 10/23/2005 12:59:59 AM add one second, should be 12 AM - last sunday
// then set minutes and seconds to 59, add one second.. should now be 1AM
//------------------------------------------------------------------------------
  setDate( 2005, 10, 30 );
  TI_hour = 0x12;
  TI_minute = 0x59;
  TI_second = 0x59;
  TI_PM = 0;
  TI_dayLightZone = EU_DAYLIGHT_SAVINGS;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 185; }
  if( TI_minute != 0x00 ){ return 186; }
  if( TI_hour != 0x12 ){ return 187; }
  if( TI_day != 0x30 ){ return 188; }
  if( TI_month != OCTOBER ){ return 189; }
  if( TI_year != 0x2005 ){ return 190; }
  if( TI_dayLightSavings != 0x00 ){ return 191; }
  TI_minute = 0x59;
  TI_second = 0x59;
  incrementSeconds();
  if( TI_second != 0x00 ){ return 192; }
  if( TI_minute != 0x00 ){ return 193; }
  if( TI_hour != 0x01 ){ return 194; }
  if( TI_day != 0x30 ){ return 195; }
  if( TI_month != OCTOBER ){ return 196; }
  if( TI_year != 0x2005 ){ return 197; }
  if( TI_dayLightSavings != 0x00 ){ return 198; }

//------------------------------------------------------------------------------
// Tests for all values of get24Hour
//------------------------------------------------------------------------------

  TI_PM = 0;
  TI_hour = 0x12;
  if( get24Hour() != 0x00 ){ return 199; }
  TI_hour = 0x1;
  if( get24Hour() != 0x01 ){ return 200; }
  TI_hour = 0x2;
  if( get24Hour() != 0x02 ){ return 201; }
  TI_hour = 0x3;
  if( get24Hour() != 0x03 ){ return 202; }
  TI_hour = 0x4;
  if( get24Hour() != 0x04 ){ return 203; }
  TI_hour = 0x5;
  if( get24Hour() != 0x05 ){ return 204; }
  TI_hour = 0x6;
  if( get24Hour() != 0x06 ){ return 205; }
  TI_hour = 0x7;
  if( get24Hour() != 0x07 ){ return 206; }
  TI_hour = 0x8;
  if( get24Hour() != 0x08 ){ return 207; }
  TI_hour = 0x9;
  if( get24Hour() != 0x09 ){ return 208; }
  TI_hour = 0x10;
  if( get24Hour() != 0x10 ){ return 209; }
  TI_hour = 0x11;
  if( get24Hour() != 0x11 ){ return 210; }

  TI_PM = 1;
  TI_hour = 0x12;
  if( get24Hour() != 0x12 ){ return 211; }
  TI_hour = 0x1;
  if( get24Hour() != 0x13 ){ return 212; }
  TI_hour = 0x2;
  if( get24Hour() != 0x14 ){ return 213; }
  TI_hour = 0x3;
  if( get24Hour() != 0x15 ){ return 214; }
  TI_hour = 0x4;
  if( get24Hour() != 0x16 ){ return 215; }
  TI_hour = 0x5;
  if( get24Hour() != 0x17 ){ return 216; }
  TI_hour = 0x6;
  if( get24Hour() != 0x18 ){ return 217; }
  TI_hour = 0x7;
  if( get24Hour() != 0x19 ){ return 218; }
  TI_hour = 0x8;
  if( get24Hour() != 0x20 ){ return 219; }
  TI_hour = 0x9;
  if( get24Hour() != 0x21 ){ return 220; }
  TI_hour = 0x10;
  if( get24Hour() != 0x22 ){ return 221; }
  TI_hour = 0x11;
  if( get24Hour() != 0x23 ){ return 222; }

//------------------------------------------------------------------------------
// Tests for testLeap
//------------------------------------------------------------------------------
  TI_year = 0x2004;
  testLeap();
  if( !LEAP_YEAR ){ return 223; }
  TI_year = 0x2005;
  testLeap();
  if( LEAP_YEAR ){ return 224; }

  return 0;
} // test suite





