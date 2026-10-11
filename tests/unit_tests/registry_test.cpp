#include "glimpse/scenes/registry.h"

#include <set>
#include <string>

//
#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"registry"> registry_suite = [] {
  using namespace boost::ut;

  "scene_registry"_test = [] {
    "names_are_unique"_test = [] {
      std::set<std::string> names;
      for (const auto& entry : sceneRegistry()) names.insert(entry.name);
      expect(names.size() == sceneRegistry().size());
    };

    // The CLI's --scene <index> and the viewer's combo both use this order; it must not change silently.
    "order_is_stable"_test = [] {
      const char* expected[] = {"directions_test", "two_diffuse_spheres", "two_spheres_check", "material_showcase",
                                "untitled_1",      "earth",               "two_perlin_spheres", "quads",
                                "simple_light",    "cornell_box",         "random_scene",       "cornell_smoke",
                                "final_scene"};
      expect(sceneRegistry().size() == std::size(expected));
      for (size_t i = 0; i < std::size(expected) && i < sceneRegistry().size(); ++i)
        expect(sceneRegistry()[i].name == std::string(expected[i])) << "index" << i;
    };

    "find_by_name"_test = [] {
      const SceneEntry* entry = findScene("cornell_box");
      expect(entry != nullptr);
      if (entry) expect(entry->make().cam.image_width == 600_i);
      expect(findScene("no_such_scene") == nullptr);
    };
  };
};
