#pragma once

extern engine::application* engine::create_appliation();

int main(int argc, char** argv)
{
  engine::logger::init();

  auto app = engine::create_appliation();
  app->run();
  delete app;
}