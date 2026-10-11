

#define MULTITHREADED
#ifdef MULTITHREADED
#include <future>
#include <thread>
#endif

#include "glimpse/camera.h"
#include "glimpse/util/rng.h"
#include "glimpse/aggregates.h"
#include "glimpse/shapes.h"
#include "glimpse/materials.h"
#include "glimpse/integrators.h"
#include "glimpse/render.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

std::atomic<bool> Renderer::stopRequested(false);

Vec3 sampleSquareStratified(int sI, int sJ, double recipSqrtSpp) {
  // Returns the vector to a random point in the square sub-pixel specified by grid
  // indices sI and sJ, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

  auto px = ((sI + randomDouble()) * recipSqrtSpp) - 0.5;
  auto py = ((sJ + randomDouble()) * recipSqrtSpp) - 0.5;

  return Vec3(px, py, 0);
}

struct RenderSectionArgs {
  Image &image;  // Image output, filled with rgb values
  Film &film;    // Intermediate buffer, accumulates samples and computes variance.
  int startRow;
  int endRow;
  const Scene &scene;
  const BVHNode &worldBvh;
  std::atomic<int> *progress;
};

void renderSection(RenderSectionArgs &args) {
  auto &image = args.image;
  auto &cam = args.scene.cam;
  auto &startRow = args.startRow;
  auto &endRow = args.endRow;
  auto &worldBvh = args.worldBvh;
  auto &scene = args.scene;
  auto progress = args.progress;
  auto &film = args.film;

  for (int sJ = 0; sJ < cam.sqrtSpp; ++sJ) {
    for (int sI = 0; sI < cam.sqrtSpp; ++sI) {
      for (int j = endRow - 1; j >= startRow; --j) {
        // Checked once per row: cheap, and a stop (viewer closing, Stop button) takes effect within a row.
        if (Renderer::stopRequested.load()) return;
        for (int i = 0; i < cam.imageWidth; ++i) {
          // color pixelColor(0, 0, 0);
          Color pixelColor(0, 0, 0);

          auto offset = sampleSquareStratified(sI, sJ, cam.recipSqrtSpp);
          auto u = (i + offset.x()) / (cam.imageWidth - 1);
          auto v = (j + offset.y()) / (cam.imageHeight - 1);
          Ray r = cam.getRay(u, v);
          pixelColor +=
              rayColor(r, scene.background, worldBvh, cam.maxDepth, scene.lights, !scene.lights.objects.empty());
          if (progress) (*progress)++;

          film.addSample(i, j, pixelColor);
          pixelColor = film.getSample(i, j);
          pixelColor = sqrt(pixelColor);  // gamma correction!
          // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
          image.setFloat(i, cam.imageHeight - 1 - j, static_cast<float>(pixelColor.x()),
                          static_cast<float>(pixelColor.y()), static_cast<float>(pixelColor.z()));
        }
      }
    }
  }
  std::cout << "Section done ... rows[" << startRow << ", " << endRow << "] thread " << std::this_thread::get_id()
            << std::endl;
}

void renderSectionUncap(RenderSectionArgs &args) {
  auto &image = args.image;
  auto &cam = args.scene.cam;
  auto &startRow = args.startRow;
  auto &endRow = args.endRow;
  auto &worldBvh = args.worldBvh;
  auto &scene = args.scene;
  auto progress = args.progress;
  auto &film = args.film;

  while (!Renderer::stopRequested.load()) {
    for (int sJ = 0; sJ < cam.sqrtSpp; ++sJ) {
      for (int sI = 0; sI < cam.sqrtSpp; ++sI) {
        for (int j = endRow - 1; j >= startRow; --j) {
          for (int i = 0; i < cam.imageWidth; ++i) {
            // color pixelColor(0, 0, 0);
            Color pixelColor(0, 0, 0);

            auto offset = sampleSquareStratified(sI, sJ, cam.recipSqrtSpp);
            auto u = (i + offset.x()) / (cam.imageWidth - 1);
            auto v = (j + offset.y()) / (cam.imageHeight - 1);
            Ray r = cam.getRay(u, v);
            pixelColor +=
                rayColor(r, scene.background, worldBvh, cam.maxDepth, scene.lights, !scene.lights.objects.empty());
            if (progress) (*progress)++;

            film.addSample(i, j, pixelColor);
            pixelColor = film.getSample(i, j);
            pixelColor = sqrt(pixelColor);  // gamma correction!
            // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
            image.setFloat(i, cam.imageHeight - 1 - j, static_cast<float>(pixelColor.x()),
                            static_cast<float>(pixelColor.y()), static_cast<float>(pixelColor.z()));
          }
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::cout << "Stopping section ... rows[" << startRow << ", " << endRow << "] thread " << std::this_thread::get_id()
            << std::endl;
}

void Renderer::renderScene(Scene scene, Image &image, std::atomic<int> *progress) {
  // Derived camera fields (imageHeight, sqrtSpp, viewport) must be current. Callers set public fields and
  // may forget initialize(); it only recomputes, so calling it again is harmless.
  scene.cam.initialize();

  auto worldBvh = BVHNode(scene.world);

  film.initialize(scene.cam.imageWidth, scene.cam.imageHeight);

#ifdef MULTITHREADED
  const int numThreads = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;
  int rowsPerThread = scene.cam.imageHeight / numThreads;

  for (int t = 0; t < numThreads; ++t) {
    int startRow = t * rowsPerThread;
    int endRow = (t == numThreads - 1) ? scene.cam.imageHeight : startRow + rowsPerThread;

    RenderSectionArgs args{image, film, startRow, endRow, scene, worldBvh, progress};
    if (scene.cam.uncappedSpp) {
      futures.push_back(                  //
          std::async(std::launch::async,  //
                     [args]() mutable {   //
                       renderSectionUncap(args);
                     }));
    } else {
      futures.push_back(                  //
          std::async(std::launch::async,  //
                     [args]() mutable {   //
                       renderSection(args);
                     }));
    }
  }

  for (auto &f : futures) {
    f.get();
  }

  std::cout << __FUNCTION__ << " : All threads finished rendering..." << std::endl;
  Renderer::stopRequested = false;
#else  // SINGLETHREADED

  // Image image(imageWidth, imageHeight);
  renderSection(image, 0, imageHeight, imageWidth, imageHeight, samplesPerPixel, cam, background, worldBvh,
                 maxDepth);

#endif
}

}  // namespace glimpse