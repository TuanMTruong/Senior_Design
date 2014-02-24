;******************************************************************************
;   Code for application report - "Real Time Clock Library"
;******************************************************************************
; THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
; REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
; INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
; COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
; TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
; POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
; INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
; YOUR USE OF THE PROGRAM.
;
; IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
; CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
; THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
; OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
; OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
; EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
; REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
; OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
; USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
; AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
; YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
; (U.S.$500).
;
; Unless otherwise stated, the Program written and copyrighted
; by Texas Instruments is distributed as "freeware".  You may,
; only under TI's copyright in the Program, use and modify the
; Program without any charge or restriction.  You may
; distribute to third parties, provided that you transfer a
; copy of this license to the third party and the third party
; agrees to these terms by its first use of the Program. You
; must reproduce the copyright notice and any other legend of
; ownership on each copy or partial copy, of the Program.
;
; You acknowledge and agree that the Program contains
; copyrighted material, trade secrets and other TI proprietary
; information and is protected by copyright laws,
; international copyright treaties, and trade secret laws, as
; well as other intellectual property laws.  To protect TI's
; rights in the Program, you agree not to decompile, reverse
; engineer, disassemble or otherwise translate any object code
; versions of the Program to a human-readable form.  You agree
; that in no event will you alter, remove or destroy any
; copyright notice included in the Program.  TI reserves all
; rights not specifically granted under this license. Except
; as specifically provided herein, nothing in this agreement
; shall be construed as conferring by implication, estoppel,
; or otherwise, upon you, any license or other right under any
; TI patents, copyrights or trade secrets.
;
; You may not use the Program in non-TI devices.
;
;******************************************************************************
;   RTC Library
;
;   Description; Sets up Timer A for 1 second interrupts and
;                ticks one second in the ISR
;
;   Note:        This code assumes 32.768kHz XTAL on LFXT1
;
;   L. Westlund / D. Szmulwicz
;   Version    1.1
;   Texas Instruments, Inc
;   January 2011
;   Built with Code Composer Studio Version: 4.2.1
;******************************************************************************
;Change log
;
;1.0 - Inital version - L.Westlund
;1.1 - Changed WDT interrupt vector sect to a define to support all devices
;    - changed to use generic msp430 header file
;******************************************************************************
    .cdecls C,LIST,"msp430.h"
            .ref  incrementSeconds
            .def  TA_1sec_wake

            .text
TA_1sec_wake
            mov.w   #32768,&TACCR1
            mov.w   #CCIE,&TACCTL1          ; TACCR1 interrupt enabled
            mov.w   #TASSEL_1+MC_2,&TACTL   ; ACLK, cont. mode
            bis.b   #GIE,  SR
            ret
;-----------------------------------------------------------------------------
TA_ISR
;-----------------------------------------------------------------------------
            tst.w   &TAIV                   ; read clears flag
            call    #incrementSeconds       ; tick one second
            add.w   #32768, &TACCR1
            reti                            ;		
;-----------------------------------------------------------------------------
;           Interrupt Vectors Used
;-----------------------------------------------------------------------------
            .sect   TIMERA1_VECTOR          ; Timer_A1 Vector
            .short  TA_ISR                  ;
            .end
