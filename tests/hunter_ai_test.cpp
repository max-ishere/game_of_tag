#include "sand/component/movement_intent.hpp"
#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <box2d/box2d.h>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <gtest/gtest.h>
#include <numbers>
#include <sand/component/ai.hpp>
#include <sand/system/ai.hpp>

TEST(HunterAI, ExploreStateToChase) {
  entt::registry registry;
  b2World world(b2Vec2_zero);
  const auto hunter = registry.create();
  const auto runner = registry.create();

  b2BodyDef body;
  body.position = b2Vec2_zero;

  class TickHuntersAI TickHuntersAI;

  registry.emplace<HunterAI>(
      hunter, HunterAI{.target = hunter, .state = HunterAI::Explore});
  registry.emplace<b2Body *>(hunter, world.CreateBody(&body));

  body.position = b2Vec2(0, TickHuntersAI.tag_range + 1);

  registry.emplace<HunterTarget>(runner, HunterTarget{.chased_by = runner});
  registry.emplace<b2Body *>(runner, world.CreateBody(&body));

  body.position = b2Vec2(0, TickHuntersAI.tag_range + 2);
  ASSERT_LE(TickHuntersAI.tag_range + 2, TickHuntersAI.explore_detection_range);
  for (auto i = 0u; i < 10u; i++) {
    auto entity = registry.create();
    registry.emplace<b2Body *>(entity, world.CreateBody(&body));
    registry.emplace<HunterTarget>(entity, HunterTarget{.chased_by = entity});
  }

  for (auto i = 0u; i < 5u; i++) {
    TickHuntersAI(registry);

    EXPECT_EQ(registry.get<HunterAI>(hunter).state, HunterAI::Chase);
    EXPECT_EQ(registry.get<HunterAI>(hunter).target, runner);
    EXPECT_EQ(registry.get<HunterTarget>(runner).chased_by, hunter);
  }

  registry.get<b2Body *>(hunter)->SetTransform(
      b2Vec2(0, -TickHuntersAI.chase_detection_range * 2), 0);

  TickHuntersAI(registry);

  EXPECT_EQ(registry.get<HunterAI>(hunter).state, HunterAI::Explore);
  EXPECT_EQ(registry.get<HunterAI>(hunter).target, hunter);
  EXPECT_EQ(registry.get<HunterTarget>(runner).chased_by, runner);
}

TEST(HunterAI, MovementIntent) {
  entt::registry registry;
  b2World world(b2Vec2_zero);
  b2BodyDef body;
  const auto hunter = registry.create();
  const auto runner = registry.create();

  registry.emplace<HunterAI>(
      hunter, HunterAI{.target = runner, .state = HunterAI::Chase});

  body.position = b2Vec2_zero;
  registry.emplace<b2Body *>(hunter, world.CreateBody(&body));
  body.position = b2Vec2(1, 1);
  registry.emplace<b2Body *>(runner, world.CreateBody(&body));

  TickAIMovementIntent(registry);

  EXPECT_NEAR(registry.get<MovementIntent>(hunter).angle, std::numbers::pi / 4,
              0.001);
  EXPECT_GT(registry.get<MovementIntent>(hunter).velocity, 0);
}

TEST(HunterAI, TooCloseThenStop) {
  entt::registry registry;
  b2World world(b2Vec2_zero);
  b2BodyDef body;
  const auto hunter = registry.create();
  const auto runner = registry.create();
  TickHuntersAI TickHuntersAI;

  registry.emplace<HunterAI>(
      hunter, HunterAI{.target = runner, .state = HunterAI::Chase});
  registry.emplace<HunterTarget>(runner, HunterTarget{.chased_by = hunter});

  body.position = b2Vec2_zero;
  registry.emplace<b2Body *>(hunter, world.CreateBody(&body));
  body.position = b2Vec2(0, TickHuntersAI.tag_range - 1);
  registry.emplace<b2Body *>(runner, world.CreateBody(&body));

  for (auto i = 0u; i < 10u; i++) {
    TickHuntersAI(registry);
    TickAIMovementIntent(registry);
    EXPECT_EQ(registry.get<HunterAI>(hunter).state, HunterAI::Explore);
    EXPECT_EQ(registry.get<HunterAI>(hunter).target, hunter);
    EXPECT_EQ(registry.get<HunterTarget>(runner).chased_by, runner);

    ASSERT_TRUE(registry.all_of<TargetTagged>(runner));
    EXPECT_EQ(registry.get<TargetTagged>(runner).by, hunter);
  }
}
