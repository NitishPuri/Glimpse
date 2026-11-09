// The scene table. Its order is the CLI's --scene index and the viewer's combo order: append new scenes at the end.

#include "glimpse/scenes/registry.h"

#include <algorithm>

#include "glimpse/scenes/factories.h"

namespace glimpse {

const std::vector<SceneEntry>& sceneRegistry() {
  static const std::vector<SceneEntry> registry = {
      {"directions_test", directionsTest},
      {"two_diffuse_spheres", twoDiffuseSpheres},
      {"two_spheres_check", twoSpheresCheck},
      {"material_showcase", materialShowcase},
      {"untitled_1", untitled1},
      {"earth", earth},
      {"two_perlin_spheres", twoPerlinSpheres},
      {"quads", quads},
      {"simple_light", simpleLight},
      {"cornell_box", cornellBox},
      {"random_scene", randomScene},
      {"cornell_smoke", cornellSmoke},
      {"final_scene", finalScene},
  };
  return registry;
}

const SceneEntry* findScene(std::string_view name) {
  const auto it = std::ranges::find(sceneRegistry(), name, &SceneEntry::name);
  return it != sceneRegistry().end() ? &*it : nullptr;
}

}  // namespace glimpse
