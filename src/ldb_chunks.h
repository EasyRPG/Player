//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef _LDB_READER_CHUNKS_H_
#define _LDB_READER_CHUNKS_H_

////////////////////////////////////////////////////////////
/// LDB Reader namespace
////////////////////////////////////////////////////////////
namespace LDB_Reader {
    namespace ChunkData {
        enum ChunkData {
            Actor            = 0x0B, // RPG::Actor
            Skill            = 0x0C, // RPG::Skill
            Item             = 0x0D, // RPG::Item
            Enemy            = 0x0E, // RPG::Enemy
            Troop            = 0x0F, // RPG::Troop
            Terrain          = 0x10, // RPG::Terrain
            Attribute        = 0x11, // RPG::Attribute
            State            = 0x12, // RPG::State
            Animation        = 0x13, // RPG::Animation
            Chipset          = 0x14, // RPG::Chipset
            Terms            = 0x15, // RPG::Terms
            System           = 0x16, // RPG::System
            Switches         = 0x17, // RPG::Switchs
            Variables        = 0x18, // RPG::Variables
            CommonEvent      = 0x19, // RPG::CommonEvent
            BattleCommand    = 0x1D, // RPG::BattleCommand - RPG2003
            Class            = 0x1E, // RPG::Class - RPG2003
            BattlerAnimation = 0x20, // RPG::BattlerAnimation - RPG2003

            CommonEventD1    = 0x1A, // Duplicated? - Not used - RPG2003
            CommonEventD2    = 0x1B, // Duplicated? - Not used - RPG2003
            CommonEventD3    = 0x1C, // Duplicated? - Not used - RPG2003
            ClassD1          = 0x1F, // Duplicated? - Not used - RPG2003
            
            END              = 0x00  // End of chunk
        };
    };
    namespace ChunkActor {
        enum ChunkActor {
            name                    = 0x01, // String
            title                   = 0x02, // String
            character_name          = 0x03, // String
            character_index         = 0x04, // Integer
            transparent             = 0x05, // Flag
            initial_level           = 0x07, // Integer
            final_level             = 0x08, // Integer
            critical_hit            = 0x09, // Flag
            critical_hit_chance     = 0x0A, // Integer
            face_name               = 0x0F, // String
            face_index              = 0x10, // Integer
            two_swords_style        = 0x15, // Flag
            fix_equipment           = 0x16, // Flag
            auto_battle             = 0x17, // Flag
            super_guard             = 0x18, // Flag
            parameters              = 0x1F, // Array x 6 - Short
            exp_base                = 0x29, // Integer
            exp_inflation           = 0x2A, // Integer
            exp_correction          = 0x2B, // Integer
            initial_equipment       = 0x33, // Integer x 5
            unarmed_animation       = 0x38, // Integer
            class_id                = 0x39, // Integer - RPG2003
            battler_animation       = 0x3E, // Integer - RPG2003
            skills                  = 0x3F, // Array - RPG::Learning
            rename_skill            = 0x42, // Flag
            skill_name              = 0x43, // String
            state_ranks_size        = 0x47, // Integer
            state_ranks             = 0x48, // Array - Short
            attribute_ranks_size    = 0x49, // Integer
            attribute_ranks         = 0x4A, // Array - Short
            battle_commands         = 0x50  // Array - RPG::BattleCommand - RPG2003
        };
    };
    namespace ChunkLearning {
        enum ChunkLearning {
            level       = 0x01, // Integer
            skill_id    = 0x02  // Integer
        };
    };
    namespace ChunkSkill {
        enum ChunkSkill {
            name                = 0x01, // String
            description         = 0x02, // String
            using_message1      = 0x03, // String - RPG2000
            using_message2      = 0x04, // String - RPG2000
            failure_message     = 0x07, // Integer - RPG2000
            type                = 0x08, // Integer
            sp_type             = 0x09, // Integer - RPG2003
            sp_percent          = 0x0A, // Integer - RPG2003
            sp_cost             = 0x0B, // Integer
            scope               = 0x0C, // Integer
            switch_id           = 0x0D, // Integer
            animation_id        = 0x0E, // Integer
            sound_effect        = 0x10, // RPG::Sound
            occasion_field      = 0x12, // Flag
            occasion_battle     = 0x13, // Flag
            state_effect        = 0x14, // Flag - RPG2003
            pdef_f              = 0x15, // Integer
            mdef_f              = 0x16, // Integer
            variance            = 0x17, // Integer
            power               = 0x18, // Integer
            hit                 = 0x19, // Integer
            affect_hp           = 0x1F, // Flag
            affect_sp           = 0x20, // Flag
            affect_attack       = 0x21, // Flag
            affect_defense      = 0x22, // Flag
            affect_spirit       = 0x23, // Flag
            affect_agility      = 0x24, // Flag
            absorb_damage       = 0x25, // Flag
            ignore_defense      = 0x26, // Flag
            state_size          = 0x29, // Integer
            state_effects       = 0x2A, // Array - Flag
            attribute_size      = 0x2B, // Integer
            attribute_effects   = 0x2C, // Array - Flag
            affect_attr_defence = 0x2D, // Flag
            battler_animation   = 0x31, // Integer - RPG2003
            cba_data            = 0x32  // ? - RPG2003
        };
    };
    namespace ChunkItem {
        enum ChunkItem {
            name                = 0x01, // String
            description         = 0x02, // String
            type                = 0x03, // Integer
            price               = 0x05, // Integer
            uses                = 0x06, // Integer
            atk_points1         = 0x0B, // Integer
            def_points1         = 0x0C, // Integer
            spi_points1         = 0x0D, // Integer
            agi_points1         = 0x0E, // Integer
            two_handed          = 0x0F, // Flag
            sp_cost             = 0x10, // Integer
            hit                 = 0x11, // Integer
            critical_hit        = 0x12, // Integer
            animation_id        = 0x14, // Integer
            preemptive          = 0x15, // Flag
            dual_attack         = 0x16, // Flag
            attack_all          = 0x17, // Flag
            ignore_evasion      = 0x18, // Flag
            prevent_critical    = 0x19, // Flag
            raise_evasion       = 0x1A, // Flag
            half_sp_cost        = 0x1B, // Flag
            no_terrain_damage   = 0x1C, // Flag
            cursed              = 0x1D, // Flag - RPG2003
            entire_party        = 0x1F, // Flag
            recover_hp          = 0x20, // Integer
            recover_hp_rate     = 0x21, // Integer
            recover_sp          = 0x22, // Integer
            recover_sp_rate     = 0x23, // Integer
            ocassion_field1     = 0x25, // Flag
            ko_only             = 0x26, // Flag
            max_hp_points       = 0x29, // Integer
            max_sp_points       = 0x2A, // Integer
            atk_points2         = 0x2B, // Integer
            def_points2         = 0x2C, // Integer
            spi_points2         = 0x2D, // Integer
            agi_points2         = 0x2E, // Integer
            using_messsage      = 0x33, // Integer
            skill_id            = 0x35, // Integer - RPG2003
            switch_id           = 0x37, // Integer
            ocassion_field2     = 0x39, // Flag
            ocassion_battle     = 0x3A, // Flag
            actor_set_size      = 0x3D, // Integer
            actor_set           = 0x3E, // Array - Flag
            state_set_size      = 0x3F, // Integer
            state_set           = 0x40, // Array - Flag
            attribute_set_size  = 0x41, // Integer
            attribute_set       = 0x42, // Array - Flag
            state_chance        = 0x43, // Integer
            weapon_animation    = 0x45, // Integer - RPG2003
            use_skill           = 0x47, // Flag - RPG2003
            class_set_size      = 0x48, // Integer - RPG2003
            class_set           = 0x49, // Array - Flag - RPG2003
        };
    };
    namespace ChunkEnemy {
        enum ChunkEnemy {
            name                 = 0x01, // String
            battler_name         = 0x02, // String
            battler_hue          = 0x03, // Integer
            max_hp               = 0x04, // Integer
            max_sp               = 0x05, // Integer
            attack               = 0x06, // Integer
            defense              = 0x07, // Integer
            spirit               = 0x08, // Integer
            agility              = 0x09, // Integer
            transparent          = 0x0A, // Flag
            exp                  = 0x0B, // Integer
            gold                 = 0x0C, // Integer
            drop_id              = 0x0D, // Integer
            drop_prob            = 0x0E, // Integer
            critical_hit         = 0x15, // Flag
            critical_hit_chance  = 0x16, // Integer
            miss                 = 0x1A, // Flag
            levitate             = 0x1C, // Flag
            state_ranks_size     = 0x1F, // Integer
            state_ranks          = 0x20, // Array - Short
            attribute_ranks_size = 0x21, // Integer
            attribute_ranks      = 0x22, // Array - Short
            actions              = 0x2A  // Array - RPG::EnemyAction
        };
    };
    namespace ChunkEnemyAction {
        enum ChunkEnemyAction {
            kind                = 0x01, // Integer
            basic               = 0x02, // Integer
            skill_id            = 0x03, // Integer
            enemy_id            = 0x04, // Integer
            condition_type      = 0x05, // Integer
            condition_param1    = 0x06, // Integer
            condition_param2    = 0x07, // Integer
            switch_id           = 0x08, // Integer
            switch_on           = 0x09, // Flag
            switch_on_id        = 0x0A, // Integer
            switch_off          = 0x0B, // Flag
            switch_off_id       = 0x0C, // Integer
            rating              = 0x0D  // Integer
        };
    };
    namespace ChunkTroop {
        enum ChunkTroop {
            name             = 0x01, // String
            members          = 0x02, // Array - RPG::TroopMember
            terrain_set_size = 0x04, // Integer
            terrain_set      = 0x05, // Array - Flag
            pages            = 0x0B  // Array - RPG::TroopPage
        };
    };
    namespace ChunkTroopMember {
        enum ChunkTroopMember {
            ID      = 0x01, // Integer
            x       = 0x02, // Integer
            y       = 0x03, // Integer
            middle  = 0x04  // Flag
        };
    };
    namespace ChunkTroopPage {
        enum ChunkTroopPage {
            condition           = 0x02, // RPG::TroopPageCondition
            event_commands_size = 0x0B, // Integer
            event_commands      = 0x0C  // Array - RPG::EventCommand
        };
    };
    namespace ChunkTroopPageCondition { // TODO - Get RPG2003 unknown chunks id
        enum ChunkTroopPageCondition {
            condition_flags   = 0x01, // Bitflag - x 2 if RPG2003
            switch_a_id       = 0x02, // Integer
            switch_b_id       = 0x03, // Integer
            variable_id       = 0x04, // Integer
            variable_value    = 0x05, // Integer
            turn_a            = 0x06, // Integer
            turn_b            = 0x07, // Integer
            fatigue_min       = 0x08, // Integer
            fatigue_max       = 0x09, // Integer
            enemy_id          = 0x0A, // Integer
            enemy_hp_min      = 0x0B, // Integer
            enemy_hp_max      = 0x0C, // Integer
            actor_id          = 0x0D, // Integer
            actor_hp_min      = 0x0E, // Integer
            actor_hp_max      = 0x0F  // Integer
            //turn_enemy_id     = 0x??, // Integer - RPG2003
            //turn_enemy_a      = 0x??, // Integer - RPG2003
            //turn_enemy_b      = 0x??, // Integer - RPG2003
            //turn_actor_id     = 0x??, // Integer - RPG2003
            //turn_actor_a      = 0x??, // Integer - RPG2003
            //turn_actor_b      = 0x??, // Integer - RPG2003
            //command_actor_id  = 0x??, // Integer - RPG2003
            //command_id        = 0x??  // Integer - RPG2003
        };
    };
    namespace ChunkTerrain { // TODO - Get RPG2003 unknown chunks id
        enum ChunkTerrain { 
            name                        = 0x01, // String
            damage                      = 0x02, // Integer
            encounter_rate              = 0x03, // Integer
            background_name             = 0x04, // String
            boat_pass                   = 0x05, // Flag
            ship_pass                   = 0x06, // Flag
            airship_pass                = 0x07, // Flag
            airship_land                = 0x09, // Flag
            bush_depth                  = 0x0B, // Integer
            footstep                    = 0x0F, // RPG::Sound - RPG2003
            on_damage_se                = 0x10, // Flag - RPG2003
            background_type             = 0x11, // Integer - RPG2003
            background_a_name           = 0x15, // String - RPG2003
            background_a_scrollh        = 0x16, // Flag - RPG2003
            background_a_scrollv        = 0x17, // Flag - RPG2003
            background_a_scrollh_speed  = 0x18, // Integer - RPG2003
            background_a_scrollv_speed  = 0x19, // Integer - RPG2003
            background_b                = 0x1E, // Flag - RPG2003
            background_b_name           = 0x1F, // String - RPG2003
            background_b_scrollh        = 0x20, // Flag - RPG2003
            background_b_scrollv        = 0x21, // Flag - RPG2003
            background_b_scrollh_speed  = 0x22, // Integer - RPG2003
            background_b_scrollv_speed  = 0x23, // Integer - RPG2003
            special_flags               = 0x28, // Bitflag - RPG2003
            special_back_party          = 0x29, // Integer - RPG2003
            special_back_enemies        = 0x2A, // Integer - RPG2003
            special_lateral_party       = 0x2B, // Integer - RPG2003
            special_lateral_enemies     = 0x2C, // Integer - RPG2003
            grid_location               = 0x2D, // Integer - RPG2003
            //grid_a                      = 0x??, // Integer - RPG2003
            //grid_b                      = 0x??, // Integer - RPG2003
            //grid_c                      = 0x??  // Integer - RPG2003
        };
    };
    namespace ChunkAttribute {
        enum ChunkAttribute {
            name    = 0x01, // String
            type    = 0x02, // Integer
            a_rate  = 0x0B, // Integer
            b_rate  = 0x0C, // Integer
            c_rate  = 0x0D, // Integer
            d_rate  = 0x0E, // Integer
            e_rate  = 0x0F  // Integer
        };
    };
    namespace ChunkState {
        enum ChunkState {
            name                    = 0x01, // String
            type                    = 0x02, // Integer
            color                   = 0x03, // Integer
            priority                = 0x04, // Integer
            restriction             = 0x05, // Integer
            a_rate                  = 0x0B, // Integer
            b_rate                  = 0x0C, // Integer
            c_rate                  = 0x0D, // Integer
            d_rate                  = 0x0E, // Integer
            e_rate                  = 0x0F, // Integer
            hold_turn               = 0x15, // Integer
            auto_release_prob       = 0x16, // Integer
            release_by_damage       = 0x17, // Integer
            affect_type             = 0x1E, // Integer - RPG2003
            affect_attack           = 0x1F, // Flag
            affect_defense          = 0x20, // Flag
            affect_spirit           = 0x21, // Flag
            affect_agility          = 0x22, // Flag
            reduce_hit_ratio        = 0x23, // Integer
            avoid_attacks           = 0x24, // Flag - RPG2003
            reflect_magic           = 0x25, // Flag - RPG2003
            cursed                  = 0x26, // Flag - RPG2003
            battler_animation_id    = 0x27, // Integer - RPG2003
            restrict_skill          = 0x29, // Flag
            restrict_skill_level    = 0x2A, // Integer
            restrict_magic          = 0x2B, // Flag
            restrict_magic_level    = 0x2C, // Integer
            hp_change_type          = 0x2D, // Integer
            sp_change_type          = 0x2E, // Integer
            message_actor           = 0x33, // String
            message_enemy           = 0x34, // String
            message_already         = 0x35, // String
            message_affected        = 0x36, // String
            message_recovery        = 0x37, // String
            hp_change_max           = 0x3D, // Integer
            hp_change_val           = 0x3E, // Integer
            hp_change_map_val       = 0x3F, // Integer
            hp_change_map_steps     = 0x40, // Integer
            sp_change_max           = 0x41, // Integer
            sp_change_val           = 0x42, // Integer
            sp_change_map_val       = 0x43, // Integer
            sp_change_map_steps     = 0x44  // Integer
        };
    };
    namespace ChunkAnimation {
        enum ChunkAnimation {
            name            = 0x01, // String
            animation_name  = 0x02, // String
            timings         = 0x06, // Array - RPG::AnimationTiming
            scope           = 0x09, // Integer
            position        = 0x0A, // Integer
            frames          = 0x0C  // Array - RPG::AnimationFrames
        };
    };
    namespace ChunkAnimationTiming {
        enum ChunkAnimationTiming {
            frame       = 0x01, // Integer
            se          = 0x02, // RPG::Sound
            flash_scope = 0x03, // Integer
            flash_red   = 0x04, // Integer
            flash_green = 0x05, // Integer
            flash_blue  = 0x06, // Integer
            flash_power = 0x07  // Integer
            //screen_shake = 0x?? // Integer - RPG2003 - TODO: Get chunk id
        };
    };
    namespace ChunkAnimationFrame {
        enum ChunkAnimationFrame {
            cells = 0x01 // Array - RPG::AnimationCellData
        };
    };
    namespace ChunkAnimationCellData {
        enum ChunkAnimationCellData {
            //priority    = 0x01, // Integer - TODO: Needs confirmation
            ID          = 0x02, // Integer
            x           = 0x03, // Integer
            y           = 0x04, // Integer
            zoom        = 0x05, // Integer
            tone_red    = 0x06, // Integer
            tone_green  = 0x07, // Integer
            tone_blue   = 0x08, // Integer
            tone_gray   = 0x09, // Integer
            opacity     = 0x0A  // Integer
        };
    };
    namespace ChunkChipset {
        enum ChunkChipset {
            name                = 0x01, // String
            chipset_name        = 0x02, // String
            terrain_data        = 0x03, // Array - Short x 162
            passable_data_lower = 0x04, // Array - Bitflag x 162
            passable_data_upper = 0x05, // Array - Bitflag x 144
            animation_type      = 0x0B, // Integer
            animation_speed     = 0x0C  // Integer
        };
    };
    namespace ChunkTerms {
        enum ChunkTerms {
            encounter           = 0x01, // String
            special_combat      = 0x02, // String
            escape_success      = 0x03, // String
            escape_failure      = 0x04, // String
            victory             = 0x05, // String
            defeat              = 0x06, // String
            exp_received        = 0x07, // String
            gold_recieved_a     = 0x08, // String
            gold_recieved_b     = 0x09, // String
            item_recieved       = 0x0A, // String
            attacking           = 0x0B, // String
            actor_critical      = 0x0C, // String
            enemy_critical      = 0x0D, // String
            defending           = 0x0E, // String
            observing           = 0x0F, // String
            focus               = 0x10, // String
            autodestruction     = 0x11, // String
            enemy_escape        = 0x12, // String
            enemy_transform     = 0x13, // String
            enemy_damaged       = 0x14, // String
            enemy_undamaged     = 0x15, // String
            actor_damaged       = 0x16, // String
            actor_undamaged     = 0x17, // String
            skill_failure_a     = 0x18, // String
            skill_failure_b     = 0x19, // String
            skill_failure_c     = 0x1A, // String
            dodge               = 0x1B, // String
            use_item            = 0x1C, // String
            hp_recovery         = 0x1D, // String
            parameter_increase  = 0x1E, // String
            parameter_decrease  = 0x1F, // String
            actor_hp_absorbed   = 0x20, // String
            enemy_hp_absorbed   = 0x21, // String
            resistance_increase = 0x22, // String
            resistance_decrease = 0x23, // String
            level_up            = 0x24, // String
            skill_learned       = 0x25, // String
            shop_greeting1      = 0x29, // String
            shop_regreeting1    = 0x2A, // String
            shop_buy1           = 0x2B, // String
            shop_sell1          = 0x2C, // String
            shop_leave1         = 0x2D, // String
            shop_buy_select1    = 0x2E, // String
            shop_buy_number1    = 0x2F, // String
            shop_purchased1     = 0x30, // String
            shop_sell_select1   = 0x31, // String
            shop_sell_number1   = 0x32, // String
            shop_sold1          = 0x33, // String
            shop_greeting2      = 0x36, // String
            shop_regreeting2    = 0x37, // String
            shop_buy2           = 0x38, // String
            shop_sell2          = 0x39, // String
            shop_leave2         = 0x3A, // String
            shop_buy_select2    = 0x3B, // String
            shop_buy_number2    = 0x3C, // String
            shop_purchased2     = 0x3D, // String
            shop_sell_select2   = 0x3E, // String
            shop_sell_number2   = 0x3F, // String
            shop_sold2          = 0x40, // String
            shop_greeting3      = 0x43, // String
            shop_regreeting3    = 0x44, // String
            shop_buy3           = 0x45, // String
            shop_sell3          = 0x46, // String
            shop_leave3         = 0x47, // String
            shop_buy_select3    = 0x48, // String
            shop_buy_number3    = 0x49, // String
            shop_purchased3     = 0x4A, // String
            shop_sell_select3   = 0x4B, // String
            shop_sell_number3   = 0x4C, // String
            shop_sold3          = 0x4D, // String
            inn_a_greeting_1    = 0x50, // String
            inn_a_greeting_2    = 0x51, // String
            inn_a_greeting_3    = 0x52, // String
            inn_a_accept        = 0x53, // String
            inn_a_cancel        = 0x54, // String
            inn_b_greeting_1    = 0x55, // String
            inn_b_greeting_2    = 0x56, // String
            inn_b_greeting_3    = 0x57, // String
            inn_b_accept        = 0x58, // String
            inn_b_cancel        = 0x59, // String
            possessed_items     = 0x5C, // String
            equipped_items      = 0x5D, // String
            gold                = 0x5F, // String
            battle_fight        = 0x65, // String
            battle_auto         = 0x66, // String
            battle_escape       = 0x67, // String
            command_attack      = 0x68, // String
            command_defend      = 0x69, // String
            command_item        = 0x6A, // String
            command_skill       = 0x6B, // String
            menu_equipment      = 0x6C, // String
            menu_save           = 0x6E, // String
            menu_quit           = 0x70, // String
            new_game            = 0x72, // String
            load_game           = 0x73, // String
            exit_game           = 0x75, // String
            level               = 0x7B, // String
            health_points       = 0x7C, // String
            spirit_points       = 0x7D, // String
            normal_status       = 0x7E, // String
            exp_short           = 0x7F, // String - char x 2?
            lvl_short           = 0x80, // String - char x 2?
            hp_short            = 0x81, // String - char x 2?
            sp_short            = 0x82, // String - char x 2?
            sp_cost             = 0x83, // String
            attack              = 0x84, // String
            defense             = 0x85, // String
            spirit              = 0x86, // String
            agility             = 0x87, // String
            weapon              = 0x88, // String
            shield              = 0x89, // String
            armor               = 0x8A, // String
            helmet              = 0x8B, // String
            accessory           = 0x8C, // String
            save_game_message   = 0x92, // String
            load_game_message   = 0x93, // String
            exit_game_message   = 0x94, // String
            file                = 0x97, // String
            yes                 = 0x98, // String
            no                  = 0x99  // String
        };
    };
    namespace ChunkCommonEvent {
        enum ChunkCommonEvent {
            name                = 0x01, // String
            trigger             = 0x0B, // Integer
            switch_flag         = 0x0C, // Flag
            switch_id           = 0x0D, // Integer
            event_commands_size = 0x15, // Integer
            event_commands      = 0x16  // Array - RPG::EventCommand
        };
    };
    namespace ChunkSystem {
        enum ChunkSystem {
            ldb_id                  = 0x0A, // Integer - RPG2003
            boat_name               = 0x0B, // String
            ship_name               = 0x0C, // String
            airship_name            = 0x0D, // String
            boat_index              = 0x0E, // Integer
            ship_index              = 0x0F, // Integer
            airship_index           = 0x10, // Integer
            title_name              = 0x11, // String
            gameover_name           = 0x12, // String
            system_name             = 0x13, // String
            system2_name            = 0x14, // String - RPG2003
            party_size              = 0x15, // Integer
            party                   = 0x16, // Array - Short
            menu_commands_size      = 0x1A, // Integer - RPG2003
            menu_commands           = 0x1B, // Array - Short - RPG2003
            title_music             = 0x1F, // RPG::Music
            battle_music            = 0x20, // RPG::Music
            battle_end_music        = 0x21, // RPG::Music
            inn_music               = 0x22, // RPG::Music
            boat_music              = 0x23, // RPG::Music
            ship_music              = 0x24, // RPG::Music
            airship_music           = 0x25, // RPG::Music
            gameover_music          = 0x26, // RPG::Music
            cursor_se               = 0x29, // RPG::Sound
            decision_se             = 0x2A, // RPG::Sound
            cancel_se               = 0x2B, // RPG::Sound
            buzzer_se               = 0x2C, // RPG::Sound
            battle_se               = 0x2D, // RPG::Sound
            escape_se               = 0x2E, // RPG::Sound
            enemy_attack_se         = 0x2F, // RPG::Sound
            enemy_damaged_se        = 0x30, // RPG::Sound
            actor_damaged_se        = 0x31, // RPG::Sound
            dodge_se                = 0x32, // RPG::Sound
            enemy_death_se          = 0x33, // RPG::Sound
            item_se                 = 0x34, // RPG::Sound
            transition_out          = 0x3D, // Integer
            transition_in           = 0x3E, // Integer
            battle_start_fadeout    = 0x3F, // Integer
            battle_start_fadein     = 0x40, // Integer
            battle_end_fadeout      = 0x41, // Integer
            battle_end_fadein       = 0x42, // Integer
            message_stretch         = 0x47, // Integer
            font_id                 = 0x48, // Integer
            selected_condition      = 0x51, // Integer
            selected_hero           = 0x52, // Integer
            battletest_background   = 0x54, // String
            battletest_data         = 0x55, // Array - RPG::TestBattler
            saved_times             = 0x5B, // Integer
            //???                     = 0x5E, // ???
            //???                     = 0x60, // ???
            //???                     = 0x61, // ???
            show_frame              = 0x63, // Flag - RPG2003
            invert_animations       = 0x65  // Flag - RPG2003
        };
    };
    namespace ChunkTestBattler {
        enum ChunkTestBattler {
            ID              = 0x01, // Integer
            level           = 0x02, // Integer
            weapon_id       = 0x0B, // Integer
            shield_id       = 0x0C, // Integer
            armor_id        = 0x0D, // Integer
            helmet_id       = 0x0E, // Integer
            accessory_id    = 0x0F  // Integer
        };
    };
    namespace ChunkBattleCommand {
        enum ChunkBattleCommand {
            name = 0x01, // String
            type = 0x02  // Integer
        };
    };
    namespace ChunkBattlerAnimation {
        enum ChunkBattlerAnimation {
            name        = 0x01, // String
            speed       = 0x02, // Integer
            base_data   = 0x0A, // Array - RPG::BattlerAnimationExtension
            weapon_data = 0x0B  // Array - RPG::BattlerAnimationExtension
        };
    };
    namespace ChunkBattlerAnimationExtension {
        enum ChunkBattlerAnimationExtension {
            name            = 0x01, // String
            battler_name    = 0x02, // String
            battler_index   = 0x03, // Integer
            animation_type  = 0x04, // Integer
            animation_id    = 0x05  // Integer
        };
    };
    namespace ChunkClass {
        enum ChunkClass {
            name                  = 0x01, // String
            two_swords_style      = 0x15, // Flag
            fix_equipment         = 0x16, // Flag
            auto_battle           = 0x17, // Flag
            super_guard           = 0x18, // Flag
            parameters            = 0x1F, // Array x 6 - Short
            exp_base              = 0x29, // Integer
            exp_inflation         = 0x2A, // Integer
            exp_correction        = 0x2B, // Integer
            unarmed_animation     = 0x3E, // Integer
            skills                = 0x3F, // Array - RPG::Learning
            state_ranks_size      = 0x47, // Integer
            state_ranks           = 0x48, // Array - Short
            attribute_ranks_size  = 0x49, // Integer
            attribute_ranks       = 0x4A, // Array - Short
            battle_commands       = 0x50  // Array - Uint32
        };
    };
    namespace ChunkMusic {
        enum ChunkMusic {
            name    = 0x01, // String
            fadein  = 0x02, // Integer
            volume  = 0x03, // Integer
            tempo   = 0x04, // Integer
            balance = 0x05  // Integer
        };
    };
    namespace ChunkSound {
        enum ChunkSound {
            name    = 0x01, // String
            volume  = 0x03, // Integer
            tempo   = 0x04, // Integer
            balance = 0x05  // Integer
        };
    };
    namespace ChunkSwitch {
        enum ChunkSwitch {
            name    = 0x01 // String
        };
    };
    namespace ChunkVariable {
        enum ChunkVariable {
            name    = 0x01 // String
        };
    };
};

#endif
