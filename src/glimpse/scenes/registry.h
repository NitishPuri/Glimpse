#pragma once

// sceneRegistry() / findScene(): the ordered list of every scene and a lookup by name.

#include <string_view>
#include <vector>

#include "glimpse/scene.h"

namespace glimpse {

struct SceneEntry {
  const char* name;
  Scene (*make)();
};

// Every scene, in one ordered table: the CLI's --scene <index>, --list-scenes and the viewer's combo all read
// this order, so new scenes go at the end.
const std::vector<SceneEntry>& sceneRegistry();

// nullptr if there is no scene with that name.
const SceneEntry* findScene(std::string_view name);

}  // namespace glimpse
