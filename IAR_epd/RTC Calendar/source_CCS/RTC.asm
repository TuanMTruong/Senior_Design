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
;   Description; Keeps track of seconds, minutes, AM/PM
;
;   Typical Cycle Count per function:
;   incrementSeconds            14
;   incrementMinutes            14
;   incrementHours              21
;   get24Hour                   23
;
;   Memory Usage:               84 bytes
;                               (4 bytes RAM)
;
;   L. Westlund
;   Version    1.0
;   Texas Instruments, Inc
;   January 2011
;   Built with Code Composer Studio Version: 4.2.1
;******************************************************************************
;Change log
;
;1.0 - Inital version - L.Westlund
;******************************************************************************


            ;Variables
            .def  TI_PM
            .def  TI_second
            .def  TI_minute
            .def  TI_hour
            ;Functions
            .def  incrementSeconds
            .def  incrementMinutes
            .def  incrementHours
            .def  get24Hour

            .data
            .bss TI_second, 1
            .bss TI_minute, 1
            .bss TI_hour, 1
            .bss TI_PM, 1

            .text                           ; code goes in text memory area
;============================================================================
; incrementSeconds
;============================================================================
incrementSeconds
            clrc
            dadd.b  #0x01,   &TI_second     ; tick one second
            cmp.b   #0x60,   &TI_second     ; see if we've hit 60 seconds
            jne     return                  ; if not, return
            clr.b   &TI_second              ; if so, go back to 00
                                            ; fall down to increment minutes
;============================================================================
; incrementMinutes
;============================================================================
incrementMinutes
            clrc
            dadd.b  #0x01,   &TI_minute     ; tick one minute
            cmp.b   #0x60,   &TI_minute     ; see if we've hit 60 minutes
            jne     return                  ; if not, return
            clr.b   &TI_minute              ; if so, go back to 00
                                            ; fall down to increment hours
;============================================================================
; incrementHours
;============================================================================
incrementHours
            clrc
            dadd.b  #0x01,   &TI_hour
            cmp.b   #0x12,   &TI_hour       ; test for 12:00 o'clock
            jne     notSwitchPM             ; if not, don't switch the PM variable
            tst.b   &TI_PM                  ; see if it is PM and we should roll
            jnz     rollAM                  ; PM and roll bit set, change to AM and roll day
            xor.b   #0x01,   &TI_PM         ; change PM value
notSwitchPM
            cmp.b   #0x13,   &TI_hour       ; see if we've hit 13
            jne     return                  ; if not, return
            mov.b   #0x01,   &TI_hour       ; if so, 13 o'clock == 1 o'clock
return      ret                             ; return
rollAM
            clr.b   &TI_PM                  ; clear PM, now it is AM
            ret                             ; return

;============================================================================
; Returns hour in 24 hour format
;============================================================================
get24Hour
            tst.b  &TI_PM
            jnz    afternoon
            cmp.b  #0x12,    &TI_hour
            jeq    zero_hour
            mov.b  &TI_hour, r12
            ret
zero_hour   clr    r12
            ret
afternoon   mov.b  &TI_hour, r12
            cmp    #0x12,    r12
            jeq    return
            clrc
            dadd.b #0x12,    r12
            ret
            .end