#include "stdafx.h"
#include "CubeManager.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "locale_service.h"
#include "char.h"
#include "affect.h"
#include "utils.h"
#include "item_manager.h"
#include "item.h"
#include "config.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
# include "AchievementSystem.h"
#endif

#define contain(x) if (line.find(x) != std::string::npos)
#define econtain(x) else if (line.find(x) != std::string::npos)

void split(const std::string& s, char delim, std::vector<std::string>* elems)
{
	(*elems).clear();
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		(*elems).emplace_back(item);
	}
}

CCubeManager::CCubeManager()
{
	s_cube_proto = {};
}


bool CCubeManager::FN_check_cube_data(CUBE_DATA* cube_data)
{
	size_t i = 0;

	size_t dwEndIndex = 0;

	dwEndIndex = cube_data->npc_vnum.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->npc_vnum[i] == 0)
			return false;
	}

	dwEndIndex = cube_data->item.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->item[i].vnum == 0)
			return false;

		if (cube_data->item[i].count == 0)
			return false;
	}

	dwEndIndex = cube_data->reward.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->reward[i].vnum == 0)
			return false;

		if (cube_data->reward[i].count == 0)
			return false;
	}

	return true;
}

bool CCubeManager::Cube_init()
{
	char file_name[256 + 1];
	snprintf(file_name, sizeof(file_name), "%s/cube.txt", LocaleService_GetBasePath().c_str());
	sys_log(0, "CubeRenewal_Init %s", file_name);

	std::ifstream t(file_name);
	std::stringstream buffer;
	buffer << t.rdbuf();

	std::string line;

	CUBE_DATA* cube_data = nullptr;

	CUBE_VALUE cube_value = { 0, 0 };

	std::vector<std::string> vecOut = {};
	while (std::getline(buffer, line, '\n'))
	{
		contain("section")
		{
			cube_data = new CUBE_DATA;
			cube_data->gold = 0;
		}
		econtain("npc")
		{
			split(line, '\t', &vecOut);
			cube_data->npc_vnum.emplace_back(std::stoi(vecOut[1]));
		}
		econtain("item")
		{
			split(line, '\t', &vecOut);
			cube_value.vnum = std::stoi(vecOut[1]);
			cube_value.count = std::stoi(vecOut[2]);

			cube_data->item.emplace_back(cube_value);
		}
		econtain("reward")
		{
			split(line, '\t', &vecOut);
			cube_value.vnum = std::stoi(vecOut[1]);
			cube_value.count = std::stoi(vecOut[2]);

			cube_data->reward.emplace_back(cube_value);
		}
		econtain("percent")
		{
			split(line, '\t', &vecOut);
			cube_data->percent = std::stoi(vecOut[1]);
		}
		econtain("gold")
		{
			split(line, '\t', &vecOut);
			cube_data->gold = std::stoi(vecOut[1]);
		}
		econtain("category")
		{
			split(line, '\t', &vecOut);
			cube_data->category = vecOut[1];
		}
		econtain("not_remove")
		{
			split(line, '\t', &vecOut);
			cube_data->not_remove = std::stoi(vecOut[1]);
		}
		econtain("set_value")
		{
			split(line, '\t', &vecOut);
			cube_data->set_value = std::stoi(vecOut[1]);
		}
		econtain("gem")
		{
			split(line, '\t', &vecOut);
			cube_data->gem_point = std::stoi(vecOut[1]);
		}
		econtain("allow_copy")
		{
			split(line, '\t', &vecOut);
			cube_data->allow_copy = std::stoi(vecOut[1]) == 1;
		}
		econtain("end")
		{
			if (!FN_check_cube_data(cube_data))
			{
				delete cube_data;
				continue;
			}

			s_cube_proto.emplace_back(cube_data);
		}
	}

	return true;
}

void CCubeManager::Cube_close(LPCHARACTER ch)
{
	ch->SetCubeNpc(nullptr);
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	ch->SetOpenedWindow(W_CUBE, false);
#endif

	TEMP_BUFFER buf;
	TPacketGCCubeRenewal pack;
	TSubPacketGCCubeRenewalOpenClose sub{};
	pack.wSize = sizeof(TPacketGCCubeRenewal) + sizeof(TSubPacketGCCubeRenewalOpenClose);
	pack.bSubHeader = CUBE_RENEWAL_CLOSE;
	sub.open_state = false;

	LPDESC desc = ch->GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", ch->GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCCubeRenewal));
	buf.write(&sub, sizeof(TSubPacketGCCubeRenewalOpenClose));
	desc->Packet(buf.read_peek(), buf.size());
}

static bool IsStackableItem(uint32_t dwVnum)
{
	auto info = ITEM_MANAGER::Instance().GetTable(dwVnum);
	if (!info)
		return false;
	return (info->dwFlags & ITEM_FLAG_STACKABLE) && !IS_SET(info->dwAntiFlags, ITEM_ANTIFLAG_STACK);
}

void CCubeManager::RefineCube(LPCHARACTER ch, int vnum, int multiplier, int indexImprove, std::vector<int> listOfReqItems)
{
	if (!ch) {
		sys_err("no ch ptr");
		return;
	}

	if (ch->GetExchange() ||
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetMyShop() || ch->GetShopOwner() ||
#endif
		ch->IsOpenSafebox()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| ch->IsOpenGuildstorage()
#endif
#ifdef ENABLE_AURA_SYSTEM
		|| ch->IsAuraRefineWindowOpen()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
#ifdef ENABLE_MAILBOX
		|| ch->GetMailBox()
#endif
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;990]");
		return;
	}

	int iPulse = thecore_pulse();
	if (iPulse - ch->GetCubeLastTime() < PASSES_PER_SEC(1))
	{
		ch->ChatPacket(7, "wait 1 seconds.");
		return;
	}
	ch->SetCubeTime();

	for (const auto& v : listOfReqItems)
	{
		ch->ChatPacket(7, "mat %d", v);
	}

	int npcVnum = ch->GetTempCubeNPC();

	for (const auto& data : GetDataVector())
	{
		for (const auto& npc : data->npc_vnum)
		{
			if (npc == npcVnum)
			{
				//data->item material vector
				int itemVnum = data->reward[0].vnum;
				int itemCount = data->reward[0].count;
				int percent = data->percent;
				int gold = data->gold;
				int gem_point = data->gem_point;
				int allow_copy = data->allow_copy;
				int SetVal = data->set_value;
				int NotRem = data->not_remove;
				bool canStack = IsStackableItem(data->reward[0].vnum);

				bool itemCheck = true;

				bool sameMaterials = true;

				struct find_id : std::function<bool(CUBE_VALUE, bool)>
				{
					uint32_t itemVnum;
					find_id(uint32_t itemVnum) :itemVnum(itemVnum) { }
					bool operator()(CUBE_VALUE const& m) const {
						return m.vnum == itemVnum;
					}
				};

				for (const auto& vv : data->item)
				{
					bool ex = std::find(listOfReqItems.begin(), listOfReqItems.end(), vv.vnum) != listOfReqItems.end();
					if (ex) {
						sameMaterials = true;
					}
					else {
						sameMaterials = false;
						break;
					}
				}

				//if (sameMaterials)
					//ch->ChatPacket(7, "same materials! (vnum :%d - %d)", itemVnum, vnum);

				if (itemVnum == vnum && sameMaterials)
				{
					if (multiplier > 1 && !canStack)
					{
						ch->ChatPacket(7, "error item cant be stacked but multiplier over 1");
						return;
					}

					for(const auto & material : data->item)
					{
						//itemCheck = ch->CountSpecifyItem(material.vnum) < material.count * multiplier;
						if (ch->CountSpecifyItem(material.vnum, -1, true) < material.count * multiplier)
						{
							itemCheck = false;
							ch->ChatPacket(7, "vnum %d material %d %d < %d", vnum, material.vnum, ch->CountSpecifyItem(material.vnum, -1, true), material.count * multiplier);
						}
					}

					if (gold != 0)
					{
						if (ch->GetGold() < gold * multiplier)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You dont have the required amount of gold"));
							return;
						}
					}

					if (gem_point != 0)
					{
						if (ch->GetGemPoint() < gem_point * multiplier)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You dont have the required amount of gem"));
							return;
						}
					}
					
					if (!itemCheck)
					{
						ch->ChatPacket(7, "itemCheck = false;");
						return;
					}

					if (indexImprove != -1)
					{
						LPITEM item = ch->GetInventoryItem(indexImprove);
						if(!item)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "Can't create :%d vnum", indexImprove);
							return;
						}
						
						if (item->GetVnum() != 79605)	//Eisensiegel
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "Eisensiegel");
							return;
						}
						
						if (item->GetCount() > 40)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "222s");
							return;
						}
						
						int additionalPercent = item->GetCount();//each one item, is +1 success chance
						int substract = 0;
						
						if (percent + additionalPercent > 100) {
							substract = 100 - percent;
						}
						
						if (percent + additionalPercent <= 100) {
							substract = additionalPercent;
						}
						
						if(substract)
							item->SetCount(item->GetCount() - substract);
						
						percent += additionalPercent;
					}

					int rChance = number(1, 100);
					bool upgrade = false;
					if (rChance <= percent)
						upgrade = true;
					
					LPITEM pCheckSpace = ITEM_MANAGER::Instance().CreateItem(itemVnum, itemCount);
					if (pCheckSpace == NULL)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cant create this item."));
						sys_err("<CubeRenewal> %d can't be created.", itemVnum);
						return;
					}
					
					int iEmptyPos = pCheckSpace->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(pCheckSpace) : ch->GetEmptyInventory(pCheckSpace);
					if (iEmptyPos < 0)
					{
						// del useless item
						M2_DESTROY_ITEM(pCheckSpace);
						
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
						return;
					}
					
					// del useless item
					M2_DESTROY_ITEM(pCheckSpace);

					LPITEM pItemCopy;
					for (const auto& Material : data->item)
					{
						if (NotRem && Material.vnum == NotRem)
							continue;
						
						if (allow_copy || SetVal)
						{
							if (data->item[0].vnum == Material.vnum)
								continue;
						}

						ch->RemoveSpecifyItem(Material.vnum, Material.count, -1, true);
					}

					if (gold != 0)
					{
						ch->PointChange(POINT_GOLD, -(gold * multiplier), false);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
						if (ch && gold)
							CAchievementSystem::Instance().OnGoldChange(ch, achievements::ETaskTypes::TYPE_SPEND_UPGRADE, gold);
#endif
					}

					if (gem_point != 0)
						ch->PointChange(POINT_GEM, -(gem_point * multiplier), false);

					if (!upgrade) {
						ch->ChatPacket(7, "upgrade failed.");
						
						if (!NotRem)
						{
							if (allow_copy || SetVal)
							{
								pItemCopy = ch->FindSpecifyItem(data->item[0].vnum, true);
								if (pItemCopy)
								{
									pItemCopy->SetCount(pItemCopy->GetCount() - 1);
								}
							}
						}
						
						return;
					}
					else
						ch->ChatPacket(7, "upgrade success");
					
					LPITEM pItemRew = SetVal ? ch->FindSpecifyItem(itemVnum, true) : ITEM_MANAGER::Instance().CreateItem(itemVnum, itemCount);
					if (!pItemRew)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cant create this item."));
						sys_err("<CubeRenewal> %d can't be created.", itemVnum);
						
						if (!NotRem)
						{
							if (allow_copy || SetVal)
							{
								pItemCopy = ch->FindSpecifyItem(data->item[0].vnum, true);
								if (pItemCopy)
								{
									pItemCopy->SetCount(pItemCopy->GetCount() - 1);
								}
							}
						}
						
						return;
					}

					if (pItemRew)
					{
						iEmptyPos = pItemRew->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(pItemRew) : ch->GetEmptyInventory(pItemRew);
						if (iEmptyPos < 0)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough space in your inventory."));
							
							if (!NotRem)
							{
								if (allow_copy || SetVal)
								{
									pItemCopy = ch->FindSpecifyItem(data->item[0].vnum, true);
									if (pItemCopy)
									{
										pItemCopy->SetCount(pItemCopy->GetCount() - 1);
									}
								}
							}
							
							return;
						}
						if (allow_copy || SetVal)
						{
							pItemCopy = ch->FindSpecifyItem(data->item[0].vnum, true);
							
							if (pItemCopy)
							{
								ITEM_MANAGER::Instance().CopyAllAttrTo(pItemCopy, pItemRew);
								
								if (!SetVal)
								{
									if (!NotRem)
										pItemCopy->SetCount(pItemCopy->GetCount() - 1);
								}
							}
							
							// for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
							// {
								// pItemCopy = ch->FindSpecifyItem(data->item[0].vnum);
								// for (const auto& material : data->item)
								// {
									// if (ch->GetInventoryItem(i) && ch->GetInventoryItem(i)->GetVnum() == material.vnum)
									// {
										// if(pItemCopy)
											// ITEM_MANAGER::Instance().CopyAllAttrTo(pItemCopy, pItemRew);
									// }
								// }
							// }
						}

						if (SetVal)
						{
							std::map<uint32_t, uint8_t> SetInfo =
							{
								{ 20475, 1, },	//	Heiliger Setschmied
								{ 20476, 2, },	//	Pr?htiger Setschmied
								{ 20477, 3, },	//	Ritterlicher Setschmied
								{ 20478, 4, },	//	F?stlicher Setschmied
								{ 20479, 5, },	//	Majest. Setschmied
							};

							if (SetInfo.find(npcVnum) != SetInfo.end())
							{
								pItemRew->SetItemSetValue(SetInfo[npcVnum]);
								ch->ChatPacket(CHAT_TYPE_INFO, "SetItem created - Type: %d", SetInfo[npcVnum]);
							}
						}

						if (!SetVal)
						{
							pItemRew->AddToCharacter(ch, TItemPos(pItemRew->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos));
#ifdef ENABLE_BATTLE_PASS_SYSTEM
							ch->UpdateExtBattlePassMissionProgress(BP_ITEM_CRAFT, pItemRew->GetCount(), pItemRew->GetVnum());
#endif
						}
					}
				}
			}
		}
	}
}