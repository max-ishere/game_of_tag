#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#include <cstdlib>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <iostream>
#include <sand/component/renderer_data.hpp>
#include <sand/entity_factory.hpp>
#include <sand/systems.hpp>

int main(int argc, char *argv[]) {
  Renderer Renderer;
  Physics Physics;
  Renderer.camera_data =
      Renderer::CameraData{.x = 0, .y = 0, .hx_size = 40, .hy_size = 40};

  entt::registry registry;
  for (auto i = 0u; i < 10u; i++) {
    const auto entity = registry.create();
    MakePhysicsEntity(registry, entity, Physics.world);

    const b2Vec2 position(rand() % 20 - 10, rand() % 20 - 10);
    auto body = registry.get<b2Body *>(entity);
    body->SetTransform(position, 0);

    const b2Vec2 velocity(-position.x, -position.y);
    body->SetLinearVelocity(velocity);

    b2PolygonShape shape;
    shape.SetAsBox(.5f, .5f);
    body->CreateFixture(&shape, 1);
  }

  bool quit = false;
  while (!quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        quit = true;
    }
    std::clog << Renderer.GetDeltaTime() << std::endl;
    Physics(registry, 1.f / Renderer.GetDeltaTime());
    ConvertPhysicsToRenderData(registry, Renderer.camera_data);
    Renderer(registry);

    std::clog << "renderable entities: " << registry.view<RendererData>().size()
              << "\nphysics entities: " << registry.view<b2Body *>().size()
              << "\nwindow size: " << get<0>(Renderer.WindowSize()) << " "
              << get<1>(Renderer.WindowSize()) << "\n\n";
  }
  return 0;
}
