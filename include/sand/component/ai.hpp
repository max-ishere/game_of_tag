#pragma once
#include <box2d/b2_math.h>
#include <entt/entity/fwd.hpp>
#include <sand/dependencies/wise_enum/wise_enum.hpp>

struct HunterAI {
  entt::entity target;
  WISE_ENUM(State, Explore, Chase);
  State state = Explore;
  b2Vec2 explore_target;
  unsigned int exlore_ticks = 0u;
};

struct HunterTarget {
  entt::entity chased_by;
};

struct TargetTagged {
  entt::entity by;
};
