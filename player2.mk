##
## Auto Generated makefile, please do not edit
##
WXWIN:=C:\wxWidgets-2.8.10
WXCFG:=gcc_dll\mswu
ProjectName:=player2

## Debug
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "C:\Program Files\CodeLite\Juegos"
ProjectPath            := "C:\easyrpg\player2"
CurrentFileName        :=
CurrentFulePath        :=
CurrentFileFullPath    :=
User                   :=Paulo
Date                   :=10/31/09
CodeLitePath           :="C:\Program Files\CodeLite"
LinkerName             :=g++
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=g++
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=
CmpOptions := -g $(Preprocessors)
LinkOptions :=  
IncludePath :=  "$(IncludeSwitch)." "$(IncludeSwitch)." "$(IncludeSwitch)./libSDL/include/" 
RcIncludePath :=
Libs :=
LibPath := "$(LibraryPathSwitch)." 


Objects=$(IntermediateDirectory)/audio$(ObjectSuffix) $(IntermediateDirectory)/bitmap$(ObjectSuffix) $(IntermediateDirectory)/color$(ObjectSuffix) $(IntermediateDirectory)/font$(ObjectSuffix) $(IntermediateDirectory)/game_actors$(ObjectSuffix) $(IntermediateDirectory)/game_map$(ObjectSuffix) $(IntermediateDirectory)/game_party$(ObjectSuffix) $(IntermediateDirectory)/game_player$(ObjectSuffix) $(IntermediateDirectory)/game_screen$(ObjectSuffix) $(IntermediateDirectory)/game_switches$(ObjectSuffix) \
	$(IntermediateDirectory)/game_system$(ObjectSuffix) $(IntermediateDirectory)/game_temp$(ObjectSuffix) $(IntermediateDirectory)/game_troop$(ObjectSuffix) $(IntermediateDirectory)/game_variables$(ObjectSuffix) $(IntermediateDirectory)/graphics$(ObjectSuffix) $(IntermediateDirectory)/input$(ObjectSuffix) $(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/plane$(ObjectSuffix) $(IntermediateDirectory)/rect$(ObjectSuffix) $(IntermediateDirectory)/scene_title$(ObjectSuffix) \
	$(IntermediateDirectory)/sprite$(ObjectSuffix) $(IntermediateDirectory)/table$(ObjectSuffix) $(IntermediateDirectory)/tilemap$(ObjectSuffix) $(IntermediateDirectory)/tone$(ObjectSuffix) $(IntermediateDirectory)/viewport$(ObjectSuffix) $(IntermediateDirectory)/window$(ObjectSuffix) $(IntermediateDirectory)/zobj$(ObjectSuffix) $(IntermediateDirectory)/rpg_actor$(ObjectSuffix) $(IntermediateDirectory)/rpg_skill$(ObjectSuffix) $(IntermediateDirectory)/RPG$(ObjectSuffix) \
	$(IntermediateDirectory)/actor$(ObjectSuffix) $(IntermediateDirectory)/ldb_reader$(ObjectSuffix) $(IntermediateDirectory)/skill$(ObjectSuffix) $(IntermediateDirectory)/item$(ObjectSuffix) $(IntermediateDirectory)/enemy$(ObjectSuffix) $(IntermediateDirectory)/e_action$(ObjectSuffix) $(IntermediateDirectory)/troop$(ObjectSuffix) $(IntermediateDirectory)/tools$(ObjectSuffix) $(IntermediateDirectory)/ldb_data$(ObjectSuffix) $(IntermediateDirectory)/ldbstr$(ObjectSuffix) \
	$(IntermediateDirectory)/stevent$(ObjectSuffix) 

##
## Main Build Tragets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep PrePreBuild $(Objects)
	@makedir $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@makedir "./Debug"
PrePreBuild: 



PreBuild:


##
## Objects
##
$(IntermediateDirectory)/audio$(ObjectSuffix): src/audio.cpp $(IntermediateDirectory)/audio$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/audio.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/audio$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/audio$(DependSuffix): src/audio.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/audio$(ObjectSuffix) -MF$(IntermediateDirectory)/audio$(DependSuffix) -MM "C:/easyrpg/player2/src/audio.cpp"

$(IntermediateDirectory)/bitmap$(ObjectSuffix): src/bitmap.cpp $(IntermediateDirectory)/bitmap$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/bitmap.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/bitmap$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/bitmap$(DependSuffix): src/bitmap.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/bitmap$(ObjectSuffix) -MF$(IntermediateDirectory)/bitmap$(DependSuffix) -MM "C:/easyrpg/player2/src/bitmap.cpp"

$(IntermediateDirectory)/color$(ObjectSuffix): src/color.cpp $(IntermediateDirectory)/color$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/color.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/color$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/color$(DependSuffix): src/color.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/color$(ObjectSuffix) -MF$(IntermediateDirectory)/color$(DependSuffix) -MM "C:/easyrpg/player2/src/color.cpp"

$(IntermediateDirectory)/font$(ObjectSuffix): src/font.cpp $(IntermediateDirectory)/font$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/font.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/font$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/font$(DependSuffix): src/font.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/font$(ObjectSuffix) -MF$(IntermediateDirectory)/font$(DependSuffix) -MM "C:/easyrpg/player2/src/font.cpp"

$(IntermediateDirectory)/game_actors$(ObjectSuffix): src/game_actors.cpp $(IntermediateDirectory)/game_actors$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_actors.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_actors$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_actors$(DependSuffix): src/game_actors.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_actors$(ObjectSuffix) -MF$(IntermediateDirectory)/game_actors$(DependSuffix) -MM "C:/easyrpg/player2/src/game_actors.cpp"

$(IntermediateDirectory)/game_map$(ObjectSuffix): src/game_map.cpp $(IntermediateDirectory)/game_map$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_map.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_map$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_map$(DependSuffix): src/game_map.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_map$(ObjectSuffix) -MF$(IntermediateDirectory)/game_map$(DependSuffix) -MM "C:/easyrpg/player2/src/game_map.cpp"

$(IntermediateDirectory)/game_party$(ObjectSuffix): src/game_party.cpp $(IntermediateDirectory)/game_party$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_party.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_party$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_party$(DependSuffix): src/game_party.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_party$(ObjectSuffix) -MF$(IntermediateDirectory)/game_party$(DependSuffix) -MM "C:/easyrpg/player2/src/game_party.cpp"

$(IntermediateDirectory)/game_player$(ObjectSuffix): src/game_player.cpp $(IntermediateDirectory)/game_player$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_player.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_player$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_player$(DependSuffix): src/game_player.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_player$(ObjectSuffix) -MF$(IntermediateDirectory)/game_player$(DependSuffix) -MM "C:/easyrpg/player2/src/game_player.cpp"

$(IntermediateDirectory)/game_screen$(ObjectSuffix): src/game_screen.cpp $(IntermediateDirectory)/game_screen$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_screen.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_screen$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_screen$(DependSuffix): src/game_screen.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_screen$(ObjectSuffix) -MF$(IntermediateDirectory)/game_screen$(DependSuffix) -MM "C:/easyrpg/player2/src/game_screen.cpp"

$(IntermediateDirectory)/game_switches$(ObjectSuffix): src/game_switches.cpp $(IntermediateDirectory)/game_switches$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_switches.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_switches$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_switches$(DependSuffix): src/game_switches.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_switches$(ObjectSuffix) -MF$(IntermediateDirectory)/game_switches$(DependSuffix) -MM "C:/easyrpg/player2/src/game_switches.cpp"

$(IntermediateDirectory)/game_system$(ObjectSuffix): src/game_system.cpp $(IntermediateDirectory)/game_system$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_system.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_system$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_system$(DependSuffix): src/game_system.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_system$(ObjectSuffix) -MF$(IntermediateDirectory)/game_system$(DependSuffix) -MM "C:/easyrpg/player2/src/game_system.cpp"

$(IntermediateDirectory)/game_temp$(ObjectSuffix): src/game_temp.cpp $(IntermediateDirectory)/game_temp$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_temp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_temp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_temp$(DependSuffix): src/game_temp.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_temp$(ObjectSuffix) -MF$(IntermediateDirectory)/game_temp$(DependSuffix) -MM "C:/easyrpg/player2/src/game_temp.cpp"

$(IntermediateDirectory)/game_troop$(ObjectSuffix): src/game_troop.cpp $(IntermediateDirectory)/game_troop$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_troop.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_troop$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_troop$(DependSuffix): src/game_troop.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_troop$(ObjectSuffix) -MF$(IntermediateDirectory)/game_troop$(DependSuffix) -MM "C:/easyrpg/player2/src/game_troop.cpp"

$(IntermediateDirectory)/game_variables$(ObjectSuffix): src/game_variables.cpp $(IntermediateDirectory)/game_variables$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/game_variables.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/game_variables$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/game_variables$(DependSuffix): src/game_variables.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/game_variables$(ObjectSuffix) -MF$(IntermediateDirectory)/game_variables$(DependSuffix) -MM "C:/easyrpg/player2/src/game_variables.cpp"

$(IntermediateDirectory)/graphics$(ObjectSuffix): src/graphics.cpp $(IntermediateDirectory)/graphics$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/graphics.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/graphics$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/graphics$(DependSuffix): src/graphics.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/graphics$(ObjectSuffix) -MF$(IntermediateDirectory)/graphics$(DependSuffix) -MM "C:/easyrpg/player2/src/graphics.cpp"

$(IntermediateDirectory)/input$(ObjectSuffix): src/input.cpp $(IntermediateDirectory)/input$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/input.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/input$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/input$(DependSuffix): src/input.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/input$(ObjectSuffix) -MF$(IntermediateDirectory)/input$(DependSuffix) -MM "C:/easyrpg/player2/src/input.cpp"

$(IntermediateDirectory)/main$(ObjectSuffix): src/main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/main.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): src/main.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "C:/easyrpg/player2/src/main.cpp"

$(IntermediateDirectory)/plane$(ObjectSuffix): src/plane.cpp $(IntermediateDirectory)/plane$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/plane.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/plane$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/plane$(DependSuffix): src/plane.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/plane$(ObjectSuffix) -MF$(IntermediateDirectory)/plane$(DependSuffix) -MM "C:/easyrpg/player2/src/plane.cpp"

$(IntermediateDirectory)/rect$(ObjectSuffix): src/rect.cpp $(IntermediateDirectory)/rect$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/rect.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rect$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rect$(DependSuffix): src/rect.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rect$(ObjectSuffix) -MF$(IntermediateDirectory)/rect$(DependSuffix) -MM "C:/easyrpg/player2/src/rect.cpp"

$(IntermediateDirectory)/scene_title$(ObjectSuffix): src/scene_title.cpp $(IntermediateDirectory)/scene_title$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/scene_title.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/scene_title$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/scene_title$(DependSuffix): src/scene_title.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/scene_title$(ObjectSuffix) -MF$(IntermediateDirectory)/scene_title$(DependSuffix) -MM "C:/easyrpg/player2/src/scene_title.cpp"

$(IntermediateDirectory)/sprite$(ObjectSuffix): src/sprite.cpp $(IntermediateDirectory)/sprite$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/sprite.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/sprite$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sprite$(DependSuffix): src/sprite.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/sprite$(ObjectSuffix) -MF$(IntermediateDirectory)/sprite$(DependSuffix) -MM "C:/easyrpg/player2/src/sprite.cpp"

$(IntermediateDirectory)/table$(ObjectSuffix): src/table.cpp $(IntermediateDirectory)/table$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/table.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/table$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/table$(DependSuffix): src/table.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/table$(ObjectSuffix) -MF$(IntermediateDirectory)/table$(DependSuffix) -MM "C:/easyrpg/player2/src/table.cpp"

$(IntermediateDirectory)/tilemap$(ObjectSuffix): src/tilemap.cpp $(IntermediateDirectory)/tilemap$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/tilemap.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/tilemap$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tilemap$(DependSuffix): src/tilemap.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/tilemap$(ObjectSuffix) -MF$(IntermediateDirectory)/tilemap$(DependSuffix) -MM "C:/easyrpg/player2/src/tilemap.cpp"

$(IntermediateDirectory)/tone$(ObjectSuffix): src/tone.cpp $(IntermediateDirectory)/tone$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/tone.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/tone$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tone$(DependSuffix): src/tone.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/tone$(ObjectSuffix) -MF$(IntermediateDirectory)/tone$(DependSuffix) -MM "C:/easyrpg/player2/src/tone.cpp"

$(IntermediateDirectory)/viewport$(ObjectSuffix): src/viewport.cpp $(IntermediateDirectory)/viewport$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/viewport.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/viewport$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/viewport$(DependSuffix): src/viewport.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/viewport$(ObjectSuffix) -MF$(IntermediateDirectory)/viewport$(DependSuffix) -MM "C:/easyrpg/player2/src/viewport.cpp"

$(IntermediateDirectory)/window$(ObjectSuffix): src/window.cpp $(IntermediateDirectory)/window$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/window.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/window$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/window$(DependSuffix): src/window.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/window$(ObjectSuffix) -MF$(IntermediateDirectory)/window$(DependSuffix) -MM "C:/easyrpg/player2/src/window.cpp"

$(IntermediateDirectory)/zobj$(ObjectSuffix): src/zobj.cpp $(IntermediateDirectory)/zobj$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/zobj.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/zobj$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zobj$(DependSuffix): src/zobj.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/zobj$(ObjectSuffix) -MF$(IntermediateDirectory)/zobj$(DependSuffix) -MM "C:/easyrpg/player2/src/zobj.cpp"

$(IntermediateDirectory)/rpg_actor$(ObjectSuffix): src/rpg_actor.cpp $(IntermediateDirectory)/rpg_actor$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/rpg_actor.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rpg_actor$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rpg_actor$(DependSuffix): src/rpg_actor.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rpg_actor$(ObjectSuffix) -MF$(IntermediateDirectory)/rpg_actor$(DependSuffix) -MM "C:/easyrpg/player2/src/rpg_actor.cpp"

$(IntermediateDirectory)/rpg_skill$(ObjectSuffix): src/rpg_skill.cpp $(IntermediateDirectory)/rpg_skill$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/rpg_skill.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rpg_skill$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rpg_skill$(DependSuffix): src/rpg_skill.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rpg_skill$(ObjectSuffix) -MF$(IntermediateDirectory)/rpg_skill$(DependSuffix) -MM "C:/easyrpg/player2/src/rpg_skill.cpp"

$(IntermediateDirectory)/RPG$(ObjectSuffix): src/Database/RPG.cpp $(IntermediateDirectory)/RPG$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/RPG.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/RPG$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/RPG$(DependSuffix): src/Database/RPG.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/RPG$(ObjectSuffix) -MF$(IntermediateDirectory)/RPG$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/RPG.cpp"

$(IntermediateDirectory)/actor$(ObjectSuffix): src/Database/actor.cpp $(IntermediateDirectory)/actor$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/actor.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/actor$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/actor$(DependSuffix): src/Database/actor.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/actor$(ObjectSuffix) -MF$(IntermediateDirectory)/actor$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/actor.cpp"

$(IntermediateDirectory)/ldb_reader$(ObjectSuffix): src/Database/ldb_reader.cpp $(IntermediateDirectory)/ldb_reader$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/ldb_reader.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldb_reader$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldb_reader$(DependSuffix): src/Database/ldb_reader.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldb_reader$(ObjectSuffix) -MF$(IntermediateDirectory)/ldb_reader$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/ldb_reader.cpp"

$(IntermediateDirectory)/skill$(ObjectSuffix): src/Database/skill.cpp $(IntermediateDirectory)/skill$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/skill.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/skill$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/skill$(DependSuffix): src/Database/skill.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/skill$(ObjectSuffix) -MF$(IntermediateDirectory)/skill$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/skill.cpp"

$(IntermediateDirectory)/item$(ObjectSuffix): src/Database/item.cpp $(IntermediateDirectory)/item$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/item.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/item$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/item$(DependSuffix): src/Database/item.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/item$(ObjectSuffix) -MF$(IntermediateDirectory)/item$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/item.cpp"

$(IntermediateDirectory)/enemy$(ObjectSuffix): src/Database/enemy.cpp $(IntermediateDirectory)/enemy$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/enemy.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/enemy$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/enemy$(DependSuffix): src/Database/enemy.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/enemy$(ObjectSuffix) -MF$(IntermediateDirectory)/enemy$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/enemy.cpp"

$(IntermediateDirectory)/e_action$(ObjectSuffix): src/Database/e_action.cpp $(IntermediateDirectory)/e_action$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/Database/e_action.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/e_action$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/e_action$(DependSuffix): src/Database/e_action.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/e_action$(ObjectSuffix) -MF$(IntermediateDirectory)/e_action$(DependSuffix) -MM "C:/easyrpg/player2/src/Database/e_action.cpp"

$(IntermediateDirectory)/troop$(ObjectSuffix): src/troop.cpp $(IntermediateDirectory)/troop$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/troop.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/troop$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/troop$(DependSuffix): src/troop.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/troop$(ObjectSuffix) -MF$(IntermediateDirectory)/troop$(DependSuffix) -MM "C:/easyrpg/player2/src/troop.cpp"

$(IntermediateDirectory)/tools$(ObjectSuffix): src/tools.cpp $(IntermediateDirectory)/tools$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/tools.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/tools$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tools$(DependSuffix): src/tools.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/tools$(ObjectSuffix) -MF$(IntermediateDirectory)/tools$(DependSuffix) -MM "C:/easyrpg/player2/src/tools.cpp"

$(IntermediateDirectory)/ldb_data$(ObjectSuffix): src/ldb_data.cpp $(IntermediateDirectory)/ldb_data$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/ldb_data.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldb_data$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldb_data$(DependSuffix): src/ldb_data.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldb_data$(ObjectSuffix) -MF$(IntermediateDirectory)/ldb_data$(DependSuffix) -MM "C:/easyrpg/player2/src/ldb_data.cpp"

$(IntermediateDirectory)/ldbstr$(ObjectSuffix): src/ldbstr.cpp $(IntermediateDirectory)/ldbstr$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/ldbstr.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/ldbstr$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ldbstr$(DependSuffix): src/ldbstr.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/ldbstr$(ObjectSuffix) -MF$(IntermediateDirectory)/ldbstr$(DependSuffix) -MM "C:/easyrpg/player2/src/ldbstr.cpp"

$(IntermediateDirectory)/stevent$(ObjectSuffix): src/stevent.cpp $(IntermediateDirectory)/stevent$(DependSuffix)
	@makedir "./Debug"
	$(CompilerName) $(SourceSwitch) "C:/easyrpg/player2/src/stevent.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/stevent$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stevent$(DependSuffix): src/stevent.cpp
	@makedir "./Debug"
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/stevent$(ObjectSuffix) -MF$(IntermediateDirectory)/stevent$(DependSuffix) -MM "C:/easyrpg/player2/src/stevent.cpp"

##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/audio$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/audio$(DependSuffix)
	$(RM) $(IntermediateDirectory)/audio$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/bitmap$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/bitmap$(DependSuffix)
	$(RM) $(IntermediateDirectory)/bitmap$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/color$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/color$(DependSuffix)
	$(RM) $(IntermediateDirectory)/color$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/font$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/font$(DependSuffix)
	$(RM) $(IntermediateDirectory)/font$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_actors$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_actors$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_actors$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_map$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_map$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_map$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_party$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_party$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_party$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_player$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_player$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_player$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_screen$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_screen$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_screen$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_switches$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_switches$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_switches$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_system$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_system$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_system$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_temp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_temp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_temp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_troop$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_troop$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_troop$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/game_variables$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/game_variables$(DependSuffix)
	$(RM) $(IntermediateDirectory)/game_variables$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/graphics$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/graphics$(DependSuffix)
	$(RM) $(IntermediateDirectory)/graphics$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/input$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/input$(DependSuffix)
	$(RM) $(IntermediateDirectory)/input$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/plane$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/plane$(DependSuffix)
	$(RM) $(IntermediateDirectory)/plane$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rect$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rect$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rect$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/scene_title$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/scene_title$(DependSuffix)
	$(RM) $(IntermediateDirectory)/scene_title$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(DependSuffix)
	$(RM) $(IntermediateDirectory)/sprite$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/table$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/table$(DependSuffix)
	$(RM) $(IntermediateDirectory)/table$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/tilemap$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/tilemap$(DependSuffix)
	$(RM) $(IntermediateDirectory)/tilemap$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/tone$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/tone$(DependSuffix)
	$(RM) $(IntermediateDirectory)/tone$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/viewport$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/viewport$(DependSuffix)
	$(RM) $(IntermediateDirectory)/viewport$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/window$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/window$(DependSuffix)
	$(RM) $(IntermediateDirectory)/window$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/zobj$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/zobj$(DependSuffix)
	$(RM) $(IntermediateDirectory)/zobj$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rpg_actor$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rpg_actor$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rpg_actor$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rpg_skill$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rpg_skill$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rpg_skill$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/RPG$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/RPG$(DependSuffix)
	$(RM) $(IntermediateDirectory)/RPG$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/actor$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/actor$(DependSuffix)
	$(RM) $(IntermediateDirectory)/actor$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldb_reader$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldb_reader$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldb_reader$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/skill$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/skill$(DependSuffix)
	$(RM) $(IntermediateDirectory)/skill$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/item$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/item$(DependSuffix)
	$(RM) $(IntermediateDirectory)/item$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/enemy$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/enemy$(DependSuffix)
	$(RM) $(IntermediateDirectory)/enemy$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/e_action$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/e_action$(DependSuffix)
	$(RM) $(IntermediateDirectory)/e_action$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/troop$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/troop$(DependSuffix)
	$(RM) $(IntermediateDirectory)/troop$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/tools$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/tools$(DependSuffix)
	$(RM) $(IntermediateDirectory)/tools$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldb_data$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ldbstr$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(DependSuffix)
	$(RM) $(IntermediateDirectory)/stevent$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe

-include $(IntermediateDirectory)/*$(DependSuffix)


