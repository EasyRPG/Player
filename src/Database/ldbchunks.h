#ifndef LDBC_H
#define LDBC_H

enum eLDBChunks
{
	CHUNK_Hero_data= 0x0B,
	CHUNK_Skill= 0x0C,
	CHUNK_Item_data= 0x0D,
	CHUNK_Monster= 0x0E,
	CHUNK_MonsterP=0x0F,
	CHUNK_Terrain= 0x10,
	CHUNK_Attribute= 0x11,
	CHUNK_States= 0x12,
	CHUNK_Animation= 0x13,
	CHUNK_Tileset= 0x14,
	CHUNK_String= 0x15,
	CHUNK_System= 0x16,
	CHUNK_Switch= 0x17,
	CHUNK_Variable= 0x18,
	CHUNK_Event= 0x19,
	CHUNK_Event1= 0x1A,//Eventos espesiales
	CHUNK_Event2= 0x1B,//Eventos espesiales
	CHUNK_Event3= 0x1C,//Eventos espesiales
	CHUNK_Comand= 0x1D,//Comandos de combate
	CHUNK_Profession= 0x1E,//Profession
	CHUNK_Profession2= 0x1F,//Profession
	CHUNK_Fightanim= 0x20,// Fighting animation 2
	CHUNK_LDB_END_OF_BLOCK =0x00
};
enum eLDBHeroChunks
{
	CHUNK_Name =0x01,
	CHUNK_Class =0x02,
	CHUNK_Graphicfile =0x03,
	CHUNK_Graphicindex =0x04,
	CHUNK_Transparent =0x05,
	CHUNK_Startlevel =0x07,
	CHUNK_Maxlevel =0x08,
	CHUNK_Crithit =0x09,
	CHUNK_Hitchance =0x0A,
	CHUNK_Facegraphic =0x0F,
	CHUNK_Faceindex =0x10,
	CHUNK_Dualwield =0x15,
	CHUNK_Fixedequipment =0x16,
	CHUNK_AI =0x17,
	CHUNK_Highdefense =0x18,
	CHUNK_Statisticscurves =0x1F,
	CHUNK_EXPBaseline =0x29,
	CHUNK_EXPAdditional =0x2A,
	CHUNK_EXPCorrection =0x2B,
	CHUNK_Startequip =0x33,
	CHUNK_Skills =0x3F,
	CHUNK_RenameMagic =0x42,
	CHUNK_Magicname =0x43,
	CHUNK_Condlength =0x47,
	CHUNK_Condeffects =0x48,
	CHUNK_Attriblength =0x49,
	CHUNK_Attribeffects =0x4A,
	CHUNK_Combat_Command=0x50
};
enum eLDBMagicblockChunks
{
	MagicblockChunk_Level=0x01,
	MagicblockChunk_Spell_ID=0x02
};
enum eLDBSkillChunks
{
	SkillChunk_Name =0x01,
	SkillChunk_Description =0x02,
	SkillChunk_text =0x03,
	SkillChunk_textline=0x04,
	SkillChunk_Failure =0x07,
	SkillChunk_Spelltype =0x08,
	SkillChunk_Cost =0x0B,
	SkillChunk_Range =0x0C,
	SkillChunk_Switch =0x0D,
	SkillChunk_Battleanimation =0x0E,
	SkillChunk_Soundeffect =0x10,
	SkillChunk_Fieldusage =0x12,
	SkillChunk_Combatusage =0x13,
	SkillChunk_Strengtheffect =0x15,
	SkillChunk_Mindeffect =0x16,
	SkillChunk_Variance =0x17,
	SkillChunk_Basevalue =0x18,
	SkillChunk_Successrate =0x19,
	SkillChunk_AffectHP =0x1F,
	SkillChunk_AffectMP =0x20,
	SkillChunk_AffectAttack =0x21,
	SkillChunk_AffectDefense =0x22,
	SkillChunk_AffectMind =0x23,
	SkillChunk_AffectAgility =0x24,
	SkillChunk_Absorbdamage =0x25,
	SkillChunk_Ignoredefense =0x26,
	SkillChunk_Conditionslength =0x29,
	SkillChunk_Changecondition =0x2A,
	SkillChunk_Attributeslength =0x2B,
	SkillChunk_Attackattribute =0x2C,
	SkillChunk_Affectresistance =0x2D
};
enum eLDBItemChunks
{
	ItemChunk_Name =0x01,
	ItemChunk_Description =0x02,
	ItemChunk_Type =0x03,
	ItemChunk_Cost =0x05,
	ItemChunk_Uses =0x06,
	ItemChunk_Attack  =0x0B,
	ItemChunk_Defense  =0x0C,
	ItemChunk_Mind  =0x0D,
	ItemChunk_Speed  =0x0E,
	ItemChunk_Equip  =0x0F,
	ItemChunk_MPcost =0x10,
	ItemChunk_Chancetohit  =0x11,
	ItemChunk_Criticalhit  =0x12,
	ItemChunk_Battleanimation =0x14,
	ItemChunk_Preemptiveattack =0x15,
	ItemChunk_Doubleattack =0x16,
	ItemChunk_Attackallenemies =0x17,
	ItemChunk_Ignoreevasion =0x18,
	ItemChunk_Preventcriticalhits =0x19,
	ItemChunk_Raiseevasion =0x1A,
	ItemChunk_MPusecutinhalf =0x1B,
	ItemChunk_Noterraindamage =0x1C,
	ItemChunk_Healsparty =0x1F,
	ItemChunk_HPrecovery =0x20,
	ItemChunk_HPrecoveryvalue =0x21,
	ItemChunk_MPrecovery =0x22,
	ItemChunk_MPrecoveryvalue =0x23,
	ItemChunk_Useonfieldonly =0x25,
	ItemChunk_Onlyondeadheros =0x26,
	ItemChunk_MaxHPmodify =0x29,
	ItemChunk_MaxMPmodify =0x2A,
	ItemChunk_Attackmodify =0x2B,
	ItemChunk_Defensemodify =0x2C,
	ItemChunk_Mindmodify =0x2D,
	ItemChunk_Speedmodify =0x2E,
	ItemChunk_Usagemessage =0x33,
	ItemChunk_Switchtoturnon =0x37,
	ItemChunk_Useonfield =0x39,
	ItemChunk_Useinbattle =0x3A,
	ItemChunk_Heroeslength =0x3D,
	ItemChunk_Heroescanuse =0x3E,
	ItemChunk_Conditionslength =0x3F,
	ItemChunk_Conditionchanges =0x40,
	ItemChunk_Attributeslength =0x41,
	ItemChunk_Attributes =0x42,
	ItemChunk_Chancetochange =0x43
};

enum eLDBMonsterChunks
{
	MonsterChunk_Name = 0x01,
	MonsterChunk_Graphicfile= 0x02,
	MonsterChunk_Huealteration= 0x03,
	MonsterChunk_MaxHP= 0x04,
	MonsterChunk_MaxMP= 0x05,
	MonsterChunk_Attack= 0x06,
	MonsterChunk_Defense= 0x07,
	MonsterChunk_Mind= 0x08,
	MonsterChunk_Speed= 0x09,
	MonsterChunk_Translucentgraphic= 0x0A,
	MonsterChunk_Experience= 0x0B,
	MonsterChunk_Gold= 0x0C,
	MonsterChunk_SpoilsitemID= 0x0D,
	MonsterChunk_Spoilschance= 0x0E,
	MonsterChunk_Canusecriticalhits= 0x15,
	MonsterChunk_Criticalhitchance= 0x16,
	MonsterChunk_Usuallymiss= 0x1A,
	MonsterChunk_Airborne= 0x1C,
	MonsterChunk_Conditionslength= 0x1F,
	MonsterChunk_Conditionseffects= 0x20,
	MonsterChunk_Attributeslength= 0x21,
	MonsterChunk_Attributeseffect= 0x22,
	MonsterChunk_Actionslist= 0x2A
};
enum eLDBMonsterActionsChunks
{
	MonsterActionsChunk_Action=0x01,
	MonsterActionsChunk_Action_data=0x02,
	MonsterActionsChunk_Skill_ID=0x03,
	MonsterActionsChunk_Monster_ID=0x04,
	MonsterActionsChunk_Condition=0x05,
	MonsterActionsChunk_Lower_limit=0x06,
	MonsterActionsChunk_Upper_limit=0x07,
	MonsterActionsChunk_Priority=0x0D
};
enum eLDBMonsterPartyChunks
{
	MonsterPartyChunk_Name =0x01,
	MonsterPartyChunk_Monsterdata=0x02,
	MonsterPartyChunk_Terrainlength=0x04,
	MonsterPartyChunk_Terraindata=0x05,
	MonsterPartyChunk_eventpages=0x0B
};
enum eLDBMonsterPartyMonsterChunks
{
	Monster_ID=0x01,
	X_position=0x02,
	Y_position=0x03
};
enum eLDBMonsterPartyevent_pageChunks
{
	Page_conditions=0x02,
	Event_length=0x0B,
	Event_Monster=0x0C
};

enum eLDBMonsterPartyEventconditionChunks
{
	Condition_flags=0x01,
	Switch_A=0x02,
	Turn_number_A=0x06,
	Lower_limit=0x0B,
	Upper_limit=0x0C
};
enum eLDBTerrainChunks
{
	TerrainChunk_Name=0x01,
	TerrainChunk_Damageontravel=0x02,
	TerrainChunk_Encounterate=0x03,
	TerrainChunk_Battlebackground=0x04,
	TerrainChunk_Skiffmaypass=0x05,
	TerrainChunk_Boatmaypass=0x06,
	TerrainChunk_Airshipmaypass=0x07,
	TerrainChunk_Airshipmayland=0x09,
	TerrainChunk_Heroopacity=0x0B
};
enum eLDBAttributeChunks
{
	AttributeChunk_Name=0x01,
	AttributeChunk_Type=0x02,
	AttributeChunk_A_damage=0x0B,
	AttributeChunk_B_damage=0x0C,
	AttributeChunk_C_damage=0x0D,
	AttributeChunk_D_damage=0x0E,
	AttributeChunk_E_damage=0x0F
};
enum eLDBStatesChunks
{
	StatesChunk_Name=0x01,
	StatesChunk_Length=0x02,
	StatesChunk_Color=0x03,
	StatesChunk_Priority=0x04,
	StatesChunk_Limitation=0x05,
	StatesChunk_A_chance=0x0B,
	StatesChunk_B_chance=0x0C,
	StatesChunk_C_chance=0x0D,
	StatesChunk_D_chance=0x0E,
	StatesChunk_E_chance=0x0F,
	StatesChunk_Turnsforhealing=0x15,
	StatesChunk_Healperturn=0x16,
	StatesChunk_Healonshock=0x17,
	StatesChunk_HalveAttack=0x1F,
	StatesChunk_HalveDefense=0x20,
	StatesChunk_HalveMind=0x21,
	StatesChunk_HalveAgility=0x22,
	StatesChunk_Hitratechange=0x23,
	StatesChunk_Preventskilluse=0x29,
	StatesChunk_Minimumskilllevel=0x2A,
	StatesChunk_Preventmagicuse=0x2B,
	StatesChunk_Minimummindlevel=0x2C,
	StatesChunk_Allyenterstate=0x33,
	StatesChunk_Enemyentersstate=0x34,
	StatesChunk_Alreadyinstate=0x35,
	StatesChunk_Affectedbystate=0x36,
	StatesChunk_Statusrecovered=0x37,
	StatesChunk_HPloss=0x3D,
	StatesChunk_HPlossvalue=0x3E,
	StatesChunk_HPmaploss=0x3F,
	StatesChunk_HPmapsteps=0x40,
	StatesChunk_MPloss=0x41,
	StatesChunk_MPlossvalue=0x42,
	StatesChunk_MPmaploss=0x43,
	StatesChunk_MPmapsteps=0x44
};
enum eLDBAnimationChunks
{
	AnimationChunk_Name=0x01,
	AnimationChunk_Animation_file=0x02,
	AnimationChunk_Timing_data=0x06,
	AnimationChunk_Apply_to=0x09,
	AnimationChunk_Y_coordinate_line=0x0A,
	AnimationChunk_Framedata=0x0C
};
enum eLDBAnimationTimingChunks
{
	Frame=0x01,
	Sound_effect=0x02,
	Flash_effect=0x03,
	Green_component=0x05,
	Blue_component=0x06,
	Flash_power=0x07
};
enum eLDBAnimationCellChunks
{
	Cell_data=0x01
};
enum eLDBAnimationCelldataChunks
{
	AnimationCelldata_Cell_source=0x02,
	AnimationCelldata_X_location=0x03,
	AnimationCelldata_Y_location=0x04,
	AnimationCelldata_Magnification=0x05,
	AnimationCelldata_Red_component=0x06,
	AnimationCelldata_Green_component=0x07,
	AnimationCelldata_Blue_component=0x08,
	AnimationCelldata_Chroma_component=0x09,
	AnimationCelldata_Alpha=0x0A
};
enum eLDBTilesetChunks
{
	TilesetChunk_Name=0x01,
	TilesetChunk_Graphic=0x02,
	TilesetChunk_Lower_tile_terrain=0x03,
	TilesetChunk_Lower_tile_passable=0x04,
	TilesetChunk_Upper_tile_passable=0x05,
	TilesetChunk_Water_animation=0x0B,
	TilesetChunk_Animation_speed=0x0C
};

enum eLDBStringdatChunks
{
	Enemy_encounter=0x01,
	Headstart_attack=0x02,
	Escape_success=0x03,
	Escape_failure=0x04,
	Battle_victory=0x05,
	Battle_defeat=0x06,
	Experience_received=0x07,
	Money_recieved_A=0x08,
	Money_recieved_B=0x09,
	Item_recieved=0x0A,
	Attack_message=0x0B,
	Ally_critical_hit=0x0C,
	Enemy_critical_hit=0x0D,
	Defend_message=0x0E,
	Watch_message=0x0F,
	Gathering_energy=0x10,
	Sacrificial_attack=0x11,
	Enemy_escape=0x12,
	Enemy_transform=0x13,
	Enemy_damaged=0x14,
	Enemy_undamaged=0x15,
	Ally_damaged=0x16,
	Ally_undamaged=0x17,
	Skill_failure_A=0x18,
	Skill_failure_B=0x19,
	Skill_failure_C=0x1A,
	Attack_dodged=0x1B,
	Item_use=0x1C,
	Stat_recovery=0x1D,
	Stat_increase=0x1E,
	Stat_decrease=0x1F,
	Ally_lost_via_absorb=0x20,
	Enemy_lost_via_absorb=0x21,
	Resistance_increase=0x22,
	Resistance_decrease=0x23,
	Level_up_message=0x24,
	Skill_learned=0x25,
	Shop_greeting =0x29,
	Shop_regreeting =0x2A,
	Buy_message =0x2B,
	Sell_message =0x2C,
	Leave_message =0x2D,
	Buying_message =0x2E,
	Quantity_to_buy =0x2F,
	Purchase_end =0x30,
	Selling_message =0x31,
	Quantity_to_sell =0x32,
	Selling_end =0x33,
	Shop_greeting_2 =0x36,
	Shop_regreeting_2 =0x37,
	Buy_message_2 =0x38,
	Sell_message_2 =0x39,
	Leave_message_2 =0x3A,
	Buying_message_2 =0x3B,
	Quantity_to_buy_2 =0x3C,
	Purchase_end_2 =0x3D,
	Selling_message_2 =0x3E,
	Quantity_to_sell_2 =0x3F,
	Selling_end_2 =0x40,
	Shop_greeting_3 =0x43,
	Shop_regreeting_3 =0x44,
	Buy_message_3 =0x45,
	Sell_message_3 =0x46,
	Leave_message_3 =0x47,
	Buying_message_3 =0x48,
	Quantity_to_buy_3 =0x49,
	Purchase_end_3 =0x4A,
	Selling_message_3 =0x4B,
	Quantity_to_sell_3 =0x4C,
	Selling_end_3 =0x4D,
	Inn_A_Greeting_A=0x50,
	Inn_A_Greeting_B=0x51,
	Inn_A_Greeting_C=0x52,
	Inn_A_Accept=0x53,
	Inn_A_Cancel=0x54,
	Inn_B_Greeting_A=0x55,
	Inn_B_Greeting_B=0x56,
	Inn_B_Greeting_C=0x57,
	Inn_B_Accept=0x58,
	Inn_B_Cancel=0x59,
	Loose_items=0x5C,
	Equipped_items=0x5D,
	Monetary_Unit=0x5F,
	Combat_Command=0x65,
	Combat_Auto=0x66,
	Combat_Run=0x67,
	Command_Attack=0x68,
	Command_Defend=0x69,
	Command_Item=0x6A,
	Command_Skill=0x6B,
	Menu_Equipment=0x6C,
	Menu_Save=0x6E,
	Menu_Quit=0x70,
	New_Game=0x72,
	Load_Game=0x73,
	Exit_to_Windows=0x75,
	Level=0x7B,
	Health=0x7C,
	Mana=0x7D,
	Normal_status=0x7E,
	Experience=0x7F,//(short)
	Level_short=0x80,//(short)
	Health_short=0x81,//(short)
	Mana_short=0x82,//(short)
	Mana_cost=0x83,
	Attack=0x84,
	Defense=0x85,
	Mind=0x86,
	Agility=0x87,
	Weapon=0x88,
	Shield=0x89,
	Armor=0x8A,
	Helmet=0x8B,
	Accessory=0x8C,
	Save_game_message=0x92,
	Load_game_message=0x93,
	Exit_game_message=0x94,
	File_name=0x97,
	General_Yes=0x98,
	General_No=0x99
};
enum eLDBCommon_EventChunks
{
	Common_EventChunk_Name=0x01,
	Common_EventChunk_NameActivation_condition=0x0B,
	Common_EventChunk_NameActivate_on_switch=0x0C,
	Common_EventChunk_NameSwitch_ID=0x0D,
	Common_EventChunk_NameScript_length=0x15,
	Common_EventChunk_NameScript=0x16
};
enum eLDBSystem_blockChunks
{       LDB_ID=0x0A,//exclusivo 2003
	Skiff_graphic=0x0B,
	Boat_graphic=0x0C,
	Airship_graphic=0x0D,
	Skiff_index=0x0E,
	Boat_index=0x0F,
	Airship_index=0x10,
	Title_graphic=0x11,
	Game_Over_graphic=0x12,
	System_graphic=0x13,
	System_graphic_2=0x14,//exclusivo 2003
	Heroes_in_starting=0x15,
	Starting_party=0x16,
	Num_Comadns_order=0x1A,// exclusivo 2003
        Comadns_order=0x1B,// exclusivo 2003 array 2 bytes
	Title_music=0x1F,
	Battle_music=0x20,
    	Battle_end_music=0x21,
	Inn_music=0x22,
	Skiff_music=0x23,
	Boat_music=0x24,
	Airship_music=0x25,
	Game_Over_music=0x26,
	Cursor_SFX=0x29,
	Accept_SFX=0x2A,
	Cancel_SFX=0x2B,
	Illegal_SFX=0x2C,
	Battle_SFX=0x2D,
	Escape_SFX=0x2E,
	Enemy_attack_SFX=0x2F,
	Enemy_damaged_SFX=0x30,
	Ally_damaged_SFX=0x31,
	Evasion_SFX=0x32,
	Enemy_dead_SFX=0x33,
	Item_use_SFX=0x34,
	Map_exit_transition=0x3D,
	Map_enter_transition=0x3E,
	Battle_start_fadeout=0x3F,
	Battle_start_fadein=0x40,
	Battle_end_fadeout=0x41,
	Battle_end_fadein=0x42,
        Message_background=0x47,//nuevo
        Font_id=0x48,//nuevo
	Selected_condition=0x51,
	Selected_hero=0x52,
	Battle_test_BG=0x54,
	Battle_test_data=0x55,
        Times_saved=0x5B,//nuevo
        //0x5E,  //no identificados
        //0x60,   //no identificados
        //0x61,//no identificados
        Show_frame=0x63,	// exclusivo 2003
        In_battle_anim=0x65	// exclusivo 2003
};

enum eLDBCBattle_test_Chunks
{
    	Battle_test_Chunks_Perso_ID=0x01,
	Battle_test_Chunks_Level=0x02,
	Battle_test_Chunks_Weapon=0x0B,
	Battle_test_Chunks_Shield=0x0C,
	Battle_test_Chunks_Armor=0x0D,
	Battle_test_Chunks_Helmet=0x0E,
	Battle_test_Chunks_Accessory=0x0F
};

// eventos exclusivos del 2003

// eventos mismo que los del 2000

enum eLDBCombatcommand_Chunks//array bidimencional
{
	Combatcommand_Chunk_Name=0x01,
	Combatcommand_Chunk_Usage=0x02
};
enum eLDBAnimated_battle_comand_Chunks//array bidimencional
{
	Animated_battle_Name=0x01,
	Animated_battle_Motion_attack=0x02,
	Animated_battle_Combat_Anime=0x0A,
	Animated_battle_Anime_combat_weapons=0x0B
};
enum eLDBAnimated_battle_Chunks //array bidimensional
{
	Name=0x01,
	Filename=0x02,
	Position=0x03,
	Extended_expanded=0x04,
	ID_Expansion_animated_combat_ID=0x05
};
enum eLDBProfetion_Chunks //array bidimensional
{
	Profetion_Name=0x01,
	Profetion_TwoWeapon=0x15,
	Profetion_fixed_equipment=0x16,
	Profetion_AI_forced_action=0x17,
	Profetion_strong_defense=0x18,
	Profetion_Each_level=0x1F,
	Profetion_Experience_curve_basic_values=0x29,
	Profetion_Experience_curve_increase_degree=0x2A,
	Profetion_Experience_curve_correction_value=0x2B,
	Profetion_Animated_battle=0x3E,
	Profetion_Special_skills_level=0x3F,
	Profetion_Effectiveness_state_number=0x47,
	Profetion_Effectiveness_state_data=0x48,
	Profetion_Effectiveness_Attribute_number=0x49,
	Profetion_Effectiveness_Attribute_data=0x4A,
	Profetion_Combat_Command=0x50
};
enum eLDBProfetionSkill_Chunks //array bidimensional
{
	ProfetionSkill_Master_level=0x01,
	ProfetionSkill_Master_a_special_skill=0x02
};
#endif