##
## Auto Generated makefile, please do not edit
##
WXWIN:=C:\wxWidgets-2.8.7
WXCFG:=gcc_dll\mswu
ProjectName:=player

## Debug
ConfigurationName :=Debug
IntermediateDirectory :=../.obj/Debug
OutDir := $(IntermediateDirectory)
LinkerName:=g++
ArchiveTool :=ar rcus
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix :=.o
DependSuffix :=.o.d
PreprocessSuffix :=
DebugSwitch :=-gstab
IncludeSwitch :=-I
LibrarySwitch :=-l
OutputSwitch :=-o 
LibraryPathSwitch :=-L
PreprocessorSwitch :=-D
SourceSwitch :=-c 
CompilerName :=g++
OutputFile :=../easyrpg
Preprocessors :=
ObjectSwitch :=-o 
ArchiveOutputSwitch := 
PreprocessOnlySwitch :=
CmpOptions :=-Wall -Wextra -ansi -pedantic  $(shell sdl-config --cflags) -g  $(Preprocessors)
LinkOptions :=  -O2 -lSDL_mixer -lSDL_gfx -lSDL_ttf -lSDL_image $(shell sdl-config --libs) 
IncludePath :=  "$(IncludeSwitch)." 
RcIncludePath :=
Libs :=
LibPath := "$(LibraryPathSwitch)." 


Objects=$(IntermediateDirectory)/message$(ObjectSuffix) $(IntermediateDirectory)/Title_scene$(ObjectSuffix) $(IntermediateDirectory)/Batle_scene$(ObjectSuffix) $(IntermediateDirectory)/Event_management$(ObjectSuffix) $(IntermediateDirectory)/General_data$(ObjectSuffix) $(IntermediateDirectory)/Map_scene$(ObjectSuffix) $(IntermediateDirectory)/GameOver_scene$(ObjectSuffix) $(IntermediateDirectory)/scene$(ObjectSuffix) $(IntermediateDirectory)/Move_management$(ObjectSuffix) $(IntermediateDirectory)/Window_Base$(ObjectSuffix) \
	$(IntermediateDirectory)/Window_Player_Select$(ObjectSuffix) $(IntermediateDirectory)/Window_Select$(ObjectSuffix) $(IntermediateDirectory)/Skills_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Save_Load_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Objects_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Stats_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Item_use_scene$(ObjectSuffix) $(IntermediateDirectory)/Main_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Equip_Menu_scene$(ObjectSuffix) $(IntermediateDirectory)/Menu_Easy$(ObjectSuffix) \
	$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/ldb$(ObjectSuffix) $(IntermediateDirectory)/strmap$(ObjectSuffix) $(IntermediateDirectory)/ldbstr$(ObjectSuffix) $(IntermediateDirectory)/lmt$(ObjectSuffix) $(IntermediateDirectory)/xyz$(ObjectSuffix) $(IntermediateDirectory)/map$(ObjectSuffix) $(IntermediateDirectory)/iniparser$(ObjectSuffix) $(IntermediateDirectory)/stevent$(ObjectSuffix) $(IntermediateDirectory)/ldb_data$(ObjectSuffix) \
	$(IntermediateDirectory)/Sound_Manager$(ObjectSuffix) $(IntermediateDirectory)/font$(ObjectSuffix) $(IntermediateDirectory)/Timer$(ObjectSuffix) $(IntermediateDirectory)/CDeltaTime$(ObjectSuffix) $(IntermediateDirectory)/key$(ObjectSuffix) $(IntermediateDirectory)/tools$(ObjectSuffix) $(IntermediateDirectory)/audio$(ObjectSuffix) $(IntermediateDirectory)/control$(ObjectSuffix) $(IntermediateDirectory)/Batler$(ObjectSuffix) $(IntermediateDirectory)/Chara$(ObjectSuffix) \
	$(IntermediateDirectory)/chipset$(ObjectSuffix) $(IntermediateDirectory)/sprite$(ObjectSuffix) $(IntermediateDirectory)/Animacion$(ObjectSuffix) $(IntermediateDirectory)/Faceset$(ObjectSuffix) $(IntermediateDirectory)/Animation_Manager$(ObjectSuffix) $(IntermediateDirectory)/Pre_Chipset$(ObjectSuffix) $(IntermediateDirectory)/Sistem$(ObjectSuffix) $(IntermediateDirectory)/skill$(ObjectSuffix) $(IntermediateDirectory)/Enemy$(ObjectSuffix) $(IntermediateDirectory)/item$(ObjectSuffix) \
	$(IntermediateDirectory)/Player$(ObjectSuffix) $(IntermediateDirectory)/Player_Team$(ObjectSuffix) $(IntermediateDirectory)/CActor$(ObjectSuffix) 

##
## Main Build Tragets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep  $(Objects)
	@makedir $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@makedir "../.obj/Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/message$(ObjectSuffix): control/events/message.cpp $(IntermediateDirectory)/message$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/events/message.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/message$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/message$(DependSuffix): control/events/message.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/message$(ObjectSuffix) -MF$(IntermediateDirectory)/message$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/events/message.cpp"

$(IntermediateDirectory)/Title_scene$(ObjectSuffix): control/Title_scene.cpp $(IntermediateDirectory)/Title_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Title_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Title_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Title_scene$(DependSuffix): control/Title_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Title_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Title_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Title_scene.cpp"

$(IntermediateDirectory)/Batle_scene$(ObjectSuffix): control/Batle_scene.cpp $(IntermediateDirectory)/Batle_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Batle_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Batle_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Batle_scene$(DependSuffix): control/Batle_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Batle_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Batle_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Batle_scene.cpp"

$(IntermediateDirectory)/Event_management$(ObjectSuffix): control/Event_management.cpp $(IntermediateDirectory)/Event_management$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Event_management.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Event_management$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Event_management$(DependSuffix): control/Event_management.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Event_management$(ObjectSuffix) -MF$(IntermediateDirectory)/Event_management$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Event_management.cpp"

$(IntermediateDirectory)/General_data$(ObjectSuffix): control/General_data.cpp $(IntermediateDirectory)/General_data$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/General_data.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/General_data$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/General_data$(DependSuffix): control/General_data.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/General_data$(ObjectSuffix) -MF$(IntermediateDirectory)/General_data$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/General_data.cpp"

$(IntermediateDirectory)/Map_scene$(ObjectSuffix): control/Map_scene.cpp $(IntermediateDirectory)/Map_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Map_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Map_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Map_scene$(DependSuffix): control/Map_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Map_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Map_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Map_scene.cpp"

$(IntermediateDirectory)/GameOver_scene$(ObjectSuffix): control/GameOver_scene.cpp $(IntermediateDirectory)/GameOver_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/GameOver_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/GameOver_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/GameOver_scene$(DependSuffix): control/GameOver_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/GameOver_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/GameOver_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/GameOver_scene.cpp"

$(IntermediateDirectory)/scene$(ObjectSuffix): control/scene.cpp $(IntermediateDirectory)/scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/scene$(DependSuffix): control/scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/scene$(ObjectSuffix) -MF$(IntermediateDirectory)/scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/scene.cpp"

$(IntermediateDirectory)/Move_management$(ObjectSuffix): control/Move_management.cpp $(IntermediateDirectory)/Move_management$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Move_management.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Move_management$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Move_management$(DependSuffix): control/Move_management.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Move_management$(ObjectSuffix) -MF$(IntermediateDirectory)/Move_management$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/control/Move_management.cpp"

$(IntermediateDirectory)/Window_Base$(ObjectSuffix): interface/Windows/Window_Base.cpp $(IntermediateDirectory)/Window_Base$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Base.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Window_Base$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Window_Base$(DependSuffix): interface/Windows/Window_Base.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Window_Base$(ObjectSuffix) -MF$(IntermediateDirectory)/Window_Base$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Base.cpp"

$(IntermediateDirectory)/Window_Player_Select$(ObjectSuffix): interface/Windows/Window_Player_Select.cpp $(IntermediateDirectory)/Window_Player_Select$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Player_Select.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Window_Player_Select$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Window_Player_Select$(DependSuffix): interface/Windows/Window_Player_Select.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Window_Player_Select$(ObjectSuffix) -MF$(IntermediateDirectory)/Window_Player_Select$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Player_Select.cpp"

$(IntermediateDirectory)/Window_Select$(ObjectSuffix): interface/Windows/Window_Select.cpp $(IntermediateDirectory)/Window_Select$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Select.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Window_Select$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Window_Select$(DependSuffix): interface/Windows/Window_Select.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Window_Select$(ObjectSuffix) -MF$(IntermediateDirectory)/Window_Select$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Windows/Window_Select.cpp"

$(IntermediateDirectory)/Skills_Menu_scene$(ObjectSuffix): interface/Control/Skills_Menu_scene.cpp $(IntermediateDirectory)/Skills_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Skills_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Skills_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Skills_Menu_scene$(DependSuffix): interface/Control/Skills_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Skills_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Skills_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Skills_Menu_scene.cpp"

$(IntermediateDirectory)/Save_Load_Menu_scene$(ObjectSuffix): interface/Control/Save_Load_Menu_scene.cpp $(IntermediateDirectory)/Save_Load_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Save_Load_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Save_Load_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Save_Load_Menu_scene$(DependSuffix): interface/Control/Save_Load_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Save_Load_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Save_Load_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Save_Load_Menu_scene.cpp"

$(IntermediateDirectory)/Objects_Menu_scene$(ObjectSuffix): interface/Control/Objects_Menu_scene.cpp $(IntermediateDirectory)/Objects_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Objects_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Objects_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Objects_Menu_scene$(DependSuffix): interface/Control/Objects_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Objects_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Objects_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Objects_Menu_scene.cpp"

$(IntermediateDirectory)/Stats_Menu_scene$(ObjectSuffix): interface/Control/Stats_Menu_scene.cpp $(IntermediateDirectory)/Stats_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Stats_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Stats_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Stats_Menu_scene$(DependSuffix): interface/Control/Stats_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Stats_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Stats_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Stats_Menu_scene.cpp"

$(IntermediateDirectory)/Item_use_scene$(ObjectSuffix): interface/Control/Item_use_scene.cpp $(IntermediateDirectory)/Item_use_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Item_use_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Item_use_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Item_use_scene$(DependSuffix): interface/Control/Item_use_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Item_use_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Item_use_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Item_use_scene.cpp"

$(IntermediateDirectory)/Main_Menu_scene$(ObjectSuffix): interface/Control/Main_Menu_scene.cpp $(IntermediateDirectory)/Main_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Main_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Main_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Main_Menu_scene$(DependSuffix): interface/Control/Main_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Main_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Main_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Main_Menu_scene.cpp"

$(IntermediateDirectory)/Equip_Menu_scene$(ObjectSuffix): interface/Control/Equip_Menu_scene.cpp $(IntermediateDirectory)/Equip_Menu_scene$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Equip_Menu_scene.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Equip_Menu_scene$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Equip_Menu_scene$(DependSuffix): interface/Control/Equip_Menu_scene.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Equip_Menu_scene$(ObjectSuffix) -MF$(IntermediateDirectory)/Equip_Menu_scene$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Control/Equip_Menu_scene.cpp"

$(IntermediateDirectory)/Menu_Easy$(ObjectSuffix): interface/Menu_Easy.cpp $(IntermediateDirectory)/Menu_Easy$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Menu_Easy.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Menu_Easy$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Menu_Easy$(DependSuffix): interface/Menu_Easy.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Menu_Easy$(ObjectSuffix) -MF$(IntermediateDirectory)/Menu_Easy$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/interface/Menu_Easy.cpp"

$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/main.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/main.cpp"

$(IntermediateDirectory)/ldb$(ObjectSuffix): readers/ldb.cpp $(IntermediateDirectory)/ldb$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldb.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldb$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldb$(DependSuffix): readers/ldb.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldb$(ObjectSuffix) -MF$(IntermediateDirectory)/ldb$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldb.cpp"

$(IntermediateDirectory)/strmap$(ObjectSuffix): readers/strmap.cpp $(IntermediateDirectory)/strmap$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/strmap.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/strmap$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/strmap$(DependSuffix): readers/strmap.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/strmap$(ObjectSuffix) -MF$(IntermediateDirectory)/strmap$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/strmap.cpp"

$(IntermediateDirectory)/ldbstr$(ObjectSuffix): readers/ldbstr.cpp $(IntermediateDirectory)/ldbstr$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldbstr.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldbstr$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldbstr$(DependSuffix): readers/ldbstr.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldbstr$(ObjectSuffix) -MF$(IntermediateDirectory)/ldbstr$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldbstr.cpp"

$(IntermediateDirectory)/lmt$(ObjectSuffix): readers/lmt.cpp $(IntermediateDirectory)/lmt$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/lmt.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/lmt$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lmt$(DependSuffix): readers/lmt.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/lmt$(ObjectSuffix) -MF$(IntermediateDirectory)/lmt$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/lmt.cpp"

$(IntermediateDirectory)/xyz$(ObjectSuffix): readers/xyz.cpp $(IntermediateDirectory)/xyz$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/xyz.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/xyz$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/xyz$(DependSuffix): readers/xyz.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/xyz$(ObjectSuffix) -MF$(IntermediateDirectory)/xyz$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/xyz.cpp"

$(IntermediateDirectory)/map$(ObjectSuffix): readers/map.cpp $(IntermediateDirectory)/map$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/map.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/map$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/map$(DependSuffix): readers/map.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/map$(ObjectSuffix) -MF$(IntermediateDirectory)/map$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/map.cpp"

$(IntermediateDirectory)/iniparser$(ObjectSuffix): readers/iniparser.cpp $(IntermediateDirectory)/iniparser$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/iniparser.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/iniparser$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iniparser$(DependSuffix): readers/iniparser.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/iniparser$(ObjectSuffix) -MF$(IntermediateDirectory)/iniparser$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/iniparser.cpp"

$(IntermediateDirectory)/stevent$(ObjectSuffix): readers/stevent.cpp $(IntermediateDirectory)/stevent$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/stevent.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/stevent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stevent$(DependSuffix): readers/stevent.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/stevent$(ObjectSuffix) -MF$(IntermediateDirectory)/stevent$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/stevent.cpp"

$(IntermediateDirectory)/ldb_data$(ObjectSuffix): readers/ldb_data.cpp $(IntermediateDirectory)/ldb_data$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldb_data.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldb_data$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldb_data$(DependSuffix): readers/ldb_data.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldb_data$(ObjectSuffix) -MF$(IntermediateDirectory)/ldb_data$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/readers/ldb_data.cpp"

$(IntermediateDirectory)/Sound_Manager$(ObjectSuffix): tools/Sound_Manager.cpp $(IntermediateDirectory)/Sound_Manager$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/Sound_Manager.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Sound_Manager$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Sound_Manager$(DependSuffix): tools/Sound_Manager.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Sound_Manager$(ObjectSuffix) -MF$(IntermediateDirectory)/Sound_Manager$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/Sound_Manager.cpp"

$(IntermediateDirectory)/font$(ObjectSuffix): tools/font.cpp $(IntermediateDirectory)/font$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/font.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/font$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/font$(DependSuffix): tools/font.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/font$(ObjectSuffix) -MF$(IntermediateDirectory)/font$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/font.cpp"

$(IntermediateDirectory)/Timer$(ObjectSuffix): tools/Timer.cpp $(IntermediateDirectory)/Timer$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/Timer.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Timer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Timer$(DependSuffix): tools/Timer.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Timer$(ObjectSuffix) -MF$(IntermediateDirectory)/Timer$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/Timer.cpp"

$(IntermediateDirectory)/CDeltaTime$(ObjectSuffix): tools/CDeltaTime.cpp $(IntermediateDirectory)/CDeltaTime$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/CDeltaTime.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/CDeltaTime$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CDeltaTime$(DependSuffix): tools/CDeltaTime.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/CDeltaTime$(ObjectSuffix) -MF$(IntermediateDirectory)/CDeltaTime$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/CDeltaTime.cpp"

$(IntermediateDirectory)/key$(ObjectSuffix): tools/key.cpp $(IntermediateDirectory)/key$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/key.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/key$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/key$(DependSuffix): tools/key.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/key$(ObjectSuffix) -MF$(IntermediateDirectory)/key$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/key.cpp"

$(IntermediateDirectory)/tools$(ObjectSuffix): tools/tools.cpp $(IntermediateDirectory)/tools$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/tools.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/tools$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tools$(DependSuffix): tools/tools.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/tools$(ObjectSuffix) -MF$(IntermediateDirectory)/tools$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/tools.cpp"

$(IntermediateDirectory)/audio$(ObjectSuffix): tools/audio.cpp $(IntermediateDirectory)/audio$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/audio.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/audio$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/audio$(DependSuffix): tools/audio.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/audio$(ObjectSuffix) -MF$(IntermediateDirectory)/audio$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/audio.cpp"

$(IntermediateDirectory)/control$(ObjectSuffix): tools/control.cpp $(IntermediateDirectory)/control$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/control.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/control$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/control$(DependSuffix): tools/control.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/control$(ObjectSuffix) -MF$(IntermediateDirectory)/control$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/tools/control.cpp"

$(IntermediateDirectory)/Batler$(ObjectSuffix): sprites/Batler.cpp $(IntermediateDirectory)/Batler$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Batler.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Batler$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Batler$(DependSuffix): sprites/Batler.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Batler$(ObjectSuffix) -MF$(IntermediateDirectory)/Batler$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Batler.cpp"

$(IntermediateDirectory)/Chara$(ObjectSuffix): sprites/Chara.cpp $(IntermediateDirectory)/Chara$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Chara.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Chara$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Chara$(DependSuffix): sprites/Chara.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Chara$(ObjectSuffix) -MF$(IntermediateDirectory)/Chara$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Chara.cpp"

$(IntermediateDirectory)/chipset$(ObjectSuffix): sprites/chipset.cpp $(IntermediateDirectory)/chipset$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/chipset.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/chipset$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/chipset$(DependSuffix): sprites/chipset.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/chipset$(ObjectSuffix) -MF$(IntermediateDirectory)/chipset$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/chipset.cpp"

$(IntermediateDirectory)/sprite$(ObjectSuffix): sprites/sprite.cpp $(IntermediateDirectory)/sprite$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/sprite.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/sprite$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sprite$(DependSuffix): sprites/sprite.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/sprite$(ObjectSuffix) -MF$(IntermediateDirectory)/sprite$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/sprite.cpp"

$(IntermediateDirectory)/Animacion$(ObjectSuffix): sprites/Animacion.cpp $(IntermediateDirectory)/Animacion$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Animacion.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Animacion$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Animacion$(DependSuffix): sprites/Animacion.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Animacion$(ObjectSuffix) -MF$(IntermediateDirectory)/Animacion$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Animacion.cpp"

$(IntermediateDirectory)/Faceset$(ObjectSuffix): sprites/Faceset.cpp $(IntermediateDirectory)/Faceset$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Faceset.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Faceset$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Faceset$(DependSuffix): sprites/Faceset.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Faceset$(ObjectSuffix) -MF$(IntermediateDirectory)/Faceset$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Faceset.cpp"

$(IntermediateDirectory)/Animation_Manager$(ObjectSuffix): sprites/Animation_Manager.cpp $(IntermediateDirectory)/Animation_Manager$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Animation_Manager.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Animation_Manager$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Animation_Manager$(DependSuffix): sprites/Animation_Manager.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Animation_Manager$(ObjectSuffix) -MF$(IntermediateDirectory)/Animation_Manager$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Animation_Manager.cpp"

$(IntermediateDirectory)/Pre_Chipset$(ObjectSuffix): sprites/Pre_Chipset.cpp $(IntermediateDirectory)/Pre_Chipset$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Pre_Chipset.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Pre_Chipset$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Pre_Chipset$(DependSuffix): sprites/Pre_Chipset.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Pre_Chipset$(ObjectSuffix) -MF$(IntermediateDirectory)/Pre_Chipset$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Pre_Chipset.cpp"

$(IntermediateDirectory)/Sistem$(ObjectSuffix): sprites/Sistem.cpp $(IntermediateDirectory)/Sistem$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Sistem.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Sistem$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Sistem$(DependSuffix): sprites/Sistem.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Sistem$(ObjectSuffix) -MF$(IntermediateDirectory)/Sistem$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/sprites/Sistem.cpp"

$(IntermediateDirectory)/skill$(ObjectSuffix): attributes/skill.cpp $(IntermediateDirectory)/skill$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/skill.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/skill$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/skill$(DependSuffix): attributes/skill.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/skill$(ObjectSuffix) -MF$(IntermediateDirectory)/skill$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/skill.cpp"

$(IntermediateDirectory)/Enemy$(ObjectSuffix): attributes/Enemy.cpp $(IntermediateDirectory)/Enemy$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Enemy.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Enemy$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Enemy$(DependSuffix): attributes/Enemy.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Enemy$(ObjectSuffix) -MF$(IntermediateDirectory)/Enemy$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Enemy.cpp"

$(IntermediateDirectory)/item$(ObjectSuffix): attributes/item.cpp $(IntermediateDirectory)/item$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/item.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/item$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/item$(DependSuffix): attributes/item.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/item$(ObjectSuffix) -MF$(IntermediateDirectory)/item$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/item.cpp"

$(IntermediateDirectory)/Player$(ObjectSuffix): attributes/Player.cpp $(IntermediateDirectory)/Player$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Player.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Player$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Player$(DependSuffix): attributes/Player.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Player$(ObjectSuffix) -MF$(IntermediateDirectory)/Player$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Player.cpp"

$(IntermediateDirectory)/Player_Team$(ObjectSuffix): attributes/Player_Team.cpp $(IntermediateDirectory)/Player_Team$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Player_Team.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Player_Team$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Player_Team$(DependSuffix): attributes/Player_Team.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/Player_Team$(ObjectSuffix) -MF$(IntermediateDirectory)/Player_Team$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/Player_Team.cpp"

$(IntermediateDirectory)/CActor$(ObjectSuffix): attributes/CActor.cpp $(IntermediateDirectory)/CActor$(DependSuffix)
	@makedir "../.obj/Debug"
	$(CompilerName) $(SourceSwitch) "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/CActor.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/CActor$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CActor$(DependSuffix): attributes/CActor.cpp
	@makedir "../.obj/Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/CActor$(ObjectSuffix) -MF$(IntermediateDirectory)/CActor$(DependSuffix) -MM "C:/Documents and Settings/Paulo/Escritorio/EasyRPG/player/src/attributes/CActor.cpp"

##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/message$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/message$(DependSuffix)
	$(RM) $(IntermediateDirectory)/message$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Title_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Title_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Title_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Batle_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Batle_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Batle_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Event_management$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Event_management$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Event_management$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/General_data$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/General_data$(DependSuffix)
	$(RM) $(IntermediateDirectory)/General_data$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Map_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Map_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Map_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/GameOver_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/GameOver_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/GameOver_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Move_management$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Move_management$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Move_management$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Window_Base$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Window_Base$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Window_Base$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Window_Player_Select$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Window_Player_Select$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Window_Player_Select$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Window_Select$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Window_Select$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Window_Select$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Skills_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Skills_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Skills_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Save_Load_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Save_Load_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Save_Load_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Objects_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Objects_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Objects_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Stats_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Stats_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Stats_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Item_use_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Item_use_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Item_use_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Main_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Main_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Main_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Equip_Menu_scene$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Equip_Menu_scene$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Equip_Menu_scene$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Menu_Easy$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Menu_Easy$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Menu_Easy$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldb$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldb$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldb$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/strmap$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/strmap$(DependSuffix)
	$(RM) $(IntermediateDirectory)/strmap$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/lmt$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/lmt$(DependSuffix)
	$(RM) $(IntermediateDirectory)/lmt$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/xyz$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/xyz$(DependSuffix)
	$(RM) $(IntermediateDirectory)/xyz$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/map$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/map$(DependSuffix)
	$(RM) $(IntermediateDirectory)/map$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/iniparser$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/iniparser$(DependSuffix)
	$(RM) $(IntermediateDirectory)/iniparser$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Sound_Manager$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Sound_Manager$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Sound_Manager$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/font$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/font$(DependSuffix)
	$(RM) $(IntermediateDirectory)/font$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Timer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Timer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Timer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CDeltaTime$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CDeltaTime$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CDeltaTime$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/key$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/key$(DependSuffix)
	$(RM) $(IntermediateDirectory)/key$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/tools$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/tools$(DependSuffix)
	$(RM) $(IntermediateDirectory)/tools$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/audio$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/audio$(DependSuffix)
	$(RM) $(IntermediateDirectory)/audio$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/control$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/control$(DependSuffix)
	$(RM) $(IntermediateDirectory)/control$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Batler$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Batler$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Batler$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Chara$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Chara$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Chara$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/chipset$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/chipset$(DependSuffix)
	$(RM) $(IntermediateDirectory)/chipset$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(DependSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Animacion$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Animacion$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Animacion$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Faceset$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Faceset$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Faceset$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Animation_Manager$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Animation_Manager$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Animation_Manager$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Pre_Chipset$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Pre_Chipset$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Pre_Chipset$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Sistem$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Sistem$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Sistem$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/skill$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/skill$(DependSuffix)
	$(RM) $(IntermediateDirectory)/skill$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Enemy$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Enemy$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Enemy$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/item$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/item$(DependSuffix)
	$(RM) $(IntermediateDirectory)/item$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Player$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Player$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Player$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Player_Team$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Player_Team$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Player_Team$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CActor$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CActor$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CActor$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe

-include $(IntermediateDirectory)/*$(DependSuffix)


