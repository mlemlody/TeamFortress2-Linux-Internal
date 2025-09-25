#include "aimbot.hpp"

#include "../../math.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/convar_system.hpp"
#include "../../gui/config.hpp"

void aimbot_hitscan(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles) {
  if (!target_player)
    return;

  int bone = target_player->get_tf_class() == CLASS_ENGINEER ? 5 : 2;
  if (localplayer->get_tf_class() == CLASS_SNIPER) {
    if (localplayer->is_scoped() && target_player->get_health() > 50)
      bone = target_player->get_head_bone();
  } else if (localplayer->get_tf_class() == CLASS_SPY) {
    if (weapon->is_headshot_weapon())
      bone = target_player->get_head_bone();
  }

  Vec3 diff = {
    target_player->get_bone_pos(bone).x - localplayer->get_shoot_pos().x,
    target_player->get_bone_pos(bone).y - localplayer->get_shoot_pos().y,
    target_player->get_bone_pos(bone).z - localplayer->get_shoot_pos().z
  };
  float yaw_hyp = sqrt((diff.x * diff.x) + (diff.y * diff.y));
  float pitch_angle = atan2(diff.z, yaw_hyp) * radpi;
  float yaw_angle   = atan2(diff.y, diff.x) * radpi;
  Vec3 view_angles = { -pitch_angle, yaw_angle, 0 };

  bool scoped_only = ((config.aimbot.scoped_only && weapon->is_sniper_rifle() && localplayer->is_scoped()) || !config.aimbot.scoped_only || !weapon->is_sniper_rifle());
  bool use_key = ((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key);

  if (config.aimbot.auto_unscope && (localplayer->get_tickbase() * TICK_INTERVAL) - localplayer->get_fov_time() >= 1 && localplayer->get_tf_class() == CLASS_SNIPER && localplayer->is_scoped()) {
    user_cmd->buttons |= IN_ATTACK2;
  }

  if (use_key && weapon->can_primary_attack() && scoped_only)
    user_cmd->view_angles = (view_angles - localplayer->get_punch_angles());

  if (use_key && config.aimbot.auto_shoot && localplayer->can_shoot(target_player)) {
    if (config.aimbot.auto_scope && localplayer->get_tf_class() == CLASS_SNIPER && weapon->is_sniper_rifle() && !localplayer->is_scoped() && weapon->can_primary_attack() && localplayer->get_ground_entity() != nullptr)
      user_cmd->buttons |= IN_ATTACK2;
    if (!(user_cmd->buttons & IN_ATTACK2) && scoped_only)
      user_cmd->buttons |= IN_ATTACK;
  }
}
