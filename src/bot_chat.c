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

static void lower_ascii( char *s )
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
    char tmp[MAX_STRING_LENGTH];

    if ( !bot->turkce )
        transliterate( in, tmp, sizeof(tmp) );
    else
        snprintf( tmp, sizeof(tmp), "%s", in );
    if ( bot->kucuk_harf && !ic )
        lower_ascii( tmp );

    if ( ic )
    {
        if ( bot->kisilik == BOT_K_AGRESIF && number_percent() < 30 )
        {
            size_t l = strlen( tmp );
            if ( l > 0 && tmp[l - 1] == '.' )
                tmp[l - 1] = '\0';
            strncat( tmp, ic_agresif_tail[number_range( 0, PN(ic_agresif_tail) - 1 )], sizeof(tmp) - strlen( tmp ) - 1 );
        }
    }
    else if ( bot->kisilik == BOT_K_AGRESIF && number_percent() < 35 )
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
    bot_fill_ch( bot, tmpl, other, out, size, FALSE );
}

void bot_fill_ch( BOT_DATA *bot, const char *tmpl, CHAR_DATA *other, char *out, size_t size, bool ic )
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
                else if ( klen == 2 && !strncmp( p + 1, "yp", 2 ) )    { snprintf( num, sizeof(num), "%d", ch != NULL && ch->max_hit > 0 ? ch->hit * 100 / ch->max_hit : 100 ); val = num; }
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
        if ( bot_pulse - last_kdg_pulse < 4 * 45 )
            return;
        last_kdg_pulse = bot_pulse;
        bot_cmd( bot, "kdg %s", text );
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

    bot_fill_ch( bot, tmpl, to, out, sizeof(out), channel_ic( channel ) );
    bot_queue_reply( bot, to != NULL ? to->name : "", channel, delay, out );
    /* odada birine cevap verecekse konuşmak için durur (yürüyüp gitmez) */
    if ( ( channel == BOT_CH_SAY || channel == BOT_CH_SOCIAL || channel == BOT_CH_EMOTE )
      && to != NULL && bot->ch != NULL && to->in_room == bot->ch->in_room && bot_is_human( to ) )
        bot->hold_until = UMAX( bot->hold_until, bot_pulse + delay * 4 + 12 );
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
#define INTENT_CABAL    15

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
    if ( has_word( padded, "kabal" ) && ( has_word( padded, "katil" ) || has_word( padded, "uye" )
      || has_word( padded, "induct" ) || has_word( padded, "al beni" ) || has_word( padded, "alir mi" )
      || has_word( padded, "girmek" ) || has_word( padded, "kabul" ) ) )
        return INTENT_CABAL;
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

/* rol içi / konu dışı havuz seçimi */
#define PICK2(ic, icpool, oocpool) ( (ic) ? pick( icpool, PN(icpool) ) : pick( oocpool, PN(oocpool) ) )

void bot_chat_react( BOT_DATA *bot, CHAR_DATA *speaker, int channel, const char *text )
{
    CHAR_DATA *ch = bot->ch;
    int intent;
    int reply_ch = channel;
    const char *tmpl = NULL;
    bool speaker_bot = IS_BOT(speaker);
    bool human = bot_is_human( speaker );
    bool ic;

    if ( ch == NULL || IS_NPC(speaker) )
        return;
    if ( channel == BOT_CH_SOCIAL || channel == BOT_CH_EMOTE )
    {
        /* sosyale sosyalle karşılık */
        if ( speaker->in_room == ch->in_room && number_percent() < ( human ? 60 : 15 ) )
            say_later( bot, BOT_CH_SOCIAL, speaker, pick( ic_social, PN(ic_social) ), number_range( 6, 20 ) );
        return;
    }

    intent = detect_intent( text );

    /* botlar birbirleriyle sonsuza dek konuşmasın */
    if ( speaker_bot )
    {
        if ( intent != INTENT_GROUP && intent != INTENT_CABAL && !( channel == BOT_CH_CABAL && intent == INTENT_HELP ) )
        {
            if ( channel == BOT_CH_KDG && number_percent() > 25 )
                return;
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
    ic = channel_ic( reply_ch );

    snprintf( bot->last_speaker, BOT_NAME_LEN, "%s", speaker->name );

    if ( intent == INTENT_CABAL && ( channel == BOT_CH_TELL || channel == BOT_CH_SAY ) )
    {
        if ( bot_leader_handle( bot, speaker, channel ) )
            return;
    }
    if ( channel == BOT_CH_CABAL && intent == INTENT_HELP )
    {
        bot_war_help( bot, speaker, text );
        return;
    }

    if ( human && IS_IMMORTAL(speaker) && number_percent() < 60 )
    {
        say_later( bot, reply_ch, speaker, PICK2( ic, ic_god, ooc_god ), reply_delay( text ) );
        return;
    }

    switch ( intent )
    {
    case INTENT_GREET:   tmpl = PICK2( ic, ic_greet, ooc_greet );       break;
    case INTENT_HOW:     tmpl = PICK2( ic, ic_how, ooc_how );           break;
    case INTENT_LEVEL:   tmpl = PICK2( ic, ic_level, ooc_level );       break;
    case INTENT_WHERE:   tmpl = PICK2( ic, ic_where, ooc_where );       break;
    case INTENT_CLASS:   tmpl = PICK2( ic, ic_class, ooc_class );       break;
    case INTENT_HELP:    tmpl = PICK2( ic, ic_help, ooc_help );         break;
    case INTENT_THANKS:  tmpl = PICK2( ic, ic_thanks, ooc_thanks );     break;
    case INTENT_BYE:
        tmpl = PICK2( ic, ic_bye, ooc_bye );
        if ( bot->state == BOT_ST_FOLLOW && bot_char_by_id( bot->leader_id ) == speaker )
            bot_stop_follow( bot, FALSE );
        break;
    case INTENT_YES:     tmpl = PICK2( ic, ic_yes, ooc_yes );           break;
    case INTENT_NO:      tmpl = PICK2( ic, ic_no, ooc_no );             break;
    case INTENT_INSULT:  tmpl = PICK2( ic, ic_insult, ooc_insult );     break;
    case INTENT_QUESTION: tmpl = channel == BOT_CH_KDG ? pick( ooc_kdg_reply, PN(ooc_kdg_reply) )
                               : PICK2( ic, ic_question, ooc_question ); break;
    case INTENT_GROUP:
        if ( speaker_bot && bot_wants_group_with( bot, speaker ) && speaker->in_room == ch->in_room
          && ch->master == NULL && speaker->master == NULL && number_percent() < 80 )
        {
            reply_ch = BOT_CH_SAY;
            tmpl = pick( ic_group_accept, PN(ic_group_accept) );
            bot_start_follow( bot, speaker );
        }
        else if ( human && bot_wants_group_with( bot, speaker ) )
        {
            if ( speaker->in_room == ch->in_room )
            {
                reply_ch = BOT_CH_SAY;
                tmpl = pick( ic_group_accept, PN(ic_group_accept) );
                bot_start_follow( bot, speaker );
            }
            else
            {
                tmpl = PICK2( ic, ic_group_far, ooc_group_far );
                bot_offer_meeting( bot, speaker );
            }
        }
        else
            tmpl = PICK2( ic, ic_group_no, ooc_group_no );
        break;
    case INTENT_STOP:
        if ( bot->state == BOT_ST_FOLLOW && bot_char_by_id( bot->leader_id ) == speaker )
        {
            tmpl = PICK2( ic, ic_stop, ooc_stop );
            bot_stop_follow( bot, FALSE );
        }
        else if ( number_percent() < 30 )
            tmpl = PICK2( ic, ic_default, ooc_default );
        break;
    default:
        if ( channel == BOT_CH_TELL || number_percent() < 45 )
            tmpl = PICK2( ic, ic_default, ooc_default );
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
            say_later( bot, BOT_CH_YELL, NULL, pick( ic_level_up, PN(ic_level_up) ), number_range( 8, 30 ) );
        else if ( any_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_level_up, PN(ooc_level_up) ), number_range( 20, 80 ) );
        else if ( number_percent() < 40 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_level_up, PN(ic_level_up) ), number_range( 8, 30 ) );
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
            say_later( bot, BOT_CH_YELL, NULL, pick( ic_death, PN(ic_death) ), number_range( 12, 40 ) );
        else if ( any_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_death, PN(ooc_death) ), number_range( 30, 120 ) );
        else if ( number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_death, PN(ic_death) ), number_range( 12, 40 ) );
        break;
    case BOT_EV_KILL:
        if ( room_h != NULL && number_percent() < 12 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_kill, PN(ic_kill) ), number_range( 6, 20 ) );
        break;
    case BOT_EV_FLEE:
        if ( room_h != NULL && number_percent() < 50 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_flee, PN(ic_flee) ), number_range( 8, 20 ) );
        else if ( area_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_YELL, NULL, pick( ic_flee, PN(ic_flee) ), number_range( 8, 20 ) );
        break;
    case BOT_EV_QUEST_GET:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_quest_get, PN(ic_quest_get) ), number_range( 8, 20 ) );
        break;
    case BOT_EV_QUEST_DONE:
        if ( room_h != NULL && number_percent() < 60 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_quest_done, PN(ic_quest_done) ), number_range( 8, 20 ) );
        else if ( any_h != NULL && number_percent() < 15 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_quest_done, PN(ooc_quest_done) ), number_range( 20, 60 ) );
        break;
    case BOT_EV_LOOT:
        if ( current_time - boot_time < 600 || current_time - bot->login_time < 180 )
            break;                                    /* açılış/giriş teçhizatı için övünme */
        if ( room_h != NULL && number_percent() < 35 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_loot, PN(ic_loot) ), number_range( 8, 20 ) );
        break;
    case BOT_EV_LOGIN:
        if ( current_time - boot_time < 600 )
            break;                                    /* açılışta toplu girişlerde susulur */
        if ( any_h != NULL && number_percent() < 25 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_login, PN(ooc_login) ), number_range( 120, 360 ) );
        break;
    case BOT_EV_LOGOUT:
        if ( room_h != NULL && number_percent() < 70 )
            say_later( bot, BOT_CH_SAY, NULL, pick( ic_logout, PN(ic_logout) ), 2 );
        else if ( any_h != NULL && number_percent() < 30 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_logout, PN(ooc_logout) ), 2 );
        break;
    case BOT_EV_CABAL:
        say_later( bot, BOT_CH_YELL, NULL, pick( ic_cabal, PN(ic_cabal) ), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_KILL:
        say_later( bot, BOT_CH_YELL, other, pick( ic_pk_kill, PN(ic_pk_kill) ), number_range( 8, 24 ) );
        break;
    case BOT_EV_PK_TAUNT:
        if ( other != NULL )
        {
            char out[MAX_STRING_LENGTH];
            bot_fill_ch( bot, pick( ic_pk_taunt, PN(ic_pk_taunt) ), other, out, sizeof(out), TRUE );
            bot_talk( bot, BOT_CH_YELL, NULL, out );
        }
        break;
    case BOT_EV_HUMAN_LOGIN:
        if ( other != NULL )
            say_later( bot, BOT_CH_TELL, other, pick( ooc_human_login, PN(ooc_human_login) ), number_range( 60, 320 ) );
        break;
    case BOT_EV_HUMAN_ROOM:
        if ( other != NULL )
        {
            say_later( bot, BOT_CH_SAY, other, pick( ic_room_greet, PN(ic_room_greet) ), number_range( 6, 16 ) );
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
        bot_fill_ch( bot, pick( ic_pray, PN(ic_pray) ), NULL, out, sizeof(out), TRUE );
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
            bot_talk( bot, BOT_CH_SOCIAL, room_h, pick( ic_social, PN(ic_social) ) );
            return;
        }
        if ( bot->kisilik == BOT_K_ACEMI && roll < 60 )
            tmpl = pick( ic_acemi, PN(ic_acemi) );
        else if ( bot->kisilik == BOT_K_GIZEMLI && roll < 60 )
            tmpl = pick( ic_gizemli, PN(ic_gizemli) );
        else
            tmpl = pick( ic_idle_say, PN(ic_idle_say) );
        say_later( bot, BOT_CH_SAY, room_h, tmpl, number_range( 2, 12 ) );
        return;
    }
    if ( area_h != NULL && number_percent() < 55 )
    {
        say_later( bot, BOT_CH_YELL, NULL, pick( ic_idle_yell, PN(ic_idle_yell) ), number_range( 2, 12 ) );
        return;
    }
    if ( any_h != NULL && bot_pulse >= bot->next_kd )
    {
        bot->next_kd = bot_pulse + 4 * 60 * ( bot->kisilik == BOT_K_GEVEZE ? 12 : 25 ) + number_range( 0, 4 * 60 * 10 );
        if ( number_percent() < 55 )
            say_later( bot, BOT_CH_KDG, NULL, pick( ooc_kdg_idle, PN(ooc_kdg_idle) ), number_range( 2, 12 ) );
        else if ( !IS_IMMORTAL(any_h) )
            say_later( bot, BOT_CH_TELL, any_h, pick( ooc_kd_idle, PN(ooc_kd_idle) ), number_range( 2, 12 ) );
        return;
    }
    if ( bot_in_group( ch ) && number_percent() < 40 )
    {
        say_later( bot, BOT_CH_GTELL, NULL, pick( ooc_gtell, PN(ooc_gtell) ), number_range( 2, 12 ) );
        return;
    }
    if ( ch->cabal != CABAL_NONE && number_percent() < 25 )
    {
        say_later( bot, BOT_CH_CABAL, NULL, pick( ooc_cabal, PN(ooc_cabal) ), number_range( 2, 12 ) );
        return;
    }
    if ( number_percent() < 30 )
    {
        const char *tmpl = bot->kisilik == BOT_K_GIZEMLI ? pick( ic_gizemli, PN(ic_gizemli) )
                         : bot->kisilik == BOT_K_ACEMI ? pick( ic_acemi, PN(ic_acemi) )
                         : pick( ic_idle_say, PN(ic_idle_say) );
        say_later( bot, BOT_CH_SAY, NULL, tmpl, number_range( 2, 12 ) );
    }
    else if ( number_percent() < 30 )
        bot_talk( bot, BOT_CH_SOCIAL, NULL, pick( ic_social, PN(ic_social) ) );
}
