#include "d/d_com_inf_game.h"
#include "f_op/f_op_scene.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "ui_shortcuts_api_3ds.h"
#include "ui_play_3ds.h"

static uint8_t equipped(int btn) {
  const uint8_t live = dComIfGp_getSelectItem(btn);
  if (live != 0xFF) return live;
  const uint8_t slot = dComIfGs_getSelectItem(btn);
  return slot == 0xFF ? 0xFF : dComIfGs_getItem(slot);
}

static bool inPlayScene() {
  for (node_class* node = g_fopScnTg_SceneList.mpHead; node; node = node->mpNextNode) {
    auto* tag = reinterpret_cast<create_tag_class*>(node);
    auto* scene = static_cast<scene_class*>(tag->mpTagData);
    if (scene && fpcM_GetName(scene) == fpcNm_PLAY_SCENE_e) return true;
  }
  return false;
}

extern "C" void n3dsCompassSample(void) {
  n3dsUiPublishItems(equipped(0), equipped(1), equipped(2));
  const bool play = inPlayScene();
  n3dsUiPublishInPlay(play ? 1 : 0);
  if (!play) {
    n3dsUiPublishHeading(0, 0);
    return;
  }
  fopAc_ac_c* player = dComIfGp_getPlayer(0);
  if (!player) return;
  n3dsUiPublishHeading(static_cast<int16_t>(player->shape_angle.y + 0x8000), 1);
}
