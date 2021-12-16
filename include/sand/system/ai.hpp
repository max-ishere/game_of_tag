#pragma once
#include <box2d/b2_math.h>
#include <entt/entity/fwd.hpp>

struct HunterAI;

class TickHuntersAI {
public:
  unsigned int explore_detection_range{10u},
      chase_detection_range{explore_detection_range * 2}, tag_range{1u};

  void operator()(entt::registry &);
};

void TickAIMovementIntent(entt::registry &);
