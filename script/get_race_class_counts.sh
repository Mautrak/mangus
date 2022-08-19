#!/bin/bash
irklar=`grep Race /home/ozgur/mud/mangus/player/*|awk '{print substr($2, 1, length($2)-1)}'|sort|uniq -c|sort|sed 's/$/,/'|xargs`
echo $irklar
siniflar=`grep Cla /home/ozgur/mud/mangus/player/*|awk '{print $2}'|sort|uniq -c|sort|sed 's/$/,/'|xargs|sed 's/ 0,/ Gan,/g;s/ 1,/ Ermis,/g;s/ 2,/ Hirsiz,/g;s/ 3,/ Savasci,/g;s/ 4,/ Adbolar,/g;s/ 5,/ Kembolar,/g;s/ 6,/ Ninja,/g;s/ 7,/ Korucu,/g;s/ 8,/ Bicimci,/g;s/ 9,/ Samuray,/g;s/ 10,/ Vampir,/g;s/ 11,/ Karakam,/g;s/ 12,/ Ogeci,/g'`
echo $siniflar
