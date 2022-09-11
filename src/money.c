#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"

// TRUE - buy
// FALSE - sell
long pazarlik_sonu_ucreti_hesapla(CHAR_DATA *ch, long cost, bool buy_or_sell)
{
    int roll = number_percent();

    if( IS_PC(ch) && get_skill(ch,gsn_haggle) >= roll )
    {
        check_improve(ch,gsn_haggle,TRUE,4);

        if(roll<=50)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.9)):UMAX(1,long(cost * 1.1));
        }
        else if(roll<=70)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.8)):UMAX(1,long(cost * 1.2));
        }
        else if(roll<=85)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.7)):UMAX(1,long(cost * 1.4));
        }
        else if(roll<=90)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.6)):UMAX(1,long(cost * 1.6)););
        }
        else if(roll<=94)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.5)):UMAX(1,long(cost * 1.9));
        }
        else if(roll<=96)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.4)):UMAX(1,long(cost * 2.2));
        }
        else if(roll<=98)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.3)):UMAX(1,long(cost * 2.5));
        }
        else if(roll<=99)
        {
            return buy_or_sell?UMAX(1,long(cost * 0.2)):UMAX(1,long(cost * 2.8));
        }
        else
        {
            return buy_or_sell?UMAX(1,long(cost * 0.1)):UMAX(1,long(cost * 3));
        }
    }

    return cost;

}

// TRUE - buy
// FALSE - sell
bool hizmet_bedeli_odeme(CHAR_DATA *ch, CHAR_DATA *victim, long , bool buy_or_sell)
{
    int cost_haggle = 0;
    int roll = number_percent();

    if(IS_PC(ch))
        cost_haggle = pazarlik_sonu_ucreti_hesapla(ch,cost,buy_or_sell);

    if(cost != cost_haggle)
    {
        printf_to_char(ch,"Pazarlýk ediyorsun. ");
    }

    //buy
    if(buy_or_sell)
    {
        if (cost_haggle > ch->silver && long(cost_haggle*1.05) > ch->pcdata->bank_s)
        {
            printf_to_char(ch,"Ne üzerinde ne de banka hesabýnda bu ücreti karþýlayacak akçen yok.\n\r");
            return FALSE;
        }

        if (cost_haggle <= ch->silver)
        {
            ch->silver -= cost_haggle;
            printf_to_char(ch,"%ld akçe ödeme yapýyorsun.\n\r",cost_haggle);
        }
        else if (long(cost_haggle*1.05) <= ch->pcdata->bank_s)
        {
            ch->pcdata->silver -= long(cost_haggle*1.05);
            printf_to_char(ch,"Banka hesabýndan komisyon dahil %ld akçe ödeme yapýyorsun.\n\r",long(cost_haggle*1.05));
        }
        else
        {
            printf_to_char(ch,"Ödemede sorun çýktýðý için alýþveriþten vazgeçiyorsun.\n\r");
            return FALSE;
        }
    }
    //sell
    else
    {
        if ( IS_NPC(victim) )
        {
            if( cost > victim->silver )
            {
                if(number_percent() > get_skill(ch, gsn_haggle))
                {
                    do_say(victim,(char*)"Üzgünüm, bunun bedelini ödeyemem.");
                    return FALSE;
                }
            }
            else
            {
                victim->silver -= cost_haggle;
                ch->victim += cost_haggle; 
            }
        }
    }

    ch->silver 	 += cost;

    if (cost >= ch->silver)
    {
        ch->silver -= cost;
        printf_to_char(ch,"%ld akçe ödeme yapýyorsun.\n\r",cost);
    }
    else if (long(cost*1.05) >= ch->pcdata->bank_s)
    {
        ch->pcdata->silver -= long(cost*1.05);
        printf_to_char(ch,"Banka hesabýndan komisyon dahil %ld akçe ödeme yapýyorsun.\n\r",long(cost*1.05));
    }
    else
    {
        printf_to_char(ch,"Ödemede sorun çýktýðý için alýþveriþten vazgeçiyorsun.\n\r");
        return FALSE;
    }

    if(victim != NULL)
    {
        victim->silver += cost;
        printf_to_char(victim,"%s sana %ld akçe ödüyor.\n\r",ch->name,cost);
    }

    return TRUE;
}
