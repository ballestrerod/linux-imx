/*
 * Copyright (c) 2020 Leonardo SpA.
 * Author: Davide Ballestrero, Leonardo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/atomic.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <linux/fpga/machxo-efb.h>

#include <linux/uaccess.h>


#define ________        0x00

#define ___X____        0x10
#define __X_____        0x20
#define _X______        0x40
#define X_______        0x80

#define __XX____        0x30
#define _X_X____        0x50
#define _XX_____        0x60
#define _XXX____        0x70

#define X_X_____        0xa0
#define X_XX____        0xb0
#define X_X_X___        0xa8
#define X__X____        0x90

#define XX______        0xc0
#define XXX_____        0xe0

#define XXXX____        0xf0


/********************************************************************
 *    Font MATRIX 7x5                                               *
 ********************************************************************/

const char matrix_bitmap[] = 
{
	/* @0 '!' (1 pixels wide) */
	________, //  
	________, //  
	________, //  
	________, //  
	________, //  

	/* @0 '!' (1 pixels wide) */
	X_______, // #
	X_______, // #
	X_______, // #
	________, //  
	X_______, // #

	/* @7 '"' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	________, //    
	________, //    
	________, //    

	/* @14 '#' (4 pixels wide) */
	_X_X____, //  # #
	XXXX____, // ####
	_X_X____, //  # #
	XXXX____, // ####
	X_X_____, // # # 

	/* @21 '$' (3 pixels wide) */
	_X______, //  # 
	XXX_____, // ###
	_X______, //  # 
	XXX_____, // ###
	_X______, //  # 

	/* @28 '%' (4 pixels wide) */
	X_X_____, // # # 
	_X______, //  #  
	__X_____, //   # 
	_X_X____, //  # #
	X_X_____, // # # 

	/* @35 '&' (4 pixels wide) */
	_X______, //  #  
	_X______, //  #  
	X_XX____, // # ##
	X_X_____, // # # 
	_XXX____, //  ###

	/* @42 ''' (1 pixels wide) */
	X_______, // #
	X_______, // #
	________, //  
	________, //  
	________, //  

	/* @49 '(' (3 pixels wide) */
	_X______, //  # 
	X_______, // #  
	X_______, // #  
	X_______, // #  
	_X______, //  # 

	/* @56 ')' (3 pixels wide) */
	_X______, //  # 
	__X_____, //   #
	__X_____, //   #
	__X_____, //   #
	_X______, //  # 

	/* @63 '*' (3 pixels wide) */
	_X______, //  # 
	X_X_____, // # #
	_X______, //  # 
	X_X_____, // # #
	________, //    

	/* @70 '+' (3 pixels wide) */
	________, //    
	_X______, //  # 
	XXX_____, // ###
	_X______, //  # 
	________, //    

	/* @77 ',' (1 pixels wide) */
	________, //  
	________, //  
	________, //  
	X_______, // #
	X_______, // #

	/* @84 '-' (3 pixels wide) */
	________, //    
	________, //    
	XXX_____, // ###
	________, //    
	________, //    

	/* @91 '.' (1 pixels wide) */
	________, //  
	________, //  
	________, //  
	________, //  
	X_______, // #

	/* @98 '/' (4 pixels wide) */
	___X____, //    #
	__X_____, //   # 
	_X______, //  #  
	_X______, //  #  
	X_______, // #   

	/* @105 '0' (3 pixels wide) */
	_X______, //  # 
	X_X_____, // # #
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 

	/* @112 '1' (3 pixels wide) */
	_X______, //  # 
	XX______, // ## 
	_X______, //  # 
	_X______, //  # 
	XXX_____, // ###

	/* @119 '2' (3 pixels wide) */
	XXX_____, // ###
	__X_____, //   #
	_XX_____, //  ##
	X_______, // #  
	XXX_____, // ###

	/* @126 '3' (3 pixels wide) */
	XX______, // ## 
	__X_____, //   #
	XX______, // ## 
	__X_____, //   #
	XX______, // ## 

	/* @133 '4' (3 pixels wide) */
	X_X_____, // # # 
	X_X_____, // # # 
	XXX_____, // ### 
	__X_____, //   #
	__X_____, //   # 

	/* @140 '5' (3 pixels wide) */
	XXX_____, // ###
	X_______, // #  
	XX______, // ## 
	__X_____, //   #
	XX______, // ## 

	/* @147 '6' (3 pixels wide) */
	XXX_____, // ###
	X_______, // #  
	XXX_____, // ###
	X_X_____, // # #
	XXX_____, // ###

	/* @154 '7' (3 pixels wide) */
	XXX_____, // ###
	__X_____, //   #
	_X______, //  # 
	_X______, //  # 
	_X______, //  # 

	/* @161 '8' (3 pixels wide) */
	XXX_____, //  # 
	X_X_____, // # #
	XXX_____, //  # 
	X_X_____, // # #
	XXX_____, //  # 

	/* @168 '9' (3 pixels wide) */
	XXX_____, // ###
	X_X_____, // # #
	XXX_____, // ###
	__X_____, //   #
	__X_____, //   #

	/* @175 ':' (1 pixels wide) */
	________, //  
	________, //  
	X_______, // #
	________, //  
	X_______, // #

	/* @182 ';' (1 pixels wide) */
	________, //  
	X_______, // #
	________, //  
	X_______, // #
	X_______, // #

	/* @189 '<' (3 pixels wide) */
	__X_____, //   #
	_X______, //  # 
	X_______, // #  
	_X______, //  # 
	__X_____, //   #

	/* @196 '=' (3 pixels wide) */
	________, //    
	XXX_____, // ###
	________, //    
	XXX_____, // ###
	________, //    

	/* @203 '>' (3 pixels wide) */
	X_______, // #  
	_X______, //  # 
	__X_____, //   #
	_X______, //  # 
	X_______, // #  

	/* @210 '?' (3 pixels wide) */
	XX______, // ## 
	__X_____, //   #
	_X______, //  # 
	________, //    
	_X______, //  # 

	/* @217 '@' (4 pixels wide) */
	_X_X____, //  # #
	X__X____, // #  #
	X_XX____, // # ##
	X_______, // #   
	_XXX____, //  ###

	/* @224 'A' (4 pixels wide) */
	_X______, //  # 
	X_X_____, // # #
	XXX_____, // ###
	X_X_____, // # #
	X_X_____, // # #

	/* @231 'B' (3 pixels wide) */
	XX______, // ## 
	X_X_____, // # #
	XX______, // ## 
	X_X_____, // # #
	XXX_____, // ###

	/* @238 'C' (3 pixels wide) */
	_XX_____, //  ##
	X_______, // #  
	X_______, // #  
	X_______, // #  
	_XX_____, //  ##

	/* @245 'D' (3 pixels wide) */
	XX______, // ## 
	X_X_____, // # #
	X_X_____, // # #
	X_X_____, // # #
	XX______, // ## 

	/* @252 'E' (3 pixels wide) */
	XXX_____, // ###
	X_______, // #  
	XXX_____, // ###
	X_______, // #  
	XXX_____, // ###

	/* @259 'F' (3 pixels wide) */
	XXX_____, // ###
	X_______, // #  
	XXX_____, // ###
	X_______, // #  
	X_______, // #  

	/* @266 'G' (3 pixels wide) */
	_XX_____, //  ##
	X_______, // #  
	X_X_____, // # #
	X_X_____, // # #
	_XX_____, //  ##

	/* @273 'H' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	XXX_____, // ###
	X_X_____, // # #
	X_X_____, // # #

	/* @280 'I' (3 pixels wide) */
	XXX_____, // ###
	_X______, //  # 
	_X______, //  # 
	_X______, //  # 
	XXX_____, // ###

	/* @287 'J' (3 pixels wide) */
	_XX_____, //  ##
	__X_____, //   #
	__X_____, //   #
	__X_____, //   #
	XX______, // ## 

	/* @294 'K' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	XX______, // ## 
	X_X_____, // # #
	X_X_____, // # #

	/* @301 'L' (3 pixels wide) */
	X_______, // #  
	X_______, // #  
	X_______, // #  
	X_______, // #  
	XXX_____, // ###

	/* @308 'M' (4 pixels wide) */
	X__X____, // #  #
	XXXX____, // ####
	XXXX____, // ####
	X__X____, // #  #
	X__X____, // #  #

	/* @315 'N' (3 pixels wide) */
	X_X_____, // # #
	XXX_____, // ###
	XXX_____, // ###
	X_X_____, // # #
	X_X_____, // # #

	/* @322 'O' (4 pixels wide) */
	_XX_____, //  ## 
	X__X____, // #  #
	X__X____, // #  #
	X__X____, // #  #
	_XX_____, //  ## 

	/* @329 'P' (3 pixels wide) */
	XX______, // ## 
	X_X_____, // # #
	XX______, // ## 
	X_______, // #  
	X_______, // #  

	/* @336 'Q' (4 pixels wide) */
	_XX_____, //  ## 
	X__X____, // #  #
	X__X____, // #  #
	_XX_____, //  ## 
	__XX____, //   ##

	/* @343 'R' (3 pixels wide) */
	XX______, // ## 
	X_X_____, // # #
	XX______, // ## 
	X_X_____, // # #
	X_X_____, // # #

	/* @350 'S' (3 pixels wide) */
	_XX_____, //  ##
	X_______, // #  
	_X______, //  # 
	__X_____, //   #
	XX______, // ## 

	/* @357 'T' (3 pixels wide) */
	XXX_____, // ###
	_X______, //  # 
	_X______, //  # 
	_X______, //  # 
	_X______, //  # 

	/* @364 'U' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 

	/* @371 'V' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 
	_X______, //  # 

	/* @378 'W' (4 pixels wide) */
	X__X____, // #  #
	X__X____, // #  #
	XXXX____, // ####
	XXXX____, // ####
	_XX_____, //  ## 

	/* @385 'X' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 
	X_X_____, // # #
	X_X_____, // # #

	/* @392 'Y' (3 pixels wide) */
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 
	_X______, //  # 
	_X______, //  # 

	/* @399 'Z' (3 pixels wide) */
	XXX_____, // ###
	__X_____, //   #
	_X______, //  # 
	X_______, // #  
	XXX_____, // ###

	/* @406 '[' (2 pixels wide) */
	XX______, // ##
	X_______, // # 
	X_______, // # 
	X_______, // # 
	XX______, // ##

	/* @413 '\' (4 pixels wide) */
	X_______, // #   
	_X______, //  #  
	__X_____, //   # 
	__X_____, //   # 
	___X____, //    #

	/* @420 ']' (2 pixels wide) */
	XX______, // ##
	_X______, //  #
	_X______, //  #
	_X______, //  #
	XX______, // ##

	/* @427 '^' (3 pixels wide) */
	_X______, //  # 
	X_X_____, // # #
	________, //    
	________, //    
	________, //    

	/* @434 '_' (4 pixels wide) */
	________, //     
	________, //     
	________, //     
	________, //     
	XXXX____, // ####

	/* @441 '`' (2 pixels wide) */
	X_______, // # 
	_X______, //  #
	________, //   
	________, //   
	________, //   

	/* @448 'a' (3 pixels wide) */
	________, //    
	_XX_____, //  ##
	X_X_____, // # #
	_XX_____, //  ##
	________, //    

	/* @455 'b' (3 pixels wide) */
	X_______, // #  
	XX______, // ## 
	X_X_____, // # #
	XX______, // ## 
	________, //    

	/* @462 'c' (3 pixels wide) */
	________, //    
	_XX_____, //  ##
	X_______, // #  
	_XX_____, //  ##
	________, //    

	/* @469 'd' (3 pixels wide) */
	__X_____, //   #
	_XX_____, //  ##
	X_X_____, // # #
	_XX_____, //  ##
	________, //    

	/* @476 'e' (3 pixels wide) */
	_X______, //  # 
	X_X_____, // # #
	XX______, // ## 
	_XX_____, //  ##
	________, //    

	/* @483 'f' (2 pixels wide) */
	XX______, // ##
	X_______, // # 
	XX______, // ## 
	X_______, // # 
	________, //   

	/* @490 'g' (3 pixels wide) */
	________, //   
	XXX_____, // ###
	X_X_____, // # #
	_XX_____, //  ##
	XX______, // ## 

	/* @497 'h' (3 pixels wide) */
	X_______, // #  
	XX______, // ## 
	X_X_____, // # #
	X_X_____, // # #
	________, //   

	/* @504 'i' (1 pixels wide) */
	X_______, //  #
	________, //   
	X_______, //  #
	X_______, //  #
	________, //   

	/* @511 'j' (2 pixels wide) */
	_X______, //  #
	________, //   
	_X______, //  #
	XX______, // ##
	________, //   

	/* @518 'k' (3 pixels wide) */
	X_______, // #  
	X_X_____, // # #
	XX______, // ## 
	X_X_____, // # #
	________, //    

	/* @525 'l' (1 pixels wide) */
	X_______, // #
	X_______, // #
	X_______, // #
	X_______, // #
	________, // 

	/* @532 'm' (5 pixels wide) */
	________, // #  #
	_X_X____, // ####
	X_X_X___, // #  #
	X_X_X___, // #  #
	________, //     

	/* @539 'n' (3 pixels wide) */
	________, //    
	XX______, // ## 
	X_X_____, // # #
	X_X_____, // # #
	________, //    

	/* @546 'o' (3 pixels wide) */
	________, //    
	XXX_____, // ###
	X_X_____, // # #
	XXX_____, // ###
	________, //    

	/* @553 'p' (3 pixels wide) */
	________, //    
	XX______, // ## 
	X_X_____, // # #
	XX______, // ## 
	X_______, // #  

	/* @560 'q' (3 pixels wide) */
	________, //    
	_XX_____, //  ##
	X_X_____, // # #
	_XX_____, //  ##
	__X_____, //   #

	/* @567 'r' (2 pixels wide) */
	________, //    
	XX______, // ## 
	X_______, // #  
	X_______, // #  
	________, //    

	/* @574 's' (3 pixels wide) */
	________, //    
	_XX_____, //  ##
	_X______, //  # 
	XX______, // ## 
	________, //    

	/* @581 't' (3 pixels wide) */
	_X______, //  # 
	XXX_____, // ###
	_X______, //  # 
	_XX_____, //  ## 
	________, // 

	/* @588 'u' (3 pixels wide) */
	________, //    
	X_X_____, // # #
	X_X_____, // # #
	_XX_____, //  ##
	________, //    

	/* @595 'v' (3 pixels wide) */
	________, //    
	X_X_____, // # #
	X_X_____, // # #
	_X______, //  # 
	________, //    

	/* @602 'w' (5 pixels wide) */
	________, //     
	X_X_X___, // # # #
	X_X_X___, // # # #
	_X_X____, //  # #
	________, //     

	/* @609 'x' (3 pixels wide) */
	________, //    
	X_X_____, // # #
	_X______, //  # 
	X_X_____, // # #
	________, //    

	/* @616 'y' (3 pixels wide) */
	________, //    
	X_X_____, // # #
	X_X_____, // # #
	_XX_____, //  ##
	XX______, // ## 

	/* @623 'z' (3 pixels wide) */
	________, //    
	XX______, // ## 
	_X______, //  # 
	_XX_____, //  ##
	________, //    

	/* @630 '{' (3 pixels wide) */
	_X______, //  # 
	_X______, //  # 
	X_______, // #  
	_X______, //  # 
	_X______, //  # 

	/* @637 '|' (1 pixels wide) */
	X_______, // #
	X_______, // #
	X_______, // #
	X_______, // #
	X_______, // #

	/* @644 '}' (3 pixels wide) */
	_X______, //  # 
	_X______, //  # 
	__X_____, //   #
	_X______, //  # 
	_X______, //  # 

	/* @651 '~' (4 pixels wide) */
	________, //     
	________, //     
	_X_X____, //  # #
	X_X_____, // # # 
	________, //     
};

/* Character descriptors for Lucida Console 5pt */
/* { [Char width in bits], [Char height in bits], [Offset into lucidaConsole_5ptCharBitmaps in bytes] } */
const unsigned int matrix_bitmap_descriptor[] = 
{
	1, 		/* SPACE */ 
	1, 		/* ! */ 
	3, 		/* " */ 
	4, 		/* # */ 
	3, 		/* $ */ 
	4, 		/* % */ 
	4, 		/* & */ 
	1, 		/* ' */ 
	3, 		/* ( */ 
	3, 		/* ) */ 
	3, 		/* * */ 
	3, 		/* + */ 
	1, 		/* , */ 
	3, 		/* - */ 
	1, 		/* . */ 
	4, 		/* / */ 
	3, 		/* 0 */ 
	3, 		/* 1 */ 
	3, 		/* 2 */ 
	3, 		/* 3 */ 
	3, 		/* 4 */ 
	3, 		/* 5 */ 
	3, 		/* 6 */ 
	3, 		/* 7 */ 
	3, 		/* 8 */ 
	3, 		/* 9 */ 
	1, 		/* : */ 
	1, 		/* ; */ 
	3, 		/* < */ 
	3, 		/* = */ 
	3, 		/* > */ 
	3, 		/* ? */ 
	4, 		/* @ */ 
	3, 		/* A */ 
	3, 		/* B */ 
	3, 		/* C */ 
	3, 		/* D */ 
	3, 		/* E */ 
	3, 		/* F */ 
	3, 		/* G */ 
	3, 		/* H */ 
	3, 		/* I */ 
	3, 		/* J */ 
	3, 		/* K */ 
	3, 		/* L */ 
	4, 		/* M */ 
	3, 		/* N */ 
	4, 		/* O */ 
	3, 		/* P */ 
	4, 		/* Q */ 
	3, 		/* R */ 
	3, 		/* S */ 
	3, 		/* T */ 
	3, 		/* U */ 
	3, 		/* V */ 
	4, 		/* W */ 
	3, 		/* X */ 
	3, 		/* Y */ 
	3, 		/* Z */ 
	2, 		/* [ */ 
	4, 		/* \ */ 
	2, 		/* ] */ 
	3, 		/* ^ */ 
	4, 		/* _ */ 
	2, 		/* ` */ 
	3, 		/* a */ 
	3, 		/* b */ 
	3, 		/* c */ 
	3, 		/* d */ 
	3, 		/* e */ 
	2, 		/* f */ 
	3, 		/* g */ 
	3, 		/* h */ 
	1, 		/* i */ 
	2, 		/* j */ 
	3, 		/* k */ 
	1, 		/* l */ 
	5, 		/* m */ 
	3, 		/* n */ 
	3, 		/* o */ 
	3, 		/* p */ 
	3, 		/* q */ 
	2, 		/* r */ 
	3, 		/* s */ 
	3, 		/* t */ 
	3, 		/* u */ 
	3, 		/* v */ 
	5, 		/* w */ 
	3, 		/* x */ 
	3, 		/* y */ 
	3, 		/* z */ 
	3, 		/* { */ 
	1, 		/* | */ 
	3, 		/* } */ 
	4, 		/* ~ */ 
};



/* str = "Ciao" */
/* DOT-ROWs
 *
 *  +----------------+      0x60 0x40 0x00 0x00
 *  +      [0]       +      0x80 0x00 0xC0 0x40 
 *  +----------------+      0x80 0xC0 0x20 0xA0
 *  +      [1]       +      0x80 0x40 0xE0 0xA0 
 *  +----------------+      0x60 0x40 0xE0 0x40
 *  +      [2]       +                          
 *  +----------------+        ##  #         
 *  +      [3]       +       #      ##    #   
 *  +----------------+       #   ##   #  # #
 *  +      [4]       +       #    # ###  # #  
 *  +----------------+        ##  # ###   # 
 *
 *
 *
 */


#define ENOBU_MATRIXDISP_BASE_REG               0x20
#define ENOBU_MATRIXDISP_MODE_REG               0x34

#define MATRIXDISP_MODE_FIXED	                0x00
#define MATRIXDISP_MODE_SCROLL	                0x01

#define MATRIX_ROW_HEIGHT   5


static void matrixdisp_write_mode(u8 mode)
{
  efb_spi_write(ENOBU_MATRIXDISP_MODE_REG, mode);
}


static void matrixdisp_write_row(u8 val, int rown)
{
  efb_spi_write(ENOBU_MATRIXDISP_BASE_REG + rown, val);
}


/*************************************/
/* Visualizzazione a caratteri FIXED */
/*************************************/

static void matrixdisp_chr_fill(u_int8_t *dotrow)
{
    u_int8_t fpga_reg;
    int i;

    for (i = 0; i < MATRIX_ROW_HEIGHT; i++, dotrow++) {
        fpga_reg = *dotrow;

        matrixdisp_write_row(fpga_reg, i);
    }

    return;
}

static int matrixdisp_chr_display(const char *dispchr)
{
    char *chr_pxl_bitmap, *chr_str;
    int i, j, chr_offset;
    int chr_pxl_width;
    int curr_width = 0;
    u_int8_t dot_row[MATRIX_ROW_HEIGHT];
    u_int8_t dot_row_tmp;
    
    /* Prendo la stringa e punto al primo carattere */
    chr_str = (char *)dispchr;

    memset(dot_row, 0, sizeof(u_int8_t) * MATRIX_ROW_HEIGHT);
    
    for (j = 0; j < strlen(dispchr); j++, chr_str++) {
    
        if (*chr_str < 0x20 || *chr_str > 0x7e)
            continue;
    
        chr_offset = (*chr_str) - 0x20;
    
        chr_pxl_bitmap = (char *) &matrix_bitmap[chr_offset * 5]; 
        chr_pxl_width = matrix_bitmap_descriptor[chr_offset];
 
        if ((curr_width + chr_pxl_width + 1) > 7)
            break;
    
        /* scrivo il carattere nella sua casella REDUCED (7-bit) */
        for (i = 0, dot_row_tmp = 0; i < MATRIX_ROW_HEIGHT; i++, chr_pxl_bitmap++) {
    
            dot_row_tmp = ((u_int8_t) *chr_pxl_bitmap) >> 1;
            dot_row[i] |= dot_row_tmp >> curr_width;
        }
    
        /* avanzo nei pixel */
        curr_width += chr_pxl_width + 1;
    }
    
    /* scrivo byte a byte la stringa nella FPGA */
    matrixdisp_chr_fill(dot_row);

    return 0;
}


/* Public functions */

int matrixdisp_char(const char *dchr)
{
    matrixdisp_write_mode(MATRIXDISP_MODE_FIXED);
    matrixdisp_chr_display(dchr);

    printk(KERN_ERR "[MATRIX-CHAR] %s\n", dchr);

    return 0;
}



/***************************************/
/* Visualizzazione a stringa SCROLLING */
/***************************************/

void matrixdisp_str_fill(u_int32_t *dotrow)
{
    u_int8_t fpga_reg, fpga_regb, fpga_regc, fpga_regd;
    int i;

    for (i = 0; i < MATRIX_ROW_HEIGHT; i++, dotrow++) {
        fpga_reg = (char)((*dotrow & 0xff000000) >> 24);
        fpga_regb = (char)((*dotrow & 0x00ff0000) >> 16);
        fpga_regc = (char)((*dotrow & 0x0000ff00) >> 8);
        fpga_regd = (char)(*dotrow & 0x000000ff);
        
        matrixdisp_write_row(fpga_reg, i);
        matrixdisp_write_row(fpga_regb, i + 5);
        matrixdisp_write_row(fpga_regc, i + 10);
        matrixdisp_write_row(fpga_regd, i + 15);
    }

    return;
}



int matrixdisp_str_display(const char *dispstr)
{
    char *chr_pxl_bitmap, *chr_str;
    int i, j, chr_offset;
    int chr_pxl_width;
    int curr_width = 0;
    u_int32_t dot_row[MATRIX_ROW_HEIGHT];
    u_int32_t dot_row_tmp;
    
    memset(dot_row, 0, sizeof(u_int32_t) * MATRIX_ROW_HEIGHT);
    
    /* Prendo la stringa e punto al primo carattere */
    chr_str = (char *)dispstr;
    
    for (j = 0; j < strlen(dispstr); j++, chr_str++) {
    
        if (*chr_str < 0x20 || *chr_str > 0x7e)
            continue;
    
        chr_offset = (*chr_str) - 0x20;
    
        chr_pxl_bitmap = (char *) &matrix_bitmap[chr_offset * 5]; 
        chr_pxl_width = matrix_bitmap_descriptor[chr_offset];
            
        if ((curr_width + chr_pxl_width + 1) > 32)
            break;
    
        /* scrivo il carattere nella sua casella EXTENDED (8-bit)*/
        for (i = 0, dot_row_tmp = 0; i < MATRIX_ROW_HEIGHT; i++, chr_pxl_bitmap++) {
    
            dot_row_tmp = ((u_int32_t) *chr_pxl_bitmap) << 24;
            dot_row[i] |= dot_row_tmp >> curr_width;
        }
    
        /* avanzo nei pixel */
        curr_width += chr_pxl_width + 1;
    }
    
    /* scrivo byte a byte la stringa nella FPGA */
    matrixdisp_str_fill(dot_row);
    
    return 0;
}


int matrixdisp_prnt(const char *dstr)
{
    matrixdisp_write_mode(MATRIXDISP_MODE_SCROLL);
    matrixdisp_str_display(dstr);

    printk(KERN_ERR "[MATRIX-PRINT] %s\n", dstr);

    return 0;
}


/* Device single-open policy control */
static atomic_t ledmatrix_available = ATOMIC_INIT(1);

//OLDCHRDEV static int ledmatrix_major; /* dynamic allocation */
//OLDCHRDEV static struct class *ledmatrix_class;

#define DEVICE_NAME		"ledmatrix"
#define LEDMATRIX_BUFSIZ        64

struct enobu_ledmatrix {
        u8	                *ledmatrix_buffer;
	struct miscdevice       miscdev;
        u8                      mode;
};


static ssize_t show_ledmatrix_mode(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct enobu_ledmatrix *ldmtrx = dev_get_drvdata(dev);

        //TODO or NOT read from SPI ???
	switch (ldmtrx->mode) {
	        case MATRIXDISP_MODE_FIXED:
	        	return sprintf(buf, "fixed\n");
	        case MATRIXDISP_MODE_SCROLL:
	        	return sprintf(buf, "scroll\n");
	        default:
	        	return sprintf(buf, "unknown state\n");
	}
}


static ssize_t store_ledmatrix_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct enobu_ledmatrix *ldmtrx = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "fixed")) {
                matrixdisp_write_mode(MATRIXDISP_MODE_FIXED);
                ldmtrx->mode = MATRIXDISP_MODE_FIXED;
        } else if (sysfs_streq(buf, "scroll")) {
                matrixdisp_write_mode(MATRIXDISP_MODE_SCROLL);
                ldmtrx->mode = MATRIXDISP_MODE_SCROLL;
        } else {
		return -EINVAL;
	}

	return n;
}

static DEVICE_ATTR(ledmatrix_mode, 0644, show_ledmatrix_mode, store_ledmatrix_mode);







/* Misc device layer */

static inline struct enobu_ledmatrix *to_enobu_ledmatrix_data(struct file *file)
{
	struct miscdevice *dev = file->private_data;
	return container_of(dev, struct enobu_ledmatrix, miscdev);
}


static long enobu_ledmatrix_ioctl(struct file *filp, u_int cmd,
		     u_long arg)
{
	int ret = 0;

        //TODO with ioctl select scrolling or fixed
        //     saving in a private data stuct and choosing
        //     during write cmd

	switch (cmd) {
	default:
		{
			ret = -EINVAL;
			break;
		}
	}
	return ret;
}



static ssize_t enobu_ledmatrix_write(struct file *file, const char __user *buf,
			     size_t count, loff_t *ppos)
{
        struct enobu_ledmatrix *ldmtrx = to_enobu_ledmatrix_data(file);

	if (count > LEDMATRIX_BUFSIZ)
		return -EMSGSIZE;

	// mutex_lock(&spidev->buf_lock);

	if (copy_from_user(ldmtrx->ledmatrix_buffer, buf, count))
		return -EFAULT;

        if (ldmtrx->mode == MATRIXDISP_MODE_FIXED)
                matrixdisp_char(ldmtrx->ledmatrix_buffer);
        else
                matrixdisp_prnt(ldmtrx->ledmatrix_buffer);

	// mutex_unlock(&spidev->buf_lock);

	return count;
}


static int enobu_ledmatrix_open(struct inode *inode, struct file *file)
{
        struct enobu_ledmatrix *ldmtrx = to_enobu_ledmatrix_data(file);
	int ret;

	ret = -EBUSY;
	if (!atomic_dec_and_test(&ledmatrix_available))
		goto fail;	/* open only once at a time */

	ret = -EPERM;
	if (file->f_mode & FMODE_READ)	/* device is write-only */
		goto fail;

	if (!ldmtrx->ledmatrix_buffer) {
		ldmtrx->ledmatrix_buffer = kmalloc(LEDMATRIX_BUFSIZ, GFP_KERNEL);
		if (!ldmtrx->ledmatrix_buffer) {
			pr_debug("ledmatrix: open/ENOMEM\n");
			ret = -ENOMEM;
			goto fail;
		}
	}

        pr_debug("2. ledmatrix: MODE %x\n", ldmtrx->mode);

        memset(ldmtrx->ledmatrix_buffer, 0, LEDMATRIX_BUFSIZ);

        pr_debug("3. ledmatrix: MODE %x\n", ldmtrx->mode);

	return nonseekable_open(inode, file);

 fail:
	kfree(ldmtrx->ledmatrix_buffer);
        ldmtrx->ledmatrix_buffer = NULL;
	atomic_inc(&ledmatrix_available);
	return ret;
}


static int enobu_ledmatrix_release(struct inode *inode, struct file *file)
{
        struct enobu_ledmatrix *ldmtrx = to_enobu_ledmatrix_data(file);

	kfree(ldmtrx->ledmatrix_buffer);
        ldmtrx->ledmatrix_buffer = NULL;
	atomic_inc(&ledmatrix_available);
	return 0;
}



static const struct file_operations enobu_ledmatrix_fops = {
	.owner          = THIS_MODULE,
        .write          = enobu_ledmatrix_write,
	.open           = enobu_ledmatrix_open,
	.release        = enobu_ledmatrix_release,
	.unlocked_ioctl = enobu_ledmatrix_ioctl,
	.llseek         = no_llseek,
};




static int enobu_ledmatrix_probe(struct platform_device *pdev)
{
        struct enobu_ledmatrix *ldmtrx;
	struct device *dev = &pdev->dev;
	int ret = 0;
        
	ldmtrx = devm_kzalloc(dev, sizeof(struct enobu_ledmatrix), GFP_KERNEL);
	if (!ldmtrx)
		return -ENOMEM;

        ldmtrx->mode = MATRIXDISP_MODE_SCROLL;
        ldmtrx->ledmatrix_buffer = NULL;

        //TODO register sysfs files
	/* choose ledmatrix mode */
	ret = device_create_file(dev, &dev_attr_ledmatrix_mode);
	if (ret < 0)
		goto out;

        pr_debug("1. ledmatrix: MODE %x\n", ldmtrx->mode);

	ldmtrx->miscdev.minor	= MISC_DYNAMIC_MINOR;
	ldmtrx->miscdev.name	= DEVICE_NAME;
	ldmtrx->miscdev.fops	= &enobu_ledmatrix_fops;
	ldmtrx->miscdev.parent	= &pdev->dev;

        //TODO eventually request device resources

	/* register misc device */
	ret = misc_register(&ldmtrx->miscdev);
	if (ret < 0) {
		dev_err(dev, "misc_register() failed\n");
		goto out;
	}

	platform_set_drvdata(pdev, ldmtrx);

        pr_debug("0. ledmatrix: MODE %x\n", ldmtrx->mode);

        dev_info(dev, "eNOBU LED matrix initialized\n");
        return 0;

out:        
	return ret;
}


static const struct of_device_id enobu_ledmatrix_of_match[] = {
	{ .compatible = "leonardo,enobu-fpga-ledmatrix" },
	{ }
};
MODULE_DEVICE_TABLE(of, enobu_ledmatrix_of_match);

static struct platform_driver enobu_ledmatrix_driver = {
	.probe = enobu_ledmatrix_probe,
	.driver = {
		.name = "enobu-fpga-ledmatrix",
		.of_match_table = of_match_ptr(enobu_ledmatrix_of_match),
	},
};

module_platform_driver(enobu_ledmatrix_driver);

MODULE_AUTHOR("Davide Ballestrero, Leonardo");
MODULE_DESCRIPTION("Driver for eNOBU FPGA LED matrix");
MODULE_LICENSE("GPL");
