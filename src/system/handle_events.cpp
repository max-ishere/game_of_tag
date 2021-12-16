#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>
#include <algorithm>
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <numbers>
#include <sand/component/player_controllable.hpp>
#include <sand/system/kbd_control.hpp>
#include <type_traits>

bool ResetTaggedStatus(entt::registry &registry) {
  return SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_TAB];
}

MovementIntent HandleEvents(bool &quit, HUDstate &state) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      quit = true;
  }

  float player_speed = 3;

  int keycount = 0;
  static const auto *keys = SDL_GetKeyboardState(&keycount);

  state.x = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
  state.y = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];

  if (keys[SDL_SCANCODE_LSHIFT])
    player_speed = 10;

  static_assert(std::is_integral_v<decltype(state.x)> and
                    std::is_integral_v<decltype(state.y)>,
                "Must be integral for bool-like state checks");

  std::clamp(state.x, -1, 1);
  std::clamp(state.y, -1, 1);

  if (state.x == 0 and state.y == 0)
    return MovementIntent{};

  float angle = 0;
  switch (state.y) {
  case 1:
    angle = 2 - state.x;
    break;
  case -1:
    angle = 6 + state.x;
    break;
  case 0:
    angle = 2 - 2 * state.x;
    break;
  }
  angle *= std::numbers::pi / 4;

  return MovementIntent{.angle = angle, .velocity = player_speed};
}

void HandleControlIntents(entt::registry &registry) {
  registry.view<MovementIntent, b2Body *>().each(
      [](const MovementIntent &intent, b2Body *physics_body) {
        const b2Vec2 &body_velocity = physics_body->GetLinearVelocity();

        b2Vec2 velocity(cos(intent.angle) * intent.velocity,
                        sin(intent.angle) * intent.velocity);

        physics_body->SetLinearVelocity(velocity);
      });
}
