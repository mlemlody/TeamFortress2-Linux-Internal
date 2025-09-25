#include "aimbot.hpp"

#include "../../math.hpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/engine_trace.hpp"
#include "../../interfaces/convar_system.hpp"

#include "../../gui/config.hpp"
#include "../../classes/player.hpp"

#include "../../entity_cache.hpp"
#include "../../print.hpp"

bool is_player_visible(Player* localplayer, Player* entity, int bone) {
  Vec3 target_pos = entity->get_bone_pos(bone);
  Vec3 start_pos  = localplayer->get_shoot_pos();

  struct ray_t ray = engine_trace->init_ray(&start_pos, &target_pos);
  struct trace_filter filter;
  engine_trace->init_trace_filter(&filter, localplayer);

  struct trace_t trace;
  engine_trace->trace_ray(&ray, 0x4200400b, &filter, &trace);

  if (trace.entity == entity || trace.fraction > 0.97f) {
    return true;
  }

  return false;
}

static int choose_best_bone(Player* localplayer, Player* player, Weapon* weapon) {
  int bone = player->get_tf_class() == CLASS_ENGINEER ? 5 : 2; // body default
  if (localplayer->get_tf_class() == CLASS_SNIPER) {
    if (localplayer->is_scoped() && player->get_health() > 50)
      bone = player->get_head_bone();
  } else if (localplayer->get_tf_class() == CLASS_SPY) {
    if (weapon->is_headshot_weapon())
      bone = player->get_head_bone();
  }
  return bone;
}

void aimbot(user_cmd* user_cmd, Vec3 original_view_angles) {
  if (!config.aimbot.master) {
    target_player = nullptr;
    return;
  }

  if (entity_cache[class_id::PLAYER].empty()) {
    target_player = nullptr;
    return;
  }

  Player* localplayer = entity_list->get_localplayer();
  if (!localplayer || localplayer->get_lifestate() != 1) {
    target_player = nullptr;
    return;
  }

  Weapon* weapon = localplayer->get_weapon();
  if (!weapon) {
    target_player = nullptr;
    return;
  }

  if (target_player && target_player->is_friend() && config.aimbot.ignore_friends)
    target_player = nullptr;

  bool friendlyfire = false;
  static Convar* friendlyfirevar = convar_system->find_var("mp_friendlyfire");
  if (friendlyfirevar && friendlyfirevar->get_int() != 0)
    friendlyfire = true;

  float smallest_fov_angle = __FLT_MAX__;
  float smallest_distance  = __FLT_MAX__;
  int   smallest_health    = INT_MAX;
  int   largest_health     = INT_MIN;

  for (unsigned int i = 0; i < entity_cache[class_id::PLAYER].size(); ++i) {
    Player* player = (Player*)entity_cache[class_id::PLAYER].at(i);
    if (!player || player == localplayer || player->is_dormant() || player->get_lifestate() != 1 || player->is_invulnerable())
      continue;
    if ((player->get_team() == localplayer->get_team()) && !friendlyfire)
      continue;
    if (config.aimbot.ignore_friends && player->is_friend())
      continue;

    int bone = choose_best_bone(localplayer, player, weapon);

    Vec3 diff = {
      player->get_bone_pos(bone).x - localplayer->get_shoot_pos().x,
      player->get_bone_pos(bone).y - localplayer->get_shoot_pos().y,
      player->get_bone_pos(bone).z - localplayer->get_shoot_pos().z
    };

    float yaw_hyp = sqrt((diff.x * diff.x) + (diff.y * diff.y));
    float pitch_angle = atan2(diff.z, yaw_hyp) * radpi;
    float yaw_angle   = atan2(diff.y, diff.x) * radpi;

    Vec3 view_angles = { -pitch_angle, yaw_angle, 0 };

    float x = remainderf(view_angles.x - original_view_angles.x, 360.0f);
    float y = remainderf(view_angles.y - original_view_angles.y, 360.0f);
    float clamped_x = x > 89.0f ? 89.0f : x < -89.0f ? -89.0f : x;
    float clamped_y = y > 180.0f ? 180.0f : y < -180.0f ? -180.0f : y;

    bool visible = is_player_visible(localplayer, player, bone);
    float fov = hypotf(clamped_x, clamped_y);
    float distance = distance_3d(localplayer->get_origin(), player->get_origin());
    int   health = player->get_health();

    if (config.aimbot.target_type == Aim::TargetType::FOV) {
      if (visible && fov <= config.aimbot.fov && fov < smallest_fov_angle) {
        target_player = player;
        smallest_fov_angle = fov;
      }
    } else if (config.aimbot.target_type == Aim::TargetType::DISTANCE) {
      if (visible && fov <= config.aimbot.fov && distance < smallest_distance) {
        target_player = player;
        smallest_fov_angle = fov;
        smallest_distance = distance;
      }
    } else if (config.aimbot.target_type == Aim::TargetType::LEAST_HEALTH) {
      if (visible && fov <= config.aimbot.fov && health < smallest_health) {
        target_player = player;
        smallest_fov_angle = fov;
        smallest_health = health;
      }
    } else if (config.aimbot.target_type == Aim::TargetType::MOST_HEALTH) {
      if (visible && fov <= config.aimbot.fov && health > largest_health) {
        target_player = player;
        smallest_fov_angle = fov;
        largest_health = health;
      }
    }

    if (target_player == player && (fov > config.aimbot.fov || !visible))
      target_player = nullptr;
  }

  if (!target_player)
    return;

  if (weapon->is_melee()) {
    aimbot_melee(user_cmd, localplayer, weapon, original_view_angles);
  } else if (weapon->is_projectile()) {
    aimbot_projectile(user_cmd, localplayer, weapon, original_view_angles);
  } else {
    aimbot_hitscan(user_cmd, localplayer, weapon, original_view_angles);
  }
}
