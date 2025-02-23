#include "app.h"

int main() {
  AppWindow app;
  if (app.initApp() != 0) {
    return -1;
  }
  app.run();
  return 0;
}