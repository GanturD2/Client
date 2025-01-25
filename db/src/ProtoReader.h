#ifndef __Item_CSV_READER_H__
#define __Item_CSV_READER_H__

#include <iostream>
#include <map>

#include "CsvReader.h"

// Read the csv file and put it in the item table.
void putItemIntoTable(); // (Table, test)

int get_Item_Type_Value(std::string inputString);
int get_Item_SubType_Value(unsigned int type_value, std::string inputString);
#ifdef ENABLE_PROTO_RENEWAL
int get_Mask_Type_Value(std::string inputString);
int get_Mask_SubType_Value(unsigned int type_value, std::string inputString);
#endif
int get_Item_AntiFlag_Value(std::string inputString);
int get_Item_Flag_Value(std::string inputString);
int get_Item_WearFlag_Value(std::string inputString);
int get_Item_Immune_Value(std::string inputString);
int get_Item_LimitType_Value(std::string inputString);
int get_Item_ApplyType_Value(std::string inputString);


// You can also read Mob Prototype.
int get_Mob_Rank_Value(std::string inputString);
int get_Mob_Type_Value(std::string inputString);
int get_Mob_BattleType_Value(std::string inputString);

int get_Mob_Size_Value(std::string inputString);
int get_Mob_AIFlag_Value(std::string inputString);
int get_Mob_AIFlagEx_Value(std::string inputString);
int get_Mob_RaceFlag_Value(std::string inputString);
int get_Mob_ImmuneFlag_Value(std::string inputString);

//
bool Set_Proto_Mob_Table(TMobTable *mobTable, const cCsvTable &csvTable, std::map<int,const char*> &nameMap);
bool Set_Proto_Item_Table(TItemTable *itemTable, const cCsvTable &csvTable,std::map<int,const char*> &nameMap);

#endif
