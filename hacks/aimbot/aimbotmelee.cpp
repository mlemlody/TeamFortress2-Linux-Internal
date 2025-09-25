#include "aimbot.hpp"

#include "../../math.hpp"
#include "../../gui/config.hpp"

static float get_melee_range(Weapon* weapon) {
  // Defaults: most stock melees ~48u; swords have extended reach ~70-75u.
  switch (weapon->get_def_id()) {
  // Sniper melees
  case Sniper_t_Kukri:
  case Sniper_t_KukriR:
  case Sniper_t_TheTribalmansShiv:
  case Sniper_t_TheBushwacka:
  case Sniper_t_TheShahanshah:
    return 48.0f;

  // Demoman swords (extended reach)
  case Demoman_t_TheEyelander:
  case Demoman_t_TheClaidheamhMor:
  case Demoman_t_ThePersianPersuader:
  case Demoman_t_TheHalfZatoichi:
  case Demoman_t_TheScotsmansSkullcutter:
  case Demoman_t_HorselessHeadlessHorsemannsHeadtaker:
  case Demoman_t_NessiesNineIron:
    return 70.0f;

  // Demoman other melees (stock/caber/train)
  case Demoman_t_Bottle:
  case Demoman_t_BottleR:
  case Demoman_t_UllapoolCaber:
  case Demoman_t_ThePainTrain:
    return 48.0f;
  }
  return 48.0f;
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
  if (dist_to_target > range + 4.0f)
    return;

  float x = remainderf(view_angles.x - original_view_angles.x, 360.0f);
  float y = remainderf(view_angles.y - original_view_angles.y, 360.0f);
  float clamped_x = x > 89.0f ? 89.0f : x < -89.0f ? -89.0f : x;
  float clamped_y = y > 180.0f ? 180.0f : y < -180.0f ? -180.0f : y;
  float fov = hypotf(clamped_x, clamped_y);
  if (fov > 60.0f)
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
