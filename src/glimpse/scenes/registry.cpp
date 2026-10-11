#include "glimpse/scenes/registry.h"

#include "glimpse/scenes/factories.h"

namespace glimpse {

const std::vector<SceneEntry>& sceneRegistry() {
  static const std::vector<SceneEntry> registry = {
      {"directions_test", directions_test},
      {"two_diffuse_spheres", two_diffuse_spheres},
      {"two_spheres_check", two_spheres_check},
      {"material_showcase", material_showcase},
      {"untitled_1", untitled_1},
      {"earth", earth},
      {"two_perlin_spheres", two_perlin_spheres},
      {"quads", quads},
      {"simple_light", simple_light},
      {"cornell_box", cornell_box},
      {"random_scene", random_scene},
      {"cornell_smoke", cornell_smoke},
      {"final_scene", final_scene},
  };
  return registry;
}

const SceneEntry* findScene(std::string_view name) {
  for (const auto& entry : sceneRegistry())
    if (name == entry.name) return &entry;
  return nullptr;
}

}  // namespace glimpse
