#include "sand/system/ai.hpp"
#include "sand/system/kbd_control.hpp"
#include <SDL2/SDL.h>
#include <box2d/b2_body.h>
#include <box2d/b2_time_step.h>
#include <box2d/box2d.h>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <entt/entity/registry.hpp>
#include <functional>
#include <iostream>
#include <sand/component/ai.hpp>
#include <sand/component/player_controllable.hpp>
#include <sand/component/renderer_data.hpp>
#include <sand/core/timer.hpp>
#include <sand/core/z_index.hpp>
#include <sand/entity_factory.hpp>
#include <sand/systems.hpp>

int main(int argc, char *argv[]) {
  srand(time(NULL));

  const float fps = 24;

  Renderer Renderer;
  Physics Physics;
  TickHuntersAI TickHuntersAI;
  Renderer.camera_data =
      Renderer::CameraData{.x = 0, .y = 0, .hx_size = 40, .hy_size = 40};

  entt::registry registry;

  const auto player = registry.create();
  MakePhysicsEntity(registry, player, Physics.world);
  registry.emplace<MovementIntent>(player);
  registry.emplace<HunterTarget>(player);

  b2CircleShape character_collider;
  character_collider.m_radius = 0x1p-3;
  registry.get<b2Body *>(player)
      ->CreateFixture(&character_collider, 1)
      ->SetFriction(5);
  registry.emplace<RendererData>(
      player, true, SpriteData::SpriteId::CharacterMale, 0.f,
      0.5 - character_collider.m_radius, RendererData::z_normal_index + 1);

  const auto ai_agent = registry.create();
  MakePhysicsEntity(registry, ai_agent, Physics.world);
  registry.emplace<HunterAI>(ai_agent);

  b2Body *ai_physics_body = registry.get<b2Body *>(ai_agent);
  ai_physics_body->CreateFixture(&character_collider, 1)->SetFriction(5);
  ai_physics_body->SetTransform(b2Vec2(4, 0), 0);
  registry.emplace<RendererData>(ai_agent, true,
                                 SpriteData::SpriteId::CharacterFemale, 0.f,
                                 0.5 - character_collider.m_radius);

  auto frame_counter = 0u;

  bool quit = false;
  while (!quit) {
    frame_counter++;
    Timer timer;
    timer.start();

    HUDstate state;

    registry.emplace_or_replace<MovementIntent>(player,
                                                HandleEvents(quit, state));
    if (ResetTaggedStatus(registry) and registry.all_of<TargetTagged>(player))
      registry.erase<TargetTagged>(player);

    if (frame_counter % (int)(fps / 2) == 0) {
      TickHuntersAI(registry);
      TickAIMovementIntent(registry);
    }
    HandleControlIntents(registry);

    Physics(registry, 1.f / fps);

    const b2Vec2 &position = registry.get<b2Body *>(player)->GetPosition();
    Renderer.camera_data = position;

    const RendererData &render_data = registry.get<RendererData>(player);
    Renderer.camera_data.x += render_data.x_offset;
    Renderer.camera_data.y += render_data.y_offset;

    registry.sort<RendererData>([&registry](
                                    const entt::entity lentity,
                                    const entt::entity rentity) -> bool {
      const auto &lposition = registry.get<b2Body *>(lentity)->GetPosition(),
                 rposition = registry.get<b2Body *>(rentity)->GetPosition();

      const auto lindex = registry.get<RendererData>(lentity).z_index,
                 rindex = registry.get<RendererData>(rentity).z_index;

      return ZindexLessOrdered(lposition, lindex, rposition, rindex);
    });

    Renderer(registry);

    timer.stop();
    if (timer.elapsedMilliseconds() < 1000.f / fps) {
      int delay = 1000 / fps - timer.elapsedMilliseconds();
      SDL_Delay((delay >= 0) ? delay : 0);
    }
  }
  return 0;
}
