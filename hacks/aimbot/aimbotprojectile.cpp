#include "aimbot.hpp"

#include "../../math.hpp"
#include "../../gui/config.hpp"

// todo: add projectile projectile simulation and movement simulation.
void aimbot_projectile(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles) {
  if (!target_player)
    return;

  int bone = target_player->get_tf_class() == CLASS_ENGINEER ? 5 : 2;

  Vec3 diff = {
    target_player->get_bone_pos(bone).x - localplayer->get_shoot_pos().x,
    target_player->get_bone_pos(bone).y - localplayer->get_shoot_pos().y,
    target_player->get_bone_pos(bone).z - localplayer->get_shoot_pos().z
  };
  float yaw_hyp = sqrt((diff.x * diff.x) + (diff.y * diff.y));
  float pitch_angle = atan2(diff.z, yaw_hyp) * radpi;
  float yaw_angle   = atan2(diff.y, diff.x) * radpi;
  Vec3 view_angles = { -pitch_angle, yaw_angle, 0 };

  bool use_key = ((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key);
  if (use_key && weapon->can_primary_attack())
    user_cmd->view_angles = (view_angles - localplayer->get_punch_angles());

  if (use_key && config.aimbot.auto_shoot) {
    user_cmd->buttons |= IN_ATTACK;
  }
}
