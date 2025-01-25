#include "stdafx.h"
#include "utils.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "wedding.h"
#ifdef ENABLE_MESSENGER_BLOCK
#include "messenger_manager.h"
#endif

#define NEED_TARGET	(1 << 0)
#define NEED_PC		(1 << 1)
#define WOMAN_ONLY	(1 << 2)
#define OTHER_SEX_ONLY	(1 << 3)
#define SELF_DISARM	(1 << 4)
#define TARGET_DISARM	(1 << 5)
#define BOTH_DISARM	(SELF_DISARM | TARGET_DISARM)

struct emotion_type_s
{
	const char *	command;
	const char *	command_to_client;
	long	flag;
	float	extra_delay;
} emotion_types[] = {
	{ "Å°½º",	"french_kiss",	NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		2.0f },
	{ "»Ç»Ç",	"kiss",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		1.5f },
	{ "µû±Í",	"slap",		NEED_PC | SELF_DISARM,				1.5f },
	{ "¹Ú¼ö",	"clap",		0,						1.0f },
	{ "¿Í",		"cheer1",	0,						1.0f },
	{ "¸¸¼¼",	"cheer2",	0,						1.0f },

	// DANCE
	{ "´í½º1",	"dance1",	0,						1.0f },
	{ "´í½º2",	"dance2",	0,						1.0f },
	{ "´í½º3",	"dance3",	0,						1.0f },
	{ "´í½º4",	"dance4",	0,						1.0f },
	{ "´í½º5",	"dance5",	0,						1.0f },
	{ "´í½º6",	"dance6",	0,						1.0f },
	// END_OF_DANCE
	{ "ÃàÇÏ",	"congratulation",	0,				1.0f	},
	{ "¿ë¼­",	"forgive",			0,				1.0f	},
	{ "È­³²",	"angry",			0,				1.0f	},
	{ "À¯È¤",	"attractive",		0,				1.0f	},
	{ "½½ÇÄ",	"sad",				0,				1.0f	},
	{ "ºê²ô",	"shy",				0,				1.0f	},
	{ "ÀÀ¿ø",	"cheerup",			0,				1.0f	},
	{ "ÁúÅõ",	"banter",			0,				1.0f	},
	{ "±â»Ý",	"joy",				0,				1.0f	},

#ifdef ENABLE_EXPRESSING_EMOTION
	{ "pushup",	"pushup",				0,				1.0f	},
	{ "dance_7",	"dance_7",				0,				1.0f	},
	{ "exercise",	"exercise",				0,				1.0f	},
	{ "doze",	"doze",				0,				1.0f	},
	{ "selfie",	"selfie",				0,				1.0f	},
#endif

	{ "\n",	"\n",		0,						0.0f },
	/*
	//{ "Å°½º",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_FRENCH_KISS,	 1.0f },
	{ "»Ç»Ç",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_KISS,		 1.0f },
	{ "²¸¾È±â",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_SHORT_HUG,	 1.0f },
	{ "Æ÷¿Ë",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_LONG_HUG,		 1.0f },
	{ "¾î±úµ¿¹«",	NEED_PC | SELF_DISARM,				MOTION_ACTION_PUT_ARMS_SHOULDER, 0.0f },
	{ "ÆÈÂ¯",		NEED_PC	| WOMAN_ONLY | SELF_DISARM,		MOTION_ACTION_FOLD_ARM,		 0.0f },
	{ "µû±Í",		NEED_PC | SELF_DISARM,				MOTION_ACTION_SLAP,		 1.5f },

	{ "ÈÖÆÄ¶÷",		0,						MOTION_ACTION_CHEER_01,		 0.0f },
	{ "¸¸¼¼",		0,						MOTION_ACTION_CHEER_02,		 0.0f },
	{ "¹Ú¼ö",		0,						MOTION_ACTION_CHEER_03,		 0.0f },

	{ "È£È£",		0,						MOTION_ACTION_LAUGH_01,		 0.0f },
	{ "Å±Å±",		0,						MOTION_ACTION_LAUGH_02,		 0.0f },
	{ "¿ìÇÏÇÏ",		0,						MOTION_ACTION_LAUGH_03,		 0.0f },

	{ "¾û¾û",		0,						MOTION_ACTION_CRY_01,		 0.0f },
	{ "ÈæÈæ",		0,						MOTION_ACTION_CRY_02,		 0.0f },

	{ "ÀÎ»ç",		0,						MOTION_ACTION_GREETING_01,	0.0f },
	{ "¹ÙÀÌ",		0,						MOTION_ACTION_GREETING_02,	0.0f },
	{ "Á¤ÁßÀÎ»ç",	0,						MOTION_ACTION_GREETING_03,	0.0f },

	{ "ºñ³­",		0,						MOTION_ACTION_INSULT_01,	0.0f },
	{ "¸ð¿å",		SELF_DISARM,					MOTION_ACTION_INSULT_02,	0.0f },
	{ "¿ìÀ¡",		0,						MOTION_ACTION_INSULT_03,	0.0f },

	{ "°¼¿ì¶×",		0,						MOTION_ACTION_ETC_01,		0.0f },
	{ "²ô´ö²ô´ö",	0,						MOTION_ACTION_ETC_02,		0.0f },
	{ "µµ¸®µµ¸®",	0,						MOTION_ACTION_ETC_03,		0.0f },
	{ "±ÜÀû±ÜÀû",	0,						MOTION_ACTION_ETC_04,		0.0f },
	{ "Æ¡",		0,						MOTION_ACTION_ETC_05,		0.0f },
	{ "»×",		0,						MOTION_ACTION_ETC_06,		0.0f },
	 */
};


std::set<std::pair<uint32_t, uint32_t> > s_emotion_set;

ACMD(do_emotion_allow)
{
	if ( ch->GetArena() )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t	val = 0; str_to_number(val, arg1);
	
#ifdef ENABLE_MESSENGER_BLOCK
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(val);

	if (!tch)
		return;

	if (MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), tch->GetName());
		return;
	}
	else if (MessengerManager::Instance().IsBlocked(tch->GetName(), ch->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), tch->GetName());
		return;
	}
#endif
	s_emotion_set.insert(std::make_pair(ch->GetVID(), val));
}

#ifdef ENABLE_NEWSTUFF
#include "config.h"
#endif

bool CHARACTER_CanEmotion(CHARACTER& rch)
{
#ifdef ENABLE_NEWSTUFF
	if (g_bDisableEmotionMask)
		return true;
#endif

	if (emotion_without_mask)	//@custom016
		return true;

	if (marriage::WeddingManager::Instance().IsWeddingMap(rch.GetMapIndex()))
		return true;

	if (rch.IsEquipUniqueItem(UNIQUE_ITEM_EMOTION_MASK))
		return true;

	if (rch.IsEquipUniqueItem(UNIQUE_ITEM_EMOTION_MASK2))
		return true;

	return false;
}

ACMD(do_emotion)
{
	// 750 = Zeit, wie lange die Emotionen in MS nach dem Angreifen/Bewegen noch deaktiviert sein sollen.
	bool bAttacking = (get_dword_time() - ch->GetLastAttackTime()) < 750;	//@fixme505
	bool bMoving = (get_dword_time() - ch->GetLastMoveTime()) < 750;	//@fixme505

	int i;
	{
		if (ch->IsRiding()) {
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1043]");
			return;
		}

		if (bAttacking || bMoving) {	//@fixme505
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Du musst still stehen, umEMotionen ausführen zu können!"));
			return;
		}
	}

	for (i = 0; *emotion_types[i].command != '\n'; ++i)
	{
		if (!strcmp(cmd_info[cmd].command, emotion_types[i].command))
			break;

		if (!strcmp(cmd_info[cmd].command, emotion_types[i].command_to_client))
			break;
	}

	if (*emotion_types[i].command == '\n')
	{
		sys_err("cannot find emotion");
		return;
	}

	if (!CHARACTER_CanEmotion(*ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;414]");
		return;
	}

	if (IS_SET(emotion_types[i].flag, WOMAN_ONLY) && SEX_MALE==GET_SEX(ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;425]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	LPCHARACTER victim = nullptr;

	if (*arg1)
		victim = ch->FindCharacterInView(arg1, IS_SET(emotion_types[i].flag, NEED_PC));

	if (IS_SET(emotion_types[i].flag, NEED_TARGET | NEED_PC))
	{
		if (!victim)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;436]");
			return;
		}
	}

	if (victim)
	{
		if (!victim->IsPC() || victim == ch)
			return;

		if (victim->IsRiding())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1044]");
			return;
		}

		long distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

		if (distance < 10)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;448]");
			return;
		}

		if (distance > 500)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;459]");
			return;
		}

		if (IS_SET(emotion_types[i].flag, OTHER_SEX_ONLY))
		{
			if (GET_SEX(ch)==GET_SEX(victim) && !emotion_same_gender)	//@custom015
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;470]");
				return;
			}
		}

		if (IS_SET(emotion_types[i].flag, NEED_PC))
		{
			if (s_emotion_set.find(std::make_pair(victim->GetVID(), ch->GetVID())) == s_emotion_set.end())
			{
				if (true == marriage::CManager::Instance().IsMarried( ch->GetPlayerID() ))
				{
					const marriage::TMarriage* marriageInfo = marriage::CManager::Instance().Get( ch->GetPlayerID() );

					const uint32_t other = marriageInfo->GetOther( ch->GetPlayerID() );

					if (0 == other || other != victim->GetPlayerID())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;481]");
						return;
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;481]");
					return;
				}
			}

			s_emotion_set.insert(std::make_pair(ch->GetVID(), victim->GetVID()));
		}
	}

	char chatbuf[256+1];
	int len = snprintf(chatbuf, sizeof(chatbuf), "%s %u %u",
			emotion_types[i].command_to_client,
			(uint32_t) ch->GetVID(), victim ? (uint32_t) victim->GetVID() : 0);

	if (len < 0 || len >= (int) sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	++len;

	TPacketGCChat pack_chat{};
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(TPacketGCChat) + len;
	pack_chat.type = CHAT_TYPE_COMMAND;
	pack_chat.id = 0;
	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(TPacketGCChat));
	buf.write(chatbuf, len);

	ch->PacketAround(buf.read_peek(), buf.size());

	if (victim)
		sys_log(1, "ACTION: %s TO %s", emotion_types[i].command, victim->GetName());
	else
		sys_log(1, "ACTION: %s", emotion_types[i].command);
}
