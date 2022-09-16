#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"

DECLARE_DO_FUN( do_say  	);

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
            return buy_or_sell?UMAX(1,long(cost * 0.6)):UMAX(1,long(cost * 1.6));
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
long hizmet_bedeli_odeme(CHAR_DATA *ch, CHAR_DATA *victim, long cost, bool buy_or_sell)
{
    int cost_haggle = 0;
    int roll = number_percent();

    // ana karakter ch. Eger NULL ise fonksiyonu isletmeyelim.
    if(ch == NULL)
        return -1;

    // eger ch bir NPC ise veya victim PC ise fonksiyonu isletmeyelim.
    if(IS_NPC(ch) || (victim != NULL && IS_PC(victim)))
        return -1;
    
    if(IS_PC(ch))
        cost_haggle = pazarlik_sonu_ucreti_hesapla(ch,cost,buy_or_sell);

    if(cost != cost_haggle)
    {
        printf_to_char(ch,"Fiyat konusunda pazarlýk ediyorsun.\n\r");
    }

    // bu noktadan sonra cost aslinda cost_haggle'dir.
    cost_haggle = cost;

    //buy
    if(buy_or_sell)
    {
        if (cost_haggle > ch->silver && long(cost_haggle*1.05) > ch->pcdata->bank_s)
        {
            printf_to_char(ch,"Ne üzerinde ne de banka hesabýnda bu ücreti karþýlayacak akçen yok.\n\r");
            return -1;
        }

        if (cost_haggle <= ch->silver)
        {
            ch->silver -= cost_haggle;
            //printf_to_char(ch,"%ld akçe ödeme yapýyorsun.\n\r",cost_haggle);
        }
        else if (long(cost_haggle*1.05) <= ch->pcdata->bank_s)
        {
            ch->pcdata->bank_s -= long(cost_haggle*1.05);
            //printf_to_char(ch,"Banka hesabýndan komisyon dahil %ld akçe ödeme yapýyorsun.\n\r",long(cost_haggle*1.05));
        }
        else
        {
            return -1;
        }
        if(victim != NULL)
        {
            victim->silver += cost_haggle;
        }
    }
    //sell
    else
    {
        if(victim != NULL)
        {
            if( cost_haggle > victim->silver )
            {
                if(get_skill(ch, gsn_haggle) < 75)
                {
                    do_say(victim,(char*)"Üzgünüm, bunun bedelini ödeyemem.");
                    return -1;
                }
            }
            else
            {
                //printf_to_char(ch,"%s sana %ld akçe ödüyor.\n\r",victim->name,cost_haggle);
                victim->silver -= cost_haggle;
                ch->silver += cost_haggle; 
                if (victim->silver < 0)
                {
                    bug("deduct costs: silver %d < 0",victim->silver);
                    victim->silver = 0;
                }
            }
        }
    }

    return cost_haggle;
}
