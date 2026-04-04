#include "schema_registry.h"
#include "schemas/wotlk/schema_achievement.h"
#include "schemas/wotlk/schema_achievement_category.h"
#include "schemas/wotlk/schema_achievement_criteria.h"
#include "schemas/wotlk/schema_animation_data.h"
#include "schemas/wotlk/schema_area_group.h"
#include "schemas/wotlk/schema_area_poi.h"
#include "schemas/wotlk/schema_area_table.h"
#include "schemas/wotlk/schema_area_trigger.h"
#include "schemas/wotlk/schema_attack_anim_kits.h"
#include "schemas/wotlk/schema_attack_anim_types.h"
#include "schemas/wotlk/schema_auction_house.h"
#include "schemas/wotlk/schema_bank_bag_slot_prices.h"
#include "schemas/wotlk/schema_banned_add_ons.h"
#include "schemas/wotlk/schema_barber_shop_style.h"
#include "schemas/wotlk/schema_battlemaster_list.h"
#include "schemas/wotlk/schema_camera_shakes.h"
#include "schemas/wotlk/schema_cfg_categories.h"
#include "schemas/wotlk/schema_char_base_info.h"
#include "schemas/wotlk/schema_cfg_configs.h"
#include "schemas/wotlk/schema_char_hair_geosets.h"
#include "schemas/wotlk/schema_char_start_outfit.h"
#include "schemas/wotlk/schema_char_hair_textures.h"
#include "schemas/wotlk/schema_char_sections.h"
#include "schemas/wotlk/schema_char_titles.h"
#include "schemas/wotlk/schema_char_variations.h"
#include "schemas/wotlk/schema_character_facial_hair_styles.h"
#include "schemas/wotlk/schema_chat_channels.h"
#include "schemas/wotlk/schema_chat_profanity.h"
#include "schemas/wotlk/schema_chr_classes.h"
#include "schemas/wotlk/schema_chr_races.h"
#include "schemas/wotlk/schema_cinematic_camera.h"
#include "schemas/wotlk/schema_cinematic_sequences.h"
#include "schemas/wotlk/schema_creature_display_info.h"
#include "schemas/wotlk/schema_creature_display_info_extra.h"
#include "schemas/wotlk/schema_creature_family.h"
#include "schemas/wotlk/schema_creature_model_data.h"
#include "schemas/wotlk/schema_creature_movement_info.h"
#include "schemas/wotlk/schema_creature_sound_data.h"
#include "schemas/wotlk/schema_creature_spell_data.h"
#include "schemas/wotlk/schema_creature_type.h"
#include "schemas/wotlk/schema_currency_category.h"
#include "schemas/wotlk/schema_currency_types.h"
#include "schemas/wotlk/schema_dance_moves.h"
#include "schemas/wotlk/schema_death_thud_lookups.h"
#include "schemas/wotlk/schema_declined_word.h"
#include "schemas/wotlk/schema_declined_word_cases.h"
#include "schemas/wotlk/schema_destructible_model_data.h"
#include "schemas/wotlk/schema_dungeon_encounter.h"
#include "schemas/wotlk/schema_dungeon_map.h"
#include "schemas/wotlk/schema_dungeon_map_chunk.h"
#include "schemas/wotlk/schema_durability_costs.h"
#include "schemas/wotlk/schema_durability_quality.h"
#include "schemas/wotlk/schema_emotes.h"
#include "schemas/wotlk/schema_emotes_text.h"
#include "schemas/wotlk/schema_emotes_text_data.h"
#include "schemas/wotlk/schema_emotes_text_sound.h"
#include "schemas/wotlk/schema_environmental_damage.h"
#include "schemas/wotlk/schema_exhaustion.h"
#include "schemas/wotlk/schema_faction.h"
#include "schemas/wotlk/schema_faction_group.h"
#include "schemas/wotlk/schema_faction_template.h"
#include "schemas/wotlk/schema_file_data.h"
#include "schemas/wotlk/schema_footprint_textures.h"
#include "schemas/wotlk/schema_footstep_terrain_lookup.h"
#include "schemas/wotlk/schema_game_object_art_kit.h"
#include "schemas/wotlk/schema_game_object_display_info.h"
#include "schemas/wotlk/schema_game_tables.h"
#include "schemas/wotlk/schema_game_tips.h"
#include "schemas/wotlk/schema_gem_properties.h"
#include "schemas/wotlk/schema_glyph_properties.h"
#include "schemas/wotlk/schema_glyph_slot.h"
#include "schemas/wotlk/schema_gm_survey_answers.h"
#include "schemas/wotlk/schema_gm_survey_current_survey.h"
#include "schemas/wotlk/schema_gm_survey_questions.h"
#include "schemas/wotlk/schema_gm_survey_surveys.h"
#include "schemas/wotlk/schema_gm_ticket_category.h"
#include "schemas/wotlk/schema_ground_effect_doodad.h"
#include "schemas/wotlk/schema_ground_effect_texture.h"
#include "schemas/wotlk/schema_gt_barbershop_cost_base.h"
#include "schemas/wotlk/schema_gt_chance_to_melee_crit.h"
#include "schemas/wotlk/schema_gt_chance_to_melee_crit_base.h"
#include "schemas/wotlk/schema_gt_chance_to_spell_crit.h"
#include "schemas/wotlk/schema_gt_chance_to_spell_crit_base.h"
#include "schemas/wotlk/schema_gt_combat_ratings.h"
#include "schemas/wotlk/schema_gt_npc_mana_cost_scaler.h"
#include "schemas/wotlk/schema_gt_oct_class_combat_rating_scalar.h"
#include "schemas/wotlk/schema_gt_oct_regen_hp.h"
#include "schemas/wotlk/schema_gt_oct_regen_mp.h"
#include "schemas/wotlk/schema_gt_regen_hp_per_spt.h"
#include "schemas/wotlk/schema_gt_regen_mp_per_spt.h"
#include "schemas/wotlk/schema_helmet_geoset_vis_data.h"
#include "schemas/wotlk/schema_holiday_descriptions.h"
#include "schemas/wotlk/schema_holiday_names.h"
#include "schemas/wotlk/schema_holidays.h"
#include "schemas/wotlk/schema_item.h"
#include "schemas/wotlk/schema_item_bag_family.h"
#include "schemas/wotlk/schema_item_class.h"
#include "schemas/wotlk/schema_item_cond_ext_costs.h"
#include "schemas/wotlk/schema_item_display_info.h"
#include "schemas/wotlk/schema_item_extended_cost.h"
#include "schemas/wotlk/schema_item_group_sounds.h"
#include "schemas/wotlk/schema_item_limit_category.h"
#include "schemas/wotlk/schema_item_pet_food.h"
#include "schemas/wotlk/schema_item_purchase_group.h"
#include "schemas/wotlk/schema_item_random_properties.h"
#include "schemas/wotlk/schema_item_random_suffix.h"
#include "schemas/wotlk/schema_item_set.h"
#include "schemas/wotlk/schema_item_sub_class.h"
#include "schemas/wotlk/schema_item_sub_class_mask.h"
#include "schemas/wotlk/schema_item_visual_effects.h"
#include "schemas/wotlk/schema_item_visuals.h"
#include "schemas/wotlk/schema_language_words.h"
#include "schemas/wotlk/schema_languages.h"
#include "schemas/wotlk/schema_lfg_dungeon_expansion.h"
#include "schemas/wotlk/schema_lfg_dungeon_group.h"
#include "schemas/wotlk/schema_lfg_dungeons.h"
#include "schemas/wotlk/schema_light.h"
#include "schemas/wotlk/schema_light_float_band.h"
#include "schemas/wotlk/schema_light_int_band.h"
#include "schemas/wotlk/schema_light_params.h"
#include "schemas/wotlk/schema_light_skybox.h"
#include "schemas/wotlk/schema_liquid_material.h"
#include "schemas/wotlk/schema_liquid_type.h"
#include "schemas/wotlk/schema_loading_screen_taxi_splines.h"
#include "schemas/wotlk/schema_loading_screens.h"
#include "schemas/wotlk/schema_lock.h"
#include "schemas/wotlk/schema_lock_type.h"
#include "schemas/wotlk/schema_mail_template.h"
#include "schemas/wotlk/schema_map.h"
#include "schemas/wotlk/schema_map_difficulty.h"
#include "schemas/wotlk/schema_material.h"
#include "schemas/wotlk/schema_movie.h"
#include "schemas/wotlk/schema_movie_file_data.h"
#include "schemas/wotlk/schema_movie_variation.h"
#include "schemas/wotlk/schema_name_gen.h"
#include "schemas/wotlk/schema_names_profanity.h"
#include "schemas/wotlk/schema_names_reserved.h"
#include "schemas/wotlk/schema_npc_sounds.h"
#include "schemas/wotlk/schema_object_effect.h"
#include "schemas/wotlk/schema_object_effect_group.h"
#include "schemas/wotlk/schema_object_effect_modifier.h"
#include "schemas/wotlk/schema_object_effect_package.h"
#include "schemas/wotlk/schema_object_effect_package_elem.h"
#include "schemas/wotlk/schema_override_spell_data.h"
#include "schemas/wotlk/schema_package.h"
#include "schemas/wotlk/schema_page_text_material.h"
#include "schemas/wotlk/schema_paper_doll_item_frame.h"
#include "schemas/wotlk/schema_particle_color.h"
#include "schemas/wotlk/schema_pet_personality.h"
#include "schemas/wotlk/schema_petition_type.h"
#include "schemas/wotlk/schema_power_display.h"
#include "schemas/wotlk/schema_pvp_difficulty.h"
#include "schemas/wotlk/schema_quest_faction_reward.h"
#include "schemas/wotlk/schema_quest_info.h"
#include "schemas/wotlk/schema_quest_sort.h"
#include "schemas/wotlk/schema_quest_xp.h"
#include "schemas/wotlk/schema_rand_prop_points.h"
#include "schemas/wotlk/schema_resistances.h"
#include "schemas/wotlk/schema_scaling_stat_distribution.h"
#include "schemas/wotlk/schema_scaling_stat_values.h"
#include "schemas/wotlk/schema_screen_effect.h"
#include "schemas/wotlk/schema_server_messages.h"
#include "schemas/wotlk/schema_sheathe_sound_lookups.h"
#include "schemas/wotlk/schema_skill_costs_data.h"
#include "schemas/wotlk/schema_skill_line.h"
#include "schemas/wotlk/schema_skill_line_ability.h"
#include "schemas/wotlk/schema_skill_line_category.h"
#include "schemas/wotlk/schema_skill_race_class_info.h"
#include "schemas/wotlk/schema_skill_tiers.h"
#include "schemas/wotlk/schema_sound_ambience.h"
#include "schemas/wotlk/schema_sound_emitters.h"
#include "schemas/wotlk/schema_sound_entries.h"
#include "schemas/wotlk/schema_sound_entries_advanced.h"
#include "schemas/wotlk/schema_sound_filter.h"
#include "schemas/wotlk/schema_sound_filter_elem.h"
#include "schemas/wotlk/schema_sound_provider_preferences.h"
#include "schemas/wotlk/schema_sound_sample_preferences.h"
#include "schemas/wotlk/schema_sound_water_type.h"
#include "schemas/wotlk/schema_spam_messages.h"
#include "schemas/wotlk/schema_spell.h"
#include "schemas/wotlk/schema_spell_cast_times.h"
#include "schemas/wotlk/schema_spell_category.h"
#include "schemas/wotlk/schema_spell_chain_effects.h"
#include "schemas/wotlk/schema_spell_description_variables.h"
#include "schemas/wotlk/schema_spell_difficulty.h"
#include "schemas/wotlk/schema_spell_dispel_type.h"
#include "schemas/wotlk/schema_spell_duration.h"
#include "schemas/wotlk/schema_spell_effect_camera_shakes.h"
#include "schemas/wotlk/schema_spell_focus_object.h"
#include "schemas/wotlk/schema_spell_icon.h"
#include "schemas/wotlk/schema_spell_item_enchantment.h"
#include "schemas/wotlk/schema_spell_item_enchantment_condition.h"
#include "schemas/wotlk/schema_spell_mechanic.h"
#include "schemas/wotlk/schema_spell_missile.h"
#include "schemas/wotlk/schema_spell_missile_motion.h"
#include "schemas/wotlk/schema_spell_radius.h"
#include "schemas/wotlk/schema_spell_range.h"
#include "schemas/wotlk/schema_spell_rune_cost.h"
#include "schemas/wotlk/schema_spell_shapeshift_form.h"
#include "schemas/wotlk/schema_spell_visual.h"
#include "schemas/wotlk/schema_spell_visual_effect_name.h"
#include "schemas/wotlk/schema_spell_visual_kit.h"
#include "schemas/wotlk/schema_spell_visual_kit_area_model.h"
#include "schemas/wotlk/schema_spell_visual_kit_model_attach.h"
#include "schemas/wotlk/schema_spell_visual_precast_transitions.h"
#include "schemas/wotlk/schema_stable_slot_prices.h"
#include "schemas/wotlk/schema_startup_strings.h"
#include "schemas/wotlk/schema_stationery.h"
#include "schemas/wotlk/schema_string_lookups.h"
#include "schemas/wotlk/schema_summon_properties.h"
#include "schemas/wotlk/schema_talent.h"
#include "schemas/wotlk/schema_talent_tab.h"
#include "schemas/wotlk/schema_taxi_nodes.h"
#include "schemas/wotlk/schema_taxi_path.h"
#include "schemas/wotlk/schema_taxi_path_node.h"
#include "schemas/wotlk/schema_team_contribution_points.h"
#include "schemas/wotlk/schema_terrain_type.h"
#include "schemas/wotlk/schema_terrain_type_sounds.h"
#include "schemas/wotlk/schema_totem_category.h"
#include "schemas/wotlk/schema_transport_animation.h"
#include "schemas/wotlk/schema_transport_physics.h"
#include "schemas/wotlk/schema_transport_rotation.h"
#include "schemas/wotlk/schema_ui_sound_lookups.h"
#include "schemas/wotlk/schema_unit_blood.h"
#include "schemas/wotlk/schema_unit_blood_levels.h"
#include "schemas/wotlk/schema_vehicle.h"
#include "schemas/wotlk/schema_vehicle_seat.h"
#include "schemas/wotlk/schema_vehicle_ui_ind_seat.h"
#include "schemas/wotlk/schema_vehicle_ui_indicator.h"
#include "schemas/wotlk/schema_video_hardware.h"
#include "schemas/wotlk/schema_vocal_ui_sounds.h"
#include "schemas/wotlk/schema_weapon_impact_sounds.h"
#include "schemas/wotlk/schema_weapon_swing_sounds2.h"
#include "schemas/wotlk/schema_weather.h"
#include "schemas/wotlk/schema_wmo_area_table.h"
#include "schemas/wotlk/schema_world_chunk_sounds.h"
#include "schemas/wotlk/schema_world_map_area.h"
#include "schemas/wotlk/schema_world_map_continent.h"
#include "schemas/wotlk/schema_world_map_overlay.h"
#include "schemas/wotlk/schema_world_map_transforms.h"
#include "schemas/wotlk/schema_world_safe_locs.h"
#include "schemas/wotlk/schema_world_state_ui.h"
#include "schemas/wotlk/schema_world_state_zone_sounds.h"
#include "schemas/wotlk/schema_wow_error_strings.h"
#include "schemas/wotlk/schema_zone_intro_music_table.h"
#include "schemas/wotlk/schema_zone_music.h"
#include <cstring>

struct ExpansionSchemaEntry {
    Expansion expansion;
    const DbcSchema* schema;
};

static const ExpansionSchemaEntry all_schemas[] = {
    { Expansion::WotLK, &schema_achievement },
    { Expansion::WotLK, &schema_achievement_category },
    { Expansion::WotLK, &schema_achievement_criteria },
    { Expansion::WotLK, &schema_animation_data },
    { Expansion::WotLK, &schema_area_group },
    { Expansion::WotLK, &schema_area_poi },
    { Expansion::WotLK, &schema_area_table },
    { Expansion::WotLK, &schema_area_trigger },
    { Expansion::WotLK, &schema_attack_anim_kits },
    { Expansion::WotLK, &schema_attack_anim_types },
    { Expansion::WotLK, &schema_auction_house },
    { Expansion::WotLK, &schema_bank_bag_slot_prices },
    { Expansion::WotLK, &schema_banned_add_ons },
    { Expansion::WotLK, &schema_barber_shop_style },
    { Expansion::WotLK, &schema_battlemaster_list },
    { Expansion::WotLK, &schema_camera_shakes },
    { Expansion::WotLK, &schema_cfg_categories },
    { Expansion::WotLK, &schema_cfg_configs },
    { Expansion::WotLK, &schema_char_base_info },
    { Expansion::WotLK, &schema_char_hair_geosets },
    { Expansion::WotLK, &schema_char_hair_textures },
    { Expansion::WotLK, &schema_char_sections },
    { Expansion::WotLK, &schema_char_start_outfit },
    { Expansion::WotLK, &schema_char_titles },
    { Expansion::WotLK, &schema_char_variations },
    { Expansion::WotLK, &schema_character_facial_hair_styles },
    { Expansion::WotLK, &schema_chat_channels },
    { Expansion::WotLK, &schema_chat_profanity },
    { Expansion::WotLK, &schema_chr_classes },
    { Expansion::WotLK, &schema_chr_races },
    { Expansion::WotLK, &schema_cinematic_camera },
    { Expansion::WotLK, &schema_cinematic_sequences },
    { Expansion::WotLK, &schema_creature_display_info },
    { Expansion::WotLK, &schema_creature_display_info_extra },
    { Expansion::WotLK, &schema_creature_family },
    { Expansion::WotLK, &schema_creature_model_data },
    { Expansion::WotLK, &schema_creature_movement_info },
    { Expansion::WotLK, &schema_creature_sound_data },
    { Expansion::WotLK, &schema_creature_spell_data },
    { Expansion::WotLK, &schema_creature_type },
    { Expansion::WotLK, &schema_currency_category },
    { Expansion::WotLK, &schema_currency_types },
    { Expansion::WotLK, &schema_dance_moves },
    { Expansion::WotLK, &schema_death_thud_lookups },
    { Expansion::WotLK, &schema_declined_word },
    { Expansion::WotLK, &schema_declined_word_cases },
    { Expansion::WotLK, &schema_destructible_model_data },
    { Expansion::WotLK, &schema_dungeon_encounter },
    { Expansion::WotLK, &schema_dungeon_map },
    { Expansion::WotLK, &schema_dungeon_map_chunk },
    { Expansion::WotLK, &schema_durability_costs },
    { Expansion::WotLK, &schema_durability_quality },
    { Expansion::WotLK, &schema_emotes },
    { Expansion::WotLK, &schema_emotes_text },
    { Expansion::WotLK, &schema_emotes_text_data },
    { Expansion::WotLK, &schema_emotes_text_sound },
    { Expansion::WotLK, &schema_environmental_damage },
    { Expansion::WotLK, &schema_exhaustion },
    { Expansion::WotLK, &schema_faction },
    { Expansion::WotLK, &schema_faction_group },
    { Expansion::WotLK, &schema_faction_template },
    { Expansion::WotLK, &schema_file_data },
    { Expansion::WotLK, &schema_footprint_textures },
    { Expansion::WotLK, &schema_footstep_terrain_lookup },
    { Expansion::WotLK, &schema_game_object_art_kit },
    { Expansion::WotLK, &schema_game_object_display_info },
    { Expansion::WotLK, &schema_game_tables },
    { Expansion::WotLK, &schema_game_tips },
    { Expansion::WotLK, &schema_gem_properties },
    { Expansion::WotLK, &schema_glyph_properties },
    { Expansion::WotLK, &schema_glyph_slot },
    { Expansion::WotLK, &schema_gm_survey_answers },
    { Expansion::WotLK, &schema_gm_survey_current_survey },
    { Expansion::WotLK, &schema_gm_survey_questions },
    { Expansion::WotLK, &schema_gm_survey_surveys },
    { Expansion::WotLK, &schema_gm_ticket_category },
    { Expansion::WotLK, &schema_ground_effect_doodad },
    { Expansion::WotLK, &schema_ground_effect_texture },
    { Expansion::WotLK, &schema_gt_barbershop_cost_base },
    { Expansion::WotLK, &schema_gt_chance_to_melee_crit },
    { Expansion::WotLK, &schema_gt_chance_to_melee_crit_base },
    { Expansion::WotLK, &schema_gt_chance_to_spell_crit },
    { Expansion::WotLK, &schema_gt_chance_to_spell_crit_base },
    { Expansion::WotLK, &schema_gt_combat_ratings },
    { Expansion::WotLK, &schema_gt_npc_mana_cost_scaler },
    { Expansion::WotLK, &schema_gt_oct_class_combat_rating_scalar },
    { Expansion::WotLK, &schema_gt_oct_regen_hp },
    { Expansion::WotLK, &schema_gt_oct_regen_mp },
    { Expansion::WotLK, &schema_gt_regen_hp_per_spt },
    { Expansion::WotLK, &schema_gt_regen_mp_per_spt },
    { Expansion::WotLK, &schema_helmet_geoset_vis_data },
    { Expansion::WotLK, &schema_holiday_descriptions },
    { Expansion::WotLK, &schema_holiday_names },
    { Expansion::WotLK, &schema_holidays },
    { Expansion::WotLK, &schema_item },
    { Expansion::WotLK, &schema_item_bag_family },
    { Expansion::WotLK, &schema_item_class },
    { Expansion::WotLK, &schema_item_cond_ext_costs },
    { Expansion::WotLK, &schema_item_display_info },
    { Expansion::WotLK, &schema_item_extended_cost },
    { Expansion::WotLK, &schema_item_group_sounds },
    { Expansion::WotLK, &schema_item_limit_category },
    { Expansion::WotLK, &schema_item_pet_food },
    { Expansion::WotLK, &schema_item_purchase_group },
    { Expansion::WotLK, &schema_item_random_properties },
    { Expansion::WotLK, &schema_item_random_suffix },
    { Expansion::WotLK, &schema_item_set },
    { Expansion::WotLK, &schema_item_sub_class },
    { Expansion::WotLK, &schema_item_sub_class_mask },
    { Expansion::WotLK, &schema_item_visual_effects },
    { Expansion::WotLK, &schema_item_visuals },
    { Expansion::WotLK, &schema_language_words },
    { Expansion::WotLK, &schema_languages },
    { Expansion::WotLK, &schema_lfg_dungeon_expansion },
    { Expansion::WotLK, &schema_lfg_dungeon_group },
    { Expansion::WotLK, &schema_lfg_dungeons },
    { Expansion::WotLK, &schema_light },
    { Expansion::WotLK, &schema_light_float_band },
    { Expansion::WotLK, &schema_light_int_band },
    { Expansion::WotLK, &schema_light_params },
    { Expansion::WotLK, &schema_light_skybox },
    { Expansion::WotLK, &schema_liquid_material },
    { Expansion::WotLK, &schema_liquid_type },
    { Expansion::WotLK, &schema_loading_screen_taxi_splines },
    { Expansion::WotLK, &schema_loading_screens },
    { Expansion::WotLK, &schema_lock },
    { Expansion::WotLK, &schema_lock_type },
    { Expansion::WotLK, &schema_mail_template },
    { Expansion::WotLK, &schema_map },
    { Expansion::WotLK, &schema_map_difficulty },
    { Expansion::WotLK, &schema_material },
    { Expansion::WotLK, &schema_movie },
    { Expansion::WotLK, &schema_movie_file_data },
    { Expansion::WotLK, &schema_movie_variation },
    { Expansion::WotLK, &schema_name_gen },
    { Expansion::WotLK, &schema_names_profanity },
    { Expansion::WotLK, &schema_names_reserved },
    { Expansion::WotLK, &schema_npc_sounds },
    { Expansion::WotLK, &schema_object_effect },
    { Expansion::WotLK, &schema_object_effect_group },
    { Expansion::WotLK, &schema_object_effect_modifier },
    { Expansion::WotLK, &schema_object_effect_package },
    { Expansion::WotLK, &schema_object_effect_package_elem },
    { Expansion::WotLK, &schema_override_spell_data },
    { Expansion::WotLK, &schema_package },
    { Expansion::WotLK, &schema_page_text_material },
    { Expansion::WotLK, &schema_paper_doll_item_frame },
    { Expansion::WotLK, &schema_particle_color },
    { Expansion::WotLK, &schema_pet_personality },
    { Expansion::WotLK, &schema_petition_type },
    { Expansion::WotLK, &schema_power_display },
    { Expansion::WotLK, &schema_pvp_difficulty },
    { Expansion::WotLK, &schema_quest_faction_reward },
    { Expansion::WotLK, &schema_quest_info },
    { Expansion::WotLK, &schema_quest_sort },
    { Expansion::WotLK, &schema_quest_xp },
    { Expansion::WotLK, &schema_rand_prop_points },
    { Expansion::WotLK, &schema_resistances },
    { Expansion::WotLK, &schema_scaling_stat_distribution },
    { Expansion::WotLK, &schema_scaling_stat_values },
    { Expansion::WotLK, &schema_screen_effect },
    { Expansion::WotLK, &schema_server_messages },
    { Expansion::WotLK, &schema_sheathe_sound_lookups },
    { Expansion::WotLK, &schema_skill_costs_data },
    { Expansion::WotLK, &schema_skill_line },
    { Expansion::WotLK, &schema_skill_line_ability },
    { Expansion::WotLK, &schema_skill_line_category },
    { Expansion::WotLK, &schema_skill_race_class_info },
    { Expansion::WotLK, &schema_skill_tiers },
    { Expansion::WotLK, &schema_sound_ambience },
    { Expansion::WotLK, &schema_sound_emitters },
    { Expansion::WotLK, &schema_sound_entries },
    { Expansion::WotLK, &schema_sound_entries_advanced },
    { Expansion::WotLK, &schema_sound_filter },
    { Expansion::WotLK, &schema_sound_filter_elem },
    { Expansion::WotLK, &schema_sound_provider_preferences },
    { Expansion::WotLK, &schema_sound_sample_preferences },
    { Expansion::WotLK, &schema_sound_water_type },
    { Expansion::WotLK, &schema_spam_messages },
    { Expansion::WotLK, &schema_spell },
    { Expansion::WotLK, &schema_spell_cast_times },
    { Expansion::WotLK, &schema_spell_category },
    { Expansion::WotLK, &schema_spell_chain_effects },
    { Expansion::WotLK, &schema_spell_description_variables },
    { Expansion::WotLK, &schema_spell_difficulty },
    { Expansion::WotLK, &schema_spell_dispel_type },
    { Expansion::WotLK, &schema_spell_duration },
    { Expansion::WotLK, &schema_spell_effect_camera_shakes },
    { Expansion::WotLK, &schema_spell_focus_object },
    { Expansion::WotLK, &schema_spell_icon },
    { Expansion::WotLK, &schema_spell_item_enchantment },
    { Expansion::WotLK, &schema_spell_item_enchantment_condition },
    { Expansion::WotLK, &schema_spell_mechanic },
    { Expansion::WotLK, &schema_spell_missile },
    { Expansion::WotLK, &schema_spell_missile_motion },
    { Expansion::WotLK, &schema_spell_radius },
    { Expansion::WotLK, &schema_spell_range },
    { Expansion::WotLK, &schema_spell_rune_cost },
    { Expansion::WotLK, &schema_spell_shapeshift_form },
    { Expansion::WotLK, &schema_spell_visual },
    { Expansion::WotLK, &schema_spell_visual_effect_name },
    { Expansion::WotLK, &schema_spell_visual_kit },
    { Expansion::WotLK, &schema_spell_visual_kit_area_model },
    { Expansion::WotLK, &schema_spell_visual_kit_model_attach },
    { Expansion::WotLK, &schema_spell_visual_precast_transitions },
    { Expansion::WotLK, &schema_stable_slot_prices },
    { Expansion::WotLK, &schema_startup_strings },
    { Expansion::WotLK, &schema_stationery },
    { Expansion::WotLK, &schema_string_lookups },
    { Expansion::WotLK, &schema_summon_properties },
    { Expansion::WotLK, &schema_talent },
    { Expansion::WotLK, &schema_talent_tab },
    { Expansion::WotLK, &schema_taxi_nodes },
    { Expansion::WotLK, &schema_taxi_path },
    { Expansion::WotLK, &schema_taxi_path_node },
    { Expansion::WotLK, &schema_team_contribution_points },
    { Expansion::WotLK, &schema_terrain_type },
    { Expansion::WotLK, &schema_terrain_type_sounds },
    { Expansion::WotLK, &schema_totem_category },
    { Expansion::WotLK, &schema_transport_animation },
    { Expansion::WotLK, &schema_transport_physics },
    { Expansion::WotLK, &schema_transport_rotation },
    { Expansion::WotLK, &schema_ui_sound_lookups },
    { Expansion::WotLK, &schema_unit_blood },
    { Expansion::WotLK, &schema_unit_blood_levels },
    { Expansion::WotLK, &schema_vehicle },
    { Expansion::WotLK, &schema_vehicle_seat },
    { Expansion::WotLK, &schema_vehicle_ui_ind_seat },
    { Expansion::WotLK, &schema_vehicle_ui_indicator },
    { Expansion::WotLK, &schema_video_hardware },
    { Expansion::WotLK, &schema_vocal_ui_sounds },
    { Expansion::WotLK, &schema_weapon_impact_sounds },
    { Expansion::WotLK, &schema_weapon_swing_sounds2 },
    { Expansion::WotLK, &schema_weather },
    { Expansion::WotLK, &schema_wmo_area_table },
    { Expansion::WotLK, &schema_world_chunk_sounds },
    { Expansion::WotLK, &schema_world_map_area },
    { Expansion::WotLK, &schema_world_map_continent },
    { Expansion::WotLK, &schema_world_map_overlay },
    { Expansion::WotLK, &schema_world_map_transforms },
    { Expansion::WotLK, &schema_world_safe_locs },
    { Expansion::WotLK, &schema_world_state_ui },
    { Expansion::WotLK, &schema_world_state_zone_sounds },
    { Expansion::WotLK, &schema_wow_error_strings },
    { Expansion::WotLK, &schema_zone_intro_music_table },
    { Expansion::WotLK, &schema_zone_music },
};

static const uint32_t schema_count = sizeof(all_schemas) / sizeof(all_schemas[0]);

const DbcSchema* GetSchema(Expansion expansion, const char* dbc_name) {
    for (uint32_t i = 0; i < schema_count; i++) {
        if (all_schemas[i].expansion == expansion &&
            strcmp(all_schemas[i].schema->dbc_name, dbc_name) == 0) {
            return all_schemas[i].schema;
        }
    }
    return nullptr;
}

const DbcSchema* GetAllSchemas(Expansion expansion, uint32_t* count) {
    static const DbcSchema* buffer[256];
    uint32_t n = 0;
    for (uint32_t i = 0; i < schema_count && n < 256; i++) {
        if (all_schemas[i].expansion == expansion) {
            buffer[n++] = all_schemas[i].schema;
        }
    }
    if (count) {
        *count = n;
    }
    if (n > 0) {
        return buffer[0];
    }
    return nullptr;
}

const DbcSchema* GetSchema(const char* dbc_name) {
    return GetSchema(Expansion::WotLK, dbc_name);
}

const DbcSchema* GetAllSchemas(uint32_t* count) {
    return GetAllSchemas(Expansion::WotLK, count);
}
