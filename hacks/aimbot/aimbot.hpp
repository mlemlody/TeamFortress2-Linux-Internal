#ifndef AIMBOT_HPP
#define AIMBOT_HPP

#include "../../classes/player.hpp"
#include "../../classes/weapon.hpp"
#include "../../vec.hpp"

struct user_cmd;

inline static Player* target_player = nullptr;

bool is_player_visible(Player* localplayer, Player* entity, int bone);

void aimbot(user_cmd* user_cmd, Vec3 original_view_angles);

void aimbot_hitscan(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles);
void aimbot_projectile(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles);
void aimbot_melee(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles);

#endif
