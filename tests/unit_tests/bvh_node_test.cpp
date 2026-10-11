#include "glimpse/aggregates.h"

#include "glimpse/shapes.h"
#include "glimpse/materials.h"

//
#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"bvh"> bvhSuite = [] {
  using namespace boost::ut;

  "bvh_node"_test = [] {
    "construction"_test = [] {
      // Create a simple list of hittables
      HittableList list;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));

      // Add spheres at different positions
      list.add(make_shared<Sphere>(Point3(0, 0, 0), 1.0, mat));
      list.add(make_shared<Sphere>(Point3(4, 0, 0), 1.0, mat));
      list.add(make_shared<Sphere>(Point3(0, 4, 0), 1.0, mat));
      list.add(make_shared<Sphere>(Point3(0, 0, 4), 1.0, mat));

      // Construct BVH from list
      BVHNode bvh(list);

      // Check that the bounding box encapsulates all objects
      auto bbox = bvh.boundingBox();
      expect(bbox.x.min <= -1.0_d);
      expect(bbox.y.min <= -1.0_d);
      expect(bbox.z.min <= -1.0_d);
      expect(bbox.x.max >= 5.0_d);
      expect(bbox.y.max >= 5.0_d);
      expect(bbox.z.max >= 5.0_d);
    };

    "hit_detection"_test = [] {
      // Create a list with two separated spheres
      HittableList list;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      list.add(make_shared<Sphere>(Point3(-2, 0, 0), 1.0, mat));
      list.add(make_shared<Sphere>(Point3(2, 0, 0), 1.0, mat));

      BVHNode bvh(list);

      // Test ray that hits first sphere
      Ray r1(Point3(-10, 0, 0), Vec3(1, 0, 0));
      HitRecord rec1;
      expect(bvh.hit(r1, Interval(0.001, glimpse::math::infinity), rec1) == true);
      expect(rec1.p.x() == -3.0_d);

      // Test ray that hits second sphere
      Ray r2(Point3(10, 0, 0), Vec3(-1, 0, 0));
      HitRecord rec2;
      expect(bvh.hit(r2, Interval(0.001, glimpse::math::infinity), rec2) == true);
      expect(rec2.p.x() == 3.0_d);

      // Test ray that misses all spheres
      Ray r3(Point3(0, 10, 0), Vec3(0, -1, 0));
      HitRecord rec3;
      expect(bvh.hit(r3, Interval(0.001, glimpse::math::infinity), rec3) == false);
    };

    skip / "empty_list"_test = [] {
      // Test with empty list
      HittableList emptyList;
      BVHNode emptyBvh(emptyList);

      Ray r(Point3(0, 0, 0), Vec3(1, 0, 0));
      HitRecord rec;
      std::cout << "empty_list 1" << std::endl;
      expect(emptyBvh.hit(r, Interval(0.001, glimpse::math::infinity), rec) == false);
      std::cout << "empty_list 2" << std::endl;
    };

    "single_object"_test = [] {
      std::cout << "sinle_object" << std::endl;
      // Test with single object
      HittableList singleList;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      singleList.add(make_shared<Sphere>(Point3(0, 0, 0), 1.0, mat));

      BVHNode singleBvh(singleList);

      // Ray that hits
      Ray r1(Point3(0, 0, -5), Vec3(0, 0, 1));
      HitRecord rec1;
      expect(singleBvh.hit(r1, Interval(0.001, glimpse::math::infinity), rec1) == true);

      // Ray that misses
      Ray r2(Point3(0, 0, -5), Vec3(0, 1, 0));
      HitRecord rec2;
      expect(singleBvh.hit(r2, Interval(0.001, glimpse::math::infinity), rec2) == false);
    };

    "box_comparator"_test = [] {
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      auto sphere1 = make_shared<Sphere>(Point3(0, 0, 0), 1.0, mat);
      auto sphere2 = make_shared<Sphere>(Point3(3, 0, 0), 1.0, mat);

      // Test private methods indirectly through a derived class
      struct testBvhNode : public BVHNode {
        testBvhNode() : BVHNode(HittableList()) {}

        static bool testBoxXCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
          return boxXCompare(a, b);
        }

        static bool testBoxYCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
          return boxYCompare(a, b);
        }

        static bool testBoxZCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) {
          return boxZCompare(a, b);
        }
      };

      // Test x-axis comparator
      expect(testBvhNode::testBoxXCompare(sphere1, sphere2) == true);
      expect(testBvhNode::testBoxXCompare(sphere2, sphere1) == false);

      // Test with spheres at same x but different y
      auto sphere3 = make_shared<Sphere>(Point3(0, 3, 0), 1.0, mat);
      expect(testBvhNode::testBoxYCompare(sphere1, sphere3) == true);
      expect(testBvhNode::testBoxYCompare(sphere3, sphere1) == false);

      // Test with spheres at same x/y but different z
      auto sphere4 = make_shared<Sphere>(Point3(0, 0, 3), 1.0, mat);
      expect(testBvhNode::testBoxZCompare(sphere1, sphere4) == true);
      expect(testBvhNode::testBoxZCompare(sphere4, sphere1) == false);
    };
  };
};