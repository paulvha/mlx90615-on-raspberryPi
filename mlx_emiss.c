/*
 * MLX90615 - infra read sensor
 *
 * ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ***************************************************************************
 *
 * This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 ***************************************************************************
 * 
 * 
 * version 1.0 / paulvha / April 2017
 * 
 *  initial version ofthe program
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mlx90615.h"

typedef struct {
	char* type;
	char* material;
	char* temp;
	char* emis;
} lookuptab;

/* This table is based on emissivity table for I.R. readers created and
 * owned by Scigiene Corp. www.scigiene.com. tel 416-261-4865
 * 
 * The only changes made were to set the emissivity level to average if
 * the original table had a range of values instead of single value */
 
lookuptab emis_table [] = {
{"Asbestos ","Board ","100 (38)  ","0.96 "},
{"Asbestos ","Cement  ","32-392 (0-200) ","0.96 "},
{"Asbestos ","Cement, Red  ","2500 (1371) ","0.67 "},
{"Asbestos ","Cement, White  ","2500 (1371)  ","0.65 "},
{"Asbestos ","Cloth ","199 (93) ","0.9 "},
{"Asbestos ","Paper  ","100-700 (38-371) ","0.93 "},
{"Asbestos ","Slate  ","68 (20) ","0.97 "},
{"Asbestos ","Asphalt, pavement  ","100 (38)  ","0.93 "},
{"Asbestos ","Asphalt, tar paper  ","68 (20)  ","0.93 "},
{"Asbestos ","Basalt ","68 (20) ","0.72"},
{"Brick ","Adobe  ","68 (20)  ","0.9  "},
{"Brick ","Red, rough ","70 (21)  ","0.93 "},
{"Brick ","Gault Cream ","2500-5000 (1371-2760) ","0.30 "},
{"Brick ","Fire Clay  ","2500 (1371)  ","0.75 "},
{"Brick ","Light Buff  ","1000 (538)  ","0.8 "},
{"Brick ","Lime Clay  ","2500 (1371) ","0.43 "},
{"Brick ","Fire Brick ","1832 (1000) ","0.80 "},
{"Brick ","Magnesite, Refractory  ","1832 (1000) ","0.38 "},
{"Brick ","Grey Brick ","2012 (1100) ","0.75 "},
{"Brick ","Silica, Glazed  ","2000 (1093) ","0.88 "},
{"Brick ","Silica, Unglazed  ","2000 (1093) ","0.8 "},
{"Brick ","Sandlime  ","2500-5000 (1371-2760) ","0.60 "},
{"Brick ","Carborundum ","1850 (1010)  ","0.92 "},
{"Ceramic ","Alumina on Inconel ","800-2000 (427-1093) ","0.69 "},
{"Ceramic ","Earthenware, Glazed  ","70 (21) ","0.9 "},
{"Ceramic ","Earthenware, Matte  ","70 (21) ","0.93 "},
{"Ceramic ","Greens No. 5210-2C  "," 200-750 (93-399)  ","0.85 "},
{"Ceramic ","Coating No. C20A   ","200-750 (93-399)  ","0.69 "},
{"Ceramic ","Porcelain  ","72 (22) ","0.92 "},
{"Ceramic ","White Al2O3 ","200 (93) ","0.9 "},
{"Ceramic ","Zirconia on Inconel ","800-2000 (427-1093) ","0.57 "},
{"Ceramic ","Clay  ","68 (20) ","0.39 "},
{"Ceramic ","Fired ","158 (70) ","0.91 "},
{"Ceramic ","Shale "," 68 (20) ","0.69 "},
{"Ceramic ","Tiles, Light Red "," 2500-5000 (1371-2760) ","0.33 "},
{"Ceramic ","Tiles, Red ","2500-5000 (1371-2760) ","0.45 "},
{"Ceramic ","Tiles,Dark Purple  ","2500-5000 (1371-2760) ","0.78 "},
{"Concrete ","Rough  ","32-2000 (0-1093) ","0.94 "},
{"Concrete ","Tiles, Natural  ","2500-5000 (1371-2760) "," 0.63 "},
{"Concrete ","Brown ","2500-5000 (1371-2760) ","0.84 "},
{"Concrete ","Black ","2500-5000 (1371-2760) ","0.92 "},
{"Concrete ","Cotton Cloth  ","68 (20)  ","0.77 "},
{"Concrete ","Dolomite Lime ","68 (20) ","0.41 "},
{"Concrete ","Emery Corundum  ","176 (80) ","0.86 "},
{"Glass ","Convex D  ","212 (100) ","0.8 "},
{"Glass ","Convex D  ","600 (316) ","0.8 "},
{"Glass ","Convex D  ","932 (500) ","0.76 "},
{"Glass ","Nonex  ","212 (100) ","0.82 "},
{"Glass ","Nonex  ","600 (316) ","0.82 "},
{"Glass ","Nonex  ","932 (500) ","0.78 "},
{"Glass ","Smooth ","32-200(0-93)  ","0.93 "},
{"Glass ","Granite  ","70 (21) ","0.45 "},
{"Glass ","Gravel  ","100 (38)  ","0.28 "},
{"Glass ","Gypsum ","68 (20) ","0.88 "},
{"Glass ","Ice, Smooth ","32 (0) ","0.97 "},
{"Glass ","Ice, Rough ","32 (0) ","0.98 "},
{"Lacquer ","Black ","200 (93) ","0.96 "},
{"Lacquer ","Blue, on Al Foil ","100 (38)  ","0.78 "},
{"Lacquer ","Clear, on Al Foil (2 coats) ","200 (93) ","0.08 "},
{"Lacquer ","Clear, on Bright Cu ","200 (93)  ","0.66 "},
{"Lacquer ","Clear, on Tarnished Cu ","200 (93) ","0.64 "},
{"Lacquer ","Red, on Al Foil (2 coats) ","100 (38)  ","0.65 "},
{"Lacquer ","White ","200 (93)  ","0.95 "},
{"Lacquer ","White, on Al Foil (2 coats) ","100 (38) ","0.75 "},
{"Lacquer ","Yellow, on Al Foil (2 coats) ","100 (38) ","0.65 "},
{"Lacquer ","Lime Mortar ","100-500 (38-260) ","0.91 "},
{"Lacquer ","Limestone  ","100 (38) ","0.95 "},
{"Lacquer ","Marble, White "," 100 (38) ","0.95 "},
{"Lacquer ","Smooth, White "," 100 (38)  ","0.56 "},
{"Lacquer ","Polished Grey "," 100 (38) ","0.75 "},
{"Lacquer ","Mica  ","100 (38) ","0.75"},
{"Oil on Nickel ","0.001 Film ","72 (22) ","0.27 "},
{"Oil on Nickel ","0.002 Film ","72 (22) ","0.46 "},
{"Oil on Nickel ","0.005 Film ","72 (22)  ","0.72 "},
{"Oil on Nickel ","Thick Film ","72 (22)  ","0.82 "},
{"Oil, Linseed ","On Al Foil, uncoated ","250 (121) ","0.09"},
{"Oil, Linseed ","On Al Foil, 1 coat  ","250 (121) ","0.56 "},
{"Oil, Linseed ","On Al Foil, 2 coats  ","250 (121) ","0.51 "},
{"Oil, Linseed ","On Polished Iron, .00  Film ","100 (38)  ","0.22 "},
{"Oil, Linseed ","On Polished Iron, .00  Film ","100 (38) ","0.45 "},
{"Oil, Linseed ","On Polished Iron, .00  Film  ","100 (38)  ","0.65 "},
{"Oil, Linseed ","On Polished Iron, Thick Film ","100 (38)  ","0.83 "},
{"Paints ","Blue, Cu2O3  ","75 (24) ","0.94 "},
{"Paints ","Black, CuO  ","75 (24) ","0.96 "},
{"Paints ","Green, Cu2O3 ","75 (24) ","0.92 "},
{"Paints ","Red, Fe2O3  ","75 (24) ","0.91 "},
{"Paints ","White, Al2O3 ","75 (24) ","0.94 "},
{"Paints ","White, Y2O3  ","75 (24)  ","0.9 "},
{"Paints ","White, ZnO  ","75 (24)  ","0.95 "},
{"Paints ","White, MgCO3  ","75 (24)   ","0.91 "},
{"Paints ","White, ZrO2   ","75 (24) ","0.95 "},
{"Paints ","White, ThO2   ","75 (24) ","0.9 "},
{"Paints ","White, MgO  ","75 (24)  ","0.91 "},
{"Paints ","White, PbCO3   ","75 (24)  ","0.93 "},
{"Paints ","Yellow, PbO  ","75 (24)  ","0.9 "},
{"Paints ","Yellow, PbCrO4  ","75 (24)   ","0.93 "},
{"Paints ","Paints, Aluminium  ","100 (38)  ",".27-.67 "},
{"Paints ","10% Al  ","100 (38)  ","0.52 "},
{"Paints ","26% Al ","100 (38)  ","0.3 "},
{"Paints ","Dow XP-310   ","200 (93) ","0.22 "},
{"Paints ","Paints, Bronze   ","  Low   "," 0.50 "},
{"Paints ","Gum Varnish (2 coats)","   70 (21) ","0.53 "},
{"Paints ","Gum Varnish (3 coats)","   70 (21)  ","0.5 "},
{"Paints ","Cellulose Binder (2 coats)   ","70 (21)   ","0.34 "},
{"Paints, Oil ","All colours ","200 (93) "," 0.94"},
{"Paints, Oil ","Black  200 ","(93) ","0.92 "},
{"Paints, Oil ","Black Gloss   ","70 (21)   ","0.9 "},
{"Paints, Oil ","Camouflage Green ","125 (52)  ","0.85 "},
{"Paints, Oil ","Flat Black    ","80 (27)  ","0.88 "},
{"Paints, Oil ","Flat White  ","80 (27) ","0.91 "},
{"Paints, Oil ","Grey-Green  ","70 (21)   ","0.95 "},
{"Paints, Oil ","Green  ","200 (93) ","0.95 "},
{"Paints, Oil ","Lamp Black "," 209 (98)  ","0.96 "},
{"Paints, Oil ","Red    ","200 (93)  ","0.95 "},
{"Paints, Oil ","White   ","200 (93)  ","0.94 "},
{"Paints, Oil ","Quartz, Rough, Fused ","70 (21) ","0.93 "},
{"Paints, Oil ","Glass, 1.98 mm  ","540 (282)   ","0.9 "},
{"Paints, Oil ","Glass, 1.98 mm ","1540 (838)   ","0.41"},
{"Paints, Oil ","Glass, 6.88 mm  ","540 (282) ","0.93 "},
{"Paints, Oil ","Glass, 6.88 mm    ","1540 (838) ","0.47 "},
{"Paints, Oil ","Opaque  ","570 (299) ","0.92 "},
{"Paints, Oil ","Opaque   ","1540 (838)  ","0.68 "},
{"Paints, Oil ","Red Lead   ","212 (100)  ","0.93 "},
{"Paints, Oil ","Rubber, Hard    ","74 (23)  ","0.94 "},
{"Paints, Oil ","Rubber, Soft, Grey  ","76 (24)   ","0.86 "},
{"Paints, Oil ","Sand   68 ","(20) ","0.76 "},
{"Paints, Oil ","Sandstone 100 ","(38) ","0.67 "},
{"Paints, Oil ","Sandstone, Red  ","100 (38) ","0.70 "},
{"Paints, Oil ","Sawdust   ","68 (20) ","0.75 "},
{"Paints, Oil ","Shale  68 ","(20) ","0.69 "},
{"Paints, Oil ","Silica,Glazed  1832 ","(1000) ","0.85 "},
{"Paints, Oil ","Silica, Unglazed   2012 ","(1100)  ","0.75 "},
{"Paints, Oil ","Silicon Carbide 300-1200 ","(149-649)  ","0.88 "},
{"Paints, Oil ","Silk Cloth 68 ","(20) ","0.78"},
{"Paints, Oil ","Slate   ","100 (38)  ","0.75 "},
{"Paints, Oil ","Snow, Fine Particles ","20 (-7) ","0.82 "},
{"Paints, Oil ","Snow, Granular  ","18 (-8)  ","0.89 "},
{"Soil ","Surface  ","100 (38) ","0.38 "},
{"Soil ","Black Loam  ","68 (20)  ","0.66 "},
{"Soil ","Plowed Field   ","68 (20)  ","0.38 "},
{"Soot ","Acetylene   ","75 (24)  ","0.97 "},
{"Soot ","Camphor  ","75 (24) ","0.94 "},
{"Soot ","Candle ","250 (121) ","0.95 "},
{"Soot ","Coal    ","68 (20)  ","0.95 "},
{"Soot ","Stonework  ","100 (38)  ","0.93 "},
{"Soot ","Water  ","100 (38) ","0.67 "},
{"Soot ","Waterglass ","68 (20) ","0.96 "},
{"Soot ","Wood   ","Low ","0.85"},
{"Soot ","Beech Planed  ","158 (70)  ","0.94 "},
{"Soot ","Oak, Planed  ","100 (38) ","0.91 "},
{"Soot ","Spruce, Sanded ","100 (38) ","0.89 "},
{"Alloys ","20-Ni,24-CR, 55-FE, Oxid","392 (200)","0.9 "},
{"Alloys ","20-Ni,24-CR, 55-FE, Oxid","932(500)","0.97 "},
{"Alloys ","60-Ni,12-CR, 28-FE, Oxid","518 (270)","0.89 "},
{"Alloys ","60-Ni,12-CR, 28-FE, Oxid","1040 (560)","0.82 "},
{"Alloys ","80-Ni,20-CR, Oxidised","212 (100) ","0.87 "},
{"Alloys ","80-Ni,20-CR, Oxidised","1112 (600) ","0.87 "},
{"Alloys ","80-Ni,20-CR, Oxidised","2372 (1300) ","0.89 "},
{"Aluminium ","Unoxidised  ","77 (25) ","0.02 "},
{"Aluminium ","Unoxidised ","212 (100)  ","0.03 "},
{"Aluminium ","Unoxidised  ","932 (500) ","0.06 "},
{"Aluminium ","Oxidised  ","390 (199) ","0.11 "},
{"Aluminium ","Oxidised ","1110 (599) ","0.19 "},
{"Aluminium ","Oxidised at 599degC(1110degF)  ","390 (199) ","0.11 "},
{"Aluminium ","Oxidised at 599degC(1110degF)  ","1110 (599) ","0.19 "},
{"Aluminium ","Heavily Oxidised ","200 (93) ","0.2 "},
{"Aluminium ","Heavily Oxidised ","940 (504) ","0.31 "},
{"Aluminium ","Highly Polished  ","212 (100) ","0.09 "},
{"Aluminium ","Roughly Polished ","212 (100) ","0.18 "},
{"Aluminium ","Commercial Sheet ","212 (100)  ","0.09 "},
{"Aluminium ","Highly Polished Plate","440 (227) ","0.04 "},
{"Aluminium ","Highly Polished Plate","1070 (577) ","0.06 "},
{"Aluminium ","Bright Rolled Plate","338 (170)    ","0.04 "},
{"Aluminium ","Bright Rolled Plate","932 (500)    ","0.05 "},
{"Aluminium ","Alloy A3003, Oxidised","600 (316) ","0.4 "},
{"Aluminium ","Alloy A3003, Oxidised","900 (482)     ","0.4 "},
{"Aluminium ","Alloy 1100-0","200-800 (93-427)    ","0.05 "},
{"Aluminium ","Alloy 24ST","75 (24)   ","0.09 "},
{"Aluminium ","Alloy 24ST, Polished","75 (24)   ","0.09 "},
{"Aluminium ","Alloy 75ST","75 (24)    ","0.11 "},
{"Aluminium ","Alloy 75ST, Polished  ","75 (24)    ","0.08 "},
{"Aluminium ","Bismuth, Bright  ","176 (80)  ","0.34 "},
{"Aluminium ","Bismuth, Unoxidised   ","77 (25)    ","0.05 "},
{"Aluminium ","Bismuth, Unoxidised  ","212 (100)    ","0.06 "},
{"Brass ","73% Cu, 27% Zn, Polished  ","476 (247)  ","0.03 "},
{"Brass ","73% Cu, 27% Zn, Polished   ","674 (357)  ","0.03 "},
{"Brass ","62% Cu, 37% Zn, Polished   ","494 (257)  ","0.03 "},
{"Brass ","62% Cu, 37% Zn, Polished   ","710 (377) ","0.04 "},
{"Brass ","83% Cu, 17% Zn, Polished ","530 (277) ","0.03 "},
{"Brass ","Matte     ","68 (20)   ","0.07 "},
{"Brass ","Burnished to Brown Colour  ","68 (20)  ","0.4 "},
{"Brass ","Cu-Zn, Brass Oxidised  ","392 (200)    ","0.61 "},
{"Brass ","Cu-Zn, Brass Oxidised  ","752 (400) ","0.6 "},
{"Brass ","Cu-Zn, Brass Oxidised   ","1112 (600)  ","0.61 "},
{"Brass ","Unoxidised   ","77 (25)   ","0.04 "},
{"Brass ","Unoxidised  ","212 (100)   ","0.04 "},
{"Brass ","Cadmium     ","77 (25) ","0.02 "},
{"Carbon ","Lampblack    ","77 (25)   ","0.95 "},
{"Carbon ","Unoxidised ","77 (25)  ","0.81 "},
{"Carbon ","Unoxidised ","212 (100)   ","0.81 "},
{"Carbon ","Unoxidised ","932 (500)    ","0.79 "},
{"Carbon ","Candle Soot","250 (121)     ","0.95 "},
{"Carbon ","Filament","500 (260)   ","0.95 "},
{"Carbon ","Graphitized","212 (100)   ","0.76 "},
{"Carbon ","Graphitized","572 (300)     ","0.75 "},
{"Carbon ","Graphitized ","932 (500)       ","0.71 "},
{"Carbon ","Chromium","100 (38)  ","0.08 "},
{"Carbon ","Chromium","1000 (538) ","0.26 "},
{"Carbon ","Chromium, Polished","302 (150)  ","0.06 "},
{"Carbon ","Cobalt, Unoxidised","932 (500)   ","0.13 "},
{"Carbon ","Cobalt, Unoxidised","1832 (1000) ","0.23 "},
{"Carbon ","Columbium, Unoxidised","1500 (816)     ","0.19 "},
{"Carbon ","Columbium, Unoxidised","2000 (1093) ","0.24 "},
{"Copper ","Cuprous Oxide","100 (38) ","0.87 "},
{"Copper ","Cuprous Oxide","500 (260) ","0.83 "},
{"Copper ","Cuprous Oxide","1000 (538) ","0.77 "},
{"Copper ","Black, Oxidised","100 (38) ","0.78 "},
{"Copper ","Etched  ","100 (38) ","0.09 "},
{"Copper ","Matte   ","100 (38)  ","0.22 "},
{"Copper ","Roughly Polished  ","100 (38)  ","0.07 "},
{"Copper ","Polished","100 (38)   ","0.03 "},
{"Copper ","Highly Polished   ","100 (38) ","0.02 "},
{"Copper ","Rolled "," 100 (38)  ","0.64 "},
{"Copper ","Rough","100 (38)   ","0.74 "},
{"Copper ","Molten","1000 (538) ","0.15 "},
{"Copper ","Molten","1970 (1077)   ","0.16 "},
{"Copper ","Molten","2230 (1221)   ","0.13"},
{"Copper ","Nickel Plated","100-500 (38-260)  ","0.37 "},
{"Copper ","Dow Metal","0.4-600 (-18-316) ","0.15 "},
{"Gold ","Enamel","212 (100)  ","0.37 "},
{"Gold ","Plate (.0001) ","213 (100)  ","0.38"},
{"Gold ","Plate on .0005 Silver","200-750 (93-399) ","0.13 "},
{"Gold ","Plate on .0005 Nickel","200-750 (93-399) "," 0.08"},
{"Gold ","Polished","100-500 (38-260) ","0.02 "},
{"Gold ","Polished  "," "," 0.03 "},
{"Haynes Alloy C ","Oxidised  "," "," 0.92 "},
{"Haynes Alloy 25 ","Oxidised   ","600-2000 (316-1093) ","0.87 "},
{"Haynes Alloy X ","Oxidised  ","600-2000 (316-1093) ","0.86 "},
{"Haynes Alloy X ","Inconel Sheet   ","1000 (538)  ","0.28 "},
{"Haynes Alloy X ","Inconel Sheet  ","1200 (649) ","0.42 "},
{"Haynes Alloy X ","Inconel Sheet  ","1400 (760) ","0.58 "},
{"Haynes Alloy X ","Inconel X, Polished   ","75 (24) ","0.19 "},
{"Haynes Alloy X ","Inconel B, Polished  ","75 (24)  ","0.21 "},
{"Iron ","Oxidised  ","212 (100)    ","0.74 "},
{"Iron ","Oxidised   ","930 (499)   ","0.84 "},
{"Iron ","Oxidised   ","2190 (1199)  ","0.89 "},
{"Iron ","Unoxidised    ","212 (100) ","0.05 "},
{"Iron ","Red Rust  ","77 (25) ","0.7 "},
{"Iron ","Rusted     ","77 (25)     ","0.65 "},
{"Iron ","Liquid   "," "," 0.43 "},
{"Cast Iron ","Oxidised   ","390 (199)   ","0.64 "},
{"Cast Iron ","Oxidised  ","1110 (599) ","0.78 "},
{"Cast Iron ","Unoxidised   ","212 (100)  ","0.21 "},
{"Cast Iron ","Strong Oxidation  ","40 (104)   ","0.95 "},
{"Cast Iron ","Strong Oxidation      ","482 (250)  ","0.95 "},
{"Cast Iron ","Liquid   ","2795 (1535)  ","0.29 "},
{"Wrought Iron ","Dull   ","77 (25)  ","0.94 "},
{"Wrought Iron ","Dull      ","660 (349) ","0.94 "},
{"Wrought Iron ","Smooth   ","100 (38)  ","0.35 "},
{"Wrought Iron ","Polished    ","100 (38) ","0.28 "},
{"Lead ","Polished","100-500 (38-260)   ","0.07 "},
{"Lead ","Rough","100 (38)  ","0.43 "},
{"Lead ","Oxidised","100 (38)  ","0.43 "},
{"Lead ","Oxidised at 1100 ","100 (38)  ","0.63 "},
{"Lead ","Gray Oxidised","100 (38) ","0.28 "},
{"Lead ","Magnesium","100-500 (38-260) "," 0.08 "},
{"Lead ","Magnesium Oxide","1880-3140 (1027-1727) ","0.18 "},
{"Lead ","Mercury"," 32 (0)   ","0.09 "},
{"Lead ","Mercury","77 (25)   ","0.1 "},
{"Lead ","Mercury","100 (38) ","0.1 "},
{"Lead ","Mercury","212 (100) ","0.12 "},
{"Lead ","Molybdenum","100 (38) ","0.06 "},
{"Lead ","Molybdenum","500 (260)  ","0.08 "},
{"Lead ","Molybdenum","1000 (538) ","0.11 "},
{"Lead ","Molybdenum","2000 (1093) ","0.18 "},
{"Lead ","Molybdenum Oxidised at 1000degF ","600 (316) ","0.8 "},
{"Lead ","Molybdenum Oxidised at 1000degF ","700 (371) ","0.84 "},
{"Lead ","Molybdenum Oxidised at 1000degF ","800 (427) ","0.84 "},
{"Lead ","Molybdenum Oxidised at 1000degF ","900 (482) ","0.83 "},
{"Lead ","Molybdenum Oxidised at 1000degF ","1000 (538) ","0.82 "},
{"Lead ","Monel, Ni-Cu","392 (200) ","0.41 "},
{"Lead ","Monel, Ni-Cu","752 (400) ","0.44 "},
{"Lead ","Monel, Ni-Cu","1112 (600)   ","0.46 "},
{"Lead ","Monel, Ni-Cu Oxidised","68 (20)  ","0.43 "},
{"Lead ","Monel, Ni-Cu Oxid. at 1110degF ","1110 (599)   ","0.46 "},
{"Nickel ","Polished  ","100 (38)  ","0.05 "},
{"Nickel ","Oxidised   ","100-500 (38-260) ","0.38 "},
{"Nickel ","Unoxidised  ","77 (25) ","0.05 "},
{"Nickel ","Unoxidised ","212 (100) ","0.06 "},
{"Nickel ","Unoxidised   ","932 (500) ","0.12 "},
{"Nickel ","Unoxidised ","1832 (1000)  ","0.19 "},
{"Nickel ","Electrolytic  ","100 (38)  ","0.04 "},
{"Nickel ","Electrolytic  ","500 (260) ","0.06 "},
{"Nickel ","Electrolytic  ","1000 (538)   ","0.1 "},
{"Nickel ","Electrolytic  ","2000 (1093) ","0.16 "},
{"Nickel ","Nickel Oxide  ","1000-2000 (538-1093) ","0.67 "},
{"Nickel ","Palladium Plate (.00005 on .0005 silver) ","200-750 (93-399) ","0.16 "},
{"Nickel ","Platinum  ","100 (38) ","0.05 "},
{"Nickel ","Platinum  ","500 (260) ","0.05 "},
{"Nickel ","Platinum  ","1000 (538) ","0.1 "},
{"Nickel ","Platinum, Black ","100 (38) ","0.93 "},
{"Nickel ","Platinum, Black ","500 (260) ","0.96 "},
{"Nickel ","Platinum, Black ","2000 (1093) ","0.97 "},
{"Nickel ","Platinum Oxidised at 1100 ","500 (260) ","0.07 "},
{"Nickel ","Platinum Oxidised at 1100 ","1000 (538) ","0.11 "},
{"Nickel ","Rhodium Flash (0.0002 on 0.0005 Ni) ","200-700 (93-371) ","0.15 "},
{"Silver  ","Plate (0.0005 on Ni) ","200-700 (93-371) ","0.07 "},
{"Silver  ","Polished ","100 (38) ","0.01 "},
{"Silver  ","Polished ","500 (260) ","0.02 "},
{"Silver  ","Polished ","1000 (538)  ","0.03 "},
{"Silver  ","Polished ","2000 (1093) ","0.03 "},
{"Steel ","Cold Rolled ","200 (93) ","0.80 "},
{"Steel ","Ground Sheet ","1720-2010 (938-1099) ","0.55 "},
{"Steel ","Polished Sheet ","100 (38) ","0.07 "},
{"Steel ","Polished Sheet ","500 (260) ","0.1 "},
{"Steel ","Polished Sheet ","1000 (538) ","0.14 "},
{"Steel ","Mild Steel, Polished   ","75 (24) ","0.1 "},
{"Steel ","Mild Steel, Smooth ","75 (24) ","0.12 "},
{"Steel ","Mild Steel,liquid ","2910-3270 (1599-1793) ","0.28 "},
{"Steel ","Steel, Unoxidised ","212 (100) ","0.08 "},
{"Steel ","Steel, Oxidised  ","77 (25) ","0.8 "},
{"Steel Alloys ","Type 301, Polished  ","75 (24) ","0.27 "},
{"Steel Alloys ","Type 301, Polished ","450 (232) ","0.57 "},
{"Steel Alloys ","Type 301, Polished  ","1740 (949) ","0.55 "},
{"Steel Alloys ","Type 303, Oxidised  ","600-2000 (316-1093) ","0.78 "},
{"Steel Alloys ","Type 310, Rolled  ","1500-2100 (816-1149) ","0.67 "},
{"Steel Alloys ","Type 316, Polished ","75 (24) ","0.28 "},
{"Steel Alloys ","Type 316, Polished ","450 (232)  ","0.57 "},
{"Steel Alloys ","Type 316, Polished ","1740 (949)  ","0.66 "},
{"Steel Alloys ","Type 321  ","200-800 (93-427) ","0.30 "},
{"Steel Alloys ","Type 321 Polished ","300-1500 (149-815) ","0.34 "},
{"Steel Alloys ","Type 321 w/BK Oxide  ","200-800 (93-427) ","0.70 "},
{"Steel Alloys ","Type 347, Oxidised  ","600-2000 (316-1093) ","0.89 "},
{"Steel Alloys ","Type 350   ","200-800 (93-427) ","0.23 "},
{"Steel Alloys ","Type 350 Polished ","300-1800 (149-982) ","0.24 "},
{"Steel Alloys ","Type 446, Polished  ","300-1500 (149-815) ","0.25 "},
{"Steel Alloys ","Type 17-7 PH ","200-600 (93-316) ","0.47 "},
{"Steel Alloys ","Type 17-7 PH Polished ","300-1500 (149-815) ","0.12 "},
{"Steel Alloys ","Type C1020,Oxidised ","600-2000 (316-1093) ","0.89 "},
{"Steel Alloys ","Type PH-15-7 MO ","300-1200 (149-649) "," 0.15 "},
{"Steel Alloys ","Stellite, Polished  ","68 (20) ","0.18 "},
{"Steel Alloys ","Tantalum, Unoxidised ","1340 (727) ","0.14 "},
{"Steel Alloys ","Tantalum, Unoxidised ","2000 (1093) ","0.19 "},
{"Steel Alloys ","Tantalum, Unoxidised ","3600 (1982) ","0.26 "},
{"Steel Alloys ","Tantalum, Unoxidised ","5306 (2930) ","0.3 "},
{"Steel Alloys ","Tin, Unoxidised ","77 (25) ","0.04 "},
{"Steel Alloys ","Tin, Unoxidised ","212 (100) ","0.05 "},
{"Steel Alloys ","Tinned Iron, Bright ","76 (24) ","0.05 "},
{"Steel Alloys ","Tinned Iron, Bright ","212 (100) ","0.08 "},
{"Titanium ","Alloy C110M,Polished  ","300-1200 (149-649) ","0.13 "},
{"Titanium "," Oxidised at 538degC(1000degF)  ","200-800 (93-427) ","0.56 "},
{"Titanium ","Alloy Ti-95A,Oxidised at 538degC(1000degF) ","200-800 (93-427) ","0.43 "},
{"Titanium ","Anodized onto SS  ","200-600 (93-316) ","0.88 "},
{"Tungsten ","Unoxidised ","77 (25) ","0.02 "},
{"Tungsten ","Unoxidised ","212 (100) ","0.03 "},
{"Tungsten ","Unoxidised ","932 (500) ","0.07 "},
{"Tungsten ","Unoxidised ","1832 (1000) ","0.15 "},
{"Tungsten ","Unoxidised ","2732 (1500) ","0.23 "},
{"Tungsten ","Unoxidised  ","3632 (2000) ","0.28 "},
{"Tungsten ","Filament (Aged) ","100 (38) ","0.03 "},
{"Tungsten ","Filament (Aged) ","1000 (538) ","0.11 "},
{"Tungsten ","Filament (Aged)  ","5000 (2760) ","0.35 "},
{"Tungsten ","Uranium Oxide  ","1880 (1027) ","0.79 "},
{"Zinc ","Bright, Galvanised ","100 (38) ","0.23 "},
{"Zinc ","Commercial 99.1%  ","500 (260) ","0.05 "},
{"Zinc ","Galvanised  ","100 (38)  ","0.28 "},
{"Zinc ","Polished  ","100 (38) ","0.02 "},
{"Zinc ","Polished ","500 (260) ","0.03 "},
{"Zinc ","Polished ","1000 (538)  ","0.04 "},
{"Zinc ","Polished ","2000 (1093) ","0.06 "},
{"0","0","0","0"}
};


/* Find entries in the emissivity table
 * @param step : 
 * 	1 = select by type, 
 * 	2 = select material within type
 * 	3 = wildcard search
 * 
 * @param lookup : 
 * 	lookup value for type in step 2 
 * 	loopup value for wildcard in step 3
 *  
 * return: 
 * 	99 = level back 
 * -1  = no match found (step 3)
 *  or entry in table
 */
int	select_emiss(int step, char *lookup)
{
	int 	inp=0, s_fnd=0;
	char	prev[50] = {0};
	int		pnt[100], answ;
	int		hdr=1;
	
	// get selection on type
	if (step == 1)
	{
		// as long as not end of table
		while (strcmp(emis_table[inp].type,"0"))
		{
			// type in current is NOT same as previous
			if (strcmp(emis_table[inp].type, prev))
			{
				// header is included once
				if(hdr) 
				{
					p_printf(3,"%-3s%-15s\n","#", "Type");
					hdr = 0;
				}
				
				pnt[s_fnd] = inp;						// save offset in table
				strncpy(prev, emis_table[inp].type,49);	// reset previous
				p_printf(2,"%-3d%-15s\n", s_fnd++,emis_table[inp].type);	// display
			}
			// next table entry
			inp++;
		}
	}
	
	// select on material by type
	else if (step == 2)
	{
		while (strcmp(emis_table[inp].type,"0"))
		{
			/* find the entries that match */
			if (! strncmp(emis_table[inp].type, lookup, strlen(lookup)))
			{
				if(hdr)	 // header only once
				{
					p_printf(3,"%-3s%-15s%-25s%-20s%-s\n","#", "Type", "Material","Temp","emissivity");
					hdr = 0;
				}
				
				pnt[s_fnd] = inp;							// save offset in table
				p_printf(2,"%-3d%-15s%-25s%-20s %-s\n", s_fnd++,emis_table[inp].type, emis_table[inp].material, 
				emis_table[inp].temp, emis_table[inp].emis);	// display
			}
			// next table entry
			inp++;
		}
	}

	// find entry based on wild card search on either or material
	else if (step == 3)
	{	
		// as long as not end of table
		while (strcmp(emis_table[inp].type,"0"))
		{
			/* find the entries that match */
			if (! strncasecmp(emis_table[inp].type, lookup, strlen(lookup)) || 
			(! strncasecmp(emis_table[inp].material, lookup, strlen(lookup)))) 
			{
				if(hdr)	 // header only once
				{
					p_printf(3,"%-3s%-15s%-25s%-20s%-s\n","#", "Type", "Material","Temp","emissivity");
					hdr = 0;
				}
				
				pnt[s_fnd] = inp;							 // save offset in table
				p_printf(2,"%-3d%-15s%-25s%-20s %-s\n", s_fnd++,emis_table[inp].type, emis_table[inp].material, 
				emis_table[inp].temp, emis_table[inp].emis); // display
			}
			
			inp++;
		}
	}
		
	// adjust the amount found
	s_fnd--;
	
	if (s_fnd < 1)	return(-1);
		
	do
	{
		printf("Make your choice between 0 and %d. (99 = return) ",s_fnd);
		answ = get_dec_input();

		if (answ == 99 || answ == -1) return(99);
		
		if (answ > s_fnd || answ < 0)
		{
			printf("invalid entry %d\n",answ);
			answ = -1;
		}
		
	} while(answ == -1);
	
	return(pnt[answ]);
}	

/* will support finding the emissivity for a certain material 
 * return 99 if no selection, otherwise the selection index from the table
 */

float find_emis()
{ 
	char	lookup[50]= {0};
	int		answ, ret, step = 1;

	/* determine what kind search is wanted */
	do
	{
		p_printf(3,"\nWant to search using: \n");
		p_printf(2,"1) on type and material\n2) wildcard search \n(99 = return) ");
		answ = get_dec_input();

		if (answ == 99 || answ == -1) return(99);
				
		else if (answ == 1) step = 1;
		
		else if (answ == 2)
		{
			p_printf(2,"\nwhat is the argument to look for ? \n(99 = return)");
			scanf("%s", lookup);
				
			if (! strcmp(lookup, "99")) return(99);
			
			step = 3;
		}
		else
		{
			p_printf(1,"invalid answer. Try again\n");
			answ=0;
		}

	}	while (answ == 0);	
	
	do
	{
		// get type / material selection from table
		ret = select_emiss(step,lookup);
		
		if (ret == 99)
		{
			// if at step 1 or wild card search, return
			if (step == 1 || step-- == 3) return(99);
		
			// reset lookup
			lookup[0] = 0x0;
		}
	
		else if (ret == -1)
		{
			p_printf(1,"could not make a match\n");
			return(99);
		}
			
		else if (step++ == 1)
			strncpy(lookup, emis_table[ret].type,49);
		 
	} while (step < 3);
	
	return(strtof(emis_table[ret].emis,NULL));
}

/* enter emissivity value directly */

float read_emis_int()
{
	float new_emiss;   
    
    while (1)
	{
		p_printf(3,"\nProvide new emissivity value between (0 - 1) \n(99 = return) ");
		scanf("%f",&new_emiss);

		if (new_emiss == 99.000000) return(99);

		if(new_emiss > 1.000000 || new_emiss < 0)
			p_printf(1,"Invalid Emissivity value : %f\n", new_emiss);
		else
			return(new_emiss);
	}
}
