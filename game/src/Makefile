CC = g++10

GAME_VERSION := $(shell cat ../../__REVISION__)

GccMajorVersion := $(shell expr `$(CC) -dumpversion | cut -f1 -d.`)
GccMinorVersion := $(shell expr `$(CC) -dumpversion | cut -f2 -d.`)
GccMinorEGT8 := $(shell expr $(GccMinorVersion) \>= 8)

INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

## LIST OF CONSTANTS BEGIN
ENABLE_LUA_5_VERSION = 0
ENABLE_GCC_AUTODEPEND = 1
ENABLE_STATIC = 0
## LIST OF CONSTANTS END

# Depend Path File
ifneq ($(ENABLE_GCC_AUTODEPEND), 1)
DEPFILE = Depend
endif

# Standard Libraries
LIBS = -lm -lmd

# Project Flags
CFLAGS  = -m32 -g -w -O2 -pipe -fexceptions -fno-strict-aliasing -pthread -D_THREAD_SAFE -DNDEBUG
CFLAGS += -Wno-deprecated-declarations -Wno-nonnull-compare -Wno-deprecated-declarations -Wno-array-bounds -Wno-address
CFLAGS += -Wno-int-in-bool-context -Wno-format-truncation -Wno-stringop-truncation
CXXFLAGS = -std=c++20 -Wl,-rpath=/usr/local/lib/gcc10

ifeq ($(ENABLE_STATIC), 1)
CFLAGS += -static
endif

ifeq ($(GccMinorEGT8), 1)
CFLAGS += -Wno-unused-local-typedefs
endif

# FreeBSD stack protector
CFLAGS += -fstack-protector-all

# Version defines
CFLAGS += -D__USER__=\"$(USER)\" -D__HOSTNAME__=\"$(HOSTNAME)\" -D__PWD__=\"$(PWD)\" -D__GAME_VERSION__=\"$(GAME_VERSION)\"

# Boost
INCDIR += -I../../../Extern/include/boost

# DevIL
INCDIR += -I../../../Extern/include/IL
LIBS += ../../../Extern/lib/libIL.a\
		../../../Extern/lib/libjasper.a\
		../../../Extern/lib/libpng.a\
		../../../Extern/lib/libtiff.a\
		../../../Extern/lib/libjbig.a\
		../../../Extern/lib/libmng.a\
		/usr/lib/liblzma.a\
		../../../Extern/lib/liblcms.a\
		../../../Extern/lib/libjpeg.a

# MySQL
INCDIR += -I/usr/local/include/mysql
LIBS += /usr/local/lib/mysql/libmysqlclient.a /usr/lib/libz.a

# CryptoPP
LIBS += ../../../Extern/lib/libcryptopp.a

# OpenSSL
INCDIR += -I/usr/include
LIBS += -lssl -lcrypto
# LIBS += /usr/lib/libssl.a

# Lua
CFLAGS += -DENABLE_LUA_5_VERSION=ENABLE_LUA_5_VERSION
ifeq ($(ENABLE_LUA_5_VERSION), 2)
INCDIR += -I../../liblua/5.2/install/include
LIBDIR += -L../../liblua/5.2/install/lib
LIBS += ../../liblua/5.2/install/lib/liblua.a
else
INCDIR += -I../../liblua/5.0/include
LIBDIR += -L../../liblua/5.0/lib
LIBS += ../../liblua/5.0/lib/liblua.a ../../liblua/5.0/lib/liblualib.a
endif

# Project Libraries
INCDIR += -I../../../Extern/include
INCDIR += -I../../libachievement
INCDIR += -I/usr/local/include
LIBDIR += -L/usr/local/lib

LIBDIR += -L../../libthecore/lib -L../../libpoly -L../../libsql -L../../libgame/lib -L../../libachievement/lib
LIBS += -lthecore -lpoly -lsql -lgame -lachievement

LIBDIR += -L../../../Extern/lib
LIBS += -lprotect

# PROJECT_SRC_FILES BEGIN
MAINCPP = main.cpp

CFILE	= minilzo.c

CPPFILE = affect.cpp\
		  ani.cpp\
		  arena.cpp\
		  banword.cpp\
		  battle.cpp\
		  BattleArena.cpp\
		  blend_item.cpp\
		  block_country.cpp\
		  BlueDragon.cpp\
		  BlueDragon_Binder.cpp\
		  buff_on_attributes.cpp\
		  buffer_manager.cpp\
		  building.cpp\
		  castle.cpp\
		  char.cpp\
		  char_affect.cpp\
		  char_battle.cpp\
		  char_change_empire.cpp\
		  char_dragonsoul.cpp\
		  char_horse.cpp\
		  char_item.cpp\
		  char_manager.cpp\
		  char_quickslot.cpp\
		  char_resist.cpp\
		  char_skill.cpp\
		  char_state.cpp\
		  cipher.cpp\
		  ClientPackageCryptInfo.cpp\
		  cmd.cpp\
		  cmd_emotion.cpp\
		  cmd_general.cpp\
		  cmd_gm.cpp\
		  cmd_oxevent.cpp\
		  config.cpp\
		  constants.cpp\
		  crc32.cpp\
		  cube.cpp\
		  db.cpp\
		  desc.cpp\
		  desc_client.cpp\
		  desc_manager.cpp\
		  desc_p2p.cpp\
		  dev_log.cpp\
		  dragon_soul_table.cpp\
		  DragonLair.cpp\
		  DragonSoul.cpp\
		  dungeon.cpp\
		  empire_text_convert.cpp\
		  entity.cpp\
		  entity_view.cpp\
		  event.cpp\
		  event_queue.cpp\
		  exchange.cpp\
		  file_loader.cpp\
		  FileMonitor_FreeBSD.cpp\
		  fishing.cpp\
		  FSM.cpp\
		  gm.cpp\
		  group_text_parse_tree.cpp\
		  guild.cpp\
		  guild_manager.cpp\
		  guild_war.cpp\
		  horse_rider.cpp\
		  horsename_manager.cpp\
		  input.cpp\
		  input_auth.cpp\
		  input_db.cpp\
		  input_login.cpp\
		  input_main.cpp\
		  input_p2p.cpp\
		  input_udp.cpp\
		  ip_ban.cpp\
		  item.cpp\
		  item_addon.cpp\
		  item_attribute.cpp\
		  item_manager.cpp\
		  item_manager_idrange.cpp\
		  item_manager_read_tables.cpp\
		  Locale.cpp\
		  locale_service.cpp\
		  log.cpp\
		  login_data.cpp\
		  lzo_manager.cpp\
		  map_location.cpp\
		  MarkConvert.cpp\
		  MarkImage.cpp\
		  MarkManager.cpp\
		  marriage.cpp\
		  messenger_manager.cpp\
		  mining.cpp\
		  mob_manager.cpp\
		  monarch.cpp\
		  motion.cpp\
		  over9refine.cpp\
		  OXEvent.cpp\
		  p2p.cpp\
		  packet_info.cpp\
		  panama.cpp\
		  party.cpp\
		  PetSystem.cpp\
		  polymorph.cpp\
		  priv_manager.cpp\
		  pvp.cpp\
		  questevent.cpp\
		  questlua.cpp\
		  questlua_affect.cpp\
		  questlua_arena.cpp\
		  questlua_ba.cpp\
		  questlua_building.cpp\
		  questlua_danceevent.cpp\
		  questlua_dragonlair.cpp\
		  questlua_dragonsoul.cpp\
		  questlua_dungeon.cpp\
		  questlua_forked.cpp\
		  questlua_game.cpp\
		  questlua_global.cpp\
		  questlua_guild.cpp\
		  questlua_horse.cpp\
		  questlua_item.cpp\
		  questlua_marriage.cpp\
		  questlua_mgmt.cpp\
		  questlua_monarch.cpp\
		  questlua_npc.cpp\
		  questlua_oxevent.cpp\
		  questlua_party.cpp\
		  questlua_pc.cpp\
		  questlua_pet.cpp\
		  questlua_quest.cpp\
		  questlua_speedserver.cpp\
		  questlua_target.cpp\
		  questmanager.cpp\
		  questnpc.cpp\
		  questpc.cpp\
		  refine.cpp\
		  regen.cpp\
		  safebox.cpp\
		  sectree.cpp\
		  sectree_manager.cpp\
		  sequence.cpp\
		  shop.cpp\
		  shop_manager.cpp\
		  shopEx.cpp\
		  shutdown_manager.cpp\
		  skill.cpp\
		  skill_power.cpp\
		  SpeedServer.cpp\
		  start_position.cpp\
		  target.cpp\
		  text_file_loader.cpp\
		  threeway_war.cpp\
		  TrafficProfiler.cpp\
		  trigger.cpp\
		  utils.cpp\
		  vector.cpp\
		  version.cpp\
		  war_map.cpp\
		  wedding.cpp\
		  xmas_event.cpp\
		  GrowthPetSystem.cpp\
		  battle_field.cpp\
		  MeleyLair.cpp\
		  questlua_meley_lair.cpp\
		  DawnMistDungeon.cpp\
		  char_battle_zodiac.cpp\
		  char_zodiac_temple.cpp\
		  questlua_zodiac_temple.cpp\
		  zodiac_temple.cpp\
		  DungeonInfo.cpp\
		  switchbot.cpp\
		  CubeManager.cpp\
		  Attr6th7th.cpp\
		  questlua_attr6th7th.cpp\
		  ranking_system.cpp\
		  SoulRoulette.cpp\
		  char_emotion_new.cpp\
		  char_aura.cpp\
		  char_acce.cpp\
		  char_move_costume_attr.cpp\
		  CsvReader.cpp\
		  ticket.cpp\
		  MailBox.cpp\
		  Transmutation.cpp\
		  GroupMatchManager.cpp\
		  event_manager.cpp\
		  minigame_manager.cpp\
		  minigame_attendance.cpp\
		  minigame_catchking.cpp\
		  minigame_easter.cpp\
		  minigame_okeycard.cpp\
		  minigame_stone.cpp\
		  battle_pass.cpp\
		  minigame_bnw.cpp\
		  minigame_find_m.cpp\
		  minigame_yut_nori.cpp\
		  char_flower_event.cpp\
		  AchievementSystem.cpp\
		  SnakeLair.cpp\
		  questlua_snakelair.cpp\
		  WhiteDragon.cpp\
		  questlua_whitedragon.cpp\
		  BiologSystemManager.cpp\
		  char_hunting.cpp\
		  char_lottery.cpp\
		  char_fishing.cpp\
		  questlua_dnd.cpp
# PROJECT_SRC_FILES END

# PROJECT_OBJ_FILES BEGIN
COBJS	= $(CFILE:%.c=$(OBJDIR)/%.o)
CPPOBJS	= $(CPPFILE:%.cpp=$(OBJDIR)/%.o)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
CDEPS	= $(COBJS:%.o=%.d)
CPPDEPS	= $(CPPOBJS:%.o=%.d)
endif

MAINOBJ = $(MAINCPP:%.cpp=$(OBJDIR)/%.o)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
MAINDEPS = $(MAINOBJ:%.o=%.d)
endif
# PROJECT_OBJ_FILES END

# Target Paths
MAIN_TARGET = $(BINDIR)/game_r$(GAME_VERSION)

default: $(MAIN_TARGET)
test: $(TEST_TARGET)

$(OBJDIR)/%.o: %.c
	@echo -e "\033[0;32m [OK] \033[0m       \033[0;33m Compiling:\033[0m" $<
#	@echo compiling $<
	@$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CC) -MM -MG -MP $(CFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i '' -e's/$*.o:/$(OBJDIR)\/$*.o:/g' $(OBJDIR)/$*.d
endif

$(OBJDIR)/%.o: %.cpp
	@echo -e "\033[0;32m [OK] \033[0m       \033[0;33m Compiling:\033[0m" $<
#	@echo compiling $<
	@$(CC) $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CC) -MM -MG -MP $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i '' -e's/$*.o:/$(OBJDIR)\/$*.o:/g' $(OBJDIR)/$*.d
endif

limit_time:
	@echo update limit time
	@python update_limit_time.py

$(MAIN_TARGET): $(CPPOBJS) $(COBJS) $(MAINOBJ)
	@echo -e "\033[0;31m [linking] \033[0m " $(MAIN_TARGET)
#	@echo linking $(MAIN_TARGET)
	@$(CC) $(CFLAGS) $(CXXFLAGS) $(LIBDIR) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(LIBS) -o $(MAIN_TARGET)

dump:
	@$(CC) -dM -E - < /dev/null > .nope.nope

symlink:
	@ln -fs game_r$(GAME_VERSION) $(BINDIR)/game_symlink

strip:
	@cp $(MAIN_TARGET) $(BINDIR)/game_r
	@strip $(BINDIR)/game_r

clean:
	@echo cleaning $(MAIN_TARGET) $(OBJDIR)
	@rm -f $(COBJS) $(CPPOBJS) $(MAINOBJ)
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@rm -f $(CDEPS) $(CPPDEPS) $(MAINDEPS)
endif
	@rm -f $(BINDIR)/game_r* $(BINDIR)/test_r* $(BINDIR)/conv

dep:
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@echo "Note: gcc autodepend is autodetected, so target dep skipped"
else
	makedepend -f $(DEPFILE) $(INCDIR) -I/usr/include/c++/3.3 -I/usr/include/c++/4.2 -p$(OBJDIR)/ $(CPPFILE) $(CFILE) $(MAINCPP) $(TESTCPP) 2> /dev/null > $(DEPFILE)
endif

# AUTO_DEPEND_CHECK BEGIN
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
sinclude $(CDEPS)
sinclude $(CPPDEPS)
sinclude $(MAINDEPS)
else
sinclude $(DEPFILE)
endif
# AUTO_DEPEND_CHECK END
