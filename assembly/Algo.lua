_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}

--- from Assembly 'Algo'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.Algo = _G.EasyRPGPlayer.Assembly.Algo or {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.N3lcf3rpg4ItemE):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.GetNumberOfAttacks(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.SkillTargetsAllies(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.SkillTargetsEnemies(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.IsNormalOrSubskill(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.IsSkillUsable(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.N12Game_Battler6WeaponE, arg3: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg4: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcNormalAttackToHit(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcSkillHpCost(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcSelfDestructEffect(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE, arg3: EasyRPGPlayer.Assembly.bool, arg4: EasyRPGPlayer.Assembly.bool, arg5: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg6: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcSkillEffect(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.N12Game_Battler6WeaponE, arg3: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcCriticalHitChance(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.N12Game_Battler6WeaponE, arg3: EasyRPGPlayer.Assembly.bool, arg4: EasyRPGPlayer.Assembly.bool, arg5: EasyRPGPlayer.Assembly.bool, arg6: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg7: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcNormalAttackEffect(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.12Game_Battler, arg2: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE, arg3: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg4: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcSkillToHit(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE, arg1: EasyRPGPlayer.Assembly.int, arg2: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.CalcSkillCost(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.12Game_Battler):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.AdjustDamageForDefend(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg5SkillE):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.SkillTargetsOne(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.i, arg1: EasyRPGPlayer.Assembly.i):EasyRPGPlayer.Assembly.int
function EasyRPGPlayer.Assembly.Algo.VarianceAdjustEffect(...) end
---@overload fun(arg0: EasyRPGPlayer.Assembly.N3lcf3rpg9SaveActor7RowTypeE, arg1: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg2: EasyRPGPlayer.Assembly.b):EasyRPGPlayer.Assembly.bool
---@overload fun(arg0: EasyRPGPlayer.Assembly.12Game_Battler, arg1: EasyRPGPlayer.Assembly.N3lcf3rpg6System15BattleConditionE, arg2: EasyRPGPlayer.Assembly.bool, arg3: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
function EasyRPGPlayer.Assembly.Algo.IsRowAdjusted(...) end
