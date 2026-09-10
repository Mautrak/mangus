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

/* =====================================================================
 * ROL İÇİ (söyle / haykır / duygu): diyarın diliyle, oyun dışı sözcük yok
 * ===================================================================== */
POOL(ic_greet) {
    "Selam olsun {hedef}.", "Yolun açık olsun, {hedef}.", "Hoş bulduk, yolcu.", "Merhaba yabancı, nereden gelirsin?",
    "Selam sana {hedef}.", "Tanrılar yolunu aydınlatsın.", "Hoş geldin {hedef}, buralar tehlikeli.", "Merhabalar."
};
POOL(ic_how) {
    "İyiyim, sağ ol. Sen nasılsın?", "Yorgunum lakin ayaktayım.", "Az evvel ölümden döndüm, sorma.",
    "Şükür, {bolge}'de avlanıyorum.", "Kesem hafif, kılıcım ağır; yani her zamanki gibi.", "{mob} canımı çıkardı ama iyiyim.",
    "Fena değil. Senden ne haber?", "Karnım aç, gerisi iyi."
};
POOL(ic_level) {
    "{seviye}. mertebedeyim.", "Henüz {seviye}.", "{seviye}, yavaş yavaş güçleniyorum.", "{seviye}. Sen kaçıncı mertebedesin?",
    "{seviye}; bir sonrakine az kaldı."
};
POOL(ic_where) {
    "{bolge}'deyim.", "{bolge} taraflarında dolaşıyorum.", "Şu an {bolge}'de avlanıyorum.", "{bolge}. Gel, birlikte avlanalım."
};
POOL(ic_class) {
    "Ben bir {sinif}.", "{sinif}im.", "{sinif}. Ya sen?", "Mesleğim {sinif}."
};
POOL(ic_group_accept) {
    "Olur, seninle gelirim.", "Peki, ardındayım.", "Birlikte daha güçlüyüz; hadi.", "Kabul, yol arkadaşı olalım.",
    "Tamam, beni takımına al."
};
POOL(ic_group_far) {
    "Sen buraya gel, ben {bolge}'deyim.", "Olur ama ben {bolge}'deyim; sen gel.", "{bolge}'ye gelirsen olur.",
    "Şu işi bitireyim, sonra gelirim."
};
POOL(ic_group_no) {
    "Gücümüz denk değil, kusura bakma.", "Şimdilik yalnız yürüyorum.", "Bugün olmaz.", "Belki daha sonra."
};
POOL(ic_stop) {
    "Peki.", "Olur, yollarımız ayrılsın.", "Tamam, ben kendi yoluma gideyim.", "Sağlıcakla."
};
POOL(ic_help) {
    "Ne lazım?", "Nasıl yardım edebilirim?", "Söyle bakalım, derdin ne?", "Neredesin, geleyim.", "Buyur."
};
POOL(ic_thanks) {
    "Rica ederim.", "Lafı mı olur.", "Ne demek.", "Her zaman.", "Tanrılar seni korusun."
};
POOL(ic_bye) {
    "Yolun açık olsun.", "Esen kal {hedef}.", "Görüşmek üzere.", "Sağlıcakla kal.", "Tanrılar yanında olsun."
};
POOL(ic_yes) { "Peki.", "Öyle olsun.", "Olur.", "Hadi o halde.", "Tamam." };
POOL(ic_no)  { "Peki.", "Anladım.", "Olsun.", "Nasıl istersen." };
POOL(ic_insult) {
    "Dilini tut, yoksa kılıcım tutar!", "Bak sen şu küstaha!", "Bir daha söyle de görelim.", "Ağzını topla yabancı.",
    "Sen kimsin de bana böyle söz edersin?", "Ha ha! Cesaretini beğendim."
};
POOL(ic_question) {
    "Bilmem ki.", "Sanmam.", "Olabilir.", "Tanrılar bilir.", "Emin değilim.", "Galiba öyle.", "Bence hayır."
};
POOL(ic_default) {
    "Hmm.", "Doğru dersin.", "Öyle mi?", "Bak sen.", "Ha.", "Evet.", "Sanmam.", "Olsun.", "İlginç."
};
POOL(ic_god) {
    "Tanrım!", "Ölümsüzlere selam olsun.", "Efendim, buyurun.", "Yüce olan, bize bir bak.",
    "Tanrım, ne zamandır yoktunuz.", "Hoş geldiniz efendim."
};
POOL(ic_idle_say) {
    "Bu {mob} hayli çetin çıktı.", "Kılıcım kan istiyor.", "Karnım zil çalıyor; bir somun ekmek olsa.",
    "{bolge} yolları tehlikeli bu aralar.", "Tanrılar bugün bize gülümsedi.", "Şu {esya} kimin işine yarar, bilmem.",
    "Kesem hafif, kılıcım ağır.", "Yeni bir silah edinmenin vakti geldi.", "Görevcinin işi bitmek bilmiyor.",
    "Burası zifiri karanlık.", "Nerelerde avlanırsın yolcu?", "Şu ustadan yeni bir hüner öğrenmeliyim.",
    "Biraz soluklanayım.", "Bu diyarın her köşesi tuzak dolu.", "Kimse yok mu buralarda?", "Yaralarım sızlıyor."
};
POOL(ic_idle_yell) {
    "{bolge}'de dolaşan kimse var mı?", "İmdat! {mob} peşimde!", "Kim yiğitse gelsin, {bolge}'de kesim var!",
    "Yoldaş arıyorum, {bolge} taraflarındayım!", "Sesimi duyan var mı?", "Şifacı olan gelsin, yaram derin!",
    "{mob} nerede, bilen var mı?"
};
POOL(ic_room_greet) {
    "Selam olsun {hedef}.", "Sen de mi buralarda avlanıyorsun?", "Merhaba {hedef}, yolun açık olsun.",
    "Oo, {hedef}! Ne var ne yok?", "Hoş geldin {hedef}."
};
POOL(ic_level_up) {
    "Gücüm arttı, hissediyorum!", "Tanrılar bana yeni bir kudret bahşetti!", "Bir mertebe daha yükseldim!",
    "Damarlarımda yeni bir güç dolaşıyor."
};
POOL(ic_death) {
    "Ölüm soğukmuş.", "{mob} beni yere serdi.", "Bedenim {bolge}'de kaldı, ruhum burada.",
    "Kimse imdadıma yetişmedi.", "{mob}'dan uzak durun; beni öldürdü."
};
POOL(ic_kill) {
    "{mob} gitti.", "Bir {mob} daha toprağa düştü.", "Hah, {mob} öldü."
};
POOL(ic_flee) {
    "Canımı zor kurtardım!", "Bu {mob} fazlasıyla güçlü, kaçtım.", "Az kalsın ölüyordum."
};
POOL(ic_quest_get) {
    "Görevci bana {mob}'u emanet etti.", "Yeni bir vazife: {mob}, {bolge}'deymiş.", "Görevciden yeni bir iş aldım."
};
POOL(ic_quest_done) {
    "Vazifeyi tamamladım, görevci memnun kaldı.", "Bir görev daha bitti.", "Görevci beni ödüllendirdi."
};
POOL(ic_loot) {
    "{esya} buldum!", "Bak ne çıktı: {esya}.", "{esya}... işime yarar."
};
POOL(ic_logout) {
    "Yolum uzun, hoşça kalın.", "Dinlenmeye çekiliyorum, esen kalın.", "Ben ayrılıyorum, tanrılar sizi korusun."
};
POOL(ic_cabal) {
    "Artık {kabal} saflarındayım!", "{kabal} beni kabul etti!", "Yaşasın {kabal}!"
};
POOL(ic_pk_kill) {
    "{kabal} güçlüdür!", "{hedef}, bir daha karşıma çıkma!", "Bu kadar mıydı {hedef}?", "{hedef} toprağı boyladı!"
};
POOL(ic_pk_taunt) {
    "{hedef}, karşıma çık!", "{kabal} için!", "Kaç {hedef}, kaç!", "{hedef}, neredesin? Çık ortaya!"
};
POOL(ic_pray) {
    "Tanrım, bir bakar mısın?", "Tanrılar bugün de bizimle mi?", "Yüce olan, {mob} çok güçlü; yardım et."
};
POOL(ic_acemi) {
    "Bu dünyada nasıl güçlenilir?", "Ustalar nerede bulunur?", "Bu zırhı nasıl kuşanırım?",
    "Bu diyara yeni geldim, yardım eden olur mu?", "Görevci nerede?", "Para nasıl kazanılır burada?"
};
POOL(ic_gizemli) {
    "Rüzgar bu gece başka esiyor.", "Gölgeler yine uzuyor.", "Diyarın sırları tükenmez.", "Yolum uzun, sabrım geniş.",
    "Her ceset bir hikaye anlatır.", "Sessizlik en iyi zırhtır."
};
POOL(ic_social) {
    "esne", "gülümse", "omuzsilk", "ıslık", "ürper", "gözdik", "kıkırda", "reverans", "elsalla"
};

/* =====================================================================
 * KONU DIŞI (kd / kdg): oyuncu ağzı, kısaltmalar serbest
 * ===================================================================== */
POOL(ooc_greet) {
    "selam {hedef}", "slm", "merhaba :)", "selamlar", "selam kanka", "sa", "selam naber",
    "hoş geldin {hedef}", "merhabalar", "selam, kaç level oldun?", "oo {hedef}, selam", "iyi oyunlar {hedef}"
};
POOL(ooc_how) {
    "iyiyim sen?", "idare eder, {bolge}'de kesiyorum", "yorgunum biraz ama iyi", "eh işte, senden?",
    "iyi iyi, level kasıyorum", "bomba gibiyim :)", "az önce öldüm ya, kötü", "iyiyim, {mob} kesiyorum",
    "sıkıldım biraz, grup olsak mı", "fena değil, sen naber?"
};
POOL(ooc_level) {
    "{seviye} oldum", "daha {seviye}", "{seviye}, yavaş yavaş", "{seviye} lvl", "şimdilik {seviye}",
    "{seviye}, bir sonrakine az kaldı", "{seviye}, sen kaçsın?"
};
POOL(ooc_where) {
    "{bolge}'deyim", "{bolge} civarındayım", "şu an {bolge}", "{bolge}, gel istersen", "{bolge}'de kesiyorum"
};
POOL(ooc_class) {
    "{sinif}", "{sinif}im ben", "{sinif}, sen?", "ben {sinif}", "{sinif} olarak açtım"
};
POOL(ooc_group_far) {
    "sen gel, ben {bolge}'deyim", "olur ama ben {bolge}'deyim, sen gel", "{bolge}'ye gelirsen olur",
    "sen gel, seni burada bekliyorum", "gel, {bolge}'de bekliyorum"
};
POOL(ooc_group_no) {
    "seviyeler tutmuyor ya", "şu an olmaz kusura bakma", "biraz sonra belki", "yalnız takılıyorum bu ara"
};
POOL(ooc_stop) { "tamam", "peki", "ok, ben devam ediyorum o zaman", "tamam görüşürüz", "olur" };
POOL(ooc_help) { "ne lazım?", "nasıl yardım edeyim?", "söyle bakalım", "ne oldu?", "buyur", "yardım mı? nerdesin?" };
POOL(ooc_thanks) { "rica ederim", "ne demek", "önemli değil :)", "eyv", "her zaman", "lafı mı olur" };
POOL(ooc_bye) { "görüşürüz", "bb", "iyi oyunlar", "hadi eyv", "görüşürüz {hedef}", "kendine iyi bak", "bb, iyi geceler" };
POOL(ooc_yes) { "tamam", "ok", "olur", "hadi o zaman", "peki" };
POOL(ooc_no)  { "peki", "tamam sorun değil", "olsun", "anladım" };
POOL(ooc_insult) {
    "ne dedin sen?", "hoop, sakin", "bak sen", "ağzını topla", "hahaha", "sen kimsin ya", "off ne kadar kabasın"
};
POOL(ooc_question) {
    "bilmem ki", "sanmıyorum", "olabilir", "valla bilmiyorum", "yardıma sor", "hmm, emin değilim",
    "galiba öyle", "ne bileyim :)", "bence hayır", "evet gibi"
};
POOL(ooc_default) {
    "hmm", "aynen", "valla", "öyle mi", ":)", "ne diyosun", "anlamadım", "bak sen", "hehe",
    "hah", "evet", "aynen öyle", "doğru", "yok yok", "olsun", "eyw", "hmm ilginç"
};
POOL(ooc_god) {
    "tanrım!", "hoş geldiniz efendim", "aa bir tanrı", "selam tanrım, ne zamandır yoktunuz", "efendim buyrun"
};
POOL(ooc_kd_idle) {
    "selam, naber?", "grup olalım mı? {bolge}'deyim", "kaç level oldun?", "yardım lazım mı?",
    "nerdesin?", "bugün {mob} kestim, {esya} düştü", "{bolge}'ye geliyor musun?", "ne yapıyorsun?",
    "sıkıldım ya", "level {seviye} oldum :)", "sende fazla eq var mı?", "görev yapıyor musun?",
    "{bolge} nasıl, gidilir mi?", "bir ara beraber keselim"
};
POOL(ooc_kdg_idle) {
    "kim var oyunda?", "grup arayan var mı? {bolge}'deyim", "level {seviye} oldum :)", "{bolge} nasıl, gidilir mi?",
    "sende fazla eq var mı, satın alırım", "bugün {mob} kestim, {esya} düştü", "görev yapan var mı, gp kaç oldu?",
    "cure light atabilecek biri var mı?", "yeni başlayan varsa yardım ederim", "hangi bölge {seviye} level için iyi?",
    "kabal kuran var mı?", "mezatta bir şey var mı?", "sıkıldım, bir şey yapalım", "kim nerde kesiyor?"
};
POOL(ooc_kdg_reply) {
    "ben varım", "buradayım, {bolge}'de", "{seviye} lvl {sinif}, {bolge}'deyim", "ben de sıkıldım :)",
    "ben {bolge}'de kesiyorum", "hangi level?", "bende yok ya", "olabilir, nerdesin?"
};
POOL(ooc_gtell) {
    "yp %{yp}", "mana bitti", "biraz dinlenelim mi", "sıradaki {mob} olsun", "dikkat, kaçarım gerekirse",
    "hangi yöne?", "bekle, yiyorum", "tamam devam"
};
POOL(ooc_cabal) {
    "eşya yerinde mi?", "{bolge}'de düşman gördüm", "hq'ya gelen var mı?", "kim var kabalda?",
    "koruma nöbetini kim alıyor?", "seviye {seviye} oldum"
};
POOL(ooc_level_up) { "level {seviye} :)", "{seviye} oldum sonunda", "level atladım, {seviye}" };
POOL(ooc_death) { "öldüm ya, {mob} gebertti", "cesedim {bolge}'de kaldı :(", "öldüm, eq'ler orda kaldı" };
POOL(ooc_quest_done) { "görev tamam, {gp} gp oldu", "bir görev daha bitti, gp {gp}" };
POOL(ooc_login) { "selam ben geldim :)", "geldim, kim var?", "selam, bugün kim var?" };
POOL(ooc_logout) { "ben kaçıyorum, iyi oyunlar", "yatıyorum bb", "çıkıyorum, görüşürüz", "görüşürüz millet" };
POOL(ooc_human_login) {
    "hoş geldin {hedef}", "selam {hedef}, naber?", "oo {hedef} gelmiş", "{hedef}! ne zamandır yoktun",
    "selam {hedef}, grup var mı?", "hoş geldin, {bolge}'deyim"
};

/* kişilik ekleri */
POOL(agresif_tail) { " lan", " be", "!", " ha", "" };
POOL(esprili_tail) { " :P", " xd", " hehe", " :))", "" };
POOL(ic_agresif_tail) { "!", "!", " Duydun mu?", "", "" };

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

/* yalnızca ASCII harfleri küçültür (çok baytlı UTF-8 dokunulmaz) */
void bot_lower_ascii( char *s )
{
    for ( ; *s != '\0'; s++ )
        if ( (unsigned char) *s < 128 )
            *s = (char) tolower( (unsigned char) *s );
}


static bool channel_ic( int channel )
{
    return channel == BOT_CH_SAY || channel == BOT_CH_YELL || channel == BOT_CH_EMOTE || channel == BOT_CH_SOCIAL;
}

void bot_style( BOT_DATA *bot, const char *in, char *out, size_t size )
{
    bot_style_ch( bot, in, out, size, FALSE );
}

/* ic: rol içi kanal (söyle/haykır) - oyun dışı kısaltma ve surat eklenmez */
void bot_style_ch( BOT_DATA *bot, const char *in, char *out, size_t size, bool ic )
{
    char tmp[MAX_INPUT_LENGTH];        /* sonuç bir komut satırı olur */

    if ( !bot->turkce )
        transliterate( in, tmp, sizeof(tmp) );
    else
        snprintf( tmp, sizeof(tmp), "%s", in );
    if ( bot->kucuk_harf && !ic )
        bot_lower_ascii( tmp );

    if ( ic )
    {
        if ( bot->kisilik == BOT_K_AGRESIF && number_percent() < 30 )
        {
            size_t l = strlen( tmp );
            if ( l > 0 && tmp[l - 1] == '.' )
                tmp[l - 1] = '\0';
            strncat( tmp, ic_agresif_tail[number_range( 0, BOT_PN(ic_agresif_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
        }
    }
    else if ( bot->kisilik == BOT_K_AGRESIF && number_percent() < 35 )
        strncat( tmp, agresif_tail[number_range( 0, BOT_PN(agresif_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
    else if ( bot->kisilik == BOT_K_ESPRILI && number_percent() < 35 )
        strncat( tmp, esprili_tail[number_range( 0, BOT_PN(esprili_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
    else if ( bot->kisilik == BOT_K_GEVEZE && number_percent() < 20 )
        strncat( tmp, " :)", sizeof(tmp) - strlen( tmp ) - 1 );

    snprintf( out, size, "%s", tmp );
}

/* şablon alanlarını doldur */
void bot_fill( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size )
{
    bot_fill_ch( bot, tmpl, other, out, size, FALSE );
}

/* sınırlı ekleme: buf+*o'ya val'i sığdığı kadar yazar */
static void fill_append( char *buf, size_t size, size_t *o, const char *val )
{
    size_t l = strlen( val );

    if ( *o >= size - 1 )
        return;
    if ( l > size - 1 - *o )
        l = size - 1 - *o;
    memcpy( buf + *o, val, l );
    *o += l;
    buf[*o] = '\0';
}

/* {anahtar} değeri; sayılar num'a yazılır. Bilinmeyen anahtar: NULL */
static const char *fill_value( BOT_DATA *bot, const char *key, size_t klen, CHAR_DATA *other,
                               char *num, size_t nsize )
{
    CHAR_DATA *ch = bot->ch;
#define KEY(k) ( klen == sizeof(k) - 1 && !strncmp( key, k, klen ) )
    if ( KEY("isim") )   return ch != NULL ? ch->name : bot->name;
    if ( KEY("hedef") )  return other != NULL ? ( IS_NPC(other) ? other->short_descr : other->name ) : "dostum";
    if ( KEY("bolge") )  return bot_area_name( bot );
    if ( KEY("sinif") )  return class_table[bot->iclass].name[1];
    if ( KEY("mob") )    return bot->last_mob[0] != '\0' ? bot->last_mob : "bir yaratık";
    if ( KEY("esya") )   return bot->last_item[0] != '\0' ? bot->last_item : "bir şey";
    if ( KEY("kabal") )  return ch != NULL && ch->cabal != CABAL_NONE ? cabal_table[ch->cabal].long_name : "kabal";
    if ( KEY("seviye") ) { snprintf( num, nsize, "%d", ch != NULL ? ch->level : 1 ); return num; }
    if ( KEY("gp") )     { snprintf( num, nsize, "%d", ch != NULL ? ch->pcdata->questpoints : 0 ); return num; }
    if ( KEY("yp") )     { snprintf( num, nsize, "%d", ch != NULL ? bot_pct( ch->hit, ch->max_hit ) : 100 ); return num; }
#undef KEY
    return NULL;
}

void bot_fill_ch( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size, bool ic )
{
    char buf[MAX_INPUT_LENGTH];
    size_t o = 0;
    const char *p = tmpl;

    buf[0] = '\0';
    while ( *p != '\0' && o < sizeof(buf) - 1 )
    {
        if ( *p == '{' )
        {
            const char *end = strchr( p, '}' );
            char num[32];
            const char *val;

            if ( end != NULL && ( val = fill_value( bot, p + 1, (size_t) ( end - p - 1 ), other, num, sizeof(num) ) ) != NULL )
            {
                fill_append( buf, sizeof(buf), &o, val );
                p = end + 1;
                continue;
            }
        }
        buf[o++] = *p++;
    }
    buf[o] = '\0';
    bot_style_ch( bot, buf, out, size, ic );
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

/* kdg kanalı için dünya çapında hız sınırı: botlar kanalı boğmasın */
static int last_kdg_pulse = -100000;

void bot_talk( BOT_DATA *bot, int channel, CHAR_DATA *to, const char *text )
{
    CHAR_DATA *ch = bot->ch;

    if ( ch == NULL || text == NULL || text[0] == '\0' || tilde_or_brace( text ) )
        return;
    if ( ch->position <= POS_SLEEPING && channel != BOT_CH_TELL && channel != BOT_CH_KDG )
        return;

    switch ( channel )
    {
    case BOT_CH_SAY:    bot_cmd( bot, "söyle %s", text ); break;
    case BOT_CH_TELL:
        if ( to == NULL || to->in_room == NULL || !can_see( ch, to ) )
            return;
        bot_cmd( bot, "kd %s %s", to->name, text );
        break;
    case BOT_CH_KDG:
        if ( bot_pulse - last_kdg_pulse < BOT_SEC(45) )
            return;
        last_kdg_pulse = bot_pulse;
        bot_cmd( bot, "kdg %s", text );
        break;
    case BOT_CH_YELL:   bot_cmd( bot, "haykır %s", text ); break;
    case BOT_CH_GTELL:  bot_cmd( bot, "ganlat %s", text ); break;
    case BOT_CH_CABAL:  if ( ch->cabal != CABAL_NONE ) bot_cmd( bot, "kk %s", text ); break;
    case BOT_CH_EMOTE:  bot_cmd( bot, "duygu %s", text ); break;
    case BOT_CH_IMM:    if ( IS_IMMORTAL(ch) ) bot_cmd( bot, "immtalk %s", text ); break;
    case BOT_CH_SOCIAL:
        if ( to != NULL && to->in_room == ch->in_room )
            bot_cmd( bot, "%s %s", text, to->name );
        else
            bot_cmd( bot, "%s", text );
        break;
    }
}

static void say_later( BOT_DATA *bot, int channel, CHAR_DATA *to, const char *tmpl, int delay )
{
    char out[MAX_INPUT_LENGTH];

    bot_fill_ch( bot, tmpl, to, out, sizeof(out), channel_ic( channel ) );
    bot_queue_reply( bot, to != NULL ? to->name : "", channel, delay, out );
    /* odada birine cevap verecekse konuşmak için durur (yürüyüp gitmez) */
    if ( ( channel == BOT_CH_SAY || channel == BOT_CH_SOCIAL || channel == BOT_CH_EMOTE )
      && to != NULL && bot->ch != NULL && to->in_room == bot->ch->in_room && bot_is_human( to ) )
        bot->hold_until = UMAX( bot->hold_until, bot_pulse + BOT_SEC(delay) + 12 );
}

/* ---------------------------------------------------------------------
 * anlama
 * ------------------------------------------------------------------ */
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
#define INTENT_CABAL    15

/*
 * Niyet tablosu: sıra önemlidir (ilk eşleşen kazanır). Sözcükler ASCII'ye
 * indirgenmiş, küçük harfli ve iki yanı boşlukla çevrili metinde aranır.
 * 'need' verilmişse o sözcük de geçmelidir.
 */
struct intent_words
{
    int         intent;
    const char *need;
    const char *words[10];
};

static const struct intent_words intent_table[] =
{
    { INTENT_INSULT,   NULL,    { "salak", "aptal", "gerizekali", " mal ", "gebert", "oldururum", "siktir", "ahmak", NULL } },
    { INTENT_THANKS,   NULL,    { "sagol", "tesekkur", " eyv", " tsk", "eyvallah", NULL } },
    { INTENT_BYE,      NULL,    { " bb ", "gorusuruz", " bay ", "hoscakal", "iyi geceler", "kactim", "cikiyorum", "yatiyorum", NULL } },
    { INTENT_STOP,     NULL,    { " dur ", " kal ", "birak", "takibi", "ayril", " git ", NULL } },
    { INTENT_CABAL,    "kabal", { "katil", "uye", "induct", "al beni", "alir mi", "girmek", "kabul", NULL } },
    { INTENT_GROUP,    NULL,    { "grup", " gel ", "gelsene", "takip", "beraber", "birlikte", "party", "gel.", NULL } },
    { INTENT_HELP,     NULL,    { "yardim", " help", "lazim", "imdat", NULL } },
    { INTENT_WHERE,    NULL,    { "nerdesin", "neredesin", "nerde", "nerede", NULL } },
    { INTENT_LEVEL,    NULL,    { "level", " lvl", "seviye", "kacsin", NULL } },
    { INTENT_CLASS,    NULL,    { "sinif", "class", "nesin", NULL } },
    { INTENT_HOW,      NULL,    { "nasilsin", "naber", "ne var ne yok", "napiyorsun", "ne yapiyorsun", "napiyon", NULL } },
    { INTENT_GREET,    NULL,    { "selam", " slm", "merhaba", " mrb", " sa ", " hey ", "hosgeldin", "hos geldin", NULL } },
    { INTENT_YES,      NULL,    { " evet", " tamam", " ok ", " olur", " hadi", " peki", NULL } },
    { INTENT_NO,       NULL,    { " hayir", " yok ", "istemem", " olmaz", NULL } },
    { INTENT_QUESTION, NULL,    { "?", " mi ", " mu ", " mi?", " mu?", NULL } },
};

static int detect_intent( const char *raw )
{
    char text[MAX_INPUT_LENGTH];
    char padded[MAX_INPUT_LENGTH + 2];
    int i, w;

    transliterate( raw, text, sizeof(text) );
    bot_lower_ascii( text );
    snprintf( padded, sizeof(padded), " %s ", text );

    for ( i = 0; i < BOT_PN(intent_table); i++ )
    {
        const struct intent_words *t = &intent_table[i];

        if ( t->need != NULL && strstr( padded, t->need ) == NULL )
            continue;
        for ( w = 0; t->words[w] != NULL; w++ )
            if ( strstr( padded, t->words[w] ) != NULL )
                return t->intent;
    }
    return INTENT_NONE;
}

static int reply_delay( const char *text )
{
    return 8 + number_range( 0, 12 ) + (int) UMIN( strlen( text ), 60 ) / 5;
}

/* rol içi / konu dışı havuz seçimi */
#define PICK2(ic, icpool, oocpool) ( (ic) ? BOT_PICK(icpool) : BOT_PICK(oocpool) )

/* bot-bot sohbeti sınırlı: botlar birbirleriyle sonsuza dek konuşmasın. Cevap verilecekse TRUE */
static bool react_bot_allowed( BOT_DATA *bot, int channel, int intent )
{
    if ( intent != INTENT_GROUP && intent != INTENT_CABAL && !( channel == BOT_CH_CABAL && intent == INTENT_HELP ) )
    {
        if ( channel == BOT_CH_KDG && number_percent() > 25 )
            return FALSE;
        if ( bot_pulse - bot->last_bot_talk < BOT_SEC(90) )
            return FALSE;
        if ( intent == INTENT_NONE || intent == INTENT_YES || intent == INTENT_NO || intent == INTENT_THANKS )
            return FALSE;
        if ( number_percent() > 40 )
            return FALSE;
    }
    bot->last_bot_talk = bot_pulse;
    return TRUE;
}

/* grup isteği: aynı odadaysa takibe başla, uzaktaysa buluşma teklif et; şablonu döndürür */
static const char *react_group( BOT_DATA *bot, CHAR_DATA *speaker, bool ic, int *reply_ch )
{
    CHAR_DATA *ch = bot->ch;
    bool wants = bot_wants_group_with( bot, speaker );

    if ( IS_BOT(speaker) && wants && speaker->in_room == ch->in_room
      && ch->master == NULL && speaker->master == NULL && number_percent() < 80 )
    {
        *reply_ch = BOT_CH_SAY;
        bot_start_follow( bot, speaker );
        return BOT_PICK(ic_group_accept);
    }
    if ( bot_is_human( speaker ) && wants )
    {
        if ( speaker->in_room == ch->in_room )
        {
            *reply_ch = BOT_CH_SAY;
            bot_start_follow( bot, speaker );
            return BOT_PICK(ic_group_accept);
        }
        bot_offer_meeting( bot, speaker );
        return PICK2( ic, ic_group_far, ooc_group_far );
    }
    return PICK2( ic, ic_group_no, ooc_group_no );
}

/* niyete göre cevap şablonu (NULL: sus); reply_ch gerekirse değişir */
static const char *react_template( BOT_DATA *bot, CHAR_DATA *speaker, int channel, int intent, int *reply_ch )
{
    bool ic = channel_ic( *reply_ch );
    bool my_leader = bot->state == BOT_ST_FOLLOW && bot_char_by_id( bot->leader_id ) == speaker;

    switch ( intent )
    {
    case INTENT_GREET:   return PICK2( ic, ic_greet, ooc_greet );
    case INTENT_HOW:     return PICK2( ic, ic_how, ooc_how );
    case INTENT_LEVEL:   return PICK2( ic, ic_level, ooc_level );
    case INTENT_WHERE:   return PICK2( ic, ic_where, ooc_where );
    case INTENT_CLASS:   return PICK2( ic, ic_class, ooc_class );
    case INTENT_HELP:    return PICK2( ic, ic_help, ooc_help );
    case INTENT_THANKS:  return PICK2( ic, ic_thanks, ooc_thanks );
    case INTENT_YES:     return PICK2( ic, ic_yes, ooc_yes );
    case INTENT_NO:      return PICK2( ic, ic_no, ooc_no );
    case INTENT_INSULT:  return PICK2( ic, ic_insult, ooc_insult );
    case INTENT_QUESTION:
        return channel == BOT_CH_KDG ? BOT_PICK(ooc_kdg_reply) : PICK2( ic, ic_question, ooc_question );
    case INTENT_BYE:
        if ( my_leader )
            bot_stop_follow( bot, FALSE );
        return PICK2( ic, ic_bye, ooc_bye );
    case INTENT_GROUP:
        return react_group( bot, speaker, ic, reply_ch );
    case INTENT_STOP:
        if ( my_leader )
        {
            bot_stop_follow( bot, FALSE );
            return PICK2( ic, ic_stop, ooc_stop );
        }
        return number_percent() < 30 ? PICK2( ic, ic_default, ooc_default ) : NULL;
    default:
        return ( channel == BOT_CH_TELL || number_percent() < 45 ) ? PICK2( ic, ic_default, ooc_default ) : NULL;
    }
}

void bot_chat_react( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text )
{
    CHAR_DATA *ch = bot->ch;
    int intent;
    int reply_ch = channel;
    const char *tmpl;
    bool human = bot_is_human( speaker );

    if ( ch == NULL || IS_NPC(speaker) )
        return;
    if ( channel == BOT_CH_SOCIAL || channel == BOT_CH_EMOTE )
    {
        /* sosyale sosyalle karşılık */
        if ( speaker->in_room == ch->in_room && number_percent() < ( human ? 60 : 15 ) )
            say_later( bot, BOT_CH_SOCIAL, speaker, BOT_PICK(ic_social), number_range( 6, 20 ) );
        return;
    }

    intent = detect_intent( text );
    if ( IS_BOT(speaker) ? !react_bot_allowed( bot, channel, intent ) : !human )
        return;

    /* uzaktan haykırana konu dışı kanaldan (kd) dönebilir; kdg'ye çoğunlukla kd ile cevap */
    if ( channel == BOT_CH_YELL && speaker->in_room != ch->in_room && number_percent() < 50 )
        reply_ch = BOT_CH_TELL;
    if ( channel == BOT_CH_KDG )
    {
        if ( intent == INTENT_NONE && number_percent() < 70 )
            return;
        if ( number_percent() < 60 )
            reply_ch = BOT_CH_TELL;
    }

    snprintf( bot->last_speaker, BOT_NAME_LEN, "%s", speaker->name );

    if ( intent == INTENT_CABAL && ( channel == BOT_CH_TELL || channel == BOT_CH_SAY )
      && bot_leader_handle( bot, speaker, channel ) )
        return;
    if ( channel == BOT_CH_CABAL && intent == INTENT_HELP )
    {
        bot_war_help( bot, speaker, text );
        return;
    }
    if ( human && IS_IMMORTAL(speaker) && number_percent() < 60 )
    {
        say_later( bot, reply_ch, speaker, PICK2( channel_ic( reply_ch ), ic_god, ooc_god ), reply_delay( text ) );
        return;
    }
    if ( ( tmpl = react_template( bot, speaker, channel, intent, &reply_ch ) ) != NULL )
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
            say_later( bot, BOT_CH_YELL, NULL, BOT_PICK(ic_level_up), number_range( 8, 30 ) );
        else if ( any_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_level_up), number_range( 20, 80 ) );
        else if ( number_percent() < 40 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_level_up), number_range( 8, 30 ) );
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
            say_later( bot, BOT_CH_YELL, NULL, BOT_PICK(ic_death), number_range( 12, 40 ) );
        else if ( any_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_death), number_range( 30, 120 ) );
        else if ( number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_death), number_range( 12, 40 ) );
        break;
    case BOT_EV_KILL:
        if ( room_h != NULL && number_percent() < 12 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_kill), number_range( 6, 20 ) );
        break;
    case BOT_EV_FLEE:
        if ( room_h != NULL && number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_flee), number_range( 8, 20 ) );
        else if ( area_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_YELL, NULL, BOT_PICK(ic_flee), number_range( 8, 20 ) );
        break;
    case BOT_EV_QUEST_GET:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_quest_get), number_range( 8, 20 ) );
        break;
    case BOT_EV_QUEST_DONE:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_quest_done), number_range( 8, 20 ) );
        else if ( any_h != NULL && number_percent() < 15 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_quest_done), number_range( 20, 60 ) );
        break;
    case BOT_EV_LOOT:
        if ( current_time - boot_time < 600 || current_time - bot->login_time < 180 )
            break;                                    /* açılış/giriş teçhizatı için övünme */
        if ( room_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_loot), number_range( 8, 20 ) );
        break;
    case BOT_EV_LOGIN:
        if ( current_time - boot_time < 600 )
            break;                                    /* açılışta toplu girişlerde susulur */
        if ( any_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_login), number_range( 120, 360 ) );
        break;
    case BOT_EV_LOGOUT:
        if ( room_h != NULL && number_percent() < 70 )
            say_later( bot, BOT_CH_SAY, NULL, BOT_PICK(ic_logout), 2 );
        else if ( any_h != NULL && number_percent() < 30 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_logout), 2 );
        break;
    case BOT_EV_CABAL:
        say_later( bot, BOT_CH_YELL, NULL, BOT_PICK(ic_cabal), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_KILL:
        say_later( bot, BOT_CH_YELL, other, BOT_PICK(ic_pk_kill), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_TAUNT:
        if ( other != NULL )
        {
            char out[MAX_INPUT_LENGTH];
            bot_fill_ch( bot, BOT_PICK(ic_pk_taunt), other, out, sizeof(out), TRUE );
            bot_talk( bot, BOT_CH_YELL, NULL, out );
        }
        break;
    case BOT_EV_HUMAN_LOGIN:
        if ( other != NULL )
            say_later( bot, BOT_CH_TELL, other, BOT_PICK(ooc_human_login), number_range( 60, 320 ) );
        break;
    case BOT_EV_HUMAN_ROOM:
        if ( other != NULL )
        {
            say_later( bot, BOT_CH_SAY, other, BOT_PICK(ic_room_greet), number_range( 6, 16 ) );
            if ( number_percent() < 40 )
                say_later( bot, BOT_CH_SOCIAL, other, "selamla", number_range( 20, 40 ) );
        }
        break;
    }
}

/* ---------------------------------------------------------------------
 * boş zaman gevezeliği
 * ------------------------------------------------------------------ */
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
    case BOT_K_GEVEZE:  base = BOT_MIN(3);  break;
    case BOT_K_SAKIN:   base = BOT_MIN(14); break;
    case BOT_K_GIZEMLI: base = BOT_MIN(10); break;
    default:            base = BOT_MIN(6);  break;
    }
    bot->next_chat = bot_pulse + base + number_range( 0, base );

    room_h = human_in_room( ch );
    area_h = human_in_area( ch );
    any_h  = bot_random_human();

    /* odada bir insan varsa selam ver (bir kez) */
    if ( room_h != NULL && ( bot_pulse - bot->greeted_pulse > BOT_MIN(10) || str_cmp( bot->last_human, room_h->name ) ) )
    {
        bot->greeted_pulse = bot_pulse;
        snprintf( bot->last_human, BOT_NAME_LEN, "%s", room_h->name );
        if ( number_percent() < 65 )
        {
            bot_chat_event( bot, BOT_EV_HUMAN_ROOM, room_h );
            return;
        }
    }

    if ( number_percent() < 6 && bot_human_immortal() != NULL )
    {
        char out[MAX_INPUT_LENGTH];
        bot_fill_ch( bot, BOT_PICK(ic_pray), NULL, out, sizeof(out), TRUE );
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
            bot_talk( bot, BOT_CH_SOCIAL, room_h, BOT_PICK(ic_social) );
            return;
        }
        if ( bot->kisilik == BOT_K_ACEMI && roll < 60 )
            tmpl = BOT_PICK(ic_acemi);
        else if ( bot->kisilik == BOT_K_GIZEMLI && roll < 60 )
            tmpl = BOT_PICK(ic_gizemli);
        else
            tmpl = BOT_PICK(ic_idle_say);
        say_later( bot, BOT_CH_SAY, room_h, tmpl, number_range( 2, 12 ) );
        return;
    }
    if ( area_h != NULL && number_percent() < 55 )
    {
        say_later( bot, BOT_CH_YELL, NULL, BOT_PICK(ic_idle_yell), number_range( 2, 12 ) );
        return;
    }
    if ( any_h != NULL && bot_pulse >= bot->next_kd )
    {
        bot->next_kd = bot_pulse + BOT_MIN( bot->kisilik == BOT_K_GEVEZE ? 12 : 25 ) + number_range( 0, BOT_MIN(10) );
        if ( number_percent() < 55 )
            say_later( bot, BOT_CH_KDG, NULL, BOT_PICK(ooc_kdg_idle), number_range( 2, 12 ) );
        else if ( !IS_IMMORTAL(any_h) )
            say_later( bot, BOT_CH_TELL, any_h, BOT_PICK(ooc_kd_idle), number_range( 2, 12 ) );
        return;
    }
    if ( bot_in_group( ch ) && number_percent() < 40 )
    {
        say_later( bot, BOT_CH_GTELL, NULL, BOT_PICK(ooc_gtell), number_range( 2, 12 ) );
        return;
    }
    if ( ch->cabal != CABAL_NONE && number_percent() < 25 )
    {
        say_later( bot, BOT_CH_CABAL, NULL, BOT_PICK(ooc_cabal), number_range( 2, 12 ) );
        return;
    }
    if ( number_percent() < 30 )
    {
        const char *tmpl = bot->kisilik == BOT_K_GIZEMLI ? BOT_PICK(ic_gizemli)
                         : bot->kisilik == BOT_K_ACEMI ? BOT_PICK(ic_acemi)
                         : BOT_PICK(ic_idle_say);
        say_later( bot, BOT_CH_SAY, NULL, tmpl, number_range( 2, 12 ) );
    }
    else if ( number_percent() < 30 )
        bot_talk( bot, BOT_CH_SOCIAL, NULL, BOT_PICK(ic_social) );
}
