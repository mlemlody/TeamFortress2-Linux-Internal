#include "aimbot.hpp"

#include "../../math.hpp"
#include "../../gui/config.hpp"

static float get_melee_range(Weapon* weapon) {
  switch (weapon->get_def_id()) {

  case Demoman_t_TheEyelander:
  case Demoman_t_TheClaidheamhMor:
  case Demoman_t_ThePersianPersuader:
  case Demoman_t_TheHalfZatoichi:
  case Demoman_t_TheScotsmansSkullcutter:
  case Demoman_t_HorselessHeadlessHorsemannsHeadtaker:
  case Demoman_t_NessiesNineIron:
    return 120.0f;
  }
  return 84.0f;
}

void aimbot_melee(user_cmd* user_cmd, Player* localplayer, Weapon* weapon, Vec3 original_view_angles) {
  if (!target_player)
    return;

  int bone = target_player->get_tf_class() == CLASS_ENGINEER ? 5 : 2;
  Vec3 target_pos = target_player->get_bone_pos(bone);
  Vec3 eye = localplayer->get_shoot_pos();
  Vec3 diff = { target_pos.x - eye.x, target_pos.y - eye.y, target_pos.z - eye.z };
  float yaw_hyp = sqrt((diff.x * diff.x) + (diff.y * diff.y));
  float pitch_angle = atan2(diff.z, yaw_hyp) * radpi;
  float yaw_angle   = atan2(diff.y, diff.x) * radpi;
  Vec3 view_angles = { -pitch_angle, yaw_angle, 0 };

  float range = get_melee_range(weapon);
  float dist_to_target = distance_3d(eye, target_pos);
  if (dist_to_target > range + 10.0f) // buffer accounts for anim desync/latency
    return;


  if (!is_player_visible(localplayer, target_player, bone))
    return;

  bool use_key = ((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key);
  if (use_key && weapon->can_primary_attack()) {
    user_cmd->view_angles = (view_angles - localplayer->get_punch_angles());
    if (config.aimbot.auto_shoot)
      user_cmd->buttons |= IN_ATTACK;
  }
}
