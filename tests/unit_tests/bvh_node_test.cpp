#include "core/hittables/bvh_node.h"

#include "core/hittables/sphere.h"
#include "core/material.h"

//
#include "../test_cfg.h"

using namespace glimpse;

void bvh_test() {
  using namespace boost::ut;

  "bvh_node"_test = [] {
    "construction"_test = [] {
      // Create a simple list of hittables
      hittable_list list;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));

      // Add spheres at different positions
      list.add(make_shared<sphere>(point3(0, 0, 0), 1.0, mat));
      list.add(make_shared<sphere>(point3(4, 0, 0), 1.0, mat));
      list.add(make_shared<sphere>(point3(0, 4, 0), 1.0, mat));
      list.add(make_shared<sphere>(point3(0, 0, 4), 1.0, mat));

      // Construct BVH from list
      bvh_node bvh(list);

      // Check that the bounding box encapsulates all objects
      auto bbox = bvh.bounding_box();
      expect(bbox.x.min <= -1.0_d);
      expect(bbox.y.min <= -1.0_d);
      expect(bbox.z.min <= -1.0_d);
      expect(bbox.x.max >= 5.0_d);
      expect(bbox.y.max >= 5.0_d);
      expect(bbox.z.max >= 5.0_d);
    };

    "hit_detection"_test = [] {
      // Create a list with two separated spheres
      hittable_list list;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      list.add(make_shared<sphere>(point3(-2, 0, 0), 1.0, mat));
      list.add(make_shared<sphere>(point3(2, 0, 0), 1.0, mat));

      bvh_node bvh(list);

      // Test ray that hits first sphere
      ray r1(point3(-10, 0, 0), vec3(1, 0, 0));
      hit_record rec1;
      expect(bvh.hit(r1, interval(0.001, glimpse::math::infinity), rec1) == true);
      expect(rec1.p.x() == -3.0_d);

      // Test ray that hits second sphere
      ray r2(point3(10, 0, 0), vec3(-1, 0, 0));
      hit_record rec2;
      expect(bvh.hit(r2, interval(0.001, glimpse::math::infinity), rec2) == true);
      expect(rec2.p.x() == 3.0_d);

      // Test ray that misses all spheres
      ray r3(point3(0, 10, 0), vec3(0, -1, 0));
      hit_record rec3;
      expect(bvh.hit(r3, interval(0.001, glimpse::math::infinity), rec3) == false);
    };

    skip / "empty_list"_test = [] {
      // Test with empty list
      hittable_list empty_list;
      bvh_node empty_bvh(empty_list);

      ray r(point3(0, 0, 0), vec3(1, 0, 0));
      hit_record rec;
      std::cout << "empty_list 1" << std::endl;
      expect(empty_bvh.hit(r, interval(0.001, glimpse::math::infinity), rec) == false);
      std::cout << "empty_list 2" << std::endl;
    };

    "single_object"_test = [] {
      std::cout << "sinle_object" << std::endl;
      // Test with single object
      hittable_list single_list;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      single_list.add(make_shared<sphere>(point3(0, 0, 0), 1.0, mat));

      bvh_node single_bvh(single_list);

      // Ray that hits
      ray r1(point3(0, 0, -5), vec3(0, 0, 1));
      hit_record rec1;
      expect(single_bvh.hit(r1, interval(0.001, glimpse::math::infinity), rec1) == true);

      // Ray that misses
      ray r2(point3(0, 0, -5), vec3(0, 1, 0));
      hit_record rec2;
      expect(single_bvh.hit(r2, interval(0.001, glimpse::math::infinity), rec2) == false);
    };

    "box_comparator"_test = [] {
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      auto sphere1 = make_shared<sphere>(point3(0, 0, 0), 1.0, mat);
      auto sphere2 = make_shared<sphere>(point3(3, 0, 0), 1.0, mat);

      // Test private methods indirectly through a derived class
      struct test_bvh_node : public bvh_node {
        test_bvh_node() : bvh_node(hittable_list()) {}

        static bool test_box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
          return box_x_compare(a, b);
        }

        static bool test_box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
          return box_y_compare(a, b);
        }

        static bool test_box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
          return box_z_compare(a, b);
        }
      };

      // Test x-axis comparator
      expect(test_bvh_node::test_box_x_compare(sphere1, sphere2) == true);
      expect(test_bvh_node::test_box_x_compare(sphere2, sphere1) == false);

      // Test with spheres at same x but different y
      auto sphere3 = make_shared<sphere>(point3(0, 3, 0), 1.0, mat);
      expect(test_bvh_node::test_box_y_compare(sphere1, sphere3) == true);
      expect(test_bvh_node::test_box_y_compare(sphere3, sphere1) == false);

      // Test with spheres at same x/y but different z
      auto sphere4 = make_shared<sphere>(point3(0, 0, 3), 1.0, mat);
      expect(test_bvh_node::test_box_z_compare(sphere1, sphere4) == true);
      expect(test_bvh_node::test_box_z_compare(sphere4, sphere1) == false);
    };
  };
}