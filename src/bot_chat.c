/*
 * bot_chat.c - Botların konuşması: kişilik, üslup, tepki ve boş zaman
 *              gevezeliği. Türkçe MUD oyuncu ağzı; şablonlardaki alanlar
 *              {isim} {hedef} {bolge} {seviye} {sinif} {mob} {esya} {kabal} {gp}
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "bot.h"

extern AUCTION_DATA *auction;

/* ---------------------------------------------------------------------
 * söz dağarcığı
 * ------------------------------------------------------------------ */
#define POOL(name) static const char *name[] =
#define PN(a) ( (int) ( sizeof(a) / sizeof(a[0]) ) )

POOL(greet_reply) {
    "selam {hedef}", "slm", "merhaba :)", "selamlar", "selam kanka", "sa", "selam naber",
    "hoş geldin {hedef}", "merhabalar", "selam, kaç level oldun?", "oo {hedef}, selam",
    "selam selam", "iyi oyunlar {hedef}"
};
POOL(howareyou_reply) {
    "iyiyim sen?", "idare eder, {bolge}'de kesiyorum", "yorgunum biraz ama iyi", "eh işte, senden?",
    "iyi iyi, level kasıyorum", "bomba gibiyim :)", "az önce öldüm ya, kötü", "iyiyim, {mob} kesiyorum",
    "sıkıldım biraz, grup olsak mı", "fena değil, sen naber?"
};
POOL(level_reply) {
    "{seviye} oldum", "daha {seviye}", "{seviye}, yavaş yavaş", "{seviye} lvl", "şimdilik {seviye}",
    "{seviye}, bir sonrakine az kaldı", "{seviye}, sen kaçsın?"
};
POOL(where_reply) {
    "{bolge}'deyim", "{bolge} civarındayım", "şu an {bolge}", "{bolge}, gel istersen",
    "{bolge}'de kesiyorum", "{bolge} taraflarındayım"
};
POOL(class_reply) {
    "{sinif}", "{sinif}im ben", "{sinif}, sen?", "ben {sinif}", "{sinif} olarak açtım"
};
POOL(group_accept) {
    "olur gel", "tamam geliyorum", "hadi", "ok bekle geliyorum", "olur, takip ediyorum",
    "tamam, gruba al beni", "hadi bakalım", "peki, ben seninleyim"
};
POOL(group_far) {
    "sen gel, ben {bolge}'deyim", "olur ama ben {bolge}'deyim, sen gel", "{bolge}'ye gelirsen olur",
    "biraz meşgulüm, birazdan", "şu görevi bitireyim gelirim"
};
POOL(group_no) {
    "seviyeler tutmuyor ya", "şu an olmaz kusura bakma", "biraz sonra belki", "yalnız takılıyorum bu ara"
};
POOL(stop_reply) {
    "tamam", "peki", "ok, ben devam ediyorum o zaman", "tamam görüşürüz", "olur"
};
POOL(help_reply) {
    "ne lazım?", "nasıl yardım edeyim?", "söyle bakalım", "ne oldu?", "buyur", "yardım mı? nerdesin?"
};
POOL(thanks_reply) {
    "rica ederim", "ne demek", "önemli değil :)", "eyv", "her zaman", "lafı mı olur"
};
POOL(bye_reply) {
    "görüşürüz", "bb", "iyi oyunlar", "hadi eyv", "görüşürüz {hedef}", "kendine iyi bak", "bb, iyi geceler"
};
POOL(yes_reply) {
    "tamam", "ok", "olur", "hadi o zaman", "peki"
};
POOL(no_reply) {
    "peki", "tamam sorun değil", "olsun", "anladım"
};
POOL(insult_reply) {
    "ne dedin sen?", "hoop, sakin", "bak sen", "ağzını topla", "hahaha", "sen kimsin ya",
    "seni bir görsem", "off ne kadar kabasın"
};
POOL(question_reply) {
    "bilmem ki", "sanmıyorum", "olabilir", "valla bilmiyorum", "yardıma sor", "hmm, emin değilim",
    "galiba öyle", "ne bileyim :)", "bence hayır", "evet gibi"
};
POOL(default_reply) {
    "hmm", "aynen", "valla", "öyle mi", ":)", "ne diyosun", "anlamadım", "bak sen", "hehe",
    "hah", "evet", "aynen öyle", "ha", "doğru", "yok yok", "olsun", "eyw", "hmm ilginç"
};
POOL(god_reply) {
    "tanrım!", "hoş geldiniz efendim", "aa bir tanrı", "selam tanrım, ne zamandır yoktunuz",
    "tanrım bize bir bak", "efendim buyrun", "ölümsüzlere selam"
};

POOL(idle_say) {
    "bu {mob} çok can yaktı ya", "off, bir türlü level atlayamıyorum", "kimde cure light var?",
    "eq lazım bana, para da yok", "bir grup olsak mı?", "ben biraz dinleneyim", "acıktım, ekmek almalıyım",
    "{bolge} fena değilmiş", "şu {esya} işe yarar mı acaba", "biraz para biriktirdim, dükkana gitsem",
    "yeni bir silah lazım", "görev alacağım ama görevci uzak", "senin level kaç?", "burası karanlık",
    "hangi bölgede kesiyorsun?", "bir kahve olsa", "kimse yok mu ya", "{seviye} lvl oldum daha yeni",
    "pratik puanlarım birikti, ustaya gitmeliyim", "sen hangi sınıfsın?"
};
POOL(idle_yell) {
    "{bolge}'de kimse var mı?", "yardım! {mob} peşimde!", "level {seviye} oldum!!", "grup arıyorum, {bolge}",
    "kimse grup istiyor mu?", "{bolge}'de eq düşüyor, gelin", "beni duyan var mı?", "ekmek lazım, veren?",
    "cure light atacak biri?", "{mob} nerde biliyor musunuz?"
};
POOL(idle_kd) {
    "selam, naber?", "grup olalım mı? {bolge}'deyim", "kaç level oldun?", "yardım lazım mı?",
    "nerdesin?", "bugün {mob} kestim, {esya} düştü", "{bolge}'ye geliyor musun?", "ne yapıyorsun?",
    "sıkıldım ya", "level {seviye} oldum :)", "sende fazla eq var mı?", "görev yapıyor musun?",
    "bu gece kim var oyunda?", "{bolge} nasıl, gidilir mi?", "bir ara beraber keselim"
};
POOL(idle_social) {
    "esne", "gülümse", "omuzsilk", "ıslık", "ürper", "gözdik", "kıkırda", "reverans", "elsalla"
};
POOL(room_greet) {
    "selam", "naber", "selam {hedef}", "kesiyor musun burda?", "oo {hedef}", "selam, ne var ne yok",
    "merhaba {hedef}"
};

POOL(ev_level) {
    "level {seviye}!!", "{seviye} oldum sonunda", "yaşasın, {seviye}", "{seviye} lvl :)",
    "sonunda {seviye}", "level atladım, {seviye} oldum"
};
POOL(ev_death) {
    "öldüm lan", "{mob} beni gebertti", "cesedim {bolge}'de kaldı :(", "off öldüm, eq'lerim orda kaldı",
    "kimse yardım etmedi, öldüm", "hayalet oldum ya", "{mob}'a dikkat edin, beni öldürdü"
};
POOL(ev_kill) {
    "{mob} gitti", "bir {mob} daha", "hah, {mob} öldü", "aldım {mob}'u"
};
POOL(ev_flee) {
    "kaçtım ya", "az kalsın ölüyordum", "off {mob} çok güçlü, kaçtım", "korktum kaçtım :)"
};
POOL(ev_quest_get) {
    "görev aldım, {mob} lazımmış", "görevci bana {mob}'u verdi", "görev: {mob}, {bolge}'deymiş",
    "yeni görev, gidiyorum"
};
POOL(ev_quest_done) {
    "görev tamam, {gp} gp oldu", "görevi teslim ettim", "gp topluyorum, {gp} oldu", "bir görev daha bitti"
};
POOL(ev_loot) {
    "{esya} buldum!", "oo {esya} düştü", "{esya} giydim, güzelmiş", "bak ne buldum: {esya}"
};
POOL(ev_login) {
    "selam ben geldim :)", "geldim, kim var?", "selam, bugün kim var?"
};
POOL(ev_logout) {
    "ben kaçıyorum, iyi oyunlar", "yatıyorum bb", "hadi bana eyv", "çıkıyorum, görüşürüz",
    "ben biraz ara vereyim, bb", "görüşürüz millet"
};
POOL(ev_cabal) {
    "artık {kabal} üyesiyim!", "{kabal} bizi kabul etti!", "{kabal}!!"
};
POOL(ev_pk_kill) {
    "hehe {hedef} gitti", "{kabal} güçlüdür!", "{hedef}, bir daha karşıma çıkma", "bu kadar mı {hedef}?"
};
POOL(ev_pk_taunt) {
    "{hedef}, geliyorum!", "{kabal} için!", "kaç {hedef}, kaç!", "{hedef} nerdesin, çıksana"
};
POOL(human_login) {
    "hoş geldin {hedef}", "selam {hedef}, naber?", "oo {hedef} gelmiş", "{hedef}! ne zamandır yoktun",
    "selam {hedef}, grup var mı?", "hoş geldin, {bolge}'deyim"
};
POOL(pray_lines) {
    "tanrım bir bakar mısın", "tanrılar bugün de bizimle mi?", "bir tanrı var mı oyunda?",
    "tanrım {mob} çok güçlü, yardım"
};

/* kişilik ekleri */
POOL(agresif_tail) { " lan", " be", "!", " ha", "" };
POOL(esprili_tail) { " :P", " xd", " hehe", " :))", "" };
POOL(acemi_say) {
    "nasıl level atlıyoruz ya?", "pratik nerde yapılıyor?", "bu eşyayı nasıl giyiyorum", "anımsa nasıl kullanılıyor?",
    "kim yardım eder, yeni başladım", "para nasıl kazanılıyor?", "görevci nerde?"
};
POOL(gizemli_say) {
    "rüzgar bu gece başka esiyor", "gölgeler yine uzuyor", "diyarın sırları tükenmez", "yolum uzun, sabrım geniş",
    "her ceset bir hikaye anlatır", "sessizlik en iyi zırhtır"
};

/* ---------------------------------------------------------------------
 * açılış
 * ------------------------------------------------------------------ */
void bot_chat_boot( void )
{
}

/* ---------------------------------------------------------------------
 * üslup
 * ------------------------------------------------------------------ */
static const char *tr_from[] = { "ç", "ğ", "ı", "ö", "ş", "ü", "Ç", "Ğ", "İ", "Ö", "Ş", "Ü", NULL };
static const char *tr_to[]   = { "c", "g", "i", "o", "s", "u", "C", "G", "I", "O", "S", "U" };

static void transliterate( const char *in, char *out, size_t size )
{
    size_t o = 0;

    while ( *in != '\0' && o + 4 < size )
    {
        int i;
        bool done = FALSE;

        for ( i = 0; tr_from[i] != NULL; i++ )
        {
            size_t l = strlen( tr_from[i] );
            if ( strncmp( in, tr_from[i], l ) == 0 )
            {
                out[o++] = tr_to[i][0];
                in += l;
                done = TRUE;
                break;
            }
        }
        if ( !done )
            out[o++] = *in++;
    }
    out[o] = '\0';
}

static void lower_ascii( char *s )
{
    for ( ; *s != '\0'; s++ )
        if ( (unsigned char) *s < 128 )
            *s = (char) tolower( (unsigned char) *s );
}

void bot_style( BOT_DATA *bot, const char *in, char *out, size_t size )
{
    char tmp[MAX_STRING_LENGTH];

    if ( !bot->turkce )
        transliterate( in, tmp, sizeof(tmp) );
    else
        snprintf( tmp, sizeof(tmp), "%s", in );
    if ( bot->kucuk_harf )
        lower_ascii( tmp );

    if ( bot->kisilik == BOT_K_AGRESIF && number_percent() < 35 )
        strncat( tmp, agresif_tail[number_range( 0, PN(agresif_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
    else if ( bot->kisilik == BOT_K_ESPRILI && number_percent() < 35 )
        strncat( tmp, esprili_tail[number_range( 0, PN(esprili_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
    else if ( bot->kisilik == BOT_K_GEVEZE && number_percent() < 20 )
        strncat( tmp, " :)", sizeof(tmp) - strlen( tmp ) - 1 );

    snprintf( out, size, "%s", tmp );
}

/* şablon alanlarını doldur */
void bot_fill( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size )
{
    CHAR_DATA *ch = bot->ch;
    char buf[MAX_STRING_LENGTH];
    size_t o = 0;
    const char *p = tmpl;

    buf[0] = '\0';
    while ( *p != '\0' && o + 80 < sizeof(buf) )
    {
        if ( *p == '{' )
        {
            const char *end = strchr( p, '}' );
            const char *val = NULL;
            char num[32];

            if ( end != NULL )
            {
                size_t klen = (size_t) ( end - p - 1 );
                if ( klen == 4 && !strncmp( p + 1, "isim", 4 ) )       val = ch != NULL ? ch->name : bot->name;
                else if ( klen == 5 && !strncmp( p + 1, "hedef", 5 ) ) val = other != NULL ? ( IS_NPC(other) ? other->short_descr : other->name ) : "dostum";
                else if ( klen == 5 && !strncmp( p + 1, "bolge", 5 ) ) val = bot_area_name( bot );
                else if ( klen == 6 && !strncmp( p + 1, "seviye", 6 ) ) { snprintf( num, sizeof(num), "%d", ch != NULL ? ch->level : 1 ); val = num; }
                else if ( klen == 5 && !strncmp( p + 1, "sinif", 5 ) ) val = class_table[bot->iclass].name[1];
                else if ( klen == 3 && !strncmp( p + 1, "mob", 3 ) )   val = bot->last_mob[0] != '\0' ? bot->last_mob : "bir yaratık";
                else if ( klen == 4 && !strncmp( p + 1, "esya", 4 ) )  val = bot->last_item[0] != '\0' ? bot->last_item : "bir şey";
                else if ( klen == 5 && !strncmp( p + 1, "kabal", 5 ) ) val = ch != NULL && ch->cabal != CABAL_NONE ? cabal_table[ch->cabal].long_name : "kabal";
                else if ( klen == 2 && !strncmp( p + 1, "gp", 2 ) )    { snprintf( num, sizeof(num), "%d", ch != NULL ? ch->pcdata->questpoints : 0 ); val = num; }
                if ( val != NULL )
                {
                    o += (size_t) snprintf( buf + o, sizeof(buf) - o, "%s", val );
                    p = end + 1;
                    continue;
                }
            }
        }
        buf[o++] = *p++;
        buf[o] = '\0';
    }
    buf[o] = '\0';
    bot_style( bot, buf, out, size );
}

/* ---------------------------------------------------------------------
 * konuşma çıkışı
 * ------------------------------------------------------------------ */
static bool tilde_or_brace( const char *s )
{
    for ( ; *s != '\0'; s++ )
        if ( *s == '~' || *s == '{' )
            return TRUE;
    return FALSE;
}

void bot_talk( BOT_DATA *bot, int channel, CHAR_DATA *to, const char *text )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || text == NULL || text[0] == '\0' || tilde_or_brace( text ) )
        return;
    if ( ch->position <= POS_SLEEPING && channel != BOT_CH_TELL )
        return;

    switch ( channel )
    {
    case BOT_CH_SAY:    bot_cmd( bot, "söyle %s", text ); break;
    case BOT_CH_TELL:
        if ( to == NULL || to->in_room == NULL || !can_see( ch, to ) )
            return;
        bot_cmd( bot, "kd %s %s", to->name, text );
        break;
    case BOT_CH_YELL:   bot_cmd( bot, "haykır %s", text ); break;
    case BOT_CH_GTELL:  bot_cmd( bot, "ganlat %s", text ); break;
    case BOT_CH_CABAL:  if ( ch->cabal != CABAL_NONE ) bot_cmd( bot, "kk %s", text ); break;
    case BOT_CH_EMOTE:  bot_cmd( bot, "duygu %s", text ); break;
    case BOT_CH_SOCIAL:
        if ( to != NULL && to->in_room == ch->in_room )
            bot_cmd( bot, "%s %s", text, to->name );
        else
            bot_cmd( bot, "%s", text );
        break;
    }
}

static const char *pick( const char **pool, int n )
{
    return pool[number_range( 0, n - 1 )];
}

static void say_later( BOT_DATA *bot, int channel, CHAR_DATA *to, const char *tmpl, int delay )
{
    char out[MAX_STRING_LENGTH];

    bot_fill( bot, tmpl, to, out, sizeof(out) );
    bot_queue_reply( bot, to != NULL ? to->name : "", channel, delay, out );
}

/* ---------------------------------------------------------------------
 * anlama
 * ------------------------------------------------------------------ */
static bool has_word( const char *text, const char *word )
{
    return strstr( text, word ) != NULL;
}

#define INTENT_NONE     0
#define INTENT_GREET    1
#define INTENT_HOW      2
#define INTENT_LEVEL    3
#define INTENT_WHERE    4
#define INTENT_CLASS    5
#define INTENT_GROUP    6
#define INTENT_STOP     7
#define INTENT_HELP     8
#define INTENT_THANKS   9
#define INTENT_BYE      10
#define INTENT_YES      11
#define INTENT_NO       12
#define INTENT_INSULT   13
#define INTENT_QUESTION 14

static int detect_intent( const char *raw )
{
    char text[MAX_STRING_LENGTH];
    char padded[MAX_STRING_LENGTH + 2];

    transliterate( raw, text, sizeof(text) );
    lower_ascii( text );
    snprintf( padded, sizeof(padded), " %s ", text );

    if ( has_word( padded, "salak" ) || has_word( padded, "aptal" ) || has_word( padded, "gerizekali" )
      || has_word( padded, " mal " ) || has_word( padded, "gebert" ) || has_word( padded, "oldururum" )
      || has_word( padded, "siktir" ) || has_word( padded, "ahmak" ) )
        return INTENT_INSULT;
    if ( has_word( padded, "sagol" ) || has_word( padded, "tesekkur" ) || has_word( padded, " eyv" )
      || has_word( padded, " tsk" ) || has_word( padded, "eyvallah" ) )
        return INTENT_THANKS;
    if ( has_word( padded, " bb " ) || has_word( padded, "gorusuruz" ) || has_word( padded, " bay " )
      || has_word( padded, "hoscakal" ) || has_word( padded, "iyi geceler" ) || has_word( padded, "kactim" )
      || has_word( padded, "cikiyorum" ) || has_word( padded, "yatiyorum" ) )
        return INTENT_BYE;
    if ( has_word( padded, " dur " ) || has_word( padded, " kal " ) || has_word( padded, "birak" )
      || has_word( padded, "takibi" ) || has_word( padded, "ayril" ) || has_word( padded, " git " ) )
        return INTENT_STOP;
    if ( has_word( padded, "grup" ) || has_word( padded, " gel " ) || has_word( padded, "gelsene" )
      || has_word( padded, "takip" ) || has_word( padded, "beraber" ) || has_word( padded, "birlikte" )
      || has_word( padded, "party" ) || has_word( padded, "gel." ) )
        return INTENT_GROUP;
    if ( has_word( padded, "yardim" ) || has_word( padded, " help" ) || has_word( padded, "lazim" )
      || has_word( padded, "imdat" ) )
        return INTENT_HELP;
    if ( has_word( padded, "nerdesin" ) || has_word( padded, "neredesin" ) || has_word( padded, "nerde" )
      || has_word( padded, "nerede" ) )
        return INTENT_WHERE;
    if ( has_word( padded, "level" ) || has_word( padded, " lvl" ) || has_word( padded, "seviye" )
      || has_word( padded, "kacsin" ) )
        return INTENT_LEVEL;
    if ( has_word( padded, "sinif" ) || has_word( padded, "class" ) || has_word( padded, "nesin" ) )
        return INTENT_CLASS;
    if ( has_word( padded, "nasilsin" ) || has_word( padded, "naber" ) || has_word( padded, "ne var ne yok" )
      || has_word( padded, "napiyorsun" ) || has_word( padded, "ne yapiyorsun" ) || has_word( padded, "napiyon" ) )
        return INTENT_HOW;
    if ( has_word( padded, "selam" ) || has_word( padded, " slm" ) || has_word( padded, "merhaba" )
      || has_word( padded, " mrb" ) || has_word( padded, " sa " ) || has_word( padded, " hey " )
      || has_word( padded, "hosgeldin" ) || has_word( padded, "hos geldin" ) )
        return INTENT_GREET;
    if ( has_word( padded, " evet" ) || has_word( padded, " tamam" ) || has_word( padded, " ok " )
      || has_word( padded, " olur" ) || has_word( padded, " hadi" ) || has_word( padded, " peki" ) )
        return INTENT_YES;
    if ( has_word( padded, " hayir" ) || has_word( padded, " yok " ) || has_word( padded, "istemem" )
      || has_word( padded, " olmaz" ) )
        return INTENT_NO;
    if ( strchr( text, '?' ) != NULL || has_word( padded, " mi " ) || has_word( padded, " mu " )
      || has_word( padded, " mi?" ) || has_word( padded, " mu?" ) )
        return INTENT_QUESTION;
    return INTENT_NONE;
}

static int reply_delay( const char *text )
{
    return 8 + number_range( 0, 12 ) + (int) UMIN( strlen( text ), 60 ) / 5;
}

void bot_chat_react( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text )
{
    CHAR_DATA *ch = bot->ch;
    int intent;
    int reply_ch = channel;
    const char *tmpl = NULL;
    bool speaker_bot = IS_BOT(speaker);
    bool human = bot_is_human( speaker );

    if ( ch == NULL || IS_NPC(speaker) )
        return;
    if ( channel == BOT_CH_SOCIAL || channel == BOT_CH_EMOTE )
    {
        /* sosyale sosyalle karşılık */
        if ( speaker->in_room == ch->in_room && number_percent() < ( human ? 60 : 15 ) )
            say_later( bot, BOT_CH_SOCIAL, speaker, pick( idle_social, PN(idle_social) ), number_range( 6, 20 ) );
        return;
    }

    intent = detect_intent( text );

    /* botlar birbirleriyle sonsuza dek konuşmasın */
    if ( speaker_bot )
    {
        if ( intent != INTENT_GROUP )
        {
            if ( bot_pulse - bot->last_bot_talk < 4 * 90 )
                return;
            if ( intent == INTENT_NONE || intent == INTENT_YES || intent == INTENT_NO || intent == INTENT_THANKS )
                return;
            if ( number_percent() > 40 )
                return;
        }
        bot->last_bot_talk = bot_pulse;
    }
    else if ( !human )
        return;

    if ( channel == BOT_CH_YELL && speaker->in_room != ch->in_room && number_percent() < 50 )
        reply_ch = BOT_CH_TELL;

    snprintf( bot->last_speaker, BOT_NAME_LEN, "%s", speaker->name );

    if ( human && IS_IMMORTAL(speaker) && number_percent() < 60 )
    {
        say_later( bot, reply_ch, speaker, pick( god_reply, PN(god_reply) ), reply_delay( text ) );
        return;
    }

    switch ( intent )
    {
    case INTENT_GREET:   tmpl = pick( greet_reply, PN(greet_reply) );       break;
    case INTENT_HOW:     tmpl = pick( howareyou_reply, PN(howareyou_reply) ); break;
    case INTENT_LEVEL:   tmpl = pick( level_reply, PN(level_reply) );       break;
    case INTENT_WHERE:   tmpl = pick( where_reply, PN(where_reply) );       break;
    case INTENT_CLASS:   tmpl = pick( class_reply, PN(class_reply) );       break;
    case INTENT_HELP:    tmpl = pick( help_reply, PN(help_reply) );         break;
    case INTENT_THANKS:  tmpl = pick( thanks_reply, PN(thanks_reply) );     break;
    case INTENT_BYE:
        tmpl = pick( bye_reply, PN(bye_reply) );
        if ( bot->state == BOT_ST_FOLLOW && bot_char_by_id( bot->leader_id ) == speaker )
            bot_stop_follow( bot, FALSE );
        break;
    case INTENT_YES:     tmpl = pick( yes_reply, PN(yes_reply) );           break;
    case INTENT_NO:      tmpl = pick( no_reply, PN(no_reply) );             break;
    case INTENT_INSULT:  tmpl = pick( insult_reply, PN(insult_reply) );     break;
    case INTENT_QUESTION: tmpl = pick( question_reply, PN(question_reply) ); break;
    case INTENT_GROUP:
        if ( speaker_bot && bot_wants_group_with( bot, speaker ) && speaker->in_room == ch->in_room
          && ch->master == NULL && speaker->master == NULL && number_percent() < 80 )
        {
            tmpl = pick( group_accept, PN(group_accept) );
            bot_start_follow( bot, speaker );
            reply_ch = BOT_CH_SAY;
        }
        else if ( human && bot_wants_group_with( bot, speaker ) )
        {
            if ( speaker->in_room == ch->in_room )
            {
                tmpl = pick( group_accept, PN(group_accept) );
                bot_start_follow( bot, speaker );
                reply_ch = BOT_CH_SAY;
            }
            else
                tmpl = pick( group_far, PN(group_far) );
        }
        else
            tmpl = pick( group_no, PN(group_no) );
        break;
    case INTENT_STOP:
        if ( bot->state == BOT_ST_FOLLOW && bot_char_by_id( bot->leader_id ) == speaker )
        {
            tmpl = pick( stop_reply, PN(stop_reply) );
            bot_stop_follow( bot, FALSE );
        }
        else if ( number_percent() < 30 )
            tmpl = pick( default_reply, PN(default_reply) );
        break;
    default:
        if ( channel == BOT_CH_TELL || number_percent() < 45 )
            tmpl = pick( default_reply, PN(default_reply) );
        break;
    }

    if ( tmpl != NULL )
        say_later( bot, reply_ch, speaker, tmpl, reply_delay( text ) );
}

/* ---------------------------------------------------------------------
 * olaylar
 * ------------------------------------------------------------------ */
static CHAR_DATA *human_in_room( CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
        if ( bot_is_human( rch ) && can_see( ch, rch ) )
            return rch;
    return NULL;
}

static CHAR_DATA *human_in_area( CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || d->character == NULL )
            continue;
        wch = d->character;
        if ( IS_BOT(wch) || wch->in_room == NULL || wch->in_room->area != ch->in_room->area )
            continue;
        if ( can_see( ch, wch ) )
            return wch;
    }
    return NULL;
}

void bot_chat_event( BOT_DATA *bot, int event, CHAR_DATA *other )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *room_h, *area_h, *any_h;
    int talk = bot->kisilik == BOT_K_GEVEZE ? 80 : bot->kisilik == BOT_K_SAKIN ? 30 : 55;

    if ( ch == NULL || ch->in_room == NULL )
        return;
    room_h = human_in_room( ch );
    area_h = human_in_area( ch );
    any_h  = bot_random_human();

    switch ( event )
    {
    case BOT_EV_LEVEL:
        if ( area_h != NULL && number_percent() < talk )
            say_later( bot, BOT_CH_YELL, NULL, pick( ev_level, PN(ev_level) ), number_range( 8, 30 ) );
        else if ( any_h != NULL && number_percent() < 30 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_level, PN(ev_level) ), number_range( 20, 80 ) );
        else if ( number_percent() < 40 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_level, PN(ev_level) ), number_range( 8, 30 ) );
        if ( ch->level >= 10 && bot->lakap != NULL && number_percent() < 25
          && ch->pcdata->title != NULL && strstr( ch->pcdata->title, title_table[ch->iclass][ch->level] ) != NULL )
        {
            char buf[MAX_INPUT_LENGTH];
            snprintf( buf, sizeof(buf), " %s", bot->lakap );
            set_title( ch, buf );
        }
        break;
    case BOT_EV_DEATH:
        if ( area_h != NULL && number_percent() < talk )
            say_later( bot, BOT_CH_YELL, NULL, pick( ev_death, PN(ev_death) ), number_range( 12, 40 ) );
        else if ( any_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_death, PN(ev_death) ), number_range( 30, 120 ) );
        else if ( number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_death, PN(ev_death) ), number_range( 12, 40 ) );
        break;
    case BOT_EV_KILL:
        if ( room_h != NULL && number_percent() < 12 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_kill, PN(ev_kill) ), number_range( 6, 20 ) );
        break;
    case BOT_EV_FLEE:
        if ( room_h != NULL && number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_flee, PN(ev_flee) ), number_range( 8, 20 ) );
        else if ( area_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_YELL, NULL, pick( ev_flee, PN(ev_flee) ), number_range( 8, 20 ) );
        break;
    case BOT_EV_QUEST_GET:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_quest_get, PN(ev_quest_get) ), number_range( 8, 20 ) );
        else if ( any_h != NULL && number_percent() < 20 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_quest_get, PN(ev_quest_get) ), number_range( 20, 60 ) );
        break;
    case BOT_EV_QUEST_DONE:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_quest_done, PN(ev_quest_done) ), number_range( 8, 20 ) );
        else if ( any_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_quest_done, PN(ev_quest_done) ), number_range( 20, 60 ) );
        break;
    case BOT_EV_LOOT:
        if ( current_time - boot_time < 600 || current_time - bot->login_time < 180 )
            break;                                    /* açılış/giriş teçhizatı için övünme */
        if ( room_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_loot, PN(ev_loot) ), number_range( 8, 20 ) );
        else if ( any_h != NULL && number_percent() < 8 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_loot, PN(ev_loot) ), number_range( 20, 60 ) );
        break;
    case BOT_EV_LOGIN:
        if ( current_time - boot_time < 600 )
            break;                                    /* açılışta toplu girişlerde susulur */
        if ( any_h != NULL && number_percent() < 15 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_login, PN(ev_login) ), number_range( 120, 360 ) );
        break;
    case BOT_EV_LOGOUT:
        if ( room_h != NULL && number_percent() < 70 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ev_logout, PN(ev_logout) ), 2 );
        else if ( any_h != NULL && number_percent() < 30 )
            say_later( bot, BOT_CH_TELL, any_h, pick( ev_logout, PN(ev_logout) ), 2 );
        break;
    case BOT_EV_CABAL:
        say_later( bot, BOT_CH_YELL, NULL, pick( ev_cabal, PN(ev_cabal) ), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_KILL:
        say_later( bot, BOT_CH_YELL, other, pick( ev_pk_kill, PN(ev_pk_kill) ), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_TAUNT:
        if ( other != NULL )
        {
            char out[MAX_STRING_LENGTH];
            bot_fill( bot, pick( ev_pk_taunt, PN(ev_pk_taunt) ), other, out, sizeof(out) );
            bot_talk( bot, BOT_CH_YELL, NULL, out );
        }
        break;
    case BOT_EV_HUMAN_LOGIN:
        if ( other != NULL )
            say_later( bot, BOT_CH_TELL, other, pick( human_login, PN(human_login) ), number_range( 60, 320 ) );
        break;
    case BOT_EV_HUMAN_ROOM:
        if ( other != NULL )
        {
            say_later( bot, BOT_CH_SAY, other, pick( room_greet, PN(room_greet) ), number_range( 6, 16 ) );
            if ( number_percent() < 40 )
                say_later( bot, BOT_CH_SOCIAL, other, "selamla", number_range( 20, 40 ) );
        }
        break;
    }
}

/* ---------------------------------------------------------------------
 * boş zaman gevezeliği
 * ------------------------------------------------------------------ */
static bool immortal_online( void )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && d->character != NULL && IS_IMMORTAL(d->character)
          && !IS_BOT(d->character) )
            return TRUE;
    return FALSE;
}

static void bot_try_auction( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    OBJ_DATA *obj;
    char kw[MAX_INPUT_LENGTH];

    if ( auction == NULL )
        return;
    if ( auction->item != NULL )
    {
        /* başkasının mezadına teklif ver */
        if ( auction->seller != ch && auction->buyer != ch && ch->silver > 1500
          && auction->bet < ch->silver / 4 && number_percent() < 35 )
            bot_cmd( bot, "mezat teklif %d", UMAX( auction->bet + 1, auction->bet + auction->bet / 5 + number_range( 5, 60 ) ) );
        return;
    }
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc != WEAR_NONE || obj->cost < 200 || gorev_ekipmani_mi( obj ) )
            continue;
        if ( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
            continue;
        if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
            continue;
        bot_obj_keyword( ch, obj, ch->carrying, kw, sizeof(kw) );
        bot_cmd( bot, "mezat %s", kw );
        return;
    }
}

void bot_chat_idle( BOT_DATA *bot )
{
    CHAR_DATA *ch = bot->ch;
    CHAR_DATA *room_h, *area_h, *any_h;
    int base;

    if ( ch == NULL || ch->in_room == NULL || ch->position <= POS_SLEEPING || ch->fighting != NULL )
        return;

    switch ( bot->kisilik )
    {
    case BOT_K_GEVEZE:  base = 4 * 60 * 3;  break;
    case BOT_K_SAKIN:   base = 4 * 60 * 14; break;
    case BOT_K_GIZEMLI: base = 4 * 60 * 10; break;
    default:            base = 4 * 60 * 6;  break;
    }
    bot->next_chat = bot_pulse + base + number_range( 0, base );

    room_h = human_in_room( ch );
    area_h = human_in_area( ch );
    any_h  = bot_random_human();

    /* odada bir insan varsa selam ver (bir kez) */
    if ( room_h != NULL && ( bot_pulse - bot->greeted_pulse > 4 * 60 * 10 || str_cmp( bot->last_human, room_h->name ) ) )
    {
        bot->greeted_pulse = bot_pulse;
        snprintf( bot->last_human, BOT_NAME_LEN, "%s", room_h->name );
        if ( number_percent() < 65 )
        {
            bot_chat_event( bot, BOT_EV_HUMAN_ROOM, room_h );
            return;
        }
    }

    if ( number_percent() < 6 && immortal_online() )
    {
        char out[MAX_STRING_LENGTH];
        bot_fill( bot, pick( pray_lines, PN(pray_lines) ), NULL, out, sizeof(out) );
        bot_cmd( bot, "dua %s", out );
        return;
    }
    if ( number_percent() < 7 && ch->level >= 6 )
    {
        bot_try_auction( bot );
        return;
    }

    if ( room_h != NULL )
    {
        int roll = number_percent();
        const char *tmpl;

        if ( roll < 20 )
        {
            bot_talk( bot, BOT_CH_SOCIAL, room_h, pick( idle_social, PN(idle_social) ) );
            return;
        }
        if ( bot->kisilik == BOT_K_ACEMI && roll < 60 )
            tmpl = pick( acemi_say, PN(acemi_say) );
        else if ( bot->kisilik == BOT_K_GIZEMLI && roll < 60 )
            tmpl = pick( gizemli_say, PN(gizemli_say) );
        else
            tmpl = pick( idle_say, PN(idle_say) );
        say_later( bot, BOT_CH_SAY, room_h, tmpl, number_range( 2, 12 ) );
        return;
    }
    if ( area_h != NULL && number_percent() < 55 )
    {
        say_later( bot, BOT_CH_YELL, NULL, pick( idle_yell, PN(idle_yell) ), number_range( 2, 12 ) );
        return;
    }
    if ( any_h != NULL && bot_pulse >= bot->next_kd && !IS_IMMORTAL(any_h) )
    {
        bot->next_kd = bot_pulse + 4 * 60 * ( bot->kisilik == BOT_K_GEVEZE ? 12 : 25 ) + number_range( 0, 4 * 60 * 10 );
        say_later( bot, BOT_CH_TELL, any_h, pick( idle_kd, PN(idle_kd) ), number_range( 2, 12 ) );
        return;
    }
    if ( bot_in_group( ch ) && number_percent() < 40 )
    {
        say_later( bot, BOT_CH_GTELL, NULL, pick( idle_say, PN(idle_say) ), number_range( 2, 12 ) );
        return;
    }
    if ( ch->cabal != CABAL_NONE && number_percent() < 25 )
    {
        say_later( bot, BOT_CH_CABAL, NULL, pick( idle_say, PN(idle_say) ), number_range( 2, 12 ) );
        return;
    }
    if ( number_percent() < 30 )
    {
        const char *tmpl = bot->kisilik == BOT_K_GIZEMLI ? pick( gizemli_say, PN(gizemli_say) )
                         : bot->kisilik == BOT_K_ACEMI ? pick( acemi_say, PN(acemi_say) )
                         : pick( idle_say, PN(idle_say) );
        say_later( bot, BOT_CH_SAY, NULL, tmpl, number_range( 2, 12 ) );
    }
    else if ( number_percent() < 30 )
        bot_talk( bot, BOT_CH_SOCIAL, NULL, pick( idle_social, PN(idle_social) ) );
}
