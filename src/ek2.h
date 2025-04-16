/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar açık kaynak Türkçe Mud projesidir.                        *
 * Oyun geliştirmesi Jai ve Maru tarafından yönetilmektedir.               *
 * Unutulmaması gerekenler: Nir, Kame, Randalin, Nyah, Sint                          *
 *                                                                         *
 * Github  : https://github.com/yelbuke/UzakDiyarlar                       *
 * Web     : http://www.uzakdiyarlar.net                                   *
 * Discord : https://discord.gg/kXyZzv                                     *
 *                                                                         *
 ***************************************************************************/
 
struct sonek_type
{
	char *ek[5];
};

extern	const	struct	sonek_type	sonek_table[];


bool	bu_harf_unlu_mu		args((char harf));
bool	son_harf_unlu_mu	args((char *sozcuk));
char	son_unlu_harf_hangisi	args((char *sozcuk));
char	*ek_olustur		args((char *sozcuk, char tip));
