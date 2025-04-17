# sed script for src/magic2.c
# BACK UP YOUR FILE FIRST!

# Fix -Wempty-body on line 1238
# Insert semicolon on new line AFTER 1238
1239i\
    ;
# Remove semicolon from end of line 1238
1238s/;$//

# Fix -Wformat-security on line 1241
1241s/sprintf(short_buf, tmp_vict->name)/sprintf(short_buf, "%s", tmp_vict->name)/

# Fix -Wunused-variable / -Wunused-but-set-variable
# Note: line numbers below are ORIGINAL line numbers from warnings

# delete: int dam; (-Wunused-but-set-variable) on line 2433
2433d
# delete: OBJ_DATA *tattoo; on line 275
275d
# Edit line 274: remove unused 'i'
274s/int i,dam=0;/int dam=0;/
# delete: OBJ_DATA *obj_next; on line 273
273d
# delete: OBJ_DATA *obj; on line 272
272d
# delete: CHAR_DATA *tmp_ch; on line 271
271d
# delete: OBJ_DATA *tattoo; on line 232
232d
# Edit line 231: remove unused 'i'
231s/int i,dam=0;/int dam=0;/
# delete: OBJ_DATA *obj_next; on line 230
230d
# delete: OBJ_DATA *obj; on line 229
229d
# delete: CHAR_DATA *tmp_ch; on line 228
228d

# Fix -Wunused-parameter (inserting (void) cast)
# Line numbers below are FUNCTION DEFINITION lines + 2 (assuming brace on line+1)
# Lines are processed roughly bottom-up from warning list

# Line 6469: unused 'target'
6471i\
    (void)target;
# Line 6441: unused 'target'
6443i\
    (void)target;
# Line 6414: unused 'target'
6416i\
    (void)target;
# Line 6387: unused 'target'
6389i\
    (void)target;
# Line 6361: unused 'sn', 'level', 'target'
6363i\
    (void)sn;
6363i\
    (void)level;
6363i\
    (void)target;
# Line 6178: unused 'target'
6180i\
    (void)target;
# Line 6155: unused 'target'
6157i\
    (void)target;
# Line 6126: unused 'target'
6128i\
    (void)target;
# Line 6093: unused 'vo', 'target'
6095i\
    (void)vo;
6095i\
    (void)target;
# Line 6041: unused 'vo', 'target'
6043i\
    (void)vo;
6043i\
    (void)target;
# Line 5985: unused 'vo', 'target'
5987i\
    (void)vo;
5987i\
    (void)target;
# Line 5949: unused 'sn', 'level', 'vo', 'target'
5951i\
    (void)sn;
5951i\
    (void)level;
5951i\
    (void)vo;
5951i\
    (void)target;
# Line 5874: unused 'vo', 'target'
5876i\
    (void)vo;
5876i\
    (void)target;
# Line 5845: unused 'target'
5847i\
    (void)target;
# Line 5798: unused 'vo', 'target'
5800i\
    (void)vo;
5800i\
    (void)target;
# Line 5776: unused 'target'
5778i\
    (void)target;
# Line 5763: unused 'target'
5765i\
    (void)target;
# Line 5712: unused 'target'
5714i\
    (void)target;
# Line 5671: unused 'target'
5673i\
    (void)target;
# Line 5648: unused 'target'
5650i\
    (void)target;
# Line 5636: unused 'sn', 'target'
5638i\
    (void)sn;
5638i\
    (void)target;
# Line 5605: unused 'sn', 'level', 'vo', 'target'
5607i\
    (void)sn;
5607i\
    (void)level;
5607i\
    (void)vo;
5607i\
    (void)target;
# Line 5537: unused 'vo', 'target'
5539i\
    (void)vo;
5539i\
    (void)target;
# Line 5507: unused 'target'
5509i\
    (void)target;
# Line 5477: unused 'target'
5479i\
    (void)target;
# Line 5440: unused 'target'
5442i\
    (void)target;
# Line 5385: unused 'vo', 'target'
5387i\
    (void)vo;
5387i\
    (void)target;
# Line 5362: unused 'vo', 'target'
5364i\
    (void)vo;
5364i\
    (void)target;
# Line 5339: unused 'vo', 'target'
5341i\
    (void)vo;
5341i\
    (void)target;
# Line 5278: unused 'target'
5280i\
    (void)target;
# Line 5239: unused 'sn', 'vo', 'target'
5241i\
    (void)sn;
5241i\
    (void)vo;
5241i\
    (void)target;
# Line 5197: unused 'vo', 'target'
5199i\
    (void)vo;
5199i\
    (void)target;
# Line 5168: unused 'vo', 'target'
5170i\
    (void)vo;
5170i\
    (void)target;
# Line 5139: unused 'vo', 'target'
5141i\
    (void)vo;
5141i\
    (void)target;
# Line 5110: unused 'vo', 'target'
5112i\
    (void)vo;
5112i\
    (void)target;
# Line 5081: unused 'vo', 'target'
5083i\
    (void)vo;
5083i\
    (void)target;
# Line 5052: unused 'vo', 'target'
5054i\
    (void)vo;
5054i\
    (void)target;
# Line 5010: unused 'sn', 'level', 'vo', 'target'
5012i\
    (void)sn;
5012i\
    (void)level;
5012i\
    (void)vo;
5012i\
    (void)target;
# Line 4937: unused 'vo', 'target'
4939i\
    (void)vo;
4939i\
    (void)target;
# Line 4865: unused 'vo', 'target'
4867i\
    (void)vo;
4867i\
    (void)target;
# Line 4789: unused 'vo', 'target'
4791i\
    (void)vo;
4791i\
    (void)target;
# Line 4712: unused 'vo', 'target'
4714i\
    (void)vo;
4714i\
    (void)target;
# Line 4673: unused 'sn', 'target'
4675i\
    (void)sn;
4675i\
    (void)target;
# Line 4653: unused 'sn', 'vo', 'target'
4655i\
    (void)sn;
4655i\
    (void)vo;
4655i\
    (void)target;
# Line 4640: unused 'sn', 'target'
4642i\
    (void)sn;
4642i\
    (void)target;
# Line 4627: unused 'sn', 'target'
4629i\
    (void)sn;
4629i\
    (void)target;
# Line 4605: unused 'vo', 'target'
4607i\
    (void)vo;
4607i\
    (void)target;
# Line 4552: unused 'target'
4554i\
    (void)target;
# Line 4491: unused 'vo', 'target'
4493i\
    (void)vo;
4493i\
    (void)target;
# Line 4475: unused 'target'
4477i\
    (void)target;
# Line 4447: unused 'target'
4449i\
    (void)target;
# Line 4423: unused 'ch', 'target'
4425i\
    (void)ch;
4425i\
    (void)target;
# Line 4399: unused 'ch', 'target'
4401i\
    (void)ch;
4401i\
    (void)target;
# Line 4371: unused 'target'
4373i\
    (void)target;
# Line 4339: unused 'vo', 'target'
4341i\
    (void)vo;
4341i\
    (void)target;
# Line 4310: unused 'target'
4312i\
    (void)target;
# Line 4298: unused 'target'
4300i\
    (void)target;
# Line 4231: unused 'vo', 'target'
4233i\
    (void)vo;
4233i\
    (void)target;
# Line 4225: unused 'sn', 'level', 'vo', 'target'
4227i\
    (void)sn;
4227i\
    (void)level;
4227i\
    (void)vo;
4227i\
    (void)target;
# Line 4203: unused 'vo', 'target'
4205i\
    (void)vo;
4205i\
    (void)target;
# Line 4118: unused 'level', 'vo', 'target'
4120i\
    (void)level;
4120i\
    (void)vo;
4120i\
    (void)target;
# Line 4084: unused 'sn', 'target'
4086i\
    (void)sn;
4086i\
    (void)target;
# Line 4029: unused 'sn', 'level', 'vo', 'target'
4031i\
    (void)sn;
4031i\
    (void)level;
4031i\
    (void)vo;
4031i\
    (void)target;
# Line 3984: unused 'sn', 'target'
3986i\
    (void)sn;
3986i\
    (void)target;
# Line 3938: unused 'sn', 'target'
3940i\
    (void)sn;
3940i\
    (void)target;
# Line 3911: unused 'sn', 'target'
3913i\
    (void)sn;
3913i\
    (void)target;
# Line 3870: unused 'vo'
3872i\
    (void)vo;
# Line 3796: unused 'target'
3798i\
    (void)target;
# Line 3718: unused 'sn', 'target'
3720i\
    (void)sn;
3720i\
    (void)target;
# Line 3613: unused 'vo', 'target'
3615i\
    (void)vo;
3615i\
    (void)target;
# Line 3563: unused 'sn', 'level', 'vo', 'target'
3565i\
    (void)sn;
3565i\
    (void)level;
3565i\
    (void)vo;
3565i\
    (void)target;
# Line 3523: unused 'target'
3525i\
    (void)target;
# Line 3509: unused 'sn', 'level', 'target'
3511i\
    (void)sn;
3511i\
    (void)level;
3511i\
    (void)target;
# Line 3467: unused 'target'
3469i\
    (void)target;
# Line 3418: unused 'sn', 'level', 'target'
3420i\
    (void)sn;
3420i\
    (void)level;
3420i\
    (void)target;
# Line 3380: unused 'sn', 'vo', 'target'
3382i\
    (void)sn;
3382i\
    (void)vo;
3382i\
    (void)target;
# Line 3338: unused 'sn', 'vo', 'target'
3340i\
    (void)sn;
3340i\
    (void)vo;
3340i\
    (void)target;
# Line 3274: unused 'sn', 'vo', 'target'
3276i\
    (void)sn;
3276i\
    (void)vo;
3276i\
    (void)target;
# Line 3235: unused 'target'
3237i\
    (void)target;
# Line 3208: unused 'target'
3210i\
    (void)target;
# Line 3179: unused 'target'
3181i\
    (void)target;
# Line 2967: unused 'vo', 'target'
2969i\
    (void)vo;
2969i\
    (void)target;
# Line 2910: unused 'vo', 'target'
2912i\
    (void)vo;
2912i\
    (void)target;
# Line 2861: unused 'vo', 'target'
2863i\
    (void)vo;
2863i\
    (void)target;
# Line 2688: unused 'target'
2690i\
    (void)target;
# Line 2661: unused 'target'
2663i\
    (void)target;
# Line 2631: unused 'target'
2633i\
    (void)target;
# Line 2575: unused 'vo', 'target'
2577i\
    (void)vo;
2577i\
    (void)target;
# Line 2539: unused 'target'
2541i\
    (void)target;
# Line 2513: unused 'target'
2515i\
    (void)target;
# Line 2497: unused 'vo', 'target'
2499i\
    (void)vo;
2499i\
    (void)target;
# Line 2474: unused 'vo', 'target'
2476i\
    (void)vo;
2476i\
    (void)target;
# Line 2430: unused 'sn', 'target'
2432i\
    (void)sn;
2432i\
    (void)target;
# Line 2373: unused 'vo', 'target'
2375i\
    (void)vo;
2375i\
    (void)target;
# Line 2292: unused 'vo', 'target'
2294i\
    (void)vo;
2294i\
    (void)target;
# Line 2260: unused 'vo', 'target'
2262i\
    (void)vo;
2262i\
    (void)target;
# Line 2216: unused 'target'
2218i\
    (void)target;
# Line 2202: unused 'target'
2204i\
    (void)target;
# Line 2162: unused 'vo', 'target'
2164i\
    (void)vo;
2164i\
    (void)target;
# Line 2136: unused 'sn', 'level', 'target'
2138i\
    (void)sn;
2138i\
    (void)level;
2138i\
    (void)target;
# Line 2068: unused 'target'
2070i\
    (void)target;
# Line 1996: unused 'target'
1998i\
    (void)target;
# Line 1883: unused 'sn', 'target'
1885i\
    (void)sn;
1885i\
    (void)target;
# Line 1801: unused 'sn', 'vo', 'target'
1803i\
    (void)sn;
1803i\
    (void)vo;
1803i\
    (void)target;
# Line 1719: unused 'vo', 'target'
1721i\
    (void)vo;
1721i\
    (void)target;
# Line 1656: unused 'vo', 'target'
1658i\
    (void)vo;
1658i\
    (void)target;
# Line 1607: unused 'target'
1609i\
    (void)target;
# Line 1588: unused 'sn', 'level', 'target'
1590i\
    (void)sn;
1590i\
    (void)level;
1590i\
    (void)target;
# Line 1544: unused 'sn', 'level', 'target'
1546i\
    (void)sn;
1546i\
    (void)level;
1546i\
    (void)target;
# Line 1506: unused 'vo', 'target'
1508i\
    (void)vo;
1508i\
    (void)target;
# Line 1490: unused 'sn', 'level', 'ch', 'target'
1492i\
    (void)sn;
1492i\
    (void)level;
1492i\
    (void)ch;
1492i\
    (void)target;
# Line 1464: unused 'target'
1466i\
    (void)target;
# Line 1440: unused 'vo', 'target'
1442i\
    (void)vo;
1442i\
    (void)target;
# Line 1413: unused 'vo', 'target'
1415i\
    (void)vo;
1415i\
    (void)target;
# Line 1386: unused 'ch', 'target'
1388i\
    (void)ch;
1388i\
    (void)target;
# Line 1340: unused 'target'
1342i\
    (void)target;
# Line 1301: unused 'target'
1303i\
    (void)target;
# Line 1202: unused 'sn', 'target'
1204i\
    (void)sn;
1204i\
    (void)target;
# Line 1154: unused 'vo', 'target'
1156i\
    (void)vo;
1156i\
    (void)target;
# Line 1120: unused 'target'
1122i\
    (void)target;
# Line 1090: unused 'sn', 'level', 'vo', 'target'
1092i\
    (void)sn;
1092i\
    (void)level;
1092i\
    (void)vo;
1092i\
    (void)target;
# Line 1016: unused 'vo', 'target'
1018i\
    (void)vo;
1018i\
    (void)target;
# Line 934: unused 'vo', 'target'
936i\
    (void)vo;
936i\
    (void)target;
# Line 869: unused 'vo', 'target'
871i\
    (void)vo;
871i\
    (void)target;
# Line 832: unused 'target'
834i\
    (void)target;
# Line 785: unused 'target'
787i\
    (void)target;
# Line 729: unused 'vo', 'target'
731i\
    (void)vo;
731i\
    (void)target;
# Line 650: unused 'vo', 'target'
652i\
    (void)vo;
652i\
    (void)target;
# Line 615: unused 'target'
617i\
    (void)target;
# Line 591: unused 'level', 'target'
593i\
    (void)level;
593i\
    (void)target;
# Line 566: unused 'vo', 'target'
568i\
    (void)vo;
568i\
    (void)target;
# Line 519: unused 'sn', 'level', 'target'
521i\
    (void)sn;
521i\
    (void)level;
521i\
    (void)target;
# Line 480: unused 'vo', 'target'
482i\
    (void)vo;
482i\
    (void)target;
# Line 409: unused 'vo', 'target'
411i\
    (void)vo;
411i\
    (void)target;
# Line 383: unused 'target'
385i\
    (void)target;
# Line 346: unused 'sn', 'level', 'vo', 'target'
348i\
    (void)sn;
348i\
    (void)level;
348i\
    (void)vo;
348i\
    (void)target;
# Line 310: unused 'sn', 'level', 'vo', 'target'
312i\
    (void)sn;
312i\
    (void)level;
312i\
    (void)vo;
312i\
    (void)target;
# Line 268: unused 'target'
270i\
    (void)target;
# Line 225: unused 'target'
227i\
    (void)target;
# Line 154: unused 'sn', 'vo', 'target'
156i\
    (void)sn;
156i\
    (void)vo;
156i\
    (void)target;
# Line 104: unused 'sn', 'vo', 'target'
106i\
    (void)sn;
106i\
    (void)vo;
106i\
    (void)target;