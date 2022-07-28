/***************************************************************************
 *                                                                         *
 * Uzak Diyarlar a��k kaynak T�rk�e Mud projesidir.                        *
 * Oyun geli�tirmesi Jai ve Maru taraf�ndan y�netilmektedir.               *
 * Unutulmamas� gerekenler: Nir, Kame, Nyah, Sint                          *
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
